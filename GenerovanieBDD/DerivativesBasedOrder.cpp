#include "DerivativesBasedOrder.hpp"

DerivativesBasedOrder::DerivativesBasedOrder(bool use_var_reordering_heuristics, bool ascending, bool generate_graph_before_order, bool generate_graph_after_order) :
    Strategy(use_var_reordering_heuristics, generate_graph_before_order, generate_graph_after_order)
{
    this->ascending = ascending;
}

DerivativesBasedOrder::~DerivativesBasedOrder() {}

bool DerivativesBasedOrder::compare_by_true_density_desc(const td_var& a, const td_var& b) {
    return a.true_density > b.true_density;
}

bool DerivativesBasedOrder::compare_by_true_density_asc(const td_var& a, const td_var& b) {
    return a.true_density < b.true_density;
}

void DerivativesBasedOrder::get_td_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<td_var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    for (int i = 0; i < list_for_reordering.size(); ++i) {
        // variable xi increases from 0 -> 1 while function decreases from 1 -> 0 or increases from 0 -> 1
        teddy::bss_manager::diagram_t dpbd_any_change = default_manager.dpld({ i, 0, 1 }, teddy::dpld::basic_undirectional(), diagram);
        // number of ones in undirectional derivative
        int number_of_ones_any_change = default_manager.satisfy_all<std::vector<int>>(1, dpbd_any_change).size() / 2;
        // number of zeros in undirectional derivative
        //int number_of_zeros_any_change = default_manager.satisfy_all<std::vector<int>>(0, dpbd_any_change).size() / 2;
        
        /*
        std::vector<std::vector<int>> vars_with_ones_any_change = default_manager.satisfy_all<std::vector<int>>(1, dpbd_any_change);
        std::cout << "Number of ones in dpbd_any_change: " << number_of_ones_any_change << std::endl;

        for (auto var : vars_with_ones_any_change) {
            for (int i = 0; i < var.size(); ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;


        std::vector<std::vector<int>> vars_with_zeros_any_change = default_manager.satisfy_all<std::vector<int>>(0, dpbd_any_change);
        std::cout << "Number of zeros in dpbd_any_change: " << number_of_zeros_any_change << std::endl;

        for (auto var : vars_with_zeros_any_change) {
            for (int i = 0; i < var.size(); ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */

        double true_density = (double)number_of_ones_any_change / (std::pow(2, default_manager.get_var_count()) / 2);
        //std::cout << "True density for f" << std::to_string(which_diagram) << " and variable x" << std::to_string(i) << " is: " << std::to_string(true_density) << std::endl;

        td_var var = td_var();
        var.true_density = true_density;
        var.variable = i;

        list_for_reordering[i] = var;
    }
}

void DerivativesBasedOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput *csv, int which_function, std::string file_name_without_extension) {
    auto start = std::chrono::high_resolution_clock::now();
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_before_order && !generate_diagram(file_name_without_extension + "_before", diagram, default_manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

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
    // vector with new order in teddy format 
    std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

    // moving reordered list to teddy format vector
    for (int i = 0; i < number_of_vars; ++i) {
        order_after[i] = list_for_reordering[i].variable;
    }

    // creating manager with new order of variables based on true density
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

std::string DerivativesBasedOrder::to_string() {
    std::string return_string = "";
    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (order);";

    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";

    return return_string;
}

std::string DerivativesBasedOrder::get_strategy_name() {
    std::string return_string = "Derivatives based order, ";
    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD, Reordering heuristic: ";
    return_string += this->use_var_reordering_heuristics ? "YES" : "NO";
    return_string += " Generate graph before order: ";
    return_string += this->generate_graph_before_order ? "YES" : "NO";
    return_string += " Generate graph after order: ";
    return_string += this->generate_graph_after_order ? "YES" : "NO";

    return return_string;
}