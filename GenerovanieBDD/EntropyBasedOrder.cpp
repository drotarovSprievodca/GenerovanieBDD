#include "EntropyBasedOrder.hpp"

EntropyBasedOrder::EntropyBasedOrder(bool use_var_reordering_heuristics, bool generate_graph_before_order, bool generate_graph_after_order) :
    Strategy(use_var_reordering_heuristics, generate_graph_before_order, generate_graph_after_order) {}

EntropyBasedOrder::~EntropyBasedOrder() {}

bool EntropyBasedOrder::compare_by_conditional_entropy_asc(const ce_var& a, const ce_var& b) {
    return a.conditional_entropy < b.conditional_entropy;
}

double EntropyBasedOrder::log_2(double value) {
    const double epsilon = 1e-9;

    if (std::fabs(value) < epsilon) {
        return 0.0;
    }
    else {
        return log2(value);
    }
}

void EntropyBasedOrder::get_ce_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<ce_var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    for (int i = 0; i < list_for_reordering.size(); ++i) {

        int number_of_ones_in_function = default_manager.satisfy_count(1, diagram);
        int number_of_zeros_in_function = default_manager.satisfy_count(0, diagram);

        bool save_ones = true; // if variables whose function value is 1 or 0 should be saved
        if (number_of_zeros_in_function < number_of_ones_in_function) {
            save_ones = false;
        }

        int number_of_vars = default_manager.get_var_count();
        
        std::vector<std::vector<bool>> variables_of_function = default_manager.satisfy_all<std::vector<bool>>(save_ones ? 1 : 0, diagram);
        
        double P_that_var_is_one_or_zero = 1.0 / 2.0;
        
        /*
        for (std::vector<bool> variables_of_one_case : variables_of_function) {
            for (bool variable : variables_of_one_case) {
                std::cout << variable << " ";
            }
            std::cout << std::endl;
        }
        */

        // number of ones in i-th variable column of full truth table of function 
        // but only where function evaluates to 1 if <save_ones> is true or to 0 if <save_ones> is false
        int number_of_ones = 0;

        for (std::vector<bool> variables_of_one_case : variables_of_function) {
            if (variables_of_one_case[i]) {
                number_of_ones++;
            }
        }

        // number of zeros in i-th variable column of full truth table of function 
        // but only where function evaluates to 1 if <save_ones> is true or to 0 if <save_ones> is false
        int number_of_zeros = variables_of_function.size() - number_of_ones;

        // number of ones or zeros in variable column of full truth table of function
        int sum_of_var = std::pow(2, number_of_vars) / 2;

        // Conditional probability that function has value 1 or 0 given the condition that variable value is 1 or 0 
        double P_f_0_xi_0 = 0.0;
        double P_f_1_xi_0 = 0.0;
        double P_f_0_xi_1 = 0.0;
        double P_f_1_xi_1 = 0.0;

        // save_ones is true

        if (save_ones) {
            // probability that function is 1 given that xi is 1
            P_f_1_xi_1 = (double)number_of_ones / sum_of_var;

            // probability that function is 0 given that xi is 1
            //         = (sum_of_var - number_of_ones) / sum_of_var
            P_f_0_xi_1 = 1.0 - P_f_1_xi_1;

            // probability that function is 1 given that xi is 0
            P_f_1_xi_0 = (double)number_of_zeros / sum_of_var;

            // probability that function is 0 given that xi is 0
            //         = (sum_of_var - number_of_zeros) / sum_of_var
            P_f_0_xi_0 = 1.0 - P_f_1_xi_0;
        } else {
            // probability that function is 0 given that xi is 1
            P_f_0_xi_1 = (double)number_of_ones / sum_of_var;

            // probability that function is 1 given that xi is 1
            //         = (sum_of_var - number_of_ones) / sum_of_var
            P_f_1_xi_1 = 1.0 - P_f_0_xi_1;

            // probability that function is 0 given that xi is 0
            P_f_0_xi_0 = (double)number_of_zeros / sum_of_var;

            // probability that function is 1 given that xi is 0
            //         = (sum_of_var - number_of_zeros) / sum_of_var
            P_f_1_xi_0 = 1.0 - P_f_0_xi_0;
        }

        /*
        std::cout << "x" << std::to_string(i) << std::endl;
        std::cout << std::to_string(P_f_0_xi_0) << std::endl;
        std::cout << std::to_string(P_f_1_xi_0) << std::endl;
        std::cout << std::to_string(P_f_0_xi_1) << std::endl;
        std::cout << std::to_string(P_f_1_xi_1) << std::endl;
        */

        double H_f_xi_0 = - 1.0 * (P_f_0_xi_0 * log_2(P_f_0_xi_0) + P_f_1_xi_0 * log_2(P_f_1_xi_0));
        double H_f_xi_1 = - 1.0 * (P_f_0_xi_1 * log_2(P_f_0_xi_1) + P_f_1_xi_1 * log_2(P_f_1_xi_1));

        double H_f_xi = P_that_var_is_one_or_zero * H_f_xi_0 + P_that_var_is_one_or_zero * H_f_xi_1;

        //std::cout << "H(f" << std::to_string(which_diagram) << "|x" << std::to_string(i) <<  ") = " << std::to_string(H_f_xi) << std::endl;

        ce_var var = ce_var();
        var.conditional_entropy = H_f_xi;
        var.variable = i;

        list_for_reordering[i] = var;
    }
}

void EntropyBasedOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {
    auto start = std::chrono::high_resolution_clock::now();
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_before_order && !generate_diagram(file_name_without_extension + "_before", diagram, default_manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    // list for conditional entropies of all variables of this function
    std::vector<ce_var> list_for_reordering = std::vector<ce_var>(number_of_vars);

    // save all conditional entropies of all variables in this function in list_for_reordering
    get_ce_of_all_vars_in_function(default_manager, list_for_reordering, diagram, which_function);
    /*
    std::cout << "Before sorting: " << std::endl;
    for (const auto& item : list_for_reordering) {
        std::cout << item.variable << " " << item.conditional_entropy << std::endl;
    }
    */
    //delete &diagram;

    // sort list of structs based on conditional entropy
    std::sort(list_for_reordering.begin(), list_for_reordering.end(), 
        [this](const ce_var& a, const ce_var& b) { return this->compare_by_conditional_entropy_asc(a, b); });
    /*
    std::cout << "After sorting: " << std::endl;
    for (const auto& item : list_for_reordering) {
        std::cout << item.variable << " " << item.conditional_entropy << std::endl;
    }
    */
    // vector with new order in teddy format 
    std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

    // moving reordered list to teddy format vector
    for (int i = 0; i < number_of_vars; ++i) {
        order_after[i] = list_for_reordering[i].variable;
    }

    // creating manager with new order of variables based on conditional entropy
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

std::string EntropyBasedOrder::to_string() {
    std::string return_string = "";
    return_string += "Ascending CE w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (order);";

    return_string += "Ascending CE w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";

    return return_string;
}

std::string EntropyBasedOrder::get_strategy_name() {
    std::string return_string = "Entropy based order, Ascending conditional entropy, Reordering heuristic: ";
    return_string += this->use_var_reordering_heuristics ? "YES" : "NO";
    return_string += " Generate graph before order: ";
    return_string += this->generate_graph_before_order ? "YES" : "NO";
    return_string += " Generate graph after order: ";
    return_string += this->generate_graph_after_order ? "YES" : "NO";

    return return_string;
}