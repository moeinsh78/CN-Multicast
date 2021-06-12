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
vector<string> make_frames(string file, string source, string destination) {
    int pipe = open(file.c_str(), O_RDONLY | O_NONBLOCK);
    char temp[100000];
    read(pipe, temp, 100000);
    string msg(temp);
    vector<string> frames;
    while(1) {
        string frame;
        frame = source + " " + destination + " ";
        if(msg.size() <= 50) { 
            frame+=msg;
            frame += " 1";
            frames.push_back(frame);
            return frames;
        }
        for(string::size_type i = 0; i < 50; ++i) {
            frame += msg[i];
        }
        frame += " 0";
        frames.push_back(frame);
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

    string router_reading_pipe;
    string router_writing_pipe;

    fd_set rfds;
    int max_fd;
    int output_num = 1;
    while (1) {
        FD_ZERO(&rfds);
        int router_p; 
        int manager = open(manager_pipe.c_str(), O_RDONLY | O_NONBLOCK);
        FD_SET(manager, &rfds);
        max_fd = manager;

        if(router_reading_pipe != "") {
            router_p = open(router_reading_pipe.c_str(), O_RDONLY | O_NONBLOCK);
            FD_SET(router_p, &rfds);
            max_fd = router_p;
        }

        if (!select(max_fd + 1, &rfds, NULL, NULL, &tv)) {
            if(router_reading_pipe != "") {
                close(max_fd);
            }
            close(manager);
            continue;
        }

        if(FD_ISSET(manager, &rfds)) {
            string message = read_message_from_pipe(manager);
            vector <string> command_tokens = tokenize(message);
            if (command_tokens[0] == "CONNECTED_TO_ROUTER") {
                router_reading_pipe = command_tokens[1];
                mkfifo(router_reading_pipe.c_str(), 0666);
                cout << "Pipe created with name: " << router_reading_pipe << endl;
                router_writing_pipe = "./client_" + client_num + "_" + router_reading_pipe.erase(0,2) ;
            }
            else if(command_tokens[0] == "SEND") {
                string destination = command_tokens[2];
                string file = command_tokens[1];
                vector<string> frames = make_frames(file, client_num, destination);
                for(int i = 0; i < frames.size(); i++) {
                    write_on_pipe(router_writing_pipe,frames[i]);
                    sleep(2);
                }
                cout << "client " << client_num << " sent all the frames to system " << destination << endl;
            }
            else if(command_tokens[0] == "RECEIVE") {
                string frame =  client_num + " " + command_tokens[2] + " "; 
                if(command_tokens[1].size() < 50) {
                    frame+=command_tokens[1] + " ";
                    for(int i = command_tokens[1].size(); i <= 49; i++) frame+='x';
                }
                else if(command_tokens[1].size() > 50) {
                    for(int i = 0; i < 50; i++) frame+=command_tokens[1][i];
                }
                frame += " x";
                write_on_pipe(router_writing_pipe, frame);
                cout << "Message to " << router_writing_pipe << " : " << frame << "\n";
            }
            close(manager);
        }
        else if(FD_ISSET(router_p, &rfds)) {
            string message = read_message_from_pipe(router_p);
            char source = message[0];
            if(message[2] != client_num[0]) continue;
            string file_name = "./client_"+ client_num + "_output_" + to_string(output_num) + ".txt";
            fstream output;
            if(message[message.size() - 1] == '1') {
                output.open(file_name,ios::app);
                message.erase(0,4);
                message.erase(message.size() - 2,2);
                output.write((char*) message.c_str(),message.size());
                output.close();
                cout << "client " << client_num << " received all the frames from system " << source << endl;
                output_num++;
            }
            if(message[message.size() - 1] == '0') {
                output.open(file_name,ios::app);
                message.erase(0,4);
                message.erase(message.size() - 2,2);
                output.write((char*) message.c_str(),message.size());
                output.close();
            }
            if(message[message.size() - 1] == 'x') {
                vector <string> command_tokens = tokenize(message);
                string destination = command_tokens[0];
                string file = command_tokens[2];
                vector<string> frames = make_frames(file, client_num, destination);
                for(int i = 0; i < frames.size(); i++) {
                    write_on_pipe(router_writing_pipe,frames[i]);
                    sleep(2);
                }
                cout << "client " << client_num << " sent all the frames to system " << destination << endl;
            }
            close(router_p);
        }
        
    }
    return 0;
}