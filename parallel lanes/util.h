#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <vector>
#include <sstream>

using namespace std;

// String tokenizer. From http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ");

//Convert a integer to string
string intToStr(int integer);

#endif
