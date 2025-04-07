#pragma once

#define NDEBUG

#include <libteddy/core.hpp>
#include <vector>
#include <libteddy/reliability.hpp>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <fstream>

#include "json.hpp"
#include "BDDStatisticsGenerator.hpp"


int main() {

    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        std::cerr << "Could not open config file!" << std::endl;
        return 1;
    }

    nlohmann::json config;
    config_file >> config;
    config_file.close();

    std::string working_directory = config.value("working_directory", "");
    std::string csv_output_directory = config.value("csv_output_directory", "");
    std::string pla_files_directory = config.value("pla_files_directory", "");
    // true for one csv per pla file from pla_files_directory
    // false for one big csv from all files from pla_files_directory
    bool csv_for_every_pla = config.value("csv_for_every_pla", false);
    int number_of_replications = config.value("number_of_replications", 100);

    BDDStatisticsGenerator* bddStatisticsGenerator = new BDDStatisticsGenerator(working_directory, pla_files_directory, csv_output_directory, csv_for_every_pla);
    
    bool exit_still_not_selected = true;
    while (exit_still_not_selected) {

        std::cout << "Enter number of what should be done, you have these options:" << std::endl;
        std::cout << "1 - Print out currently set strategies" << std::endl;
        std::cout << "2 - Add strategy" << std::endl;
        std::cout << "3 - Delete strategy" << std::endl;
        std::cout << "4 - Run strategies" << std::endl;
        std::cout << "5 - exit" << std::endl;

        std::string chosen_option;
        int num_option = 5;
        std::getline(std::cin, chosen_option);
        try {
            num_option = std::stoi(chosen_option);
        }
        catch (const std::exception& e) {
            std::cout << "Enter valid option!!!" << std::endl;
            break;
        }
        if (num_option < 1 || num_option > 5) {
            std::cout << "Enter valid option!!!" << std::endl;
            break;
        }

        switch (num_option) {
            case 1:
                bddStatisticsGenerator->print_strategies();
            break;
            case 2: {
                std::cout << "Which strategy should be added? Enter number of option." << std::endl;
                std::cout << "1 - OriginalOrder" << std::endl;
                std::cout << "2 - DerivativesBasedOrder" << std::endl;
                std::cout << "3 - RandomOrder" << std::endl;
                std::cout << "4 - EntropyBasedOrder" << std::endl;
                std::cout << "5 - EntropyBasedOrderDT" << std::endl;
                std::cout << "6 - DerivativesBasedOrderDT" << std::endl;
                std::cout << "7 - DerivativesBasedHigherOrder" << std::endl;
                std::cout << "8 - BestOrder" << std::endl;
                std::string for_add;
                int num_for_add;
                std::getline(std::cin, for_add);
                try {
                    num_for_add = std::stoi(for_add);
                }
                catch (const std::exception& e) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }
                if (num_for_add < 1 || num_for_add > 8) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                } 
                
                int num_for_sift = 2;
                if (num_for_add != 8) {
                    std::cout << "Should it use dynamic sift variable reordering heuristic" << std::endl;
                    std::cout << "on BDD after static order? Enter number of option." << std::endl;
                    std::cout << "1 - YES" << std::endl;
                    std::cout << "2 - NO" << std::endl;
                    std::string for_sift;
                    std::getline(std::cin, for_sift);
                    try {
                        num_for_sift = std::stoi(for_sift);
                    }
                    catch (const std::exception& e) {
                        std::cout << "Enter valid option!!!" << std::endl;
                        break;
                    }
                    if (num_for_sift < 1 || num_for_sift > 2) {
                        std::cout << "Enter valid option!!!" << std::endl;
                        break;
                    }
                }

                std::cout << "Should it generate diagram before reordering? Enter number of option." << std::endl;
                std::cout << "1 - YES" << std::endl;
                std::cout << "2 - NO" << std::endl;
                std::string for_diagram_before;
                int num_for_diagram_before;
                std::getline(std::cin, for_diagram_before);
                try {
                    num_for_diagram_before = std::stoi(for_diagram_before);
                }
                catch (const std::exception& e) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }
                if (num_for_diagram_before < 1 || num_for_diagram_before > 2) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }

                std::cout << "Should it generate diagram after reordering? Enter number of option." << std::endl;
                std::cout << "1 - YES" << std::endl;
                std::cout << "2 - NO" << std::endl;
                std::string for_diagram_after;
                int num_for_diagram_after;
                std::getline(std::cin, for_diagram_after);
                try {
                    num_for_diagram_after = std::stoi(for_diagram_after);
                }
                catch (const std::exception& e) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }
                if (num_for_diagram_after < 1 || num_for_diagram_after > 2) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }

                int num_for_TD = 2;
                if (num_for_add == 2 || num_for_add == 6 || num_for_add == 7) {
                    std::cout << "Should the truth density be ordered in ascending or descending order? Enter number of option." << std::endl;
                    std::cout << "1 - Ascending" << std::endl;
                    std::cout << "2 - Descending" << std::endl;
                    std::string for_TD;
                    std::getline(std::cin, for_TD);
                    try {
                        num_for_TD = std::stoi(for_TD);
                    }
                    catch (const std::exception& e) {
                        std::cout << "Enter valid option!!!" << std::endl;
                        break;
                    }
                    if (num_for_TD < 1 || num_for_TD > 2) {
                        std::cout << "Enter valid option!!!" << std::endl;
                        break;
                    }
                }

                bool use_var_reordering_heuristsics = false;
                if (num_for_sift == 1) {
                    use_var_reordering_heuristsics = true;
                }
                bool ascending = false;
                if (num_for_TD == 1) {
                    ascending = true;
                }
                bool generate_graph_before_order = false;
                if (num_for_diagram_before == 1) {
                    generate_graph_before_order = true;
                }
                bool generate_graph_after_order = false;
                if (num_for_diagram_after == 1) {
                    generate_graph_after_order = true;
                }

                switch (num_for_add) {
                    case 1:
                        bddStatisticsGenerator->add_strategy(new OriginalOrder(use_var_reordering_heuristsics, generate_graph_before_order, generate_graph_after_order));
                        break;
                    case 2:
                        bddStatisticsGenerator->add_strategy(new DerivativesBasedOrder(use_var_reordering_heuristsics, ascending, generate_graph_before_order, generate_graph_after_order));
                        break;
                    case 3:
                        bddStatisticsGenerator->add_strategy(new RandomOrder(use_var_reordering_heuristsics, number_of_replications));
                        break;
                    case 4:
                        bddStatisticsGenerator->add_strategy(new EntropyBasedOrder(use_var_reordering_heuristsics, generate_graph_before_order, generate_graph_after_order));
                        break;
                    case 5:
                        bddStatisticsGenerator->add_strategy(new EntropyBasedOrderDT(use_var_reordering_heuristsics, generate_graph_before_order, generate_graph_after_order));
                        break;
                    case 6:
                        bddStatisticsGenerator->add_strategy(new DerivativesBasedOrderDT(use_var_reordering_heuristsics, ascending, generate_graph_before_order, generate_graph_after_order));
                        break;
                    case 7:
                        bddStatisticsGenerator->add_strategy(new DerivativesBasedHigherOrder(use_var_reordering_heuristsics, ascending, generate_graph_before_order, generate_graph_after_order));
                        break;
                    case 8:
                        bddStatisticsGenerator->add_strategy(new BestOrder(generate_graph_before_order, generate_graph_after_order));
                        break;
                }
                break;
            }
            case 3: {
                if (bddStatisticsGenerator->size_of_strategies() == 0) {
                    std::cout << "There are no strategies set so there can be no deleting !!!" << std::endl;
                    break;
                }
                std::cout << "Which strategy should be deleted? Enter number of option." << std::endl;
                bddStatisticsGenerator->print_strategies();
                std::string for_delete;
                int num_for_delete;
                std::getline(std::cin, for_delete);
                try {
                    num_for_delete = std::stoi(for_delete);
                }
                catch (const std::exception& e) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }
                if (num_for_delete < 0 || num_for_delete >= bddStatisticsGenerator->size_of_strategies()) {
                    std::cout << "Enter valid option!!!" << std::endl;
                    break;
                }

                bddStatisticsGenerator->remove_strategy(num_for_delete);
                break;
            }
            case 4: {
                if (bddStatisticsGenerator->size_of_strategies() > 0) {
                    auto start = std::chrono::high_resolution_clock::now();
                    bddStatisticsGenerator->get_statistics();
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> duration = end - start;
                    std::cout << "Execution time: " << duration.count() << " seconds." << std::endl;
                    exit_still_not_selected = false;
                }
                break;
            }
            case 5: {
                exit_still_not_selected = false;
                break;
            }
        }
    }

    /*
    bddStatisticsGenerator->set_strategy({  
                                            new OriginalOrder(false, true, true),
                                            new OriginalOrder(true, false, false),
                                            new DerivativesBasedOrder(false, false, false, false),
                                            new DerivativesBasedOrder(false, true, false, false),
                                            new DerivativesBasedOrder(true, false, false, false),
                                            new DerivativesBasedOrder(true, true, false, false),
                                            new RandomOrder(false, number_of_replications, false, false),
                                            new RandomOrder(true, number_of_replications, false, false),
                                            new EntropyBasedOrder(false, false, false),
                                            new EntropyBasedOrder(true, false, false),
                                            new EntropyBasedOrderDT(false, false, false),
                                            new EntropyBasedOrderDT(true, false, false),
                                            new DerivativesBasedOrderDT(false, false, false, false),
                                            new DerivativesBasedOrderDT(false, true, false, false),
                                            new DerivativesBasedOrderDT(true, false, false, false),
                                            new DerivativesBasedOrderDT(true, true, false, false),
                                            new DerivativesBasedHigherOrder(false, false, false, false),
                                            new DerivativesBasedHigherOrder(false, true, false, false),
                                            new DerivativesBasedHigherOrder(true, false, false, false),
                                            new DerivativesBasedHigherOrder(true, true, false, false),
                                            new BestOrder(false, false)  
                                        });
    */
    delete bddStatisticsGenerator;
    std::cin.get();
    return 0;
}
