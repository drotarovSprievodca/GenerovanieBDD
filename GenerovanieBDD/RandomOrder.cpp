#include "RandomOrder.hpp"

RandomOrder::RandomOrder(bool use_var_reordering_heuristics, int number_of_replications) :
    Strategy(use_var_reordering_heuristics, false, false),
    eng(std::random_device{}())
{
    this->number_of_replications = number_of_replications;
    this->sum_of_node_counts_in_function_for_all_rep = 0.0;
}

RandomOrder::~RandomOrder() {}

void RandomOrder::process_function(teddy::bss_manager& default_manager, int number_of_vars, teddy::pla_file* pla, CSVOutput* csv, int which_function, std::string file_name_without_extension) {
    auto start = std::chrono::high_resolution_clock::now();
    this->sum_of_node_counts_in_function_for_all_rep = 0.0;
    for (int r = 0; r < this->number_of_replications; ++r) {
        // list of indexes of all variables of this function
        std::vector<int> list_for_random_order = std::vector<int>(number_of_vars);

        // populate list with indexes  
        //  indexes of vector -> [0][1][2][3]
        //          variables -> |0||1||2||3|    
        // means that variable x0 is first in order, x1 is second and so on ...                   
        for (int j = 0; j < number_of_vars; ++j) {
            list_for_random_order[j] = j;
        }

        // Shuffle the vector
        //  indexes of vector -> [0][1][2][3]
        //          variables -> |1||3||0||2|    
        // means that variable x0 is third in order, x1 is first and so on ...  
        std::shuffle(list_for_random_order.begin(), list_for_random_order.end(), eng);

        // vector with new order in teddy format 
        std::vector<teddy::int32> random_order = std::vector<teddy::int32>(number_of_vars);

        // moving randomly ordered list to teddy format vector
        for (int i = 0; i < number_of_vars; ++i) {
            random_order[i] = list_for_random_order[i];
        }

        // creating manager with new random order of variables
        teddy::bss_manager manager_after(number_of_vars, 10'000, random_order);
        manager_after.set_auto_reorder(false);

        // get i-th diagram from pla file 
        teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[which_function];

        if (this->use_var_reordering_heuristics) {
            // Runs the variable reordering heuristic
            manager_after.force_reorder();
        }
        this->sum_of_node_counts_in_function_for_all_rep += manager_after.get_node_count(diagram_after);
    }
    this->sum_of_node_counts_in_function_for_all_rep /= this->number_of_replications;
    csv->write_new_stats("", this->sum_of_node_counts_in_function_for_all_rep);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    this->add_to_timer(duration.count());
}

std::string RandomOrder::to_string() {
    std::string return_string = ";Random w";
    return_string += this->use_var_reordering_heuristics ? "" : "/o";
    return_string += " RH (# of nodes)";
    return return_string;
}

std::string RandomOrder::get_strategy_name() {
    std::string return_string = "Random order, Reordering heuristic: ";
    return_string += this->use_var_reordering_heuristics ? "YES" : "NO";
    return_string += " Number of replications: " + std::to_string(this->number_of_replications);

    return return_string;
}
