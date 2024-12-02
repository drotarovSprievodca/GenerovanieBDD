#include "OriginalOrder.hpp"

OriginalOrder::OriginalOrder(bool use_var_reordering_heuristics) : 
    Strategy(use_var_reordering_heuristics) 
{}

OriginalOrder::~OriginalOrder() {}

double OriginalOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, int which_function) {
    // creating manager with original order of variables
    teddy::bss_manager manager(number_of_vars, 100'000);
    manager.set_auto_reorder(false);

    // get n-th diagram from pla file 
    teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager.force_reorder();
    }

    // return number of nodes of current diagram
    return (double)manager.get_node_count(diagram);
}

std::string OriginalOrder::to_string() {
    std::string return_string = "Original order with";
    return_string += !this->use_var_reordering_heuristics ? "out" : "";
    return_string += " reordering heuristics";
    return return_string;
}
