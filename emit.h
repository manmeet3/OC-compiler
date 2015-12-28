// emit.h - msingh11@ucsc.edu

#ifndef __EMIT__
#define __EMIT__

#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <bitset>

#include "astree.h"
#include "symtable.h"
#include "yyparse.h"
using namespace std;

extern FILE* oilfile;
extern symbol_table* struct_table;

extern queue<astree*> string_queue;
extern queue<symbol*> fxn_queue;

extern symbol_table* struct_table;
extern symbol_table* table_shared;

void emit_initiate(astree* root);
void emit_stmt(astree* root);
void expression(astree* root);
#endif
