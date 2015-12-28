//    >>   Manmeet Singh - msingh11@ucsc.edu  <<    
// oc.cpp (main)  

// OC compiler - performs lexical analysis, builds a parse tree, 
// then builds a symbol table and performs symantic analysis.
// Finally, an intermediate language oil file is emitted.

// Files generated: __.str, __.tok, __.ast, __.sym, __.oil

#include <vector>
#include <fstream>
#include <string>
using namespace std;

#include <string.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lyutils.h"
#include "auxlib.h"
#include "stringset.h"
#include "symtable.h"
#include "emit.h"

const size_t LINESIZE = 1024;
extern int yy_flex_debug;
extern int yydebug;
FILE* tokfile;
FILE* symfile;
FILE* oilfile;


const string CPP = "/usr/bin/cpp";
string yyin_cpp_cmd;

string filename = "";

void yyin_cpp_popen () {
    yyin = popen (yyin_cpp_cmd.c_str(), "r");
    if (yyin == NULL) {
        syserrprintf (yyin_cpp_cmd.c_str());
        exit (get_exitstatus());
    }
}

void yyin_cpp_pclose (void) {
    int pclose_rc = pclose (yyin);
    eprint_status (yyin_cpp_cmd.c_str(), pclose_rc);
    if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

void scanopts (int argc, char **argv){
  int opt = 0;
  yy_flex_debug = 0;
  string cpp_opts = " ";
  string tmp;
  yydebug = 0;
// set flags
  while ((opt = getopt(argc, argv, "@:D:ly")) != -1){
    switch (opt){
    case '@':
      set_debugflags(optarg);
      break;
    case 'D':
      tmp = optarg;
      cpp_opts += "-D " + tmp;
      break;
    case 'l':
      yy_flex_debug = 1;
      break;
    case 'y':
      yydebug = 1;
      break;
    case '?':
      syserrprintf(optarg);
      break;
    }
  }


  if ((optind + 1) > argc) {
    fprintf(stderr, "ERR - oc: no input file \n");
    exit(1);
  } else if ((optind + 1) < argc) {
    fprintf(stderr, "ERR - oc:  more than one input file\n");
    exit(1);
  }
// .oc extension check
  filename = argv[optind];
  string name = filename;
  int z = name.find_last_of(".");

  string ext = name.substr(z);
  string ext_test = ".oc";

  if (strcmp(ext.c_str(), ext_test.c_str()) != 0){
    fprintf(stderr,
            "ERR - oc: file extension .oc could not be verified\n");
    exit(1);
  }

    yyin_cpp_cmd = CPP + cpp_opts + filename;
}

int main (int argc, char **argv){
  set_execname(argv[0]);
  scanopts (argc, argv);
  // getting input file name information.
  string base_name = filename.substr(0, filename.length() - 3);

  string strfile_name = base_name + ".str";
  string tokfile_name = base_name + ".tok";
  string astfile_name = base_name + ".ast";
  string symfile_name = base_name + ".sym";
  string oilfile_name = base_name + ".oil";


  FILE* strfile;
  FILE* astfile;

  strfile = fopen (strfile_name.c_str(), "w");
  tokfile = fopen (tokfile_name.c_str(), "w");
  astfile = fopen (astfile_name.c_str(), "w");
  symfile = fopen (symfile_name.c_str(), "w");
  oilfile = fopen (oilfile_name.c_str(), "w");

    // scan and write
    yyin_cpp_popen();
    yyparse();
    yyin_cpp_pclose();

    // symbol table and typechecking
    int a = parse_astree(yyparse_astree);

    // emitter and oil file
    if (a == 0) emit_initiate(yyparse_astree);
    // str file
    dump_stringset(strfile);
    // ast file
    dump_astree(astfile, yyparse_astree);
    free_ast (yyparse_astree);

    fclose(tokfile);
    fclose(strfile);
    fclose(astfile);
    fclose(symfile);
    fclose(oilfile);

  yylex_destroy();
  return get_exitstatus();
}

