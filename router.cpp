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
#include <sys/time.h>
#include <sys/msg.h>
#include "settings.h"

#define MAX_ROWS 256

using namespace std;

string read_message_from_pipe(int pipe) { 
    char msg[1000];
    read(pipe, msg, 1000);
    string message(msg);
    return message;
}

void write_on_pipe(string pipe, string message) {
    int fd = open(pipe.c_str(), O_TRUNC | O_WRONLY );
    //sleep(2);
    write(fd, message.c_str(), message.size()+ 1);
    close(fd);
}

vector<string> tokenize(string message) {
    stringstream s_stream(message);
    istream_iterator <string> begin(s_stream);
    istream_iterator <string> end;
    vector <string> command_tokens(begin, end);
    return command_tokens;    
}

int search_ft(vector < vector <string> > forwarding_table, string host) {
    for(int i = 0; i < forwarding_table.size(); i++) {
        if(forwarding_table[i][0] == host) return stoi(forwarding_table[i][1]);
    }
    return -1;
}

int search_writings(vector < vector<string> > writing_list, string switch_num) {
    for(int i = 0; i < writing_list.size(); i++) {
        if(writing_list[i][0] == switch_num) return stoi(writing_list[i][1]);
    }
    return -1;
}

void broadcast(string message, string switch_num, int ports_num, int source_port) {
    cout << "msg : " << message << endl;
    for(int i = 0; i < ports_num; i++) {
        if(source_port == i + 1) 
            continue;
        string pipe = "./router_" + switch_num + "_port_" + to_string(i+1) + ".pipe";
        cout << "router "<<  switch_num <<" write for broadcast pipe: " << pipe << " with msg : " << message << endl;
        write_on_pipe(pipe, message);
    }
    return;
}

int main(int argc, char **argv) {
    vector<string> reading_list;
    vector<vector<string> > writing_list;

    string router_num(argv[1]);
    string ports_num(argv[2]);
    int router_id = stoi(router_num);
    int number_of_ports = stoi(ports_num);

    vector< vector<std::string> >  forwarding_table = forwarding_tables[router_id - 1];
    
    cout << "New router process created -- Num: " << router_id << " number of Ports: " << number_of_ports << endl;
    string manager_pipe = "./manager_router_" + router_num + ".pipe";
    reading_list.push_back(manager_pipe);

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    fd_set rfds;
    int lut_ind = 0;
    while (1) {
        
        vector<string> new_files;
        vector<int> open_files;
        string message;
        FD_ZERO(&rfds);
        for (int i = 0; i < reading_list.size(); i++) {
            int pipe = open(reading_list[i].c_str(), O_RDONLY | O_NONBLOCK);
            FD_SET(pipe, &rfds);
            open_files.push_back(pipe);
        }

        if (!select(open_files.back() + 1, &rfds, NULL, NULL, &tv)) {
            for (int j = 0; j < open_files.size(); j++) close(open_files[j]);
            continue;
        }
        for (int i = 0; i < open_files.size(); i++) {
            int fd = open_files[i];
            if(FD_ISSET(fd, &rfds)){
                if(reading_list[i] == manager_pipe) { 
                    message = read_message_from_pipe(fd);
                    vector <string> command_tokens = tokenize(message);
                    if (command_tokens[0] == "CONNECTED_TO_CLIENT") {
                        string reading_pipe_name = command_tokens[1];
                        mkfifo(reading_pipe_name.c_str(), 0666);
                        new_files.push_back(reading_pipe_name);
                        cout << "Pipe created with name: " << reading_pipe_name << endl;
                        close(fd);
                    }
                    if (command_tokens[0] == "CONNECTED_TO_ROUTER") {
                        string reading_pipe_name = command_tokens[1];
                        mkfifo(reading_pipe_name.c_str(), 0666);
                        new_files.push_back(reading_pipe_name);
                        vector <string> tokens;
                        stringstream check(reading_pipe_name);
                        string ss;
                        while(getline(check, ss, '_'))
                        {
                            tokens.push_back(ss);
                        }
                        vector<string> row;
                        row.push_back(tokens[1]);
                        row.push_back(command_tokens[3]);
                        cout << " WRITING LIST my r : "<< router_num <<" : ROUTER: " <<tokens[1] << " WRITE ON PORT: " << command_tokens[3]<<endl;
                        writing_list.push_back(row);
                        cout << "Pipe created with name: " << reading_pipe_name << endl;
                        close(fd);
                    }
                }
                else {
                    message = read_message_from_pipe(fd);
                    vector <string> tokens;
                    stringstream check(reading_list[i]);
                    string ss;
                    while(getline(check, ss, '_'))
                    {
                        tokens.push_back(ss);
                    }
                    message = read_message_from_pipe(fd);
                    string multicast_ip, server_ip;
                    vector <string> packet = tokenize(message);
                    if (packet[0] == "BROADCAST_GROUP") {
                        multicast_ip = packet[1];
                        server_ip = packet[2];
                        int port;
                        int received_port;
                        if(tokens[0] == "./client"){
                            port = search_ft(forwarding_table, server_ip);
                            received_port = stoi(tokens[5]);
                        } 
                        else{
                            port = search_ft(forwarding_table, server_ip);
                            received_port =  search_writings(writing_list, tokens[1]);
                        }
                        if( port == received_port) {
                            cout << "ROUTER: "<<router_num <<" found port: " << port <<" reading file: "<< reading_list[i] << " received: " << received_port << endl;
                            broadcast(message, router_num, stoi(ports_num), port);
                        }
                        else {
                            cout << "ROUTER: "<<router_num <<" found port: " << port <<" reading file: "<< reading_list[i] << " received: " << received_port << " DROPED" << endl;
                            close(fd);
                            continue;
                        }
                    }
                }
            }
            else close(fd);
        }
        for (int j = 0; j < new_files.size(); j++) reading_list.push_back(new_files[j]);
    }
    return 0;
}