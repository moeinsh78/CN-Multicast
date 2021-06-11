#include <iostream>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>

#include "manager.hpp"

using namespace std;

int main() {
    string command;
    Manager network_manager = Manager();
    network_manager.start();
    while(getline(cin, command)) 
        network_manager.execute_command(command);
    return 0;
}