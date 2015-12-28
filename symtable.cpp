// Manmeet Singh - msingh11@ucsc.edu
// symtable.cpp

#include "symtable.h"
#include "astree.h"
#include "lyutils.h"
#include "typecheck.h"

int blocknr = 1;
int trav_bit = 0;
vector<int> blocknr_stack(1,0);
vector<symbol*> all_syms;
vector<symbol_table*> sym_stack;
astree* curr_fxn = nullptr;
queue<astree*> string_queue;
queue<symbol*> fxn_queue;
symbol_table* table_shared;
symbol_table* struct_table = new symbol_table();

symbol *new_symbol(astree* node) {
    symbol* sym = new symbol();
    sym->attributes = node->attributes;
    sym->filenr     = node->filenr;
    sym->linenr     = node->linenr;
    sym->offset     = node->offset;
    sym->blocknr    = node->blocknr;
    sym->parameters = nullptr;
    sym->block      = nullptr;
    sym->node       = node;
    node->symptr    = sym;
    sym->struct_name = node->struct_name;
    sym->fields     = node->fields;
    all_syms.push_back(sym);
    return sym;
}

// Traverse a nested block
void nested_block(astree* block_node) {
    block_enter(block_node);
    for (size_t i = 0; i < block_node->children.size(); i++) {
        parse_astree(block_node->children[i]); }
    block_exit();
}

void block_enter(astree* block_node) {
    block_node->blocknr = blocknr_stack.back();
    sym_stack.push_back(new symbol_table());
    blocknr_stack.push_back(blocknr); blocknr++;
}

// Exit from the nested block
void block_exit() {
    delete sym_stack.back();
    sym_stack.pop_back();
    blocknr_stack.pop_back();
}

// Add entry into the symbol table specified by params
bool add_symbol (symbol_table *table, sym_entry entry) {
    if (table == nullptr) {
        printf("Creating new table\n");
        table = new symbol_table();
    }
    symbol* sym = entry.second;
    if (table == sym_stack.back()) {
        for (unsigned i = 0; i < blocknr_stack.size() - 1; i++)
            fprintf(symfile, "   ");
    }
    fprintf(symfile, "%s (%lu.%lu.%lu) {%lu} ",
        entry.first->c_str(), sym->filenr,
        sym->linenr, sym->offset, sym->blocknr);
    if (sym->attributes.test(ATTR_void)) fprintf(symfile,"void ");
    if (sym->attributes.test(ATTR_int)) fprintf(symfile,"int ");
    if (sym->attributes.test(ATTR_char)) fprintf(symfile,"char ");
    if (sym->attributes.test(ATTR_typeid)) fprintf(symfile,"typeid ");
    if (sym->attributes.test(ATTR_param)) fprintf(symfile,"param ");
    if (sym->attributes.test(ATTR_lval)) fprintf(symfile,"lval ");
    if (sym->attributes.test(ATTR_bool)) fprintf(symfile,"bool ");
    if (sym->attributes.test(ATTR_string)) fprintf(symfile,"string ");
    if (sym->attributes.test(ATTR_field)) fprintf(symfile,"field ");
    if (sym->attributes.test(ATTR_null)) fprintf(symfile,"null ");
    if (sym->attributes.test(ATTR_array)) fprintf(symfile,"array ");
    if (sym->attributes.test(ATTR_function)) 
      fprintf(symfile,"function ");
    if (sym->attributes.test(ATTR_prototype)) 
      fprintf(symfile, "prototype ");
    if (sym->attributes.test(ATTR_variable)) 
      fprintf(symfile, "variable ");
    if (sym->attributes.test(ATTR_struct)) {
        fprintf(symfile, "struct ");
        if (sym->struct_name != nullptr)
            fprintf(symfile, "\"%s\" ", sym->struct_name->c_str());
    }
    fprintf(symfile, "\n");
    return table->insert(entry).second;
}

// Begin traversing and creating sym tables
int parse_astree(astree* root) {
    if (root->symbol == TOK_FUNCTION) {
        astree* type_node = root->children.at(0);
        if (type_node->symbol == TOK_ARRAY) 
            curr_fxn = type_node->children.at(1);
         else curr_fxn = type_node->children.at(0);
    }
    switch (root->symbol) {
        case TOK_ROOT:
            //printf("going root\n");
            sym_stack.push_back(new symbol_table());
            table_shared = sym_stack.back();
            break;
        case TOK_STRUCT:
            //printf("going struct\n");
            new_struct(root);
            return trav_bit;
        case TOK_PROTOTYPE:
            //printf("going prototype\n");
            new_proto(root);
            return trav_bit;
        case TOK_FUNCTION:
            //printf("going func\n");
            new_fxn(root);
            return trav_bit;
        case TOK_BLOCK:
            //printf("going block\n");
            nested_block(root);
            return trav_bit;
        case TOK_VARDECL:
            //printf("going vardecl\n");
            new_var(root);
            return trav_bit;
    }
    for (size_t i = 0; i < root->children.size(); i++) {
        parse_astree(root->children[i]);
    }
    root->blocknr = blocknr_stack.back();
    switch (root->symbol) {
        case TOK_ORD:
            check_ord(root);
            break;
        case TOK_CHR:
            check_chr(root);
            break;
        case '.':
            check_select(root);
            break;
        case TOK_NEW:
            check_new(root);
            break;
        case TOK_NEWSTRING:
            root->attributes.set(ATTR_string);
            root->attributes.set(ATTR_vreg);
            break;
        case TOK_NEWARRAY:
            check_array(root);
            break;
        case TOK_CALL:
            check_call(root);
            break;
        case TOK_IDENT:
            check_var(root);
            break;
        case TOK_INDEX:
            check_index(root);
            break;
        case TOK_INTCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_int);
            break;
        case TOK_CHARCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_char);
            break;
        case TOK_STRINGCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_string);
            string_queue.push(root);
            break;
        case TOK_FALSE:
        case TOK_TRUE:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_bool);
        case TOK_NULL:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_null);
            break;
        case TOK_WHILE:
        case TOK_IF:
            check_ifwhile(root);
            break;
        case TOK_FIELD:
            root->attributes.set(ATTR_field);
            case '=':
            check_asg(root);
            break;
        case TOK_RETURN:
            check_return(root);
            break;
        case TOK_RETURNVOID:
            check_returnvoid(root);
            break;
        case TOK_EQ:
        case TOK_NE:
            check_eq(root);
            break;
        case TOK_LT:
        case TOK_LE:
        case TOK_GT:
        case TOK_GE:
            check_comp(root);
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            check_binary(root);
            break;
        case TOK_POS:
        case TOK_NEG:
            check_unary(root);
            break;
        case '!':
            check_negate(root);
            break;
    }
    return trav_bit;
}

astree* type_attribute(astree* node) {
    astree* id;
    if (node->symbol == TOK_ARRAY) {
        id   = node->children.at(1);
        node = node->children.at(0);
        node->blocknr = blocknr_stack.back();
        id->attributes.set(ATTR_array);
    } else id = node->children.at(0);
    int attr = node->symbol;
    switch (attr) {
        case TOK_BOOL:
            id->attributes.set(ATTR_bool);
            break;
        case TOK_CHAR:
            id->attributes.set(ATTR_char);
            break;
        case TOK_INT:
            id->attributes.set(ATTR_int);
            break;
        case TOK_STRING:
            id->attributes.set(ATTR_string);
            break;
        case TOK_VOID:
            id->attributes.set(ATTR_void);
            break;
        case TOK_TYPEID:
            typeid_search(node);
            itor itr = struct_table->find(node->lexinfo);
            id->struct_name = node->lexinfo;
            id->fields = itr->second->fields;
            id->attributes.set(ATTR_struct);
            break;
    }
    return id;
}

// Add varaible entry to sym table
sym_entry new_var (astree* node) {
    astree* type_node = node->children.at(0);
    astree* id_node   = type_attribute(type_node);
    id_node->attributes.set(ATTR_variable);
    id_node->attributes.set(ATTR_lval);
    node->blocknr = blocknr_stack.back();
    type_node->blocknr    = blocknr_stack.back();
    id_node->blocknr      = blocknr_stack.back();
    symbol* sym   = new_symbol(id_node);
    vardecl_check(id_node);
    sym_entry entry = make_pair(id_node->lexinfo, sym);
    add_symbol(sym_stack.back(), entry);
    parse_astree(node->children.at(1));
    if (!check_types(node->children.at(1),
                id_node)) {
        trav_bit = 1;
        fprintf(stderr, "Incompatible types\n");
        fprintf(stderr, "location (%lu.%lu.%lu)\n",
                id_node->filenr, id_node->linenr, id_node->offset);
    }
    return entry;
}

// Add prototype entry to symbol table
sym_entry new_proto (astree* node) {
    sym_entry entry; astree* pid_node; astree* proto_node;
    symbol* sym; symbol* psym;
    astree* type_node   = node->children.at(0);
    astree* param_node  = node->children.at(1);
    astree* id_node     = type_attribute(type_node);
    node->blocknr = blocknr_stack.back();
    type_node->blocknr  = blocknr_stack.back();
    id_node->blocknr    = blocknr_stack.back();
    id_node->attributes.set(ATTR_prototype);
    sym = new_symbol(id_node);
    sym->parameters = new vector<symbol*>();
    fxn_queue.push(sym);
    if (param_node->children.size() > 0) {
        block_enter(param_node);
        for (size_t i = 0; i < param_node->children.size(); i++) {
            proto_node = param_node->children.at(i);
            pid_node   = type_attribute(proto_node); 
            pid_node->attributes.set(ATTR_variable);
            pid_node->attributes.set(ATTR_lval);
            pid_node->attributes.set(ATTR_param);
            proto_node->blocknr = blocknr_stack.back();
            pid_node->blocknr   = blocknr_stack.back();
            psym = new_symbol(pid_node);
            sym->parameters->push_back(psym);
        }
        block_exit();
    }
    check_proto(id_node->lexinfo, sym);
    entry = make_pair(id_node->lexinfo, sym);
    add_symbol(sym_stack.back(), entry);
    return entry;
}

// Add struct entry to sym table
sym_entry new_struct(astree* struct_node) {
    symbol *sym; sym_entry entry;
    astree* typeid_node  = struct_node->children.at(0);
    struct_node->blocknr = 0;
    typeid_node->blocknr = 0;
    typeid_node->struct_name = typeid_node->lexinfo;
    typeid_node->attributes.set(ATTR_typeid);
    sym = new_symbol(typeid_node);
    sym->fields = new symbol_table();
    entry = make_pair(typeid_node->lexinfo, sym);
    add_symbol(struct_table, entry);
    for (size_t i = 1; i < struct_node->children.size(); i++) {
        astree* field = struct_node->children.at(i);
        fprintf(symfile, "   ");
        add_symbol(sym->fields, new_field(field));
    }
    return entry;
}

// Add function entry to symbol table
sym_entry new_fxn(astree* func_node) {
    symbol* sym; symbol* psym; sym_entry entry;
    astree* pid_node; astree* proto_node;
    astree* type_node   = func_node->children.at(0);
    astree* param_node  = func_node->children.at(1);
    astree* block_node  = func_node->children.at(2);
    astree* id_node     = type_attribute(type_node);
    func_node->blocknr  = blocknr_stack.back();
    type_node->blocknr  = blocknr_stack.back();
    id_node->blocknr    = blocknr_stack.back();
    block_node->blocknr = blocknr_stack.back();
    id_node->attributes.set(ATTR_function);
    sym = new_symbol(id_node);
    sym->parameters = new vector<symbol*>();
    sym->block = block_node;
    fxn_queue.push(sym);
    entry = make_pair(id_node->lexinfo, sym);
    block_enter(param_node);
    for (size_t i = 0; i < param_node->children.size(); i++) {
        proto_node = param_node->children.at(i);
        pid_node   = type_attribute(proto_node);
        pid_node->attributes.set(ATTR_variable);
        pid_node->attributes.set(ATTR_lval);
        pid_node->attributes.set(ATTR_param);
        proto_node->blocknr = blocknr_stack.back();
        pid_node->blocknr   = blocknr_stack.back();
        psym = new_symbol(pid_node);
        sym->parameters->push_back(psym);
        add_symbol(sym_stack.back(), make_pair(pid_node->lexinfo,
                                                  psym));
    }
    check_fxn(id_node->lexinfo, sym);
    add_symbol(sym_stack.front(), entry);
    for (size_t i = 0; i < block_node->children.size(); i++) {
        parse_astree(block_node->children[i]);
    }
    block_exit();
    curr_fxn = nullptr;
    return entry;
}


// Add a field entry to sym table
sym_entry new_field(astree* field_node) {
    symbol *sym;
    sym_entry entry;
    astree* id_node;
    id_node = type_attribute(field_node);
    id_node->attributes.set(ATTR_field);
    field_node->blocknr = 0;
    id_node->blocknr    = 0;
    sym   = new_symbol(id_node);
    entry = make_pair(id_node->lexinfo, sym);
    return entry;
}

void typeid_search(astree* node) {
    const string* lexinfo = node->lexinfo;
    itor it = struct_table->find(lexinfo);
    if (it == struct_table->end()) {
        symbol *sym = new_symbol(node);
        add_symbol(struct_table, make_pair(lexinfo, sym));
    }
    node->attributes.set(ATTR_typeid);
}
