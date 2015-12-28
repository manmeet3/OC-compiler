// Manmeet Singh - msingh11@ucsc.edu
// typecheck.cpp

#include "symtable.h"
#include "astree.h"
#include "lyutils.h"
#include "typecheck.h"

void print_loc_node (astree* node){
    fprintf(stderr, "location (%lu.%lu.%lu)\n",
            node->filenr, node->linenr, node->offset);
}

void print_loc_sym (symbol *sym){
    fprintf(stderr, "location (%lu.%lu.%lu)\n",
                    sym->filenr, sym->linenr, sym->offset);
}

void bit_set (){
    if(trav_bit != 1) trav_bit = 1;
}

void check_call(astree* root) { 
    symbol_table *table;
    itor it;
    for(int i = sym_stack.size() - 1; i >= 0; i--) {
        table = sym_stack.at(i);
        it   = table->find(root->children.at(0)->lexinfo);
        if (it != table->end()) {
            vector<astree*> args    = root->children;
            vector<symbol*>* params = it->second->parameters;
            unsigned arg_len   = args.size() - 1;
            unsigned param_len = params->size();
            if (arg_len != param_len) {
                bit_set();
                fprintf(stderr, "arg length unequal to param length\n");
                print_loc_node(root);
            } else {
                for (unsigned j = 0; j < param_len; j++) {
                    if(!check_types(args.at(j+1),
                                        params->at(j))) {
                        bit_set();
                        fprintf(stderr, "improper function call\n");
                        print_loc_node(root);
                    }
                }
            }
            root->attributes = attr_bitset(it->second->attributes);
            root->attributes.set(ATTR_vreg);
            if (root->attributes.test(ATTR_struct)) {
                root->struct_name = it->second->struct_name;
                root->fields = it->second->fields;
            }
            return;
        }
    }
    bit_set();
    fprintf(stderr, "function not found\n");
    print_loc_node(root);
}

bool type_exists(const string* struct_name) {
    itor it = struct_table->find(struct_name);
    if (it == struct_table->end())
        return false;
    return true;
}

void check_asg(astree* root) { 
    if (!root->children.at(0)->attributes.test(ATTR_lval)) {
        bit_set();
        fprintf(stderr, "assignning to an immutable element\n");
        print_loc_node(root);
        return;
    }
    if (!check_types(root->children.at(0),
                        root->children.at(1))) {
        bit_set();
        fprintf(stderr, "incompatible assignment\n");
        print_loc_node(root);
        return;
    }
    root->attributes = attr_bitset(root->children.at(0)->attributes);
    root->attributes.set(ATTR_lval, 0);
    root->attributes.set(ATTR_variable, 0);
    root->attributes.set(ATTR_vreg);
    root->attributes.set(ATTR_vaddr, 0);
}
void check_return(astree* root) {
    if (curr_fxn == nullptr) {
        bit_set();
        fprintf(stderr, "non-void global return\n");
        print_loc_node(root);
        return;
    }
    astree* expr_node = root->children.at(0);
    if (expr_node->attributes.test(ATTR_null)) {
        if (curr_fxn->attributes.test(ATTR_string)
           || curr_fxn->attributes.test(ATTR_struct)
           || curr_fxn->attributes.test(ATTR_array)) return;
    } else if (check_types(expr_node, curr_fxn)) return;
     else {
        bit_set();
        fprintf(stderr, "return type does not match function type\n");
        print_loc_node(root);
    }
}
void check_returnvoid(astree* root) {
    if (curr_fxn != nullptr) {
        if (curr_fxn->attributes.test(ATTR_void)) {
            return;
        } else {
            bit_set();
            fprintf(stderr, "return from nonvoid function\n");
            print_loc_node(root);
        }
    }
}
void check_eq(astree* root) { 
    if (check_types(root->children.at(0), root->children.at(1))) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
    } else {
        bit_set();
        fprintf(stderr, "compared incompatible types\n");
        print_loc_node(root);
    }
}
void check_comp(astree* root) { 
    attr_bitset atr_1 = root->children.at(0)->attributes;
    attr_bitset atr_2 = root->children.at(1)->attributes;
    if (atr_1.test(ATTR_int) && atr_2.test(ATTR_int)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else if (atr_1.test(ATTR_char) && atr_2.test(ATTR_char)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else if (atr_1.test(ATTR_bool)
       && atr_2.test(ATTR_bool)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else {
        bit_set();
        fprintf(stderr, "compared incompatible types\n");
        print_loc_node(root);
    }
}

void check_vardecl(astree* root) { 
    astree* id_node;
    astree* type_node = root->children.at(0);
    astree* expr_node = root->children.at(1);
    attr_bitset type_bits;

    if (type_node->symbol == TOK_ARRAY) {
        id_node = type_node->children.at(1);
    } else {
        id_node = type_node->children.at(0);
    }

    if (expr_node->attributes.test(ATTR_null)) {
        if (id_node->attributes.test(ATTR_string)
         || id_node->attributes.test(ATTR_struct)
         || id_node->attributes.test(ATTR_array)) return;
         else {
            bit_set();
            fprintf(stderr, "null assignment to non-reference type.\n");
            print_loc_node(root);
        }
    } else if (expr_node->attributes.test(ATTR_void)) {
        bit_set();
        fprintf(stderr, "void assignment\n");
        print_loc_node(root);
    } else if (check_types(expr_node, id_node)) return;
    bit_set();
    fprintf(stderr, "declaration type error\n");
    print_loc_node(root);
}

void check_binary(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int)
    &&  root->children.at(1)->attributes.test(ATTR_int)
    && !root->children.at(0)->attributes.test(ATTR_array)
    && !root->children.at(1)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
        return;
    } else {
        bit_set();
        fprintf(stderr,"arithmitic between two incompatible types\n");
        print_loc_node(root);
    }
}
void check_unary(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
    } else {
        bit_set();
        fprintf(stderr,"arithmitic between two incompatible types\n");
        print_loc_node(root);
    }
    return; 
}
void check_negate(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_bool)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_bool);
    } else {
        bit_set();
        fprintf(stderr, "negated non-booleans\n");
        print_loc_node(root);
    }
    return; 
}

void check_chr(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_char);
    } else {
        bit_set();
        fprintf(stderr, "invalid argument to 'chr'\n");
        print_loc_node(root);
    }
    return; 
}
void check_select(astree* root) { 
    astree* id_node = root->children.at(0);
    if (!id_node->attributes.test(ATTR_struct)) {
        bit_set();
        fprintf(stderr, "non-struct element access\n");
        print_loc_node(root);
        return;
    }
    astree* field_node = root->children.at(1);
    if (id_node->fields == nullptr) {
        bit_set();
        fprintf(stderr, "incomplete struct access\n");
        print_loc_node(root);
        return;
    }
    itor it = id_node->fields->find(field_node->lexinfo);
    if (it == id_node->fields->end()) {
        bit_set();
        fprintf(stderr, "invalid struct field access\n");
        print_loc_node(root);
        return;
    }
    field_node->attributes = attr_bitset(it->second->attributes);
    root->attributes       = attr_bitset(it->second->attributes);
    root->attributes.set(ATTR_vaddr);
    root->attributes.set(ATTR_lval);
    root->attributes.set(ATTR_field, 0);
    if (root->attributes.test(ATTR_struct)) {
        root->struct_name = it->second->struct_name;
        root->fields = it->second->fields;
    }
    if (field_node->attributes.test(ATTR_struct)) {
        field_node->struct_name = it->second->struct_name;
        field_node->fields = it->second->fields;
    }
    return;
}
void check_new(astree* root) { 
    astree* typeid_node = root->children.at(0);
    itor it = struct_table->find(typeid_node->lexinfo);
    if (it == struct_table->end()) {
        bit_set();
        fprintf(stderr, "incomplete struct allocated\n");
        print_loc_node(root);
        return;
    }
    root->attributes.set(ATTR_struct);
    root->attributes.set(ATTR_vreg);
    root->struct_name = typeid_node->lexinfo;
    root->fields = it->second->fields;
    return; 
}

void check_ord(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_char)
    && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
    } else {
        bit_set();
        fprintf(stderr, "invalid argument to 'ord'\n");
        print_loc_node(root);
    }
    return; 
}

void check_array(astree* root) { 
    root->attributes.set(ATTR_array);
    root->attributes.set(ATTR_vreg);
    switch(root->children.at(0)->symbol) {
        case TOK_INT:
            root->attributes.set(ATTR_int);
            break;
        case TOK_CHAR:
            root->attributes.set(ATTR_char);
            break;
        case TOK_BOOL:
            root->attributes.set(ATTR_bool);
            break;
        case TOK_STRING:
            root->attributes.set(ATTR_string);
            break;
        case TOK_TYPEID: 
            if (type_exists(root->children.at(0)->lexinfo)) {
                root->attributes.set(ATTR_typeid);
                root->struct_name = root->children.at(0)->lexinfo;
            } else {
                fprintf(stderr, "unknown type '%s'", 
                        root->children.at(0)->lexinfo->c_str());
                print_loc_node(root);
            }
            break;
    }
}

void check_var(astree* root) { 
    symbol_table *table;
    itor it;
    for(int i = sym_stack.size() - 1; i >= 0; i--) {
        table = sym_stack.at(i);
        it   = table->find(root->lexinfo);
        if (it != table->end()) {
            root->attributes = attr_bitset(it->second->attributes);
            root->symptr = it->second;
            if (root->attributes.test(ATTR_struct)) {
                root->fields = it->second->fields;
                root->struct_name = it->second->struct_name;
            }
            return;
        }
    }
    bit_set();
    fprintf(stderr, "unknown identifier '%s'\n",
            root->lexinfo->c_str());
    print_loc_node(root);
}
void check_index(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_bool))
      root->attributes.set(ATTR_bool);
    else if (root->children.at(0)->attributes.test(ATTR_int)) 
      root->attributes.set(ATTR_int);
    else if (root->children.at(0)->attributes.test(ATTR_char))
      root->attributes.set(ATTR_char);
    else if (root->children.at(0)->attributes.test(ATTR_string)) {
        if (root->children.at(0)->attributes.test(ATTR_array)) {
            root->attributes.set(ATTR_string);
        } else
            root->attributes.set(ATTR_char);
    } else if (root->children.at(0)->attributes.test(ATTR_struct)) {
        root->attributes.set(ATTR_struct);
        root->struct_name = root->children.at(0)->lexinfo;
        root->fields = root->children.at(0)->fields;
    }
    root->attributes.set(ATTR_vaddr);
    root->attributes.set(ATTR_lval);
}

void check_ifwhile(astree* root) {
    if (!root->children.at(0)->attributes.test(ATTR_bool)) {
        bit_set();
        fprintf(stderr, "control structure statement must be bool\n");
        print_loc_node(root);
        return;
    }
}

// Check for variable declarations
void vardecl_check(astree* root) {
    symbol_table* table = sym_stack.back();
    itor it = table->find(root->lexinfo);
    if (it == sym_stack.back()->end()) return;
    else {
        bit_set();
        fprintf(stderr, "redefinition of '%s'\n",
                root->lexinfo->c_str());
        print_loc_node(root);
    }
}

void check_proto(const string* lexinfo, symbol* proto_sym) {
    symbol* sym;
    symbol_table* table = sym_stack.back();
    itor it = table->find(lexinfo);
    if (it == sym_stack.back()->end()) return;
    sym = it->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (proto_sym->parameters->size() != sym->parameters->size()) {
            // Conflicting param types
            bit_set();
            fprintf(stderr, "conflicting types for '%s'\n", 
                    lexinfo->c_str());
            print_loc_sym(proto_sym);
            return;
        } else if (attributes != proto_sym->attributes) {
            // Incorrect Types
            bit_set();
            fprintf(stderr, "conflicting types for '%s'\n", 
                    lexinfo->c_str());
            print_loc_sym(proto_sym);
            return;
        }
        for (unsigned i = 0; i < proto_sym->parameters->size(); i++) {
            if (proto_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                // Conflicting param types
                bit_set();
                fprintf(stderr, "conflicting types for '%s'\n",
                        lexinfo->c_str());
                print_loc_sym(proto_sym);
                return;
            }
        }
    } else if (attributes.test(ATTR_function)) {
        if (proto_sym->parameters->size() != sym->parameters->size()) {
            bit_set();
            fprintf(stderr, "conflicting types for '%s'\n", 
                    lexinfo->c_str());
            print_loc_sym(proto_sym);
            return;
        }
        attributes.set(ATTR_function, 0);
        attributes.set(ATTR_prototype);
        if (attributes != proto_sym->attributes) {
            bit_set();
            fprintf(stderr, "conflicting types for '%s'\n",
                    lexinfo->c_str());
            print_loc_sym(proto_sym);
            return;
        }
        for (unsigned i = 0; i < proto_sym->parameters->size(); i++) {
            if (proto_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                bit_set();
                fprintf(stderr, "conflicting types for '%s'\n",
                        lexinfo->c_str());
                print_loc_sym(proto_sym);
                return;
            }
        }
    } else {
        bit_set();
        fprintf(stderr, "redefinition of '%s'", lexinfo->c_str());
        print_loc_sym(proto_sym);
    }
}

void check_fxn(const string* lexinfo, symbol* fxn_sym) {
    symbol* sym;
    symbol_table* table = sym_stack.back();
    itor it = table->find(lexinfo);
    if (it == sym_stack.back()->end()) {
        return;
    }
    sym = it->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (fxn_sym->parameters->size() != sym->parameters->size()) {
            // Conflicting params
            bit_set();
            fprintf(stderr, "conflicting types for '%s'\n",
                    lexinfo->c_str());
            print_loc_sym(fxn_sym);
            return;
        }
        attributes.set(ATTR_prototype, 0);
        attributes.set(ATTR_function);
        if (attributes != fxn_sym->attributes) {
            // Type error
            bit_set();
            fprintf(stderr, "conflicting types for '%s'\n",
                    lexinfo->c_str());
            print_loc_sym(fxn_sym);
            return;
        }
        for (unsigned i = 0; i < fxn_sym->parameters->size(); i++) {
            if (fxn_sym->parameters->at(i)->attributes 
                    != sym->parameters->at(i)->attributes) {
                bit_set();
                /// Conflicting params
                fprintf(stderr, "conflicting types for '%s'\n",
                        lexinfo->c_str());
                print_loc_sym(fxn_sym);
                return;
            }
        }
    } else {
        bit_set();
        // Redefined
        fprintf(stderr, "redefinition of '%s'", lexinfo->c_str());
        print_loc_sym(fxn_sym);
    }
}

// Checks whether two types being operated on are from
// compatible pointers
bool check_types (astree* n, symbol* s) {
    attr_bitset atr_1 = n->attributes;
    attr_bitset atr_2 = s->attributes;
    if ((atr_1.test(ATTR_null) &&  (atr_2.test(ATTR_string)
        || atr_2.test(ATTR_struct) || atr_2.test(ATTR_array)))
        || (atr_2.test(ATTR_null) &&  (atr_1.test(ATTR_string)
        || atr_1.test(ATTR_struct) || atr_1.test(ATTR_array))))
        return true;
    if ((atr_1.test(ATTR_array) && !atr_2.test(ATTR_array))
      || (atr_1.test(ATTR_array) && !atr_2.test(ATTR_array)))
        return false;
    if (atr_1.test(ATTR_int) && atr_2.test(ATTR_int)) 
        return true;
    else if (atr_1.test(ATTR_char) && atr_2.test(ATTR_char)) 
        return true;
    else if (atr_1.test(ATTR_string) && atr_2.test(ATTR_string)) 
        return true;
    else if (atr_1.test(ATTR_bool) && atr_2.test(ATTR_bool)) 
        return true;
    else if (atr_1.test(ATTR_struct) && atr_2.test(ATTR_struct)) {
        if (n->struct_name == s->struct_name)
            return true;}
    return false;
}

// Compares two astrees
bool check_types(astree* n1, astree* n2) {
    attr_bitset atr_1 = n1->attributes;
    attr_bitset atr_2 = n2->attributes;
    if ((atr_1.test(ATTR_array) && !atr_2.test(ATTR_array))
      || (atr_1.test(ATTR_array) && !atr_2.test(ATTR_array))) 
    return false;
    if (atr_1.test(ATTR_int) && atr_2.test(ATTR_int)) 
        return true;
    else if (atr_1.test(ATTR_char) && atr_2.test(ATTR_char))
        return true;
     else if (atr_1.test(ATTR_string) && atr_2.test(ATTR_string)) 
        return true;
    else if (atr_1.test(ATTR_bool) && atr_2.test(ATTR_bool)) 
        return true;
    else if (atr_1.test(ATTR_struct) && atr_2.test(ATTR_struct)) {
        if (n1->struct_name == n2->struct_name) return true;} 
    else if ((atr_1.test(ATTR_null)   &&  (atr_2.test(ATTR_string) || 
              atr_2.test(ATTR_struct) || atr_2.test(ATTR_array)))  || 
             (atr_2.test(ATTR_null)   &&  (atr_1.test(ATTR_string) || 
              atr_1.test(ATTR_struct) || atr_1.test(ATTR_array))))
        return true;
    return false;
}


