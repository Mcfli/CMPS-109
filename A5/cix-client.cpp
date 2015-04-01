// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 5
// $Id: cix-client.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <utility>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

unordered_map<string,cix_command> command_map {
    {"exit", CIX_EXIT},
    {"help", CIX_HELP},
    {"ls"  , CIX_LS  },
    {"get" , CIX_GET },
    {"put" , CIX_PUT },
    {"rm"  , CIX_RM  },
};

void cix_help() {
    static vector<string> help = {
        "exit         - Exit the program.  Equivalent to EOF.",
        "get filename - Copy remote file to local host.",
        "help         - Print help summary.",
        "ls           - List names of files on remote server.",
        "put filename - Copy local file to remote host.",
        "rm filename  - Remove file from remote server.",
    };
    for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
    cix_header header;
    header.cix_command = CIX_LS;
    log << "sending header " << header << endl;
    send_packet (server, &header, sizeof header);
    recv_packet (server, &header, sizeof header);
    log << "received header " << header << endl;
    if (header.cix_command != CIX_LSOUT) {
        log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
        log << "server returned " << header << endl;
    }else {
        char buffer[header.cix_nbytes + 1];
        recv_packet (server, buffer, header.cix_nbytes);
        log << "received " << header.cix_nbytes << " bytes" << endl;
        buffer[header.cix_nbytes] = '\0';
        cout << buffer;
    }
}

void cix_get (client_socket& server, string filename){
    cix_header header;
    header.cix_command = CIX_GET;
    strncpy(header.cix_filename, filename.c_str(), CIX_FILENAME_SIZE);
    log << "sending header " << header << endl;
    send_packet (server, &header, sizeof header);
    recv_packet (server, &header, sizeof header);
    log << "received header " << header << endl;
    if(header.cix_command != CIX_FILE) {
        log << "sent CIX_GET, server did not return CIX_FILE" << endl;
        log << "server returned " << header << endl;
    }
    else{
        char buffer[header.cix_nbytes];
        recv_packet (server, buffer, header.cix_nbytes);
        log << "received " << header.cix_filename << " " <<
        header.cix_nbytes << " bytes" << endl;
        ofstream out;
        out.open(header.cix_filename);
        out.write(buffer, header.cix_nbytes);
        out.close();
    }
}

void cix_put (client_socket& server, string filename){
    cix_header header;
    header.cix_command = CIX_PUT;
    strncpy(header.cix_filename, filename.c_str(), CIX_FILENAME_SIZE);
    ifstream in (filename);
    if(in.fail()){
        cout << "Usage(): End Terminated" << endl;
        return;
    }
    else{
        in.seekg(0, std::ios::end);
        streamsize size = in.tellg();
        in.seekg(0, std::ios::beg);
        char buffer[size];
        in.read(buffer, size);
        header.cix_nbytes = size;
        log << "sending header " << header << endl;
        send_packet (server, &header, sizeof header);
        send_packet (server, buffer, size);
        recv_packet (server, &header, sizeof header);
        if(header.cix_command != CIX_ACK) {
            log << "set CIX_PUT, server did not return CIX_ACK" << endl;
            log << "server returned " << header << endl;
        }
        else{
            log << "sent CIX_PUT, return CIX_ACK, command success"
                << endl;
        }
    }
}

void cix_rm (client_socket& server, string filename){
    cix_header header;
    memset(header.cix_filename, 0, CIX_FILENAME_SIZE);
    header.cix_command = CIX_RM;
    strncpy(header.cix_filename, filename.c_str(), CIX_FILENAME_SIZE);
    send_packet (server, &header, sizeof header);
    recv_packet (server, &header, sizeof header);
    log << "received header " << header << endl;
    if(header.cix_command != CIX_ACK){
        log << "sent CIX_RM, server did not return CIX_ACK" << endl;
        log << "server returned " << header << endl;
    }
    else{
        log << "sent CIX_RM, return CIX_ACK, command success" << endl;
    }
}


void usage() {
    cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
    throw cix_exit();
}

bool SIGINT_throw_cix_exit {false};
void signal_handler (int signal) {
    log << "signal_handler: caught " << strsignal (signal) << endl;
    switch (signal) {
        case SIGINT: case SIGTERM: SIGINT_throw_cix_exit = true; break;
        default: break;
    }
}

int main (int argc, char** argv) {
    log.execname (basename (argv[0]));
    log << "starting" << endl;
    vector<string> args (&argv[1], &argv[argc]);
    signal_action (SIGINT, signal_handler);
    signal_action (SIGTERM, signal_handler);
    if (args.size() > 2) usage();
    string host = get_cix_server_host (args, 0);
    in_port_t port = get_cix_server_port (args, 1);
    if(args.size() == 1){
        string t = args[0];
        size_t found = t.find(".");
        if(found == string::npos){
            host = get_cix_server_host (args,1);
            port = get_cix_server_port (args,0);
        }
        else{
            host = get_cix_server_host (args,0);
            port = get_cix_server_port (args,1);
        }
    }
    else{
        host = get_cix_server_host (args,0);
        port = get_cix_server_port (args,1);
    }
    log << to_string (hostinfo()) << endl;
    try {
        log << "connecting to " << host << " port " << port << endl;
        client_socket server (host, port);
        log << "connected to " << to_string (server) << endl;
        for (;;) {
            string line;
            getline (cin, line);
            if (cin.eof()) throw cix_exit();
            if (SIGINT_throw_cix_exit) throw cix_exit();
            vector<string> wordvec;
            size_t end = 0;
            for(;;){
                size_t start = line.find_first_not_of(" \t", end);
                if(start == string::npos) break;
                end = line.find_first_of(" \t", start);
                wordvec.push_back(line.substr(start,end - start));
            }
            if(wordvec.size() == 0) {continue;}
            log << "command " << line << endl;
            const auto& itor = command_map.find (wordvec[0]);
            cix_command cmd = itor == command_map.end()
            ? CIX_ERROR : itor->second;
            switch (cmd) {
                case CIX_EXIT:
                    throw cix_exit();
                    break;
                case CIX_HELP:
                    cix_help();
                    break;
                case CIX_LS:
                    cix_ls (server);
                    break;
                case CIX_GET:
                    if(wordvec.size() < 2){
                        cout << "Usage(): get filename" << endl;
                        break;
                    }
                    cix_get (server, wordvec[1]);
                    break;
                case CIX_PUT:
                    if(wordvec.size() < 2){
                        cout << "Usage(): put filename" << endl;
                        break;
                    }
                    cix_put (server, wordvec[1]);
                    break;
                case CIX_RM:
                    if(wordvec.size() < 2){
                        cout << "Usage(): rm filename" << endl;
                        break;
                    }
                    cix_rm (server, wordvec[1]);
                    break;
                default:
                    log << line << ": invalid command" << endl;
                    break;
            }
        }
    }catch (socket_error& error) {
        log << error.what() << endl;
    }catch (cix_exit& error) {
        log << "caught cix_exit" << endl;
    }
    log << "finishing" << endl;
    return 0;
}
