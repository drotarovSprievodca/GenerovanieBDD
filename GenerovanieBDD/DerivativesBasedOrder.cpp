#include "DerivativesBasedOrder.hpp"

DerivativesBasedOrder::DerivativesBasedOrder(bool use_var_reordering_heuristics, bool ascending) : 
    Strategy(use_var_reordering_heuristics)
{
    this->ascending = ascending;
}

DerivativesBasedOrder::~DerivativesBasedOrder() {}

bool compare_by_true_density_desc(const var& a, const var& b) {
    return a.true_density > b.true_density;
}

bool compare_by_true_density_asc(const var& a, const var& b) {
    return a.true_density < b.true_density;
}

void DerivativesBasedOrder::get_td_of_all_vars_in_function(teddy::bss_manager& default_manager, std::vector<var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram, int which_diagram) {
    for (int i = 0; i < list_for_reordering.size(); ++i) {
        // variable xi increases from 0 -> 1 while function decreases from 1 -> 0
        teddy::bss_manager::diagram_t dpbd_increase = default_manager.dpld({ i, 0, 1 }, teddy::dpld::type_1_decrease(1), diagram);

        // variable xi decreases from 1 -> 0 while function decreases from 1 -> 0
        teddy::bss_manager::diagram_t dpbd_decrease = default_manager.dpld({ i, 1, 0 }, teddy::dpld::type_1_decrease(1), diagram);

        // number of 1 in dpbd_decrease
        int number_of_ones_decrease = default_manager.satisfy_all<std::vector<int>>(1, dpbd_decrease).size() / 2;

        // number of 0 in dpbd_decrease
        int number_of_zeros_decrease = default_manager.satisfy_all<std::vector<int>>(0, dpbd_decrease).size() / 2;

        //delete &dpbd_decrease;

        // number of 1 in dpbd_increase
        int number_of_ones_increase = default_manager.satisfy_all<std::vector<int>>(1, dpbd_increase).size() / 2;

        // number of 0 in dpbd_increase
        int number_of_zeros_increase = default_manager.satisfy_all<std::vector<int>>(0, dpbd_increase).size() / 2;

        //delete &dpbd_increase;

        //////////////
        // DECREASE //
        //////////////
        // variables with dpbd_decrease == 1
        /*
        std::vector<std::vector<int>> vars_with_ones_decrease = default_manager.satisfy_all<std::vector<int>>(1, dpbd_decrease);
        std::cout << "Number of ones in dpbd_decrease: " << number_of_ones_decrease << std::endl;

        for (auto var : vars_with_ones_decrease) {
            for (int i = 0; i < 3; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */
        // variables with dpbd_decrease == 0
        /*
        std::vector<std::vector<int>> vars_with_zeros_decrease = default_manager.satisfy_all<std::vector<int>>(0, dpbd_decrease);
        std::cout << "Number of zeros in dpbd_decrease: " << number_of_zeros_decrease << std::endl;

        for (auto var : vars_with_zeros_decrease) {
            for (int i = 0; i < 3; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */
        //////////////
        // INCREASE //
        //////////////
        // variables with dpbd_increase == 1
        //std::vector<std::vector<int>> vars_with_ones_increase = default_manager.satisfy_all<std::vector<int>>(1, dpbd_increase);
        //std::cout << "Number of ones in dpbd_increase: " << number_of_ones_increase << std::endl;
        /*
        for (auto var : vars_with_ones_increase) {
            for (int i = 0; i < 3; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */
        // variables with dpbd_increase == 0
        //std::vector<std::vector<int>> vars_with_zeros_increase = default_manager.satisfy_all<std::vector<int>>(0, dpbd_increase);
        //std::cout << "Number of zeros in dpbd_increase: " << number_of_zeros_increase << std::endl;
        /*
        for (auto var : vars_with_zeros_increase) {
            for (int i = 0; i < 3; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */
        ///////////////////////////////////////////////////////

        double td_for_decrease = (double)number_of_ones_decrease / (number_of_ones_decrease + number_of_zeros_decrease);
        double td_for_increase = (double)number_of_ones_increase / (number_of_ones_increase + number_of_zeros_increase);

        double true_density = td_for_decrease + td_for_increase;
        //std::cout << "True density for function " << std::to_string(which_diagram) << " is: " << std::to_string(true_density) << std::endl;

        var td_var = var();
        td_var.true_density = true_density;
        td_var.variable = i;

        list_for_reordering[i] = td_var;
    }
}

double DerivativesBasedOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput *csv, int which_function) {
    // get function from pla file
    teddy::bss_manager::diagram_t diagram = default_manager.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    // list for true densities of all variables of this function
    std::vector<var> list_for_reordering = std::vector<var>(number_of_vars);

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
        std::sort(list_for_reordering.begin(), list_for_reordering.end(), compare_by_true_density_asc);
    }
    else {
        std::sort(list_for_reordering.begin(), list_for_reordering.end(), compare_by_true_density_desc);
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
    teddy::bss_manager manager_after(number_of_vars, 100'000, order_after);
    manager_after.set_auto_reorder(false);
    teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[which_function];

    if (this->use_var_reordering_heuristics) {
        // Runs the variable reordering heuristic
        manager_after.force_reorder();
    }

    // return number of nodes of current diagram
    return manager_after.get_node_count(diagram_after);
}

std::string DerivativesBasedOrder::to_string() {
    std::string return_string = "Order based on ";
    return_string += this->ascending ? "ascending" : "descending";
    return_string += " true density with";
    return_string += this->use_var_reordering_heuristics ? "" : "out";
    return_string += " reordering heuristics";

    return return_string;
}