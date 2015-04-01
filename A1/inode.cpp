// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 1
// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $

#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "inode.h"

int inode::next_inode_nr {1};

//---------------------------------------------------------
// inode functions
//---------------------------------------------------------

inode::inode(inode_t init_type):
inode_nr (next_inode_nr++), type (init_type)
{
    switch (type) {
        case PLAIN_INODE:
            contents = make_shared<plain_file>();
            break;
        case DIR_INODE:
            contents = make_shared<directory>();
            break;
    }
    DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
    DEBUGF ('i', "inode = " << inode_nr);
    return inode_nr;
}

size_t inode::size(){
    if(type == PLAIN_INODE){
        return plain_file_ptr_of(contents)->size();
    }
    return directory_ptr_of(contents)->size();
}

const map<string,inode_ptr>& inode::get_contents(){
    return directory_ptr_of(contents)->get_dirents();
}

void inode::writefile(const wordvec& d){
    plain_file_ptr_of(contents)->writefile(d);
}

inode_t inode::get_type() const{
    return type;
}

void inode::set_name(const string& s){
    name = s;
}

string inode::get_name() const{
    return name;
}

//---------------------------------------------------------
// plain_file functions
//---------------------------------------------------------

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
    plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
    if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
    return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
    directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
    if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
    return dirptr;
}

size_t plain_file::size() const {
    size_t size {0};
    for(auto w : data){
        size += w.size();
    }
    if(data.size() != 0){
        size = size + data.size() - 1;
    }
    DEBUGF ('i', "size = " << size);
    return size;
}

string plain_file::readfile(){
    DEBUGF ('i', data);
    string s = "";
    for(auto element : data){
        s = s + element + " ";
    }
    return s;
}

void plain_file::writefile (const wordvec& words) {
    DEBUGF ('i', words);
    data = words;
}

//---------------------------------------------------------
// directory functions
//---------------------------------------------------------

directory::directory(){
    // Default constructor
    dirents.insert({".",nullptr});
    dirents.insert({"..",nullptr});
}

size_t directory::size() const {
    size_t size {dirents.size()};
    DEBUGF ('i', "size = " << size);
    return size;
}

void directory::remove (const string& filename) {
    DEBUGF ('i', filename);
    // Find by filename
    auto element = dirents.find(filename);
    if(element != dirents.end()){
        // Remove the file or directory
        (*element).second = nullptr;
        dirents.erase(element);
    }
    else{
        // No such file/dir
        throw yshell_exn("No such file / directory");
    }
}

inode& directory::mkdir(const string& dirname){
    inode_ptr inptr = make_shared<inode>(DIR_INODE);
    // Make sure it does not already exist
    if(dirents.find(dirname) == dirents.end()){
        // Insert directory
        dirents.insert({dirname,inptr});
    }
    return *inptr;
}

inode& directory::mkfile(const string& filename){
    inode_ptr inptr = make_shared<inode>(PLAIN_INODE);
    // Make sure it does not already exist
    if(dirents.find(filename) == dirents.end()){
        // Insert file
        dirents.insert({filename,inptr});
    }
    return *inptr;
}

void directory::set_parent(inode_ptr inptr){
    dirents.at("..") = inptr;
}

void directory::set_dot(inode_ptr inptr){
    dirents.at(".") = inptr;
}

const map<string,inode_ptr>& directory::get_dirents(){
    return dirents;
}

//---------------------------------------------------------
// inode_state functions
//---------------------------------------------------------

inode_state::inode_state() {
    // Default constructor
    DEBUGF ('i', "root = " << root << ", cwd = " << cwd
            << ", prompt = \"" << prompt << "\"");
    root = make_shared<inode>(DIR_INODE);
    cwd = root;
    directory_ptr_of(root->contents)->set_dot(root);
    directory_ptr_of(root->contents)->set_parent(root);
}

ostream& operator<< (ostream& out, const inode_state& state) {
    out << "inode_state: root = " << state.root
    << ", cwd = " << state.cwd;
    return out;
}

const map<string,inode_ptr>& inode_state::get_contents(){
    return directory_ptr_of(cwd->contents)->get_dirents();
}

const inode_ptr inode_state::get_root(){
    return root;
}

const inode_ptr inode_state::get_cwd(){
    return cwd;
}

const string& inode_state::get_prompt(){
    return prompt;
}

void inode_state::set_prompt(const wordvec& words){
    if(words.size() == 1){
        // No arguments, reset the prompt to default
        prompt = "% ";
    }
    else{
        // Clear the prompt, then iterate through words and add
        prompt = "";
        for(size_t w = 1; w < words.size(); w++){
            prompt += words.at(w) + " ";
        }
    }
    
}

void inode_state::set_cwd(const wordvec pathname, bool b){
    auto cptr = get_cwd();
    if(b == true){
        cptr = get_root();
    }
    auto i = pathname.begin();
    for(; i != pathname.end(); i++){
        auto p = cptr->get_contents().find(*i);
        if(p == cptr->get_contents().end()){
            // No directory in path
            throw yshell_exn("No such path");
        }
        cptr = p->second;
    }
    cwd = cptr;
}

void inode_state::add_directory(const wordvec pathname, bool b){
    auto cptr = get_cwd();
    if(b == true){
        auto cptr = get_root();
    }
    auto i = pathname.begin();
    for(; i != pathname.end() - 1; i++){
        auto p = cptr->get_contents().find(*i);
        if(p == cptr->get_contents().end() ||
           p->second->get_type() == PLAIN_INODE){
            // No directoy in path
            throw yshell_exn("No such path");
        }
        cptr = p->second;
    }
    // Create the directory in the path
    auto p = cptr->get_contents().find(*i);
    if(p != cptr->get_contents().end()){
        // Directory exists already
        throw yshell_exn("Directory already exists");
    }
    else{
        auto my_dir = directory_ptr_of(cptr->contents)->mkdir(*i);
        // Set dot & parent
        directory_ptr_of(my_dir.contents)->
        set_dot(make_shared<inode>(my_dir));
        directory_ptr_of(my_dir.contents)->set_parent(cptr);
    }
}

void inode_state::add_file(const wordvec& data,
                           const wordvec& pathname, bool b){
    auto cptr = get_cwd();
    if(b == true){
        auto cptr = get_root();
    }
    auto i = pathname.begin();
    for(; i != pathname.end() - 1; i++){
        auto p = cptr->get_contents().find(*i);
        if(p == cptr->get_contents().end()){
            // No directory in path
            throw yshell_exn("No such path");
        }
        cptr = p->second;
    }
    // Create the file in the path
    if(cptr->get_contents().find(*i) != cptr->get_contents().end()){
        // File exists already
        throw yshell_exn("File already exists");
    }
    else{
        // Write new file
        directory_ptr_of(cptr->contents)->mkfile(*i).writefile(data);
    }
}

void inode_state::remove(const wordvec pathname, bool b, bool x){
    auto cptr = get_cwd();
    if(b == true){
        auto cptr = get_root();
    }
    if(pathname.size() == 0){
        // Must be at root
        root = nullptr;
        return;
    }
    auto i = pathname.begin();
    for(; i != pathname.end() - 1; i++){
        auto p = cptr->get_contents().find(*i);
        if(p == cptr->get_contents().end()){
            // No path exists
            throw yshell_exn("No such path");
        }
        cptr = p->second;
    }
    // Remove last file in path
    if(cptr->get_contents().find(*(pathname.end() - 1))
       != cptr->get_contents().end()){
        if(x == true && cptr->
           get_contents().find(*(pathname.end() - 1))->
           second->get_type() == DIR_INODE){
            // Not file type
            throw yshell_exn("Cannot remove directory");
        }
        else{
            try{
                // Remove file
                directory_ptr_of(cptr->contents)->
                remove(*(pathname.end() - 1));
            }catch(yshell_exn){
                // No such file
                throw yshell_exn("No such file exists");
            }
        }
    }
    else{
        // No such file
        throw yshell_exn("No such file exists");
    }
}

string inode_state::readfile(const wordvec pathname, bool b){
    auto cptr = get_cwd();
    if(b == true){
        auto cptr = get_root();
    }
    auto i = pathname.begin();
    for(; i != pathname.end() - 1; i++){
        auto p = cptr->get_contents().find(*i);
        if(p == cptr->get_contents().end()){
            //No direcotry in path
            throw yshell_exn("No such path");
        }
        cptr = p->second;
    }
    auto file = cptr->get_contents().find(*(pathname.end() - 1));
    if(file != cptr->get_contents().end()){
        if(file->second->get_type() == DIR_INODE){
            // Not file type
            throw yshell_exn("Not a file");
            return "";
        }
        return plain_file_ptr_of(file->second->contents)->readfile();
    }
    else{
        // No such file
        throw yshell_exn("No such file exists");
        return "";
    }
}

void inode_state::pwd(ostream& out){
    wordvec wStack;
    inode_ptr inptr = cwd;
    directory_ptr diptr;
    while(inptr != root){
        // Push back as you go down
        wStack.push_back(inptr->name);
        diptr = directory_ptr_of(inptr->contents);
        auto w = diptr->get_dirents().find("..");
        if(w == diptr->get_dirents().end()){
            inptr = nullptr;
        }
        else{
            inptr = w->second;
        }
    }
    if(wStack.size() == 0){
        // At the root
        out << "/" << endl;
        return;
    }
    while(wStack.size() > 0){
        // Pop back stack as you traverse
        out << "/";
        out << wStack.back();
        wStack.pop_back();
    }
    out << "\n";
}
