#pragma once

#define NDEBUG

#include <libteddy/core.hpp>
#include <vector>
#include <libteddy/reliability.hpp>
#include <chrono>

#include "BDDStatisticsGenerator.hpp"


int main() {
    std::string working_directory = "C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD";
    int number_of_replications = 10;
    // true for one csv per pla file from pla_files_directory
    // false for one big csv from all files from pla_files_directory
    bool csv_for_every_pla = false;
    std::string csv_output_directory = "\\csv_output\\";
    std::string pla_files_directory = "\\TESTING\\";
    
    BDDStatisticsGenerator* bddStatisticsGenerator = new BDDStatisticsGenerator(working_directory, pla_files_directory, csv_output_directory, csv_for_every_pla);
    

    bddStatisticsGenerator->set_strategy({  new BestOrder(false, false)
                                            /*
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
                                            new DerivativesBasedHigherOrder(true, true, false, false)
                                            */
                                        });

    auto start = std::chrono::high_resolution_clock::now();
    
    bddStatisticsGenerator->get_statistics();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Execution time: " << duration.count() << " seconds." << std::endl;

    delete bddStatisticsGenerator;
    return 0;
}
