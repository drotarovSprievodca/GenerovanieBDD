#include "DerivativesBasedOrderDT.hpp"

DerivativesBasedOrderDT::DerivativesBasedOrderDT(bool use_var_reordering_heuristics, bool ascending) :
	DerivativesBasedOrder(use_var_reordering_heuristics, ascending) {}

DerivativesBasedOrderDT::~DerivativesBasedOrderDT() {}

void DerivativesBasedOrderDT::get_order_from_ODT(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_ODT, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    int last_variable = order_of_vars_from_ODT[0]; // root was calculated from layer 0
    int number_of_vars = default_manager.get_var_count();
    int number_of_lines = std::pow(2, number_of_vars);
    std::vector<int> new_order = std::vector<int>();
    new_order.push_back(last_variable);

    std::vector<std::vector<bool>> first_variables = default_manager.satisfy_all<std::vector<bool>>(1, diagram);
    std::vector<std::vector<bool>> second_variables = default_manager.satisfy_all<std::vector<bool>>(1, diagram);
    std::vector<std::vector<bool>>* from_vars = &first_variables;
    std::vector<std::vector<bool>>* to_vars = &second_variables;

    std::unordered_map<int, int> index_map_first_variables = std::unordered_map<int, int>(number_of_vars);
    std::unordered_map<int, int> index_map_second_variables = std::unordered_map<int, int>(number_of_vars);
    std::unordered_map<int, int>* index_map_from_vars = &index_map_first_variables;
    std::unordered_map<int, int>* index_map_to_vars = &index_map_second_variables;

    for (int v = 0; v < number_of_vars; ++v) {
        (*index_map_from_vars)[v] = v;
    }

    std::vector<bool> function_values = std::vector<bool>(number_of_lines, false);

    for (int layer = 1; layer < number_of_vars - 1; ++layer) {
        std::fill(function_values.begin(), function_values.end(), false);
        int index_of_constant = (*index_map_from_vars)[last_variable];




        for (int vars_case = 0; vars_case < (*from_vars).size(); ++vars_case) {
            std::string order_in_function_bin = "";
            bool value_of_constant;
            for (int var_from = layer - 1; var_from < number_of_vars; ++var_from) {
                if (var_from != index_of_constant) {
                    order_in_function_bin += (*from_vars)[vars_case][var_from] ? "1" : "0";
                } else {
                    value_of_constant = (*from_vars)[vars_case][var_from];
                }
            }
            int order_in_function_dec = std::stoi(order_in_function_bin, nullptr, 2);

            std::string which_table_bin = "";
            for (int var_wt = 0; var_wt < layer - 1; ++var_wt) {
                which_table_bin += (*from_vars)[vars_case][var_wt] ? "1" : "0";
            }
            int which_table_dec = std::stoi(which_table_bin, nullptr, 2);


        }

    }




















    /*
    for (int ord = 1; ord < number_of_vars; ++ord) {
        order_of_vars_from_ODT[ord] = new_order[ord];
    }
    */


    // need to replace at the end
    for (int j = 0; j < number_of_vars; ++j) {
        order_of_vars_from_ODT[j] = j;
    }
    /////////////////////////
}

void DerivativesBasedOrderDT::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) {
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
