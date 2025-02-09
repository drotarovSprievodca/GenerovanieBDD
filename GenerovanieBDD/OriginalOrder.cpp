#include "OriginalOrder.hpp"

OriginalOrder::OriginalOrder(bool use_var_reordering_heuristics) : 
    Strategy(use_var_reordering_heuristics) 
{}

OriginalOrder::~OriginalOrder() {}

void OriginalOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function) {
    // creating manager with original order of variables
    teddy::bss_manager manager(number_of_vars, 100'000);
    manager.set_auto_reorder(false);

    // get n-th diagram from pla file 
    teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager.force_reorder();
    }

    std::string var_order = "";
    for (auto x : manager.get_order()) {
        var_order += "x";
        var_order += std::to_string(x);
    }
    csv->write_new_stats(var_order, (double)manager.get_node_count(diagram));
}

std::string OriginalOrder::to_string() {
    std::string return_string = "Original w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (order);";
    return_string += "Original w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";
    return return_string;
}
