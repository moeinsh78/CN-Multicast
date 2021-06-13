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

#define INF 10000

class Manager {
public:
    Manager();
    void start();
    void execute_command(std::string);
    void create_router(int router_num, int num_of_ports);
    void create_client(int client_num);
    void create_group_server(int server_num);
    void set_multicast_ip(int group_server_num, std::string multicast_ip);
    void connect(int client_number, int router_number, int port);
    void connect_routers(int router1, int port1, int router2, int port2);
    void write_on_pipe(std::string client_pipe, std::string message);
    void send(int client_num, std::string file, std::string group);
    void show_group();
    void join_group(int client_num, std::string group_ip);
    void leave_group(int client_num, std::string group_ip);
private:
    // std::vector<int> router_ports;
    // std::vector<std::string> clients_ip;
    // std::vector<std::string> routers_ip;
    std::string logged_in_client;

    std::vector< std::vector<int> > clients_connections;
    std::vector< std::vector<int> > routers_connections;
};

#endif