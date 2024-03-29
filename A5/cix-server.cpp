// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 5
// $Id: cix-server.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

#include <iostream>
#include <fstream>
using namespace std;

#include <libgen.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

void reply_ls (accepted_socket& client_sock, cix_header& header) {
    FILE* ls_pipe = popen ("ls -l", "r");
    if (ls_pipe == NULL) {
        log << "ls -l: popen failed: " << strerror (errno) << endl;
        header.cix_command = CIX_NAK;
        header.cix_nbytes = errno;
        send_packet (client_sock, &header, sizeof header);
    }
    string ls_output;
    char buffer[0x1000];
    for (;;) {
        char* rc = fgets (buffer, sizeof buffer, ls_pipe);
        if (rc == nullptr) break;
        ls_output.append (buffer);
    }
    header.cix_command = CIX_LSOUT;
    header.cix_nbytes = ls_output.size();
    memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
    log << "sending header " << header << endl;
    send_packet (client_sock, &header, sizeof header);
    send_packet (client_sock, ls_output.c_str(), ls_output.size());
    log << "sent " << ls_output.size() << " bytes" << endl;
}

void reply_get (accepted_socket& client_sock, cix_header& header) {
    ifstream in (header.cix_filename);
    if(in.fail()) {
        memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
        header.cix_command = CIX_NAK;
        send_packet (client_sock, &header, sizeof header);
    }
    else if(!in.fail()){
        in.seekg(0, std::ios::end);
        streamsize size = in.tellg();
        in.seekg(0, std::ios::beg);
        char buffer[size];
        in.read(buffer, size);
        header.cix_command = CIX_FILE;
        header.cix_nbytes = size;
        log << "sending header " << header << endl;
        send_packet (client_sock, &header, sizeof header);
        send_packet (client_sock, buffer, size);
    }
}

void reply_put (accepted_socket& client_sock, cix_header& header) {
    log << "received " << header.cix_nbytes << " " << "bytes" << endl;
    char buffer[header.cix_nbytes];
    ofstream out;
    out.open(header.cix_filename);
    if(out.fail()){
        log << "Error: cannot open/create file "
        << header.cix_filename << endl;
        header.cix_command = CIX_NAK;
        header.cix_nbytes = errno;
    }
    else{
        recv_packet (client_sock, buffer, header.cix_nbytes);
        out.write(buffer,header.cix_nbytes);
        header.cix_command = CIX_ACK;
        out.close();
    }
    send_packet(client_sock, &header, sizeof header);
}

void reply_rm (accepted_socket& client_sock, cix_header& header) {
    int rc = unlink(header.cix_filename);
    if(rc == -1) {
        log << "Failed to rm " << header.cix_filename
        << strerror(errno) << endl;
        header.cix_nbytes = errno;
        header.cix_command = CIX_NAK;
        send_packet (client_sock, &header, sizeof header);
    }
    else {
        header.cix_command = CIX_ACK;
        memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
        log << "sending header " << header << endl;
        send_packet (client_sock, &header, sizeof header);
    }
}

bool SIGINT_throw_cix_exit = false;
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
    int client_fd = args.size() == 0 ? -1 : stoi (args[0]);
    log << "starting client_fd " << client_fd << endl;
    try {
        accepted_socket client_sock (client_fd);
        log << "connected to " << to_string (client_sock) << endl;
        for (;;) {
            if (SIGINT_throw_cix_exit) throw cix_exit();
            cix_header header;
            recv_packet (client_sock, &header, sizeof header);
            log << "received header " << header << endl;
            switch (header.cix_command) {
                case CIX_LS:
                    reply_ls (client_sock, header);
                    break;
                case CIX_GET:
                    reply_get (client_sock, header);
                    break;
                case CIX_PUT:
                    reply_put (client_sock, header);
                    break;
                case CIX_RM:
                    reply_rm (client_sock, header);
                    break;
                default:
                    log << "invalid header from client" << endl;
                    log << "cix_nbytes = "
                    << header.cix_nbytes << endl;
                    log << "cix_command = "
                    << header.cix_command << endl;
                    log << "cix_filename = "
                    << header.cix_filename << endl;
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
