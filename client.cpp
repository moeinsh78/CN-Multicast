#include <iostream>
#include <dirent.h>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h> 
#include <iterator>
#include <sstream>

using namespace std;

string read_message_from_pipe(int pipe) { 
    char msg[1000];
    read(pipe, msg, 1000);
    string message(msg);
    return message;
}
vector<string> tokenize(string message) {
    stringstream s_stream(message);
    istream_iterator <string> begin(s_stream);
    istream_iterator <string> end;
    vector <string> command_tokens(begin, end);
    return command_tokens;    
}
vector<string> make_packets(string file, string destination) {
    int pipe = open(file.c_str(), O_RDONLY | O_NONBLOCK);
    char temp[100000];
    read(pipe, temp, 100000);
    string msg(temp);
    vector<string> packets;
    while(1) {
        string packet;
        packet = "GROUP_PACKET " + destination + " ";
        if(msg.size() <= 50) { 
            packet+=msg;
            packet += " 1";
            packets.push_back(packet);
            return packets;
        }
        for(string::size_type i = 0; i < 50; ++i) {
            packet += msg[i];
        }
        packet += " 0";
        packets.push_back(packet);
        msg.erase(0,50);
    }
}
void write_on_pipe(string pipe, string message) {
    int fd = open(pipe.c_str(), O_TRUNC | O_WRONLY );
    write(fd, message.c_str(), message.size()+ 1);
    close(fd);
    return;
}
int main(int argc, char **argv) {
    vector<string> pipe_file_names;
    string client_num(argv[1]);
    string manager_pipe = "./manager_client_" + client_num + ".pipe";

    pipe_file_names.push_back(manager_pipe);
    cout << "New client process created -- Num: " << client_num << endl;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    string client_reading_pipe;
    string client_writing_pipe;

    fd_set rfds;
    int max_fd;
    int output_num = 1;

    vector<string> available_groups;
    vector<string> joined_groups;
    while (1) {
        FD_ZERO(&rfds);
        int router_p; 
        int manager = open(manager_pipe.c_str(), O_RDONLY | O_NONBLOCK);
        FD_SET(manager, &rfds);
        max_fd = manager;

        if(client_reading_pipe != "") {
            router_p = open(client_reading_pipe.c_str(), O_RDONLY | O_NONBLOCK);
            FD_SET(router_p, &rfds);
            max_fd = router_p;
        }

        if (!select(max_fd + 1, &rfds, NULL, NULL, &tv)) {
            if(client_reading_pipe != "") {
                close(max_fd);
            }
            close(manager);
            continue;
        }

        if(FD_ISSET(manager, &rfds)) {
            string message = read_message_from_pipe(manager);
            vector <string> command_tokens = tokenize(message);
            if (command_tokens[0] == "CONNECTED_TO_ROUTER") {
                client_reading_pipe = command_tokens[1];
                mkfifo(client_reading_pipe.c_str(), 0666);
                cout << "Pipe created with name: " << client_reading_pipe << endl;
                client_writing_pipe = "./client_" + client_num + "_" + client_reading_pipe.erase(0,2) ;
            }
            else if(command_tokens[0] == "SEND") {
                string group = command_tokens[2];
                string file = command_tokens[1];
                vector<string> packets = make_packets(file, group);
                for(int i = 0; i < packets.size(); i++) {
                    write_on_pipe(client_writing_pipe,packets[i]);
                    sleep(2);
                }
                cout << "client " << client_num << " sent all the packets to group " << group << endl;
            }
            else if(command_tokens[0] == "SHOW_GROUP") {
                for(int i = 0; i<available_groups.size() ; i++) cout << available_groups[i] << endl;
            }
            else if (command_tokens[0] == "JOIN_GROUP") {
                string group_ip = command_tokens[1];
                string join_request = "REQUEST_JOIN_GROUP " + client_num + " GROUP_IP " + group_ip + " ";
                joined_groups.push_back(group_ip);
                while(join_request.size() < 50)
                    join_request += "x";
                write_on_pipe(client_writing_pipe, join_request);
                cout << "Client " << client_num << " requested to joined group " << group_ip << "\n";
            }
            else if (command_tokens[0] == "LEAVE_GROUP") {
                string group_ip = command_tokens[1];
                string leave_request = "REQUEST_LEAVE_GROUP " + client_num + " GROUP_IP " + group_ip + " ";
            
                for(int j = 0; j < joined_groups.size(); j++) {
                    if(joined_groups[j] == group_ip) {
                        joined_groups.erase(joined_groups.begin() + j);
                        break;
                    }
                }
                while(leave_request.size() < 50)
                    leave_request += "x";
                write_on_pipe(client_writing_pipe, leave_request);
                cout << "Client " << client_num << " requested to leave group " << group_ip << "\n";
            }
            close(manager);
        }
        else if(FD_ISSET(router_p, &rfds)) {
            string message = read_message_from_pipe(router_p);
            vector <string> command_tokens = tokenize(message);
            string msg_des = command_tokens[1];
            if(msg_des != client_num) 
                continue;
            string file_name = "./client_"+ client_num + "output" + to_string(output_num) + ".txt";
            fstream output;
            string multicast_ip, server_ip;
            if(message[message.size() - 1] == '1') {
                output.open(file_name,ios::app);
                message.erase(0,4);
                message.erase(message.size() - 2,2);
                output.write((char*) message.c_str(),message.size());
                output.close();
                cout << "client " << client_num << " received all the packets from system " << command_tokens[0] << endl;
                output_num++;
            }
            if(message[message.size() - 1] == '0') {
                output.open(file_name,ios::app);
                message.erase(0,4);
                message.erase(message.size() - 2,2);
                output.write((char*) message.c_str(),message.size());
                output.close();
            }
            if(command_tokens[0] == "BROADCAST_GROUP") {
                multicast_ip = command_tokens[1];
                server_ip = command_tokens[2];
                available_groups.push_back(multicast_ip);
            }
            close(router_p);
        }
        
    }
    return 0;
}