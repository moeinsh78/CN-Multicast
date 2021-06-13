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
vector<string> make_packets(string file, string source, string destination) {
    int pipe = open(file.c_str(), O_RDONLY | O_NONBLOCK);
    char temp[100000];
    read(pipe, temp, 100000);
    string msg(temp);
    vector<string> packets;
    while(1) {
        string packet;
        packet = source + " " + destination + " ";
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

void broadcast_new_group(string multicast_group_ip, string group_server_ip, string writing_pipe_name) {
    string packet = "BROADCAST_GROUP " + multicast_group_ip + " " + group_server_ip + " ";
    while(packet.size() < 50)
        packet += "x";
    cout << packet << " ---- yaaaaay --- "  << writing_pipe_name << endl;
    write_on_pipe(writing_pipe_name, packet);
}


int main(int argc, char **argv) {
    string group_multicast_ip = "";
    vector<string> pipe_file_names;
    string group_server_ip(argv[1]);
    string manager_pipe = "./manager_client_" + group_server_ip + ".pipe";

    pipe_file_names.push_back(manager_pipe);
    cout << "New group server process created -- Num: " << group_server_ip << endl;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    string client_reading_pipe;
    string client_writing_pipe;

    fd_set rfds;
    int max_fd;
    int output_num = 1;
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
                client_writing_pipe = "./client_" + group_server_ip + "_" + client_reading_pipe.erase(0,2) ;
            }
            else if(command_tokens[0] == "MULTICAST_IP") {
                group_multicast_ip = command_tokens[1];
                broadcast_new_group(group_multicast_ip, group_server_ip, client_writing_pipe);
            }
            // else if(command_tokens[0] == "SEND") {
            //     string destination = command_tokens[2];
            //     string file = command_tokens[1];
            //     vector<string> packets = make_packets(file, group_server_num, destination);
            //     for(int i = 0; i < packets.size(); i++) {
            //         write_on_pipe(client_writing_pipe,packets[i]);
            //         sleep(2);
            //     }
            //     cout << "group_server " << group_server_num << " sent all the packets to system " << destination << endl;
            // }
            // else if(command_tokens[0] == "RECEIVE") {
            //     string packet =  group_server_num + " " + command_tokens[2] + " "; 
            //     if(command_tokens[1].size() < 50) {
            //         packet+=command_tokens[1] + " ";
            //         for(int i = command_tokens[1].size(); i <= 49; i++) packet+='x';
            //     }
            //     else if(command_tokens[1].size() > 50) {
            //         for(int i = 0; i < 50; i++) packet+=command_tokens[1][i];
            //     }
            //     packet += " x";
            //     write_on_pipe(client_writing_pipe, packet);
            //     cout << "Message to " << client_writing_pipe << " : " << packet << "\n";
            // }
            close(manager);
        }
        else if(FD_ISSET(router_p, &rfds)) {
            string message = read_message_from_pipe(router_p);
            vector <string> command_tokens = tokenize(message);
            string msg_des = command_tokens[1];
            if(msg_des != group_server_ip) 
                continue;
            string file_name = "./client_"+ group_server_ip + "output" + to_string(output_num) + ".txt";
            fstream output;
            if(message[message.size() - 1] == '1') {
                output.open(file_name,ios::app);
                message.erase(0,4);
                message.erase(message.size() - 2,2);
                output.write((char*) message.c_str(),message.size());
                output.close();
                cout << "client " << group_server_ip << " received all the packets from system " << command_tokens[0] << endl;
                output_num++;
            }
            if(message[message.size() - 1] == '0') {
                output.open(file_name,ios::app);
                message.erase(0,4);
                message.erase(message.size() - 2,2);
                output.write((char*) message.c_str(),message.size());
                output.close();
            }
            /*if(message[message.size() - 1] == 'x') {
                string destination = command_tokens[0];
                string file = command_tokens[2];
                vector<string> packets = make_packets(file, group_server_ip, destination);
                for(int i = 0; i < packets.size(); i++) {
                    write_on_pipe(client_writing_pipe,packets[i]);
                    sleep(2);
                }
                cout << "client " << group_server_ip << " sent all the packets to system " << destination << endl;
            }*/
            close(router_p);
        }
        
    }
    return 0;
}