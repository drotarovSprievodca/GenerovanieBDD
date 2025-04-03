#include "DerivativesBasedHigherOrder.hpp"

DerivativesBasedHigherOrder::DerivativesBasedHigherOrder(bool use_var_reordering_heuristics, bool ascending, bool generate_graph_before_order, bool generate_graph_after_order) :
	DerivativesBasedOrder(use_var_reordering_heuristics, ascending, generate_graph_before_order, generate_graph_after_order) {}

DerivativesBasedHigherOrder::~DerivativesBasedHigherOrder() {}

void DerivativesBasedHigherOrder::get_order_from_higher_order_derivatives(teddy::bss_manager& default_manager, std::vector<int>& order_of_vars_from_HOD, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    int number_of_vars = default_manager.get_var_count();
    int number_of_lines = std::pow(2, number_of_vars);
    std::vector<int> unused_vars = std::vector<int>(number_of_vars);
    for (int var = 0; var < number_of_vars; ++var) {
        unused_vars[var] = var;
    }
    std::vector<int> new_order = std::vector<int>();
    std::vector<bool> derived_function = std::vector<bool>(std::pow(2, number_of_vars));
    std::vector<int> original_function = default_manager.to_vector(diagram);
    for (int f_v = 0; f_v < number_of_lines; ++f_v) {
        if (original_function[f_v] == 1) {
            derived_function[f_v] = true;
        } else {
            derived_function[f_v] = false;
        }
    }
    original_function = std::vector<int>(); // deallocate vector
    

    for (int layer = 0; layer < number_of_vars - 1; ++layer) {
        teddy::bss_manager layer_manager(number_of_vars - layer, 100'000);
        layer_manager.set_auto_reorder(false);
        teddy::bss_manager::diagram_t layer_diagram = layer_manager.from_vector(derived_function);

        double reference_td = 0.0;
        if (this->ascending) {
            reference_td = std::numeric_limits<double>::max();
        }
        int var_highest_td = unused_vars[0];
        int index_highest_td = 0;
        std::vector<teddy::bss_manager::diagram_t> dplds = std::vector<teddy::bss_manager::diagram_t>();

        for (int var_index = 0; var_index < unused_vars.size(); ++var_index) {
            dplds.push_back(layer_manager.dpld({var_index, 0, 1}, teddy::dpld::basic_undirectional(), layer_diagram));
            int number_of_ones_any_change = layer_manager.satisfy_all<std::vector<int>>(1, dplds[var_index]).size() / 2;
            double true_density = (double)number_of_ones_any_change / (std::pow(2, layer_manager.get_var_count()) / 2);
            
            if ((this->ascending) ? true_density < reference_td : true_density > reference_td) {
                var_highest_td = unused_vars[var_index];
                index_highest_td = var_index;
                reference_td = true_density;
            }
        }

        int size_of_original_function = std::pow(2, number_of_vars - layer);
        int index_of_removed_variable = std::distance(unused_vars.begin(), std::find(unused_vars.begin(), unused_vars.end(), var_highest_td));
        unused_vars.erase(unused_vars.begin() + index_of_removed_variable);
        new_order.push_back(var_highest_td);

        std::vector<bool> derivatives = std::vector<bool>(size_of_original_function, false);
        int size_of_block = std::pow(2, unused_vars.size() - index_of_removed_variable); // derived function is in these alternating blocks in vector: derivatives
        std::vector<std::vector<int>> vars_with_ones_any_change = layer_manager.satisfy_all<std::vector<int>>(1, dplds[index_highest_td]);
        for (std::vector<int> der_value_position : vars_with_ones_any_change) {
            int der_value_position_dec = 0;
            for (int bit : der_value_position) {
                der_value_position_dec = (der_value_position_dec << 1) | bit;
            }
            derivatives[der_value_position_dec] = true;
        }

        int index_in_derived_function = 0;
        derived_function = std::vector<bool>(size_of_original_function / 2);
        int index_in_blocks = 0;
        int current_size_of_block = 0;
        while (index_in_blocks < size_of_original_function) {
            derived_function[index_in_derived_function] = derivatives[index_in_blocks];
            index_in_derived_function++;
            current_size_of_block++;
            if (current_size_of_block == size_of_block) {
                current_size_of_block = 0;
                index_in_blocks += (size_of_block + 1);
            } else {
                index_in_blocks++;
            }
        }
        /*
        std::cout << "unused_vars:" << std::endl;
        for (int var : unused_vars) {
            std::cout << var << std::endl;
        }

        std::cout << "derived_function:" << std::endl;
        for (int value : derived_function) {
            std::cout << value << std::endl;
        }
        */
    }
    
    new_order.push_back(unused_vars[0]);
    /*
    for (int i = 0; i < new_order.size(); ++i) {
        std::cout << new_order[i] << std::endl;
    }
    */
    for (int i = 0; i < new_order.size(); ++i) {
        order_of_vars_from_HOD[i] = new_order[i];
    }
}

void DerivativesBasedHigherOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->generate_graph_before_order && !generate_diagram(file_name_without_extension + "_before", diagram, default_manager, which_function)) {
        std::cout << "Couldn't generate diagram!!!" << std::endl;
        return;
    }

    // list for order of variables from ODT from top to bottom
    std::vector<int> order_of_vars_from_HOD = std::vector<int>(number_of_vars);

    // get order of variables from ODT from top to bottom
    get_order_from_higher_order_derivatives(default_manager, order_of_vars_from_HOD, diagram, which_function);

    // vector with new order in teddy format 
    std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

    // moving reordered list to teddy format vector
    for (int i = 0; i < number_of_vars; ++i) {
        order_after[i] = order_of_vars_from_HOD[i];
    }

    // creating manager with new order of variables based on true density
    teddy::bss_manager manager_after(number_of_vars, 100'000, order_after);
    manager_after.set_auto_reorder(false);
    teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager_after.force_reorder();
    }

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

std::string DerivativesBasedHigherOrder::to_string() {
    std::string return_string = "";
    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD_HOD w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (order);";

    return_string += this->ascending ? "Ascending" : "Descending";
    return_string += " TD_HOD w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";

    return return_string;
}

std::string DerivativesBasedHigherOrder::get_strategy_name() {
    return "DerivativesBasedHigherOrder";
}
