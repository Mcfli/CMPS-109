// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 3
// $Id: main.cpp,v 1.7 2015-02-03 15:39:46-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_pair = xpair<const string,string>;
using str_str_map = listmap<string,string>;

void doOper(string s, str_str_map& sm){
    // comment
    if(s[s.find_first_not_of(" ")] == '#'){
        return;
    }
    // key
    if(s.find_first_of("=") == string::npos){
        if(!s.empty()){
            auto element = sm.find(s);
            if(element != sm.end()){
                // found, print out
                cout << (*element).first << " = "
                << (*element).second << endl;
            }
            else{
                // not found in sm
                cout << s << ": key not found" << endl;
            }
        }
    }
    // = or = values
    else if(s.find_first_of("=") == 0){
        if(s.find_first_not_of("= ") != string::npos){
            auto vals =
            split(s.substr(s.find_first_of("=") + 1,
                           string::npos), " ");
            auto i = vals.begin();
            for(; i != vals.end(); ++i){
                for(auto im = sm.begin(); im != sm.end(); ++im){
                    if(*i == (*im).second){
                        // value found, print the pair
                        cout << (*im).first << " = " <<
                        (*im).second << endl;
                    }
                }
            }
        }
        else{
            // = only, print the whole list
            cout << sm;
        }
    }
    
    // key =
    else{
        auto keyStr = s.substr(s.find_first_not_of("= "),
                               s.find_first_of("= "));
        if(s.find_first_not_of("= ",s.find_first_of("= "))
           == string::npos){
            cout << "delete" << endl;
            auto element = sm.find(keyStr);
            if(element != sm.end()){
                // found, erase element
                sm.erase(element);
            }
            else{
                cout << "Unable to delete: not in map" << endl;
            }
        }
        else{
            auto valStr =
            s.substr(s.find_first_not_of("= ",s.find_first_of("= ")),
                     string::npos);
            // print out new pair
            cout << keyStr << " = " << valStr << endl;
            auto element = sm.find(keyStr);
            if(element != sm.end()){
                // key exists, add new value
                element->second = valStr;
            }
            else{
                // add new pair
                sm.insert(str_str_pair(keyStr,valStr));
            }
        }
        return;
    }
}

void scan_options (int argc, char** argv) {
    opterr = 0;
    for (;;) {
        int option = getopt (argc, argv, "@:");
        if (option == EOF) break;
        switch (option) {
            case '@':
                traceflags::setflags (optarg);
                break;
            default:
                complain() << "-" << (char) optopt << ": invalid option"
                << endl;
                break;
        }
    }
}

int main (int argc, char** argv) {
    sys_info::set_execname (argv[0]);
    scan_options (argc, argv);
    fstream in;
    string str ("");
    for(int i = 1; i < argc; ++i){
        str_str_map sm {};
        int count = 0;
        in.open(argv[i]);
        if(in.is_open()){
            while(getline(in,str)){
                if(str.empty()) break;
                cout << argv[i] << ": " << ++count << ": "
                << str << endl;
                doOper(str, sm);
            }
        }
        else{
            cout << "keyvalue: " << argv[i]
            << ": No such file or directory" << endl;
        }
        in.close();
    }
    if(argc < 2){
        char charLine[1024];
        int count = 0;
        str_str_map sm{};
        for(;;){
            if(cin.eof()) break;
            cin.getline(charLine,1024);
            string strLine(charLine);
            // empty last line of file
            if(strLine.empty()) break;
            cout << "-: " << ++count << ": " << strLine << endl;
            doOper(strLine, sm);
        }
    }
    delete sys_info::execname;
    return EXIT_SUCCESS;
}
