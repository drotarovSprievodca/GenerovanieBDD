#include "EntropyBasedOrderDT.hpp"

EntropyBasedOrderDT::EntropyBasedOrderDT(bool use_var_reordering_heuristics) :
    EntropyBasedOrder(use_var_reordering_heuristics) {}

EntropyBasedOrderDT::~EntropyBasedOrderDT() {}

void EntropyBasedOrderDT::get_order_from_ODT(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_ODT, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    int root = order_of_vars_from_ODT[0]; // root was calculated from layer 0
    int number_of_vars = default_manager.get_var_count();
    std::vector<std::vector<bool>> variables_of_function = default_manager.satisfy_all<std::vector<bool>>(1, diagram);

    // candidates for being a new members of final order (order_of_vars_from_ODT) that is output of this function 
    std::vector<int> unused_vars = std::vector<int>();
    for (int v = 0; v < number_of_vars; v++) {
        if (v == root) { continue; }
        unused_vars.push_back(v);
    }

    // already known variables from upper layers of ODT (from order_of_vars_from_ODT)
    std::vector<int> known_vars = std::vector<int>();
    known_vars.push_back(root);
    
    for (int layer = 1; layer < number_of_vars - 1; layer++) {
        int number_of_max_occurences = std::pow(2, number_of_vars - (known_vars.size() + 1));
        for (int H = 0; H < number_of_vars - layer; H++) {
            // Calculating: H(f|x_<candidate>,x_<known_vars[0]>,x_<known_vars[1]>,...) saved in variable: H_f_c_kv
            double H_f_c_kv = 0.0;
            int candidate_var = unused_vars[H];
            // example of partial entropy: H(f,x3,x2=0,x0=1) 
            int number_of_partial_entropies = std::pow(2, known_vars.size());

            // constant_in_partial_entropy in example: H(f,x3,x2=0,x0=1) is: 01
            for (int constant_in_partial_entropy = 0; constant_in_partial_entropy < number_of_partial_entropies; constant_in_partial_entropy++) {
                
                double partial_entropy = 0.0;
                
                int number_of_known_vars = known_vars.size();
                std::vector<bool> constant_in_binary = std::vector<bool>(number_of_known_vars, false);
                
                // coverting decimal constant_in_partial_entropy to binary form and saving this binary form in: constant_in_binary
                for (int b = 0; b < number_of_known_vars; ++b) {
                    constant_in_binary[number_of_known_vars - 1 - b] = (constant_in_partial_entropy & (1 << b)) != 0;
                }

                bool candidate_value = false;
                int number_of_matches = 0;
              
                for (int c = 0; c < 2; c++) {
                    for (std::vector<bool> variables_of_one_case : variables_of_function) {
                        if (candidate_value != variables_of_one_case[candidate_var]) {
                            continue;
                        }
                        for (int index_to_known_var = 0; index_to_known_var < number_of_known_vars; index_to_known_var++) {
                            if (constant_in_binary[index_to_known_var] != variables_of_one_case[known_vars[index_to_known_var]]) {
                                continue;
                            }
                        }
                        number_of_matches++;
                    }

                    // partial_entropy += (number_of_matches / number_of_max_occurences)* log toho;
                    // partial_entropy += ((number_of_max_occurences - number_of_matches) / number_of_max_occurences)* log toho;
                    //partial_entropy s minusom potom

                    candidate_value = true;
                    number_of_matches = 0; 
                }
                H_f_c_kv += partial_entropy;

                


            }

            //add H_f_c_kv to list for comparing in this layer



        
        }
        // find best next var from list for comparing in this layer
        // add to new order
        // add to known_vars
        // remove from unused_var

    }
    //we have new order























    






    for (int i = 0; i < default_manager.get_var_count(); i++) {
        order_of_vars_from_ODT[i] = i;
    }
}

void EntropyBasedOrderDT::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) {
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    // list for conditional entropies from ODT of all variables of this function
    std::vector<ce_var> list_for_reordering = std::vector<ce_var>(number_of_vars);

    // save all conditional entropies from ODT of all variables in this function in list_for_reordering
    get_ce_of_all_vars_in_function(default_manager, list_for_reordering, diagram, which_function);
    
    /*
    std::cout << "Before sorting: " << std::endl;
    for (const auto& item : list_for_reordering) {
        std::cout << item.variable << " " << item.conditional_entropy << std::endl;
    }
    */

    //delete &diagram;

    // sort list of structs based on conditional entropy from ODT
    std::sort(list_for_reordering.begin(), list_for_reordering.end(),
        [this](const ce_var& a, const ce_var& b) { return this->compare_by_conditional_entropy_asc(a, b); });
    
    /*
    std::cout << "After sorting: " << std::endl;
    for (const auto& item : list_for_reordering) {
        std::cout << item.variable << " " << item.conditional_entropy << std::endl;
    }
    */
    
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
    teddy::bss_manager manager_after(number_of_vars, 100'000, order_after);
    manager_after.set_auto_reorder(false);
    teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager_after.force_reorder();
    }

    std::string var_order = "";
    for (auto x : manager_after.get_order()) {
        var_order += "x";
        var_order += std::to_string(x);
    }
    csv->write_new_stats(var_order, (double)manager_after.get_node_count(diagram_after));
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