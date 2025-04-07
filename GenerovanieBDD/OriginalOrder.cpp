#include "OriginalOrder.hpp"

OriginalOrder::OriginalOrder(bool use_var_reordering_heuristics, bool generate_graph_before_order, bool generate_graph_after_order) :
    Strategy(use_var_reordering_heuristics, generate_graph_before_order, generate_graph_after_order)
{}

OriginalOrder::~OriginalOrder() {}

void OriginalOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {
    auto start = std::chrono::high_resolution_clock::now();
    // creating manager with original order of variables
    teddy::bss_manager manager(number_of_vars, 10'000);
    manager.set_auto_reorder(false);

    // get n-th diagram from pla file 
    teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_before_order && !generate_diagram(file_name_without_extension + "_before", diagram, manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager.force_reorder();
    }

    if (this->generate_graph_after_order && !generate_diagram(file_name_without_extension + "_after", diagram, manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    std::string var_order = "";
    for (auto x : manager.get_order()) {
        var_order += "x";
        var_order += std::to_string(x);
    }
    csv->write_new_stats(var_order, (double)manager.get_node_count(diagram));

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    this->add_to_timer(duration.count());
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

std::string OriginalOrder::get_strategy_name() {
    std::string return_string = "Original order, Reordering heuristic: ";
    return_string += this->use_var_reordering_heuristics ? "YES" : "NO";
    return_string += " Generate graph before order: ";
    return_string += this->generate_graph_before_order ? "YES" : "NO";
    return_string += " Generate graph after order: ";
    return_string += this->generate_graph_after_order ? "YES" : "NO";

    return return_string;
}
