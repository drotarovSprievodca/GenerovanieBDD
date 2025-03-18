#include "DerivativesBasedOrderDT.hpp"

DerivativesBasedOrderDT::DerivativesBasedOrderDT(bool use_var_reordering_heuristics, bool ascending, bool generate_graph_before_order, bool generate_graph_after_order) :
	DerivativesBasedOrder(use_var_reordering_heuristics, ascending, generate_graph_before_order, generate_graph_after_order) {}

DerivativesBasedOrderDT::~DerivativesBasedOrderDT() {}

void DerivativesBasedOrderDT::get_order_from_ODT(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_ODT, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    int last_variable = order_of_vars_from_ODT[0]; // root was calculated from layer 0
    int number_of_vars = default_manager.get_var_count();
    int number_of_lines = std::pow(2, number_of_vars);
    std::vector<int> new_order = std::vector<int>();
    new_order.push_back(last_variable);

    // constants will be used for calculating in which sub-table should I put new 1 in new function
    // variables before constant(s) will be used for calculating positions of 1s in function relative to specific sub-table
    
    std::vector<std::vector<bool>> first_variables = default_manager.satisfy_all<std::vector<bool>>(1, diagram);
    std::vector<std::vector<bool>> second_variables = default_manager.satisfy_all<std::vector<bool>>(1, diagram);
    std::vector<std::vector<bool>>* from_vars = &first_variables;
    std::vector<std::vector<bool>>* to_vars = &second_variables;

    std::vector<int> order_first_variables = std::vector<int>(number_of_vars);
    std::vector<int> order_second_variables = std::vector<int>(number_of_vars);
    std::vector<int>* from_vars_order = &order_first_variables;
    std::vector<int>* to_vars_order = &order_second_variables;

    for (int v = 0; v < number_of_vars; ++v) {
        (*from_vars_order)[v] = v;
    }
    
    std::vector<bool> function_values = std::vector<bool>(number_of_lines, false);

    for (int layer = 1; layer < number_of_vars - 1; ++layer) {
        int size_of_subtable = number_of_lines / std::pow(2, layer);
        int number_of_subtables = number_of_lines / size_of_subtable;
        std::vector<td_var> list_for_reordering = std::vector<td_var>(number_of_vars - layer);

        // clear function values vector
        std::fill(function_values.begin(), function_values.end(), false);
        // find index of last_variable
        int index_of_constant = std::distance((*from_vars_order).begin(), std::find((*from_vars_order).begin(), (*from_vars_order).end(), last_variable));
        // copy (*from_vars_order) variables to (*to_vars_order) variables
        for (int v = 0; v < number_of_vars; ++v) {
            (*to_vars_order)[v] = (*from_vars_order)[v];
        }
        // remove variable that is at the index_of_constant in (*to_vars_order)
        (*to_vars_order).erase((*to_vars_order).begin() + index_of_constant);
        // add the last_variable (that will be constant) to the end of the (*to_vars_order)
        (*to_vars_order).push_back(last_variable);

        /*
        std::cout << "from_vars_order:" << std::endl;
        for (auto fwo : (*from_vars_order)) {
            std::cout << fwo << std::endl;
        }

        std::cout << "to_vars_order:" << std::endl;
        for (auto two : (*to_vars_order)) {
            std::cout << two << std::endl;
        }
        */

        for (int vars_case = 0; vars_case < (*from_vars).size(); ++vars_case) {
            // find out where the new 1 will be in function in new specific subtable 
            // copy variables before constant(s) from (*from_vars) to (*to_vars)
            std::string order_in_sub_table_bin = "";
            bool value_of_constant;
            int index_in_to_vars = 0;
            for (int var_from = 0; var_from < number_of_vars - layer + 1; ++var_from) {
                if (var_from != index_of_constant) {
                    if ((*from_vars)[vars_case][var_from]) {
                        order_in_sub_table_bin += "1";
                        (*to_vars)[vars_case][index_in_to_vars] = true;
                    } else {
                        order_in_sub_table_bin += "0";
                        (*to_vars)[vars_case][index_in_to_vars] = false;
                    }
                    index_in_to_vars++; 
                } else {
                    value_of_constant = (*from_vars)[vars_case][var_from];
                }
            }
            int order_in_sub_table_dec = std::stoi(order_in_sub_table_bin, nullptr, 2);
            // std::cout << "order_in_sub_table_dec: " << std::to_string(order_in_sub_table_dec) << std::endl;
            
            // copy constant members (beside the last one) from (*from_vars) to (*to_vars)
            for (int var_wt = number_of_vars - layer + 1; var_wt < number_of_vars; ++var_wt) {
                if ((*from_vars)[vars_case][var_wt]) {
                    (*to_vars)[vars_case][index_in_to_vars] = true;
                } else {
                    (*to_vars)[vars_case][index_in_to_vars] = false;
                }
                index_in_to_vars++;
            }
            // add last new member of constant at the end of vars_case
            (*to_vars)[vars_case][index_in_to_vars] = value_of_constant;

            // read whole finished constant, convert it to decimal and find out to which subtable the new 1 should be added
            std::string which_sub_table_bin = "";
            for (int member_of_constant = number_of_vars - layer; member_of_constant < number_of_vars; ++member_of_constant) {
                if ((*to_vars)[vars_case][member_of_constant]) {
                    which_sub_table_bin += "1";
                } else {
                    which_sub_table_bin += "0";
                }
            }
            int which_sub_table_dec = std::stoi(which_sub_table_bin, nullptr, 2);
            // std::cout << "which_sub_table_dec: " << std::to_string(which_sub_table_dec) << std::endl;
            // create new function
            function_values[(size_of_subtable * which_sub_table_dec) + order_in_sub_table_dec] = true;
        }

        // Creating layer manager with lesser number of varibles
        teddy::bss_manager layer_manager(number_of_vars - layer, 100'000);

        // Disable automatic variable reordering.
        layer_manager.set_auto_reorder(false);

        // create derivatives for each subtable and sum TD for each variable from these derivatives 
        std::vector<double> td_of_vars_of_layer = std::vector<double>(layer_manager.get_var_count(), 0.0);
        for (int subtable = 0; subtable < number_of_subtables; ++subtable) {
            int start_index = subtable * size_of_subtable;
            int end_index = start_index + size_of_subtable - 1;

            // std::cout << "start_index: " << std::to_string(start_index) << std::endl;
            // std::cout << "end_index: " << std::to_string(end_index) << std::endl;
            /*
            std::vector<bool> sub_table(function_values.begin() + start_index, function_values.begin() + end_index + 1);
            for (auto s : sub_table) {
                std::cout << s << std::endl;
            }
            */

            teddy::bss_manager::diagram_t diagram = layer_manager.from_vector(function_values.begin() + start_index, function_values.begin() + end_index + 1);

            for (int variable = 0; variable < number_of_vars - layer; ++variable) {
                teddy::bss_manager::diagram_t dpbd_any_change = layer_manager.dpld({variable, 0, 1 }, teddy::dpld::basic_undirectional(), diagram);
                int number_of_ones_any_change = layer_manager.satisfy_all<std::vector<int>>(1, dpbd_any_change).size() / 2;
                /*
                std::vector<std::vector<int>> vars_with_ones_any_change = layer_manager.satisfy_all<std::vector<int>>(1, dpbd_any_change);
                std::cout << "Number of ones in dpbd_any_change: " << number_of_ones_any_change << std::endl;

                for (auto var : vars_with_ones_any_change) {
                    for (int i = 0; i < var.size(); ++i) {
                        std::cout << var[i];
                    }
                    std::cout << " ";
                }
                std::cout << "" << std::endl;
                */
                double true_density = (double)number_of_ones_any_change / (std::pow(2, layer_manager.get_var_count()) / 2);
                td_of_vars_of_layer[variable] += true_density;
            }
        }
        /*
        std::cout << "td_of_vars_of_layer:" << std::endl;
        for (auto td : td_of_vars_of_layer) {
            std::cout << std::to_string(td) << std::endl;
        }
        */
        // map TDs to order of variables in subtables
        for (int o = 0; o < number_of_vars - layer; ++o) {
            td_var var = td_var();
            var.variable = (*to_vars_order)[o];
            var.true_density = td_of_vars_of_layer[o];
            list_for_reordering[o] = var;
        }
        /*
        std::cout << "list_for_reordering before sort:" << std::endl;
        for (auto l : list_for_reordering) {
            std::cout << "variable: " << std::to_string(l.variable) << "td: " << std::to_string(l.true_density) << std::endl;
        }
        */
        // sort list of structs based on true density
        if (this->ascending) {
            std::sort(list_for_reordering.begin(), list_for_reordering.end(),
                [this](const td_var& a, const td_var& b) { return this->compare_by_true_density_asc(a, b); });
        }
        else {
            std::sort(list_for_reordering.begin(), list_for_reordering.end(),
                [this](const td_var& a, const td_var& b) { return this->compare_by_true_density_desc(a, b); });
        }
        /*
        std::cout << "list_for_reordering after sort:" << std::endl;
        for (auto l : list_for_reordering) {
            std::cout << "variable: " << std::to_string(l.variable) << "td: " << std::to_string(l.true_density) << std::endl;
        }
        */
        last_variable = list_for_reordering[0].variable;
        new_order.push_back(last_variable);
        /*
        std::cout << "new_order:" << std::endl;
        for (auto o : new_order) {
            std::cout << std::to_string(o) << std::endl;
        }
        */

        // swaps pointers so the second_variables will be used as a source and the first_variables will be rewritten and used for new function
        std::vector<std::vector<bool>>* swap_vars = from_vars;
        from_vars = to_vars;
        to_vars = swap_vars;

        std::vector<int>* swap_vars_order = from_vars_order;
        from_vars_order = to_vars_order;
        to_vars_order = from_vars_order;
    }

    last_variable = -1;
    for (int v = 0; v < number_of_vars; ++v) {
        bool is_in_new_order = false;
        for (int i = 0; i < new_order.size(); ++i) {
            if (v == new_order[i]) {
                is_in_new_order = true;
                break;
            }
        }
        if (!is_in_new_order) {
            last_variable = v;
            break;
        }
    }
    new_order.push_back(last_variable);
    /*
    std::cout << "new_order:" << std::endl;
    for (auto o : new_order) {
        std::cout << std::to_string(o) << std::endl;
    }
    */
    for (int new_o = 1; new_o < number_of_vars; ++new_o) {
        order_of_vars_from_ODT[new_o] = new_order[new_o];
    }
}

void DerivativesBasedOrderDT::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    // list for true densities of all variables of this function
    std::vector<td_var> list_for_reordering = std::vector<td_var>(number_of_vars);

    // save all true densities of all variables in this function in list_for_reordering
    get_td_of_all_vars_in_function(default_manager, list_for_reordering, diagram, which_function);
    /*
    std::cout << "Before sorting: " << std::endl;
    for (const auto& item : list_for_reordering) {
        std::cout << item.variable << " " << item.true_density << std::endl;
    }
    */

    //delete &diagram;

    // sort list of structs based on true density
    if (this->ascending) {
        std::sort(list_for_reordering.begin(), list_for_reordering.end(),
            [this](const td_var& a, const td_var& b) { return this->compare_by_true_density_asc(a, b); });
    } else {
        std::sort(list_for_reordering.begin(), list_for_reordering.end(),
            [this](const td_var& a, const td_var& b) { return this->compare_by_true_density_desc(a, b); });
    }
    
    /*
    std::cout << "After sorting: " << std::endl;
    for (const auto& item : list_for_reordering) {
        std::cout << item.variable << " " << item.true_density << std::endl;
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

    // creating manager with new order of variables based on true density
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

std::string DerivativesBasedOrderDT::to_string() {
    std::string return_string = "";
    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD_ODT w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (order);";

    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD_ODT w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";

    return return_string;
}
