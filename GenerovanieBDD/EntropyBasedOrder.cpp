#include "EntropyBasedOrder.hpp"

EntropyBasedOrder::EntropyBasedOrder(bool use_var_reordering_heuristics) :
    Strategy(use_var_reordering_heuristics) {}

EntropyBasedOrder::~EntropyBasedOrder() {}

bool compare_by_conditional_entropy_asc(const ce_var& a, const ce_var& b) {
    return a.conditional_entropy < b.conditional_entropy;
}

void EntropyBasedOrder::get_ce_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<ce_var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    for (int i = 0; i < list_for_reordering.size(); ++i) {

        //default_manager;

        //diagram;

        double conditional_entropy = 0.0;
        //std::cout << "Conditional entropy for function " << std::to_string(which_diagram) << " is: " << std::to_string(conditional_entropy) << std::endl;

        ce_var var = ce_var();
        var.conditional_entropy = conditional_entropy;
        var.variable = i;

        list_for_reordering[i] = var;
    }
}

void EntropyBasedOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) {
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

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
    std::sort(list_for_reordering.begin(), list_for_reordering.end(), compare_by_conditional_entropy_asc);
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