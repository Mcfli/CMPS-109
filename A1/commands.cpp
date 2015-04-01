// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 1
// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

#include "commands.h"
#include "debug.h"

commands::commands(): map ({
    {"cat"   , fn_cat   },
    {"cd"    , fn_cd    },
    {"echo"  , fn_echo  },
    {"exit"  , fn_exit  },
    {"ls"    , fn_ls    },
    {"lsr"   , fn_lsr   },
    {"make"  , fn_make  },
    {"mkdir" , fn_mkdir },
    {"prompt", fn_prompt},
    {"pwd"   , fn_pwd   },
    {"rm"    , fn_rm    },
    {"rmr"   , fn_rmr   },
}){}

command_fn commands::at (const string& cmd) {
    // Note: value_type is pair<const key_type, mapped_type>
    // So: iterator->first is key_type (string)
    // So: iterator->second is mapped_type (command_fn)
    command_map::const_iterator result = map.find (cmd);
    if (result == map.end()) {
        throw yshell_exn (cmd + ": no such function");
    }
    return result->second;
}


void fn_cat (inode_state& state, const wordvec& words){
    if(words.size() > 1){
        for(auto w = words.begin() + 1; w != words.end(); w++){
            bool b = ((*w)[0] == '/');
            cout << state.readfile(split(*w,"/"),b) << endl;
        }
    }
    else{
        throw yshell_exn("cat :: No file provided");
    }
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_cd (inode_state& state, const wordvec& words){
    auto dir = state.get_contents();
    if(words.size() > 1){
        bool b = false;
        if(words[1][0] == '/'){
            b = true;
        }
        wordvec pathname = split(words[1], "/") ;
        try{
            // Set the new cwd
            state.set_cwd(pathname,b);
        }catch (yshell_exn){
            throw yshell_exn("cd :: No such directory");
        }
    }
    else { // no args
    }
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_echo (inode_state& state, const wordvec& words){
    for(auto w = words.begin() + 1; w != words.end(); w++){
        cout << *w << " ";
    }
    cout << endl;
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_exit (inode_state& state, const wordvec& words){
    wordvec temp{"","/"};
    if(words.size() > 1){
        // Convert status code
        int status = atoi(words[1].c_str());
        exit_status::set(status);
    }
    // Remove all
    fn_rmr(state,temp);
    throw ysh_exit_exn();
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_ls (inode_state& state, const wordvec& words){
    bool b = false;
    string s = pathname(state.get_contents().at("."),
                        state.get_root()->get_inode_nr());
    if(words.size() > 1){
        for(auto w = words.begin() + 1; w != words.end(); w++){
            if((*w)[0] == '/'){
                b = true;
            }
            // Print out directory
            state.set_cwd(split(*w,"/"),b);
            print_dir(state);
            state.set_cwd(split(s,"/"),true);
        }
    }
    else{
        print_dir(state);
    }
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_lsr (inode_state& state, const wordvec& words){
    wordvec path{"", pathname(state.get_contents().at("."),
                              state.get_root()->get_inode_nr())};
    fn_ls(state,words);
    fn_cd(state,words);
    for(auto element : state.get_contents()){
        if(element.second->get_type() == DIR_INODE &&
           element.first != "." && element.first != ".."){
            wordvec temp {"",element.first};
            // Recursive call
            fn_lsr(state,temp);
        }
    }
    fn_cd(state,path);
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}


void fn_make (inode_state& state, const wordvec& words){
    bool b = false;
    wordvec d;
    if(words[1][0] == '/'){
        b = true;
    }
    wordvec pathname = split(words[1],"/");
    for(auto w = words.begin() + 2; w != words.end(); w++){
        d.push_back(*w);
    }
    // Add new file
    state.add_file(d,pathname,b);
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_mkdir (inode_state& state, const wordvec& words){
    bool b = false;
    for(auto w = words.begin() + 1; w != words.end(); w++){
        if(words[1][0] == '/'){
            b = true;
        }
        wordvec pathname = split(words[1],"/");
        try{
            // Add new directory
            state.add_directory(pathname,b);
        }catch(yshell_exn e){
            string what = e.what();
            throw yshell_exn("mkdir::" + what);
        }
    }
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_prompt (inode_state& state, const wordvec& words){
    state.set_prompt(words);
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_pwd (inode_state& state, const wordvec& words){
    state.pwd(cout);
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_rm (inode_state& state, const wordvec& words){
    for(auto w = words.begin() + 1; w != words.end(); w++){
        bool b = false;
        if((*w)[0] == '/'){
            b = true;
        }
        wordvec pathname = split(*w,"/");
        try{
            state.remove(pathname,b,true);
        }catch(yshell_exn e){
            string what = e.what();
            throw yshell_exn("rm::" + what);
        }
    }
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
    if(words.size() > 1){
        bool b = (words[1][0] == '/');
        wordvec path{"",pathname(state.get_contents().at("."),
                                 state.get_root()->get_inode_nr())};
        fn_cd(state,words);
        for(auto dir : state.get_contents()){
            wordvec temp {"",dir.first};
            // Make sure dir is not . or ..
            if(dir.second->get_type() == DIR_INODE &&
               dir.first != ".." && dir.first != "."){
                // Recursive call
                fn_rmr(state,temp);
            }
        }
        // Create & hold a list of removal items
        wordvec rmList {};
        for(auto dir : state.get_contents()){
            // Push back removal list with items
            rmList.push_back(dir.first);
        }
        for(auto dir : rmList){
            try{
                state.remove(split(dir,"/"),false,false);
            }catch(yshell_exn e){
                string what = e.what();
                throw yshell_exn("rmr::" + what);
            }
        }
        fn_cd(state,path);
        state.remove(split(words[1],"/"),b,false);
    }
    DEBUGF ('c', state);
    DEBUGF ('c', words);
}

int exit_status_message() {
    int exit_status = exit_status::get();
    cout << execname() << ": exit(" << exit_status << ")" << endl;
    return exit_status;
}

void print_dir(inode_state &state){
    cout << pathname(state.get_contents().at("."),
                     state.get_root()->get_inode_nr())
         << ":" << endl;
    for(auto element : state.get_contents()){
        cout << "\t" << element.second->get_inode_nr() << " "
        << element.second->size() << " "
        << element.first << " " << endl;
    }
}

string pathname(inode_ptr inptr, int root_nr){
    int temp_nr = inptr->get_inode_nr();
    vector<string> pathname;
    // While temp is not at the root
    while(temp_nr != root_nr){
        inptr = inptr->get_contents().at("..");
        for(auto &element : inptr->get_contents()){
            if(element.second->get_inode_nr() == temp_nr){
                // Insert to pathname vector
                pathname.insert(pathname.begin(),element.first);
            }
        }
        // Move to next inode
        temp_nr = inptr->get_inode_nr();
    }
    string path = "/";
    for(auto &s : pathname){
        // Update pathname string
        path = path + s + "/";
    }
    return path;
}
