#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <string>
#include "scope.h"
#include "token.h"
#include "chkalov.h"
using namespace std;

struct func {
    string name;
    uint16_t args;
};

class Import {
public:
    vector<func> funcs;
    string name;
    bool contains(string a);
    void add(string a, uint64_t _a);
    Import(string _v) {name=_v;}
};

class Parser {
    string fn;
    bool debug;
    vector<unsigned char> code;
    vector<Pool> pool;
    vector<Import> imports;
    vector<Token> file;
    ScopeStack scopes;
    size_t p;
    void parse(const string &line);
    void parseBlock();
    void parseIf();
    void parseInstruction();
    VarType stringToType(string str);
    void render(Vm vm);
    size_t addPool(Pool _pool);

public:
    Parser(const string &_fni, const string &_fno, bool _debug=false);
    void finally();
    void parseFile();
};

#endif
