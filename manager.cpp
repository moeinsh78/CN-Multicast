#include "manager.hpp"
#include "settings.h"

using namespace std;

Manager::Manager() {
    logged_in_client = "";
    for(list<int> connection: cl_connections) {
        vector<int> new_connection;
        for(int element: connection) {
            new_connection.push_back(element);
        }
        clients_connections.push_back(new_connection);
    }

    for(list<int> connection: ro_connections) {
        vector<int> new_connection;
        for(int element: connection) {
            new_connection.push_back(element);
        }
        routers_connections.push_back(new_connection);
    }
}

void Manager::start() {
    for(int i = 0; i < NUM_OF_CLIENTS; i++) {
        if(is_group_server[i]) {
            create_group_server(i + 1);
        }
        else {
            create_client(i + 1);
        }
    }
    for(int i = 0; i < NUM_OF_ROUTERS; i++) 
        create_router(i + 1, router_ports[i]);
    
    for(int i = 0; i < clients_connections.size(); i++) 
        connect(clients_connections[i][0], clients_connections[i][1], clients_connections[i][2]);
    
    for(int i = 0; i < routers_connections.size(); i++) 
        connect_routers(routers_connections[i][0], routers_connections[i][1], routers_connections[i][2], routers_connections[i][3]);

    return;
}

void Manager::execute_command(string command) {
    stringstream s_stream(command);
    istream_iterator <std::string> begin(s_stream);
    istream_iterator <std::string> end;
    vector <std::string> command_tokens(begin, end);
    if (command_tokens.size() < 1)
        return;
    if (command_tokens[0] == "login") {
        logged_in_client = command_tokens[1];
    }
    else if (command_tokens[0] == "logout") {
        logged_in_client = "";
    }
    else if (command_tokens[0] == "set_multicast_ip") {
        int user_num = stoi(logged_in_client);
        if(is_group_server[user_num - 1])
            set_multicast_ip(user_num, command_tokens[1]);
    }
    else if (command_tokens[0] == "show_group") {
        if(logged_in_client == "") {
            cout << "Need to login to a client!\n";
            return;
        }
        else if(is_group_server[stoi(logged_in_client)]) {
            cout << "Only clients can see groups\n";
            return;
        }
        show_group();
    }
    else if (command_tokens[0] == "join_group") {
        if(logged_in_client == "") {
            cout << "Need to login to a client!\n";
            return;
        }
        else if(is_group_server[stoi(logged_in_client)]) {
            cout << "Only clients can join groups\n";
            return;
        }
        join_group(stoi(logged_in_client), command_tokens[1]);
    }
    else if (command_tokens[0] == "leave_group") {
        if(logged_in_client == "") {
            cout << "Need to login to a client!\n";
            return;
        }
        else if(is_group_server[stoi(logged_in_client)]) {
            cout << "Only clients can join groups\n";
            return;
        }
        leave_group(stoi(logged_in_client), command_tokens[1]);
    }
    else if (command_tokens[0] == "send") {
        string file = command_tokens[1];
        string group = command_tokens[2];
        if(logged_in_client == "") {
            cout << "Need to login to a client!\n";
            return;
        }
        else if(is_group_server[stoi(logged_in_client)]) {
            cout << "Only clients can send files\n";
            return;
        }
        send(stoi(logged_in_client), file, group);
    }
    return;
}

void Manager::show_group() {
    string client_pipe = "./manager_client_" + logged_in_client + ".pipe";
    string message = "SHOW_GROUP ";
    write_on_pipe(client_pipe, message);    
}

void Manager::write_on_pipe(string pipe, string message) {
    int fd = open(pipe.c_str(), O_WRONLY | O_TRUNC);
    write(fd, message.c_str(), message.size() + 1);
    close(fd);
}

void Manager::connect(int client_number, int router_number, int port_num) {
    // Informing client that will be connected to the mentioned router
    string client_pipe = "./manager_client_" + to_string(client_number) + ".pipe";
    string client_reading_pipe = "./router_" + to_string(router_number) + "_port_" + to_string(port_num) + ".pipe";
    string message = "CONNECTED_TO_ROUTER " + client_reading_pipe;
    write_on_pipe(client_pipe, message);
    cout << "Message to " << client_pipe << " : " << message << "\n";
    

    // Informing router that will be connected to the mentioned client
    string router_pipe = "./manager_router_" + to_string(router_number) + ".pipe";
    string router_reading_pipe = "./client_" + to_string(client_number) + "_router_" + to_string(router_number) + "_port_" + to_string(port_num) + ".pipe";
    string message2 = "CONNECTED_TO_CLIENT " + router_reading_pipe;
    write_on_pipe(router_pipe,message2);
    cout << "Message to " << router_pipe << " : " << message2 << "\n";
    return;
}

void Manager::connect_routers(int router1, int port1, int router2, int port2) {
    // Informing first router that will be connected to the second router
    string router_pipe = "./manager_router_" + to_string(router1) + ".pipe";
    string first_router_reading_pipe = "./router_" + to_string(router2) + "_port_" + to_string(port2) + ".pipe";
    string message = "CONNECTED_TO_ROUTER " + first_router_reading_pipe + " WRITE_ON_PORT " + to_string(port1);
    write_on_pipe(router_pipe, message);
    cout << "Message to " << router_pipe << " : " << message << "\n";

    // Informing second router that will be connected to the first router
    string router_pipe2 = "./manager_router_" + to_string(router2) + ".pipe";
    string second_router_reading_pipe = "./router_" + to_string(router1) + "_port_" + to_string(port1) + ".pipe";
    string message2 = "CONNECTED_TO_ROUTER " + second_router_reading_pipe + " WRITE_ON_PORT " + to_string(port2);
    write_on_pipe(router_pipe2, message2);
    cout << "Message to " << router_pipe2 << " : " << message2 << "\n";
    
    return;
}

void Manager::send(int client_num, std::string file, std::string group) {
    string source_pipe = "./manager_client_" + to_string(client_num) + ".pipe";
    string message = "SEND " + file  + " " + group;
    write_on_pipe(source_pipe, message);
    cout << "Message to " << source_pipe << " : " << message << "\n";
    return;
}

void Manager::create_router(int router_num, int num_of_ports) {
    string pipe_name = "./manager_router_" + to_string(router_num) + ".pipe";
    mkfifo(pipe_name.c_str(), 0666);

    pid_t router_pid;
    router_pid = fork();
    if(router_pid == 0) {
        char **argv = new char*[4];
        
        string port_num_str = to_string(num_of_ports);
        char *port_num_ = new char[port_num_str.length() + 1];
        strcpy(port_num_, port_num_str.c_str());
        
        string router_num_str = to_string(router_num);
        char *router_num_ = new char[router_num_str.length() + 1];
        strcpy(router_num_, router_num_str.c_str());
        
        argv[0] = (char*) "./router.out";
        argv[1] = router_num_;
        argv[2] = port_num_;
        argv[3] = NULL;
        
        execv("./router.out", argv);
        exit(0);
    }
    return;
}

void Manager::create_client(int client_num) {
    string pipe_name = "./manager_client_" + to_string(client_num) + ".pipe";
    mkfifo(pipe_name.c_str(), 0666);

    pid_t client_pid;
    client_pid = fork();
    if(client_pid == 0) {
        char **argv = new char*[3];

        string client_num_str = to_string(client_num);
        char *client_num_ = new char[client_num_str.length() + 1];
        strcpy(client_num_, client_num_str.c_str());
        
        argv[0] = (char*) "./client.out";
        argv[1] = client_num_;
        argv[2] = NULL;
        execv("./client.out", argv);
        exit(0);
    }
    return;
}

void Manager::create_group_server(int server_num) {
    string pipe_name = "./manager_client_" + to_string(server_num) + ".pipe";
    mkfifo(pipe_name.c_str(), 0666);
    
    pid_t server_pid;
    server_pid = fork();
    if(server_pid == 0) {
        char **argv = new char*[3];

        string server_num_str = to_string(server_num);
        char *server_num_ = new char[server_num_str.length() + 1];
        strcpy(server_num_, server_num_str.c_str());
        
        argv[0] = (char*) "./group_server.out";
        argv[1] = server_num_;
        argv[2] = NULL;
        execv("./group_server.out", argv);
        exit(0);
    }
    return;
}


void Manager::set_multicast_ip(int group_server_num, string multicast_ip) {
    string pipe_name = "./manager_client_" + to_string(group_server_num) + ".pipe";
    string message = "MULTICAST_IP " + multicast_ip;
    cout << message << "  ------ " << pipe_name <<endl;
    write_on_pipe(pipe_name, message);
    return;   
}

void Manager::join_group(int client_num, string group_ip) {
    string pipe_name = "./manager_client_" + to_string(client_num) + ".pipe";
    string message = "JOIN_GROUP " + group_ip;
    cout << message << "  ------ " << pipe_name <<endl;
    write_on_pipe(pipe_name, message);
    return;
}
void Manager::leave_group(int client_num, string group_ip) {
    string pipe_name = "./manager_client_" + to_string(client_num) + ".pipe";
    string message = "LEAVE_GROUP " + group_ip;
    cout << message << "  ------ " << pipe_name <<endl;
    write_on_pipe(pipe_name, message);
    return;
}