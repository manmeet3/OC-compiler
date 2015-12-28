// Manmeet Singh - msingh11@ucsc.edu
// typecheck.h

#ifndef __TYPECHECK__
#define __TYPECHECK__


#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
using namespace std;

#include "symtable.h"
#include "astree.h"
#include "lyutils.h"

typedef symbol_table::const_iterator itor;

extern symbol_table* struct_table;
extern vector<symbol_table*> sym_stack;
extern astree* curr_fxn;
extern int trav_bit;

void vardecl_check(astree* root);
void check_fxn(const string* lexinfo, symbol* sym);
void check_proto(const string* lexinfo, symbol* sym);
void check(astree* root);
void check_vardecl(astree* root);
void check_return(astree* root);
void check_returnvoid(astree* root);
void check_asg(astree* root);
void check_eq(astree* root);
void check_comp(astree* root);
void check_binary(astree* root);
void check_unary(astree* root);
void check_negate(astree* root);
void check_ord(astree* root);
void check_chr(astree* root);
void check_select(astree* root);
void check_new(astree* root);
void check_array(astree* root);
void check_call(astree* root);
void check_var(astree* root);
void check_index(astree* root);
void check_ifwhile(astree* root);
bool check_types(astree* n1, astree* n2);
bool check_types(astree* n, symbol* s);

#endif
