#ifndef _MANAGER_
#define _MANAGER_

#include <iostream>
#include <vector>
#include <string.h>
#include <fstream>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include <fcntl.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/wait.h> 
#include <list> 

#include "settings.h"

#define INF 10000

typedef struct 
{
    int key_value;
    int router_number;
    int number_of_ports;
} Router;

class Manager {
public:
    Manager();
    void start();
    void execute_command(std::string);
    void create_router(int router_num, int num_of_ports);
    void create_client(int client_num);
    void connect(int client_number, int router_number, int port);
    void connect_routers(int router1, int port1, int router2, int port2);
    void write_on_pipe(std::string client_pipe, std::string message);
    void send(std::string file_path, std::string source, std::string destination);
    void receive(std::string destination, std::string file, std::string source);
private:
    std::vector<int> router_ports;
    std::vector<std::string> clients_ip;
    std::vector<std::string> routers_ip;
    std::vector< std::vector<int> > clients_connections;
    std::vector< std::vector<int> > routers_connections;
    
    std::vector<Router> routers;    
    std::vector<int> clients;
    std::vector< std::vector<int> > connected_ports;
};

#endif