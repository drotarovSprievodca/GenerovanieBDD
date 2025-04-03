#include "BestOrder.hpp"

BestOrder::BestOrder(bool generate_graph_before_order, bool generate_graph_after_order) :
    Strategy(false, generate_graph_before_order, generate_graph_after_order)
{}

BestOrder::~BestOrder() {}

void BestOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {

    // get n-th diagram from pla file 
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_before_order && !generate_diagram(file_name_without_extension + "_before", diagram, default_manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    std::vector<int> best_order_so_far = std::vector<int>(number_of_vars);
    int best_node_count_so_far = std::numeric_limits<int>::max();

    std::vector<int> order = std::vector<int>(number_of_vars);
    for (int b = 0; b < number_of_vars; ++b) {
        order[b] = b;
    }

    sort(order.begin(), order.end());

    do {
        std::vector<teddy::int32> teddy_order = std::vector<teddy::int32>(number_of_vars);

        for (int i = 0; i < number_of_vars; ++i) {
            teddy_order[i] = order[i];
        }

        teddy::bss_manager order_manager(number_of_vars, 100'000, teddy_order);
        order_manager.set_auto_reorder(false);
        teddy::bss_manager::diagram_t diagram_after = order_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];
        int node_count = order_manager.get_node_count(diagram_after);

        /*
        for (int var = 0; var < number_of_vars; ++var) {
            std::cout << "x" << std::to_string(order[var]);
        }
        std::cout << '\0' << std::endl;
        std::cout << "NODE COUNT: " << std::to_string(node_count) << std::endl;
        */
        
        if (node_count < best_node_count_so_far) {
            best_node_count_so_far = node_count;
            for (int c = 0; c < number_of_vars; ++c) {
                best_order_so_far[c] = order[c];
            }
        }
    } while (std::next_permutation(order.begin(), order.end()));
    

    // vector with new order in teddy format 
    std::vector<teddy::int32> best_order_teddy = std::vector<teddy::int32>(number_of_vars);

    // moving randomly ordered list to teddy format vector
    for (int i = 0; i < number_of_vars; ++i) {
        best_order_teddy[i] = best_order_so_far[i];
    }

    // creating manager with new random order of variables
    teddy::bss_manager manager_after(number_of_vars, 100'000, best_order_teddy);
    manager_after.set_auto_reorder(false);

    // get i-th diagram from pla file 
    teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_after_order && !generate_diagram(file_name_without_extension + "_after", diagram, manager_after, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    std::string var_order = "";
    for (auto x : manager_after.get_order()) {
        var_order += "x";
        var_order += std::to_string(x);
    }
    csv->write_new_stats(var_order, (double)manager_after.get_node_count(diagram_after));
}

std::string BestOrder::to_string() {
    std::string return_string = "Best w/o RH (order);";
    return_string += "Best w/o RH (# of nodes)";
    return return_string;
}

std::string BestOrder::get_strategy_name() {
    return "BestOrder";
}
