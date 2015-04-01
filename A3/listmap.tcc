// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 3
// $Id: listmap.tcc,v 1.5 2014-07-09 11:50:34-07 - - $

#include "listmap.h"
#include "trace.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (link* next, link* prev,
const value_type& value):
link (next, prev), value (value) {
}

//
// listmap::node::node(node*, node*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next, node* prev,
const value_type& value):
link (static_cast<node*>(next), static_cast<node*>(prev)),
value (value){
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
for(auto i = begin(); i != end() || nullptr;){
i = erase(i);
}
}

//
// listmap::empty()
//
template <typename Key, typename Value, class Less>
bool listmap<Key,Value,Less>::empty() const {
return anchor_.next == anchor();
}

//
// listmap::iterator listmap::begin()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::begin() {
return iterator (anchor_.next);
}

//
// listmap::iterator listmap::end()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::end() {
return iterator (anchor());
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
auto i = begin();
if(begin() == end()){
// list is empty
node* n = new node(begin().getNode(),begin().getNode()->next,pair);
begin().getNode()->next->prev = n;
begin().getNode()->next = n;
return i;
}
else{
for(; i!= end(); ++i){
// check all items
if(less(pair.first,(*i).first)){
node* n = new node(i.getNode(),(--i).getNode(),pair);
(++i).getNode()->prev->next = n;
(i).getNode()->prev = n;
return i;
}
}
// insert at the end
node* n = new node(i.getNode(),(--i).getNode(),pair);
(++i).getNode()->prev->next = n;
(i).getNode()->prev = n;
return i;
}
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that){
// linear search
auto i = begin();
for(; i!= end(); ++i){
if(i->first == that){
break;
}
}
return i;
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
auto curr = position.getNode();
++position;
curr->prev->next = curr->next;
curr->next->prev = curr->prev;
delete curr;
return position;
}

//
// listmap::operator<<(listmap)
//
template <typename Key, typename Value, class Less>
std::ostream& operator<<(std::ostream& out,
listmap<Key,Value,Less>&lm){
for(auto i = lm.begin(); i != lm.end(); ++i){
out << (*i).first << " = " << (*i).second << endl;
}
return out;
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
TRACE ('l', where);
return where->value;
}

//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
TRACE ('l', where);
return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
TRACE ('l', where);
where = where->next;
return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
TRACE ('l', where);
where = where->prev;
return *this;
}


//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
(const iterator& that) const {
return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
(const iterator& that) const {
return this->where != that.where;
}

//
// listmap::iterator::getNode()
//
template<typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::node*
listmap<Key,Value,Less>::iterator::getNode(){
return where;
}
