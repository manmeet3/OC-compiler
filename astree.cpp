// astree.cpp -  msingh11@ucsc.edu

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"


astree::astree (int symbol, int filenr, int linenr,
                int offset, const char* clexinfo):
        symbol (symbol), filenr (filenr), linenr (linenr),
        offset (offset), blocknr(0), attributes(0), 
        struct_name(nullptr),
        fields(nullptr), lexinfo (intern_stringset (clexinfo)) {
      DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           (void*) this, filenr, linenr, offset,
           get_yytname (symbol), lexinfo->c_str());
}

// Adopt 1,2 or 3 children depending on the operator
astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;}
astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;}
astree* adopt3 (astree* root, astree* one, astree* two, astree* three){
    adopt2 (root, one, two);
    adopt1 (root, three);
    return root;}
    
// Overloading of Adoptsym fxns
astree* adoptsym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;}
astree* adoptsym (astree* root,
    astree* left, astree* right, int symbol) {
    root = adopt2 (root, left, right);
    root->symbol = symbol;
    return root;}
astree* adoptsym (astree* root, astree* one, astree* two,
    astree* three, int symbol) {
    root = adopt3 (root, one, two, three);
    root->symbol = symbol;
    return root;}

// Attr printing into the AST dump using a set of tests
    void print_attr (astree* root, FILE* outfile){
    if (root->attributes.test(ATTR_void)) fprintf(outfile, "void ");
    if (root->attributes.test(ATTR_int)) fprintf(outfile, "int ");
    if (root->attributes.test(ATTR_char)) fprintf(outfile, "char ");
    if (root->attributes.test(ATTR_bool)) fprintf(outfile, "bool ");
    if (root->attributes.test(ATTR_string)) fprintf(outfile,"string ");
    if (root->attributes.test(ATTR_null)) fprintf(outfile, "null ");
    if (root->attributes.test(ATTR_array)) fprintf(outfile, "array ");
    if (root->attributes.test(ATTR_field)) fprintf(outfile, "field ");
    if (root->attributes.test(ATTR_typeid))fprintf(outfile, "typeid ");
    if (root->attributes.test(ATTR_param)) fprintf(outfile, "param ");
    if (root->attributes.test(ATTR_lval)) fprintf(outfile, "lval ");
    if (root->attributes.test(ATTR_const)) fprintf(outfile, "const ");
    if (root->attributes.test(ATTR_vreg)) fprintf(outfile, "vreg ");
    if (root->attributes.test(ATTR_vaddr)) fprintf(outfile, "vaddr ");
    if (root->attributes.test(ATTR_function))
      fprintf(outfile,"function ");
    if (root->attributes.test(ATTR_prototype))
      fprintf(outfile, "prototype ");
    if (root->attributes.test(ATTR_variable))
      fprintf(outfile, "variable ");
    if (root->attributes.test(ATTR_struct)) {
      fprintf(outfile, "struct ");
    if (root->struct_name != nullptr)
      fprintf(outfile, "\"%s\" ", root->struct_name->c_str());
    }
    fprintf(outfile, "\n");
}

// Print astree and its attributes recursively
static void dump_astree_rec (FILE* outfile, astree* root,
                             int depth) {
    if (root == NULL) return;
    int i;
    const char *tname = get_yytname (root->symbol);
    if (strstr (tname, "TOK_") == tname) tname += 4;
    for (i = 0; i < depth; i++) fprintf(outfile, "|  ");
    fprintf(outfile, "%s \"%s\" (%lu.%lu.%lu) {%lu} ", 
        tname, root->lexinfo->c_str(),
        root->filenr, root->linenr, root->offset,
        root->blocknr); 
    print_attr(root, outfile);
    for (size_t child = 0; child < root->children.size(); ++child) {
      dump_astree_rec (outfile, root->children[child], depth + 1);
   }
}

// Call the recursive fxn
void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

// Fxn overloading to free adopted astrees - 1,2 or 3
void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%p]-> %d:%d.%d: %s: \"%s\")\n",
           root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}
void free_ast (astree* tree1, astree* tree2) {
    free_ast (tree1);
    free_ast (tree2);
}
void free_ast (astree* tree1, astree* tree2,
                astree* tree3) {
    free_ast (tree1);
    free_ast (tree2);
    free_ast (tree3);
}

