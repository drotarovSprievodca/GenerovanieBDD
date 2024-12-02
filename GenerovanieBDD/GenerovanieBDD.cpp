﻿#pragma once

#define NDEBUG

#include <libteddy/core.hpp>
#include <vector>
#include <libteddy/reliability.hpp>

#include "BDDStatisticsGenerator.hpp"

// spytat sa ci sa da TD pre vseobecnu derivaciu vypocitat rychlejsie
// objektovo prerobit aj UML-ko tried a vztahov a atributy tried
// priemerny pocet nodov pre subor
// priemerny pocet nodov pre funkciu
// priemerny pocet nodov celkovo
// skusit vsetky subory s vacsou RAM neskor
// skusit aj preusporiadavaciu heuristiku
// 4 experimenty aj so zapnutou heuristikou -> 8 experimentov
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

int main() {
    std::string working_directory = "C:\\Users\\DELL\\git\\Diplomka\\GenerovanieBDD";
    int number_of_replications = 10;
    std::string pla_files_directory = "\\FAST_WORKING_PLA\\";
    BDDStatisticsGenerator* bddStatisticsGenerator = new BDDStatisticsGenerator(working_directory, pla_files_directory);
    bddStatisticsGenerator->set_strategy({  
                                            new OriginalOrder(false),
                                            new OriginalOrder(true),
                                            new DerivativesBasedOrder(false, true),
                                            new DerivativesBasedOrder(true, true),
                                            new DerivativesBasedOrder(false, false),
                                            new DerivativesBasedOrder(true, false), 
                                            new RandomOrder(false, number_of_replications),
                                            new RandomOrder(true, number_of_replications) 
                                        });
    bddStatisticsGenerator->get_statistics();
    delete bddStatisticsGenerator;
    return 0;
}
