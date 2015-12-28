/*
 * stringset.h
 *
 *  Created on: Oct 8, 2015
 *      Author: msingh11@ucsc.edu
 */

#ifndef STRINGSET_H_
#define STRINGSET_H_

#include <iostream>
#include <string>
#include <unordered_set>
using namespace std;

const string* intern_stringset (const char*);

void dump_stringset (FILE*);


#endif /* STRINGSET_H_ */
