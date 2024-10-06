﻿#include <libteddy/core.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <libteddy/details/types.hpp>
#include <libteddy/details/pla_file.hpp>
#include "libteddy/details/reliability_manager.hpp"
#include <libteddy/reliability.hpp>
#include <algorithm>
#include <windows.h>

struct var {
    int variable;
    double true_density;
};

bool compare_by_true_density_desc(const var& a, const var& b) {
    return a.true_density > b.true_density;
}

bool compare_by_true_density_asc(const var& a, const var& b) {
    return a.true_density < b.true_density;
}

void get_td_of_all_vars_in_function(teddy::bss_manager& manager, std::vector<var>& list_for_reordering, teddy::bss_manager::diagram_t& diagram) {
    for (int i = 0; i < list_for_reordering.size(); ++i) {
        // variable xi decreases from 1 -> 0 while function decreases from 1 -> 0
        teddy::bss_manager::diagram_t dpbd_decrease = manager.dpld({ i, 1, 0 }, teddy::dpld::type_1_decrease(1), diagram);

        // variable xi increases from 0 -> 1 while function decreases from 1 -> 0
        teddy::bss_manager::diagram_t dpbd_increase = manager.dpld({ i, 0, 1 }, teddy::dpld::type_1_decrease(1), diagram);


        // number of 1 in dpbd_decrease
        int number_of_ones_decrease = manager.satisfy_all<std::vector<int>>(1, dpbd_decrease).size() / 2;

        // number of 0 in dpbd_decrease
        int number_of_zeros_decrease = manager.satisfy_all<std::vector<int>>(0, dpbd_decrease).size() / 2;

        // number of 1 in dpbd_increase
        int number_of_ones_increase = manager.satisfy_all<std::vector<int>>(1, dpbd_increase).size() / 2;

        // number of 0 in dpbd_increase
        int number_of_zeros_increase = manager.satisfy_all<std::vector<int>>(0, dpbd_increase).size() / 2;


        double td_for_decrease = (double)number_of_ones_decrease / (number_of_ones_decrease + number_of_zeros_decrease);
        double td_for_increase = (double)number_of_ones_increase / (number_of_ones_increase + number_of_zeros_increase);

        double true_density = td_for_decrease + td_for_increase;

        var td_var;
        td_var.true_density = true_density;
        td_var.variable = i;

        list_for_reordering[i] = td_var;
    }
}

int main() {

    std::string comparing_option = "ASCENDING_TD"; // ASCENDING_TD, DESCENDING_TD, RANDOM
    std::cout << comparing_option << std::endl;

    std::string directory_path = "C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD\\ALL_PLA\\";
    WIN32_FIND_DATA find_file_data;
    HANDLE h_find = FindFirstFile((directory_path + "*").c_str(), &find_file_data);

    if (h_find == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not open directory!" << std::endl;
        return 1;
    }
        
    int sum_of_node_counts = 0;
    int number_of_diagrams = 0;

    do {
        if (std::string(find_file_data.cFileName) == "." || std::string(find_file_data.cFileName) == "..") { 
            continue; 
        }
        std::string pla_path = directory_path + find_file_data.cFileName;

        std::optional<teddy::pla_file> pla_file = teddy::pla_file::load_file(pla_path);
        if (!pla_file.has_value()) { 
            continue; 
        }

        teddy::pla_file* pla = &pla_file.value();

        // number of variables in this pla file
        int number_of_vars = pla->get_variable_count();

        // number of functions in this pla file
        int number_of_functions = pla->get_function_count();

        // Creating manager with default order of variables
        teddy::bss_manager manager(number_of_vars, 33'000);

        // Disable automatic variable reordering.
        manager.set_auto_reorder(false); 

        std::cout << "There is " << std::to_string(number_of_functions) << " functions with " << std::to_string(number_of_vars) << " variables in file: " + std::string(find_file_data.cFileName) << std::endl;
        if (comparing_option == "ORIGINAL") {
            for (int i = 0; i < number_of_functions; ++i) {
                // get function from pla file
                teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[i];
                
                // save statistics
                int node_count = manager.get_node_count(diagram);
                //std::cout << "Number of nodes in diagram (including terminal nodes): " << node_count << std::endl;
                sum_of_node_counts += node_count;
                number_of_diagrams += 1;
            }
        } else if (comparing_option == "DESCENDING_TD") {
            for (int i = 0; i < number_of_functions; ++i) {
                // get function from pla file
                teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[i];

                // list for true densities of all variables of this function
                std::vector<var> list_for_reordering = std::vector<var>(number_of_vars);
                
                // save all true densities of each variables in this function in list_for_reordering
                get_td_of_all_vars_in_function(manager, list_for_reordering, diagram);
                
                // sort list of structs based on true density
                std::sort(list_for_reordering.begin(), list_for_reordering.end(), compare_by_true_density_desc);
                
                // vector with new order in teddy format 
                std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

                // moving reordered list to teddy format vector
                for (int i = 0; i < number_of_vars; ++i) {
                    order_after[i] = list_for_reordering[i].variable;
                }

                // creating manager with new order of variables based on true density
                teddy::bss_manager manager_after(number_of_vars, 1'000, order_after);
                manager_after.set_auto_reorder(false);
                teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[0];

                // save statistics
                int node_count = manager_after.get_node_count(diagram_after);
                sum_of_node_counts += node_count;
                number_of_diagrams += 1;
            }
        } else if (comparing_option == "ASCENDING_TD") {
            for (int i = 0; i < number_of_functions; ++i) {
                // get function from pla file
                teddy::bss_manager::diagram_t diagram = manager.from_pla(*pla, teddy::fold_type::Tree)[i];

                // list for true densities of all variables of this function
                std::vector<var> list_for_reordering = std::vector<var>(number_of_vars);

                // save all true densities of each variables in this function in list_for_reordering
                get_td_of_all_vars_in_function(manager, list_for_reordering, diagram);

                // sort list of structs based on true density
                std::sort(list_for_reordering.begin(), list_for_reordering.end(), compare_by_true_density_asc);

                // vector with new order in teddy format 
                std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

                // moving reordered list to teddy format vector
                for (int i = 0; i < number_of_vars; ++i) {
                    order_after[i] = list_for_reordering[i].variable;
                }

                // creating manager with new order of variables based on true density
                teddy::bss_manager manager_after(number_of_vars, 1'000, order_after);
                manager_after.set_auto_reorder(false);
                teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[0];

                // save statistics
                int node_count = manager_after.get_node_count(diagram_after);
                sum_of_node_counts += node_count;
                number_of_diagrams += 1;
                manager_after.clear_cache();
            } 
        } else if (comparing_option == "RANDOM") {
            // pass
        }
    } while (FindNextFile(h_find, &find_file_data) != 0);
    FindClose(h_find);

    // print statistics
    if (number_of_diagrams != 0) {
        double average_number_of_nodes = sum_of_node_counts / number_of_diagrams;
        std::cout << "Average number of nodes with " + comparing_option + " order is: " + std::to_string(average_number_of_nodes) << std::endl;
    }  
}

    








































    //std::optional<teddy::pla_file> pla_file = teddy::pla_file::load_file("C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD\\PLA\\5xp1_pomlcka.pla"); //9sym.pla  alu4.pla  misex3.pla  5xp1.pla  bw.pla

    //if (pla_file.has_value()) {

        //teddy::pla_file* pla = &pla_file.value();
        //int number_of_vars = pla->get_variable_count();

        // std::cout << "Number of function in file: " << pla->get_function_count() << std::endl;
        // std::cout << "Number of variables in the file: " << number_of_vars << std::endl;
        // std::cout << "Number of lines in the file: " << pla->get_line_count() << std::endl;

        /*
        std::vector<std::string> const& input_labels = pla->get_input_labels();
        if (input_labels.size() > 0) {
            std::cout << "Input labels: " << std::endl;
            for (auto label : input_labels) {
                std::cout << label << " ";
            }
            std::cout << "" << std::endl;
        }
        
        std::vector<std::string> const& output_labels = pla->get_output_labels();
        if (output_labels.size() > 0) {
            std::cout << "Output labels: " << std::endl;
            for (auto label : output_labels) {
                std::cout << label << " ";
            }
            std::cout << "" << std::endl;
        }
        
        /*
        std::cout << "Lines: " << std::endl;
        std::vector<teddy::pla_file::pla_line> const& lines = pla->get_lines();
        for (auto line : lines) {
            for (int i = 0; i < line.cube_.size(); ++i) {
                std::cout << line.cube_.get(i);
            }
            std::cout << " ";
            for (int i = 0; i < line.fVals_.size(); ++i) {
                std::cout << line.fVals_.get(i);
            }
            std::cout << "\n";
        }
        std::cout << "" << std::endl;
        */

        // creating manager with default order of variables
        //teddy::bss_manager manager_before(number_of_vars, 1'000);
        //manager_before.set_auto_reorder(false); // Disable automatic variable reordering.
        //teddy::bss_manager::diagram_t diagram_before = manager_before.from_pla(*pla, teddy::fold_type::Tree)[0]; //first function from pla file

        //std::cout << "Number of variables: " << manager.get_var_count() << std::endl;

        /*
        std::cout << "Domains:" << std::endl;
        std::vector<int> domains = manager.get_domains();
        for (auto d : domains) {
            std::cout << d << std::endl;
        }
        
        // prints diagram to console
        // manager.to_dot_graph(std::cout, diagram);
        // prints diagram to file
        std::ofstream ofst_diag_before("diagram_before.dot");
        manager_before.to_dot_graph(ofst_diag_before, diagram_before);
        // converts .dot file to .png file 
        // .\dot.exe -Tpng diagram_before.dot -o diagram_before.png

        int node_count_before = manager_before.get_node_count(diagram_before);
        std::cout << "Number of nodes in diagram (including terminal nodes) BEFORE: " << node_count_before << std::endl;
        std::vector<int> old_order = manager_before.get_order(); // default order
        for (auto o : old_order) {
            std::cout << o << " ";
        }
        std::cout << std::endl;

        //manager.force_reorder(); // Runs variable reordering heuristic.

        std::vector<var> list_for_reordering = std::vector<var>(number_of_vars);

        for (int i = 0; i < number_of_vars; ++i) {
            // variable xi decreases from 1 -> 0 while function decreases from 1 -> 0
            teddy::bss_manager::diagram_t dpbd_decrease = manager_before.dpld({ i, 1, 0 }, teddy::dpld::type_1_decrease(1), diagram_before);
            
            // variable xi increases from 0 -> 1 while function decreases from 1 -> 0
            teddy::bss_manager::diagram_t dpbd_increase = manager_before.dpld({ i, 0, 1 }, teddy::dpld::type_1_decrease(1), diagram_before);

            // variables with dpbd == 1
            /*
            std::vector<std::vector<int>> vars_with_ones = manager_before.satisfy_all<std::vector<int>>(1, dpbd_decrease);
            int number_of_ones_in_dpbd = vars_with_ones.size();
            std::cout << "Number of ones in dpbd: " << number_of_ones_in_dpbd << std::endl;

            for (auto var : vars_with_ones) {
                for (int i = 0; i < number_of_vars; ++i) {
                    std::cout << var[i];
                }
                std::cout << " " << std::endl;
            }
            std::cout << "" << std::endl;
            

            // variables with dpbd == 0
            
            std::vector<std::vector<int>> vars_with_zeros = manager_before.satisfy_all<std::vector<int>>(0, dpbd_decrease);
            int number_of_zeros_in_dpbd = vars_with_zeros.size();
            std::cout << "Number of zeros in dpbd: " << number_of_zeros_in_dpbd << std::endl;

            for (auto var : vars_with_zeros) {
                for (int i = 0; i < number_of_vars; ++i) {
                    std::cout << var[i];
                }
                std::cout << " " << std::endl;
            }
            std::cout << "" << std::endl;
            
        
            // number of 1 in dpbd_decrease
            int number_of_ones_decrease = manager_before.satisfy_all<std::vector<int>>(1, dpbd_decrease).size() / 2;

            // number of 0 in dpbd_decrease
            int number_of_zeros_decrease = manager_before.satisfy_all<std::vector<int>>(0, dpbd_decrease).size() / 2;

            // number of 1 in dpbd_increase
            //int number_of_ones_increase = manager_before.satisfy_all<std::vector<int>>(1, dpbd_increase).size() / 2;

            // number of 0 in dpbd_increase
            //int number_of_zeros_increase = manager_before.satisfy_all<std::vector<int>>(0, dpbd_increase).size() / 2;

            /*
            double td_for_decrease = (double)number_of_ones_decrease / (number_of_ones_decrease + number_of_zeros_decrease);
            double td_for_increase = (double)number_of_ones_increase / (number_of_ones_increase + number_of_zeros_increase);

            double true_density = td_for_decrease + td_for_increase;

            var td_var;
            td_var.true_density = true_density;
            td_var.variable = i;

            list_for_reordering[i] = td_var;
            */
        //}

        /*

        // Print the unsorted vector
        std::cout << "Before sorting: " << std::endl;
        for (const auto& item : list_for_reordering) {
            std::cout << item.variable << " " << item.true_density << std::endl;
        }

        // sort list of structs based on true density
        std::sort(list_for_reordering.begin(), list_for_reordering.end(), compare_by_true_density);

        // Print the sorted vector
        std::cout << "After sorting: " << std::endl;
        for (const auto& item : list_for_reordering) {
            std::cout << item.variable << " " << item.true_density << std::endl;
        }

        // vector with new order in teddy format 
        std::vector<teddy::int32> order_after = std::vector<teddy::int32>(number_of_vars);

        // moving reordered list to teddy format vector
        for (int i = 0; i < number_of_vars; ++i) {
            order_after[i] = list_for_reordering[i].variable;
        }

        // creating manager with new order of variables based on true density
        teddy::bss_manager manager_after(number_of_vars, 1'000, order_after);
        manager_after.set_auto_reorder(false);
        teddy::bss_manager::diagram_t diagram_after = manager_after.from_pla(*pla, teddy::fold_type::Tree)[0];

        std::ofstream ofst_diag_after("diagram_after.dot");
        manager_after.to_dot_graph(ofst_diag_after, diagram_after);
        // converts .dot file to .png file 
        // .\dot.exe -Tpng diagram_after.dot -o diagram_after.png

        int node_count_after = manager_after.get_node_count(diagram_after);
        std::cout << "Number of nodes in diagram (including terminal nodes) AFTER: " << node_count_after << std::endl;
        std::vector<int> new_order = manager_after.get_order(); // default order
        for (auto o : new_order) {
            std::cout << o << " ";
        }
        std::cout << std::endl;

        */



        // variables with dpbd == 1
        /*
        std::vector<std::vector<int>> vars_with_ones = manager.satisfy_all<std::vector<int>>(1, dpbd).size();
        int number_of_ones_in_dpbd = vars_with_ones.size();
        std::cout << "Number of ones in dpbd: " << number_of_ones_in_dpbd << std::endl;

        for (auto var : vars_with_ones) {
            for (int i = 0; i < number_of_vars; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */

        // variables with dpbd == 0
        /*
        std::vector<std::vector<int>> vars_with_zeros = manager.satisfy_all<std::vector<int>>(0, dpbd);
        int number_of_zeros_in_dpbd = vars_with_zeros.size();
        std::cout << "Number of zeros in dpbd: " << number_of_zeros_in_dpbd << std::endl;

        for (auto var : vars_with_zeros) {
            for (int i = 0; i < number_of_vars; ++i) {
                std::cout << var[i];
            }
            std::cout << " ";
        }
        std::cout << "" << std::endl;
        */
    //}
//}

