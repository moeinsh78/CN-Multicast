#include "manager.hpp"
using namespace std;

Manager::Manager() {
    for(int ports_num: router_ports) 
        router_ports.push_back(ports_num);

    for(string ip: client_ip) 
        clients_ip.push_back(ip);

    for(string ip: router_ip) 
        routers_ip.push_back(ip);
    
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
    for(int i = 0; i < NUM_OF_CLIENTS; i++) 
        create_client(i + 1);

    for(int i = 0; i < NUM_OF_ROUTERS; i++) 
        create_router(i + 1, router_ports[i]);
    
    for(int i = 0; i < clients_connections.size(); i++) 
        connect(clients_connections[i][0], clients_connections[i][1], clients_connections[i][2]);
    
    for(int i = 0; i < routers_connections.size(); i++) 
        connect_routers(routers_connections[i][0], routers_connections[i][1], routers_connections[i][2], routers_connections[i][3]);

    return;
}

// void Manager::execute_command(string command) {
//     stringstream s_stream(command);
//     istream_iterator <std::string> begin(s_stream);
//     istream_iterator <std::string> end;
//     vector <std::string> command_tokens(begin, end);
//     if (command_tokens.size() < 1)
//         return;
//     if (command_tokens[0] == "myrouter") {
//         if(!check_arguments_num(command_tokens.size(), 2))
//             return;

//         int router_number = stoi(command_tokens[1]);
//         int number_of_ports = stoi(command_tokens[2]);
//         if(find_router(router_number) != -1) {
//             cout << "client with this number already exists!\n";
//             return;
//         }
//         if (number_of_ports < 0) {
//             cout << "Unacceptable number of ports!\n";
//             return;
//         }
//         create_router(router_number, number_of_ports);
//     }
//     else if (command_tokens[0] == "myclient") {
//         if(!check_arguments_num(command_tokens.size(), 1))
//             return;
            
//         int new_client_number = stoi(command_tokens[1]);
//         if(find_client(new_client_number)) {
//             cout << "client with this number already exists!\n";
//             return;
//         }
//         create_client(new_client_number);
//     }
//     else if (command_tokens[0] == "connect") {
//         if(!check_arguments_num(command_tokens.size(), 3))
//             return;

//         int client_num = stoi(command_tokens[1]);
//         int router_num = stoi(command_tokens[2]);
//         int port_num = stoi(command_tokens[3]);
//         if(!find_client(client_num)) {
//             cout << "No clients found with the entered number!\n";
//             return;
//         }
//         int ports_count = find_router(router_num);
//         if(ports_count == -1) {
//             cout << "No routers found with the entered number!\n";
//             return;
//         }
//         else if(ports_count < port_num || port_num < 1) {
//             cout << "Port number out of range!\n";
//             return;
//         }
        
//         if (!(check_port_availability(router_num, port_num)))
//             return;
//         connect(client_num, router_num, port_num);
//     }
//     else if(command_tokens[0] == "connectrouter") {
//         if(!check_arguments_num(command_tokens.size(), 4))
//             return;

//         int router1_num = stoi(command_tokens[1]);
//         int router2_num = stoi(command_tokens[3]);
//         int port1_num = stoi(command_tokens[2]);
//         int port2_num = stoi(command_tokens[4]);
//         int first_ports_count = find_router(router1_num);
//         int second_ports_count = find_router(router2_num);
        
//         if(first_ports_count == -1 || second_ports_count == -1) {
//             cout << "No routers found with the entered number!\n";
//             return;
//         }
//         else if(first_ports_count < port1_num || port1_num < 1 || second_ports_count < port2_num || port2_num < 1) {
//             cout << "Port number out of range!\n";
//             return;
//         }
//         if (!(check_port_availability(router1_num, port1_num) && check_port_availability(router2_num, port2_num)))
//             return;
        
//         connect_routers(router1_num, port1_num, router2_num, port2_num);
//     }
//     else if (command_tokens[0] == "send") {
//         // file, source, destination
//         if(!check_arguments_num(command_tokens.size(), 3))
//             return;
        
//         int source = stoi(command_tokens[2]);
//         int destination = stoi(command_tokens[3]);

//         if(!(find_client(source) && find_client(destination))) {
//             cout << "No clients found with the entered number!\n";
//             return;
//         }
//         send(command_tokens[1], command_tokens[2], command_tokens[3]);
//     }
//     else if (command_tokens[0] == "receive") {
//         if(!check_arguments_num(command_tokens.size(), 3))
//             return;
        
//         int destination = stoi(command_tokens[1]);
//         int source = stoi(command_tokens[3]);

//         if(!(find_client(source) && find_client(destination))) {
//             cout << "No clients found with the entered number!\n";
//             return;
//         }
//         receive(command_tokens[1], command_tokens[2], command_tokens[3]);
//     }
//     else if (command_tokens[0] == "spanningtree") {
//         make_spanning_tree();
//     }
//     else {
//         cout << "Command not found!\n";
//         return;
//     }
//     return;
// }


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
    vector<int> new_busy_port = {router_number, port_num};
    connected_ports.push_back(new_busy_port);
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
    write_on_pipe(router_pipe2,message2);
    cout << "Message to " << router_pipe2 << " : " << message2 << "\n";
    
    vector<int> first_busy_port = {router1, port1};
    vector<int> second_busy_port = {router2, port2};
    connected_ports.push_back(first_busy_port);
    connected_ports.push_back(second_busy_port);
    return;
}

void Manager::send(string file_path, string source, string destination) {
    string source_pipe = "./manager_client_" + source + ".pipe";
    string message = "SEND " + file_path  + " " + destination;
    write_on_pipe(source_pipe, message);
    cout << "Message to " << source_pipe << " : " << message << "\n";
    return;
}

void Manager::receive(string destination, string file, string source) {
    string source_pipe = "./manager_client_" + destination + ".pipe";
    string message = "RECEIVE " + file  + " " + source;
    write_on_pipe(source_pipe, message);
    cout << "Message to " << source_pipe << " : " << message << "\n";
    return;

}
void Manager::create_router(int router_num, int num_of_ports) {
    Router new_router_info = Router();

    string pipe_name = "./manager_router_" + to_string(router_num) + ".pipe";
    mkfifo(pipe_name.c_str(), 0666);

    new_router_info.number_of_ports = num_of_ports;
    new_router_info.router_number = router_num;
    new_router_info.key_value = INF;

    routers.push_back(new_router_info);
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
        
    clients.push_back(client_num);

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