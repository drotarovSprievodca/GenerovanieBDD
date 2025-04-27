#include "EntropyBasedOrderDT.hpp"

EntropyBasedOrderDT::EntropyBasedOrderDT(bool use_var_reordering_heuristics, bool generate_graph_before_order, bool generate_graph_after_order) :
    EntropyBasedOrder(use_var_reordering_heuristics, generate_graph_before_order, generate_graph_after_order) {}

EntropyBasedOrderDT::~EntropyBasedOrderDT() {}

void EntropyBasedOrderDT::get_order_from_ODT(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_ODT, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    int root = order_of_vars_from_ODT[0]; // root was calculated from layer 0
    int number_of_vars = default_manager.get_var_count();
    int number_of_lines = std::pow(2, number_of_vars);
    std::vector<std::vector<bool>> variables_of_function = default_manager.satisfy_all<std::vector<bool>>(1, diagram);

    // candidates for being a new members of final order (order_of_vars_from_ODT) that is output of this function 
    std::vector<int> unused_vars = std::vector<int>();
    for (int v = 0; v < number_of_vars; ++v) {
        if (v == root) { continue; }
        unused_vars.push_back(v);
    }

    // already known variables from upper layers of ODT (from order_of_vars_from_ODT)
    std::vector<int> known_vars = std::vector<int>();
    known_vars.push_back(root);
    
    for (int layer = 1; layer < number_of_vars - 1; ++layer) {
        int number_of_max_occurences = std::pow(2, number_of_vars - (known_vars.size() + 1));
        std::vector<ce_var> entropies_in_layer = std::vector<ce_var>(number_of_vars - layer);
        for (int H = 0; H < number_of_vars - layer; ++H) {
            // Calculating: H(f|x_<candidate>,x_<known_vars[0]>,x_<known_vars[1]>,...) saved in variable: H_f_c_kv
            double H_f_c_kv = 0.0;
            int candidate_var = unused_vars[H];
            // example of partial entropy: H(f,x3,x2=0,x0=1) 
            int number_of_partial_entropies = std::pow(2, known_vars.size());

            double partial_entropies = 0.0;
            // constant_in_partial_entropy in example: H(f,x3,x2=0,x0=1) is: 01
            for (int constant_in_partial_entropy = 0; constant_in_partial_entropy < number_of_partial_entropies; ++constant_in_partial_entropy) {
                int number_of_known_vars = known_vars.size();
                std::vector<bool> constant_in_binary = std::vector<bool>(number_of_known_vars, false);
                
                // coverting decimal constant_in_partial_entropy to binary form and saving this binary form in: constant_in_binary
                for (int b = 0; b < number_of_known_vars; ++b) {
                    constant_in_binary[number_of_known_vars - 1 - b] = (constant_in_partial_entropy & (1 << b)) != 0;
                }

                bool candidate_value = false;
                int number_of_matches = 0;
              
                for (int c = 0; c < 2; ++c) {
                    for (std::vector<bool> variables_of_one_case : variables_of_function) {
                        if (candidate_value != variables_of_one_case[candidate_var]) {
                            continue;
                        }
                        bool all_constant_matched = true;
                        for (int index_to_known_var = 0; index_to_known_var < number_of_known_vars; ++index_to_known_var) {
                            if (constant_in_binary[index_to_known_var] != variables_of_one_case[known_vars[index_to_known_var]]) {
                                all_constant_matched = false;
                                break;
                            }
                        }
                        if (all_constant_matched) {
                            number_of_matches++;
                        }
                    }
                    // probability that function is 1, c = 0, constant(s)
                    double P_f_1_c = (double)number_of_matches / number_of_lines;
                    partial_entropies += P_f_1_c * log_2(P_f_1_c);
                    
                    // probability that function is 1, c = 1, constant(s)
                    double P_f_0_c = (double)(number_of_max_occurences - number_of_matches) / number_of_lines;
                    partial_entropies += P_f_0_c * log_2(P_f_0_c);
                    

                    candidate_value = true;
                    number_of_matches = 0; 
                }
            }
            partial_entropies = -1.0 * partial_entropies;

            // for example: H(f|x3,x2,x0) = H(f,x3,x2,x0) - H(x3,x2,x0)
            H_f_c_kv = partial_entropies - (known_vars.size() + 1); // for example: H(x3,x2,x0) = 3 because there are 3 variables

            ce_var var = ce_var();
            var.conditional_entropy = H_f_c_kv;
            var.variable = candidate_var;

            std::string constants = "";
            for (int kv : known_vars) {
                constants += ",x" + std::to_string(kv);
            }

            entropies_in_layer[H] = var;
        }

        // sort list of structs based on conditional entropy
        std::sort(entropies_in_layer.begin(), entropies_in_layer.end(),
            [this](const ce_var& a, const ce_var& b) { return this->compare_by_conditional_entropy_asc(a, b); });

        // new variable is added to order
        ce_var winner = entropies_in_layer[0];
        int new_var_in_final_order = winner.variable;
        order_of_vars_from_ODT[layer] = new_var_in_final_order;

        known_vars.push_back(new_var_in_final_order);

        int index_of_unused_variable = 0;
        for (int index_of_unused_var = 0; index_of_unused_var < unused_vars.size(); ++index_of_unused_var) {
            if (unused_vars[index_of_unused_var] == new_var_in_final_order) {
                index_of_unused_variable = index_of_unused_var;
            }
        }

        unused_vars.erase(unused_vars.begin() + index_of_unused_variable);
    }

    order_of_vars_from_ODT[number_of_vars - 1] = unused_vars[0];
}

void EntropyBasedOrderDT::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {
    auto start = std::chrono::high_resolution_clock::now();
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_before_order && !generate_diagram(file_name_without_extension + "_before", diagram, default_manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    // list for conditional entropies from ODT of all variables of this function
    std::vector<ce_var> list_for_reordering = std::vector<ce_var>(number_of_vars);

    // save all conditional entropies from ODT of all variables in this function in list_for_reordering
    get_ce_of_all_vars_in_function(default_manager, list_for_reordering, diagram, which_function);

    // sort list of structs based on conditional entropy
    std::sort(list_for_reordering.begin(), list_for_reordering.end(),
        [this](const ce_var& a, const ce_var& b) { return this->compare_by_conditional_entropy_asc(a, b); });
    
    // variable from which we will be building ODT
    int root = list_for_reordering[0].variable;

    // list for order of variables from ODT from top to bottom
    std::vector<int> order_of_vars_from_ODT = std::vector<int>(number_of_vars);

    order_of_vars_from_ODT[0] = root;

    // get order of variables from ODT from top to bottom
    get_order_from_ODT(default_manager, order_of_vars_from_ODT, diagram, which_function);
    
    // vector with new order in teddy format 
    std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

    // moving reordered list to teddy format vector
    for (int i = 0; i < number_of_vars; ++i) {
        order_after[i] = order_of_vars_from_ODT[i];
    }

    // creating manager with new order of variables based on conditional entropy from ODT
    teddy::bss_manager manager_after(number_of_vars, 10'000, order_after);
    manager_after.set_auto_reorder(false);
    teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager_after.force_reorder();
    }

    if (this->generate_graph_after_order && !generate_diagram(file_name_without_extension + "_after", diagram_after, manager_after, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    std::string var_order = "";
    for (auto x : manager_after.get_order()) {
        var_order += "x";
        var_order += std::to_string(x);
    }
    csv->write_new_stats(var_order, (double)manager_after.get_node_count(diagram_after));

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    this->add_to_timer(duration.count());
}

std::string EntropyBasedOrderDT::to_string() {
    std::string return_string = "";
    return_string += "Ascending CE_ODT w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (order);";

    return_string += "Ascending CE_ODT w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";

    return return_string;
}

std::string EntropyBasedOrderDT::get_strategy_name() {
    std::string return_string = "Entropy and ODT based order, Ascending conditional entropy, Reordering heuristic: ";
    return_string += this->use_var_reordering_heuristics ? "YES" : "NO";
    return_string += " Generate graph before order: ";
    return_string += this->generate_graph_before_order ? "YES" : "NO";
    return_string += " Generate graph after order: ";
    return_string += this->generate_graph_after_order ? "YES" : "NO";

    return return_string;
}