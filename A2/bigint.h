// Nathan Irwin
// CMPS 109
// Winter 2015
// Assignment 2
// $Id: bigint.h,v 1.16 2014-07-02 20:01:17-07 - - $

#ifndef __BIGINT_H__
#define __BIGINT_H__

#include <exception>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;
using charvec = std::vector<unsigned char>;
#include "debug.h"

//
// Define class bigint
//
class bigint {
    friend ostream& operator<< (ostream&, const bigint&);
private:
    bool isnegative {};
    charvec num;
    long long_value {};
    using quot_rem = pair<bigint,bigint>;
    using unumber = unsigned long;
    friend quot_rem divide (const bigint&, const bigint&);
    friend void multiply_by_2 (bigint&);
    friend void divide_by_2 (bigint&);
    friend bigint do_bigadd(const bigint& left, const bigint& right);
    friend bigint do_bigsub(const bigint& left, const bigint& right);
    friend bigint do_bigmult(const bigint& left, const bigint& right);
    friend bool do_bigless(const bigint& left, const bigint& right);
public:
    
    //
    // Ensure synthesized members are genrated.
    //
    bigint() = default;
    bigint (const bigint&);
    bigint (bigint&&) = default;
    bigint& operator= (const bigint&);
    bigint& operator= (bigint&&) = default;
    ~bigint() = default;
    
    //
    // Extra ctors to make bigints.
    //
    bigint (const long);
    bigint (const string&);
    bigint (int );
    
    //
    // Basic add/sub operators.
    //
    friend bigint operator+ (const bigint&, const bigint&);
    friend bigint operator- (const bigint&, const bigint&);
    friend bigint operator+ (const bigint&);
    friend bigint operator- (const bigint&);
    void cleanUp();
    
    //
    // Extended operators implemented with add/sub.
    //
    friend bigint operator* (const bigint&, const bigint&);
    friend bigint operator/ (const bigint&, const bigint&);
    friend bigint operator% (const bigint&, const bigint&);
    
    //
    // Comparison operators.
    //
    friend bool operator== (const bigint&, const bigint&);
    friend bool operator<  (const bigint&, const bigint&);
};

//
// The rest of the operators do not need to be friends.
// Make the comparisons inline for efficiency.
//

bigint pow (const bigint& base, const bigint& exponent);

inline bool operator!= (const bigint &left, const bigint &right) {
    return not (left == right);
}
inline bool operator>  (const bigint &left, const bigint &right) {
    return right < left;
}
inline bool operator<= (const bigint &left, const bigint &right) {
    return not (right < left);
}
inline bool operator>= (const bigint &left, const bigint &right) {
    return not (left < right);
}

#endif
