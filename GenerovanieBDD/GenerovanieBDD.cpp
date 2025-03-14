#pragma once

#define NDEBUG

#include <libteddy/core.hpp>
#include <vector>
#include <libteddy/reliability.hpp>

#include "BDDStatisticsGenerator.hpp"

// spytat sa ci sa da TD pre vseobecnu derivaciu vypocitat rychlejsie
// - objektovo prerobit aj UML-ko tried a vztahov a atributy tried
// priemerny pocet nodov pre subor
// priemerny pocet nodov pre funkciu
// priemerny pocet nodov celkovo
// skusit vsetky subory s vacsou RAM neskor
// - skusit aj preusporiadavaciu heuristiku
// - 4 experimenty aj so zapnutou heuristikou -> 8 experimentov
// generovanie csv pre vacsi prehlad
// pre kazdu funkciu len pocet nodov
// priemer pre vsetky funkcie v subore, pripadne pre vsetky subory
// dat na vyber:
// a) pre kazde pla jedno csv
// b) pre vsetky pla jedno csv -> riadok charakterizovany aj nazvom suboru
// -> riadky == funkcie
// -> stlpce: 
// 1. stpec -> ktora funkcia
// 2. stlpec -> pocet premennych
// 3. ... n stlpec -> tych 8 merani -> pre kazde aj poradie premennych
// v buducnosti usporiadanie pomocou entropie



// rastuca a klesajuca entropia
// spolocna (vzajomna) informacia medzi premmennou a funkciou
// ako mi klesne entropia na funckii ak budem poznat konkretnu prememnu
// 
// 1. podobne der
// H(f) za podmienky ze som poznal hodnotu iba jednej premennej (a)
// H(f) za podmienky ze som poznal hodnotu iba jednej premennej (b)
// len o tom co dam do jeho korena
// 
// 2.
// H(f) za predpokladu ze poznam hodnoty viacerych premennych (pri b poznam uz a)
// na kazdej urovni sa pomocou tej istej premennej usporiaduvalo
// pomocou entropie sa urcovalo ako sa usporiadaju premenne na kazdej urovni
// od najvacsej a od najmensej
// ID3, C45
// usporiadany rozhodovaci strom
// ODT
//
// a experimenty
// stale teddy kniznicu na budovanie


int main() {
    std::string working_directory = "C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD";
    int number_of_replications = 10;
    // true for one csv per pla file from pla_files_directory
    // false for one big csv from all files from pla_files_directory
    bool csv_for_every_pla = false;
    std::string csv_output_directory = "\\csv_output\\";
    std::string pla_files_directory = "\\FAST_WORKING_PLA\\";
    
    BDDStatisticsGenerator* bddStatisticsGenerator = new BDDStatisticsGenerator(working_directory, pla_files_directory, csv_output_directory, csv_for_every_pla);
    bddStatisticsGenerator->set_strategy({/*
                                            new OriginalOrder(false),
                                            new OriginalOrder(true),
                                            new DerivativesBasedOrder(false, false),
                                            new DerivativesBasedOrder(false, true),
                                            new DerivativesBasedOrder(true, false),
                                            new DerivativesBasedOrder(true, true),
                                            new RandomOrder(false, number_of_replications),
                                            new RandomOrder(true, number_of_replications),
                                            new EntropyBasedOrder(false),
                                            new EntropyBasedOrder(true),
                                            new EntropyBasedOrderDT(false),
                                            new EntropyBasedOrderDT(true),
                                            new DerivativesBasedOrderDT(false, false),
                                            new DerivativesBasedOrderDT(false, true),
                                            new DerivativesBasedOrderDT(true, false),
                                            new DerivativesBasedOrderDT(true, true),*/
                                            new DerivativesBasedHigherOrder(false, false),
                                            new DerivativesBasedHigherOrder(false, true),
                                            new DerivativesBasedHigherOrder(true, false),
                                            new DerivativesBasedHigherOrder(true, true)
                                        });
    bddStatisticsGenerator->get_statistics();
    delete bddStatisticsGenerator;
    return 0;
}
