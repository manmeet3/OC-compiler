// Manmeet Singh - msingh11@ucsc.edu
// symtable.h

#ifndef __SYMTABLE__
#define __SYMTABLE__

#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <queue>
using namespace std;
extern FILE *symfile;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_prototype, ATTR_variable, ATTR_field, ATTR_typeid,
       ATTR_param, ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct astree;
struct symbol;
using symbol_table = unordered_map<const string*,symbol*>;
using sym_entry = pair<const string*,symbol*>;
using sym_ptr = symbol*;
struct symbol {
   attr_bitset attributes;
   symbol_table *fields;
   const string* struct_name;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
   astree* node;
   astree* block;
};

symbol* new_symbol(astree* node);
astree* type_attribute(astree* node);
bool add_symbol(symbol_table table, sym_entry entry);
int parse_astree(astree* root);
void typeid_search(astree* node);
void nested_block();
void block_enter(astree* root);
void block_exit();
sym_entry new_struct(astree* struct_node);
sym_entry new_field(astree* field_node);
sym_entry new_var(astree* vardecl_node);
sym_entry new_proto(astree* proto_node);
sym_entry new_fxn(astree* func_node);
#endif

