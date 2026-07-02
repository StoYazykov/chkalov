#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <string>
#include "scope.h"
#include "token.h"
#include "chkalov.h"
using namespace std;


class Import {
public:
    vector<string> funcs;
    string name;
    bool contains(string a);
    void add(string a);
    Import(string _v) {name=_v;}
};

typedef struct {
    ds fn;
    bool debug;
    char lit;
    vector<unsigned char> code;
    vector<Pool> pool;
    vector<Import> imports;
    vector<Token> file;
    ScopeStack scopes;
    size_t p;
    void parse(const string &line);
    void parseBlock();
    void parseExpression();
    void parseTerm();
    void parseFactor();
    void parseIf();
    void parseInstruction();
    char stt(const string& s);
    void render(Vm vm);
    size_t addPool(Pool _pool);

public:
    Parser(const string &_fni, const string &_fno, bool _debug=false);
    void finally();
    void parseFile();
};

#endif
