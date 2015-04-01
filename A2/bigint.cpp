// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 2
// $Id: bigint.cpp,v 1.61 2014-06-26 17:06:06-07 - - $

#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
using namespace std;
using digit_t = unsigned char;
#include "bigint.h"
#include "debug.h"

bigint::bigint (long that){
    isnegative = false;
    long place = 10.0;
    if(that == 0){
        // Zero
        num.push_back(0);
        return;
    }
    if(that < 0){
        // Negative
        isnegative = true;
        that *= -1.0;
    }
    while(that > 0){
        // Positive
        long digit = (that % place);
        that -= digit;
        digit /= (place/10.0);
        place *= 10.0;
        digit = labs(digit);
        num.push_back(static_cast<digit_t>(digit));
    }
    this->cleanUp();
    DEBUGF ('~', this << " -> " << long_value)
}

bigint::bigint (const string& that) {
    auto i = that.cbegin();
    auto ri = that.rbegin();
    auto rend = that.rend();
    isnegative = false;
    // Set positive and check
    if (i != that.cend() and *i == '_') {
        isnegative = true;
        rend--;
    }
    while(ri != rend){
        // push new number
        num.push_back(*ri++ - '0');
    }
    DEBUGF ('~', this << " -> " << long_value)
}

bigint::bigint(int that){
    isnegative = false;
    int place = 10;
    if(that == 0){
        // Zero
        num.push_back(0);
        return;
    }
    if(that < 0){
        // Negative
        isnegative = true;
        that *= -1;
    }
    if(that > 0){
        // Positive
        int digit = that % place;
        that -= digit;
        place *= 10;
        num.push_back(static_cast<digit_t>(digit));
    }
}

bigint::bigint(const bigint& big){
    long_value = big.long_value;
    isnegative = big.isnegative;
    num.clear();
    num = big.num;
}


bigint operator+ (const bigint& left, const bigint& right) {
    bigint big{};
    if(left.isnegative == right.isnegative){
        // Same sign, add normally
        big = do_bigadd(left,right);
        big.isnegative = left.isnegative;
        return big;
    }
    else{
        // Check which is larger and subtract
        big = (left < right) ?
        do_bigsub(left,right) : do_bigsub(right,left);
        if(left < right) big.isnegative = left.isnegative;
        else big.isnegative = right.isnegative;
        return big;
    }
}

bigint operator- (const bigint& left, const bigint& right) {
    bigint big{};
    if(left.isnegative != right.isnegative){
        // Signs not the same, add them together
        big = do_bigadd(left,right);
        big.isnegative = left.isnegative;
        return big;
    }
    else{
        // Signs are the same, check which is larger
        big = (left > right) ?
        do_bigsub(left,right) : do_bigsub(right,left);
        if(left > right){
            // Assign correct neg value
            big.isnegative = left.isnegative;
        }
        else{
            // Assign correct neg value
            big.isnegative = !right.isnegative;
        }
        return big;
    }
}

bigint operator+ (const bigint& right) {
    return +right;
}

bigint operator- (const bigint& right) {
    return -right;
}

void bigint::cleanUp(){
    auto ri = num.rbegin();
    for(;ri != num.rend(); ri++){
        if(*ri != 0) break;
        num.pop_back();
    }
}

bigint do_bigadd(const bigint& left, const bigint& right){
    bigint big{};
    int carryOver = 0;
    auto iL = left.num.begin();
    auto iR = right.num.begin();
    // check larger size
    int chkSize = (left.num.size() > right.num.size()) ?
    left.num.size() : right.num.size();
    for(int i = 0; i < chkSize + 1; i++){
        int l = carryOver;
        int r = 0;
        if(iL != left.num.end()){
            l += *iL;
            ++iL;
        }
        if(iR != right.num.end()){
            r += *iR;
            ++iR;
        }
        big.num.push_back((l + r) % 10);
        carryOver = (l + r) / 10;
    }
    big.cleanUp();
    return big;
}

bigint do_bigsub(const bigint& left, const bigint& right){
    // left is always larger than right when called
    bigint big{};
    int carryOver = 0;
    auto iL = left.num.begin();
    auto iR = right.num.begin();
    int chkSize = left.num.size();
    for(int i = 0; i < chkSize; i++){
        int l = 0;
        int r = 0;
        if(iL != left.num.end()){
            l = *iL;
            ++iL;
        }
        if(iR != right.num.end()){
            r = *iR;
            ++iR;
        }
        if(l - r - carryOver < 0){
            l += 10;
            big.num.push_back(l - r - carryOver);
            carryOver = 1;
        }
        else{
            big.num.push_back(l - r - carryOver);
            carryOver = 0;
        }
    }
    big.cleanUp();
    return big;
}

bigint do_bigmult(const bigint& left, const bigint& right){
    bigint big{};
    int carryOver = 0;
    int p{};
    big.num.resize(left.num.size() + right.num.size() + 1);
    auto iL = left.num.begin();
    for(int i = 0; i < static_cast<int>(left.num.size()); i++){
        carryOver = 0;
        auto iR = right.num.begin();
        for(int j = 0; j < static_cast<int>(right.num.size()); j++){
            p = big.num[i+j] + (*iL)*(*iR) + carryOver;
            big.num[i+j] = p % 10;
            carryOver = p / 10;
            ++iR;
        }
        big.num[i+right.num.size()] = carryOver;
        ++iL;
    }
    big.cleanUp();
    return big;
}

bool do_bigless (const bigint& left, const bigint& right) {
    // Verify same size
    if(left.num.size() != right.num.size()){
        cout << "Not the same size" << endl;
    }
    auto iL = left.num.rbegin();
    auto iR = right.num.rbegin();
    for(int i = 0; i < (int)left.num.size(); i++){
        if(*iL < *iR){
            return true;
        }
        else if(*iL > *iR){
            return false;
        }
        ++iL;
        ++iR;
    }
    return false;
}

//
// Multiplication algorithm.
//
bigint operator* (const bigint& left, const bigint& right) {
    bigint big = do_bigmult(left,right);
    if(left.isnegative == right.isnegative){
        // Two neg or two pos gives pos
        big.isnegative = false;
    }
    else{
        // One neg one pos gives pos
        big.isnegative = true;
    }
    return big;
}

//
// Division algorithm.
//

void multiply_by_2 (bigint& big) {
    big.num = do_bigadd(big,big).num;
    big.cleanUp();
}

void divide_by_2 (bigint& big) {
    auto i = big.num.rbegin();
    int carryOver = 0;
    for(; i != big.num.rend(); i++){
        if(*i % 2 == 1){
            *i = *i / 2 + carryOver;
            carryOver = 5;
        }
        else{
            *i = *i / 2 + carryOver;
            carryOver = 0;
        }
    }
    big.cleanUp();
}


bigint::quot_rem divide (const bigint& left, const bigint& right) {
    bigint zero(0);
    if (right == zero) throw domain_error ("divide by 0");
    if (right == zero) throw domain_error ("bigint::divide");
    bigint divisor(right);
    bigint quotient(0);
    bigint remainder(left);
    bigint power_of_2(1);
    while (divisor < remainder) {
        multiply_by_2 (divisor);
        multiply_by_2 (power_of_2);
    }
    while (zero < power_of_2) {
        if (not (remainder < divisor)) {
            remainder = remainder - divisor;
            quotient = quotient + power_of_2;
        }
        divide_by_2 (divisor);
        divide_by_2 (power_of_2);
    }
    return {quotient, remainder};
}

bigint operator/ (const bigint& left, const bigint& right) {
    bigint negOne(-1);
    // divide.first for quotient
    if(left.isnegative && !right.isnegative){
        // Left negative right positive
        bigint big = divide(left*negOne,right).first;
        big.isnegative = true;
        return big;
    }
    if(!left.isnegative && right.isnegative){
        // Left positive right negative
        bigint big = divide(left,right*negOne).first;
        big.isnegative = true;
        return big;
    }
    if(left.isnegative && right.isnegative){
        // Left negative right negative
        bigint big = divide(left*negOne,right*negOne).first;
        big.isnegative = false;
        return big;
    }
    else{
        // Left positive right positive
        bigint big = divide(left,right).first;
        big.isnegative = false;
        return big;
    }
}

bigint operator% (const bigint& left, const bigint& right) {
    bigint negOne(-1);
    // divide.second for remainder
    if(left.isnegative && !right.isnegative){
        // Left negative right positive
        bigint big = divide(left*negOne,right).second;
        big.isnegative = true;
        return big;
    }
    if(!left.isnegative && right.isnegative){
        // Left positive right negative
        bigint big = divide(left,right*negOne).second;
        big.isnegative = true;
        return big;
    }
    if(left.isnegative && right.isnegative){
        // Left negative right negative
        bigint big = divide(left*negOne,right*negOne).second;
        big.isnegative = false;
        return big;
    }
    else{
        // Left positive right positive
        bigint big = divide(left,right).second;
        big.isnegative = false;
        return big;
    }
}

bool operator== (const bigint& left, const bigint& right) {
    // Left == right
    if(left > right){
        return false;
    }
    else if(left < right){
        return false;
    }
    else{
        return true;
    }
}

bool operator< (const bigint& left, const bigint& right) {
    // Left < right
    bool equal = (left.isnegative == right.isnegative);
    if(left.num.size() == right.num.size()){
        if(!equal && left.isnegative){
            // Left < right
            return true;
        }
        if(!equal && right.isnegative)
        {
            // Right < left
            return false;
        }
        return do_bigless(left,right);
    }
    // size is not equal, check if left is smaller size
    bool larger = (left.num.size() < right.num.size());
    if(!equal){
        // Not equal size or sign
        // If left is negative, it is smaller
        return (left.isnegative);
    }
    if(!right.isnegative){
        // Right is positive
        // check size of left and right
        return larger;
    }
    else{
        // check size
        return !larger;
    }
}

ostream& operator<< (ostream& out, const bigint& that) {
    auto ri = that.num.rbegin();
    int maxDig = 69;
    if(ri == that.num.rend()){
        out << "0";
        return out;
    }
    if(that.isnegative){
        out << "-";
    }
    int digCount {};
    for(; ri != that.num.rend(); ri++){
        out << static_cast<unsigned>(*ri);
        if(++digCount == maxDig){
            // Check if we have reached the maximum line length
            cout << "\\" << endl;
            digCount = 0;
        }
    }
    return out;
}


bigint pow (const bigint& base, const bigint& exponent) {
    DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
    if (base == 0) return 0;
    bigint lim(numeric_limits<long>::max());
    if(lim < exponent){
        cout << "Exponent too large" << endl;
    }
    bigint base_copy = base;
    bigint expt = exponent;
    bigint zero(0);
    bigint result(1);
    bigint negOne(-1);
    bigint one(1);
    bigint two(2);
    if (expt < zero) {
        base_copy = 1 / base_copy;
        expt = expt * negOne;
    }
    while (expt > zero) {
        if (expt % two == one) { //odd
            result = result * base_copy;
            expt = expt - one;
        }else { //even
            base_copy = base_copy * base_copy;
            divide_by_2(expt);
        }
    }
    DEBUGF ('^', "result = " << result);
    return result;
}

bigint& bigint::operator= (const bigint& big){
    if(this == &big){
        return *this;
    }
    long_value = big.long_value;
    isnegative = big.isnegative;
    num.clear();
    num = big.num;
    return *this;
}
