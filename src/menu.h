/**
* @file menu.h
 * @brief Interface de menu
 */

#ifndef DA_SECOND_PROJECT_MENU_H
#define DA_SECOND_PROJECT_MENU_H

#include "parser.h"
#include <string>

class Menu {
public:
    static int executeBatch(const std::string& rangesFile, const std::string& registersFile, const std::string& outputFile);

    static int executeMenu();

};

#endif /* DA_SECOND_PROJECT_MENU_H */