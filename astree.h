// astree.h  msingh11@ucsc.edu

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"
#include "symtable.h"

struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   size_t blocknr;
   size_t regnr;          
   attr_bitset attributes;
   const string* struct_name; 
   symbol_table* fields;
   sym_ptr symptr;
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   astree (int symbol, int filenr, int linenr,
           int offset, const char* clexinfo);
};

void free_ast (astree* tree);
void free_ast (astree* tree1, astree* tree2);
void free_ast (astree* tree1, astree* tree2, astree* tree3);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt3 (astree* root,astree* one,astree* two,astree* three);
astree* adoptsym (astree* root, astree* child, int symbol);
astree* adoptsym (astree* root, astree* left, astree* right, 
                  int symbol);
astree* adoptsym (astree* root, astree* one, astree* two,
                   astree* three, int symbol);
void dump_astree (FILE* outfile, astree* root);

#endif

