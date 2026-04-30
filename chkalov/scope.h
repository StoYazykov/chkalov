#ifndef _SCOPE_H
#define _SCOPE_H

#include <string>
#include <vector>
using namespace std;

enum VarType {
	VNOT=0,
	VINT,
	VSTRING
};

struct variable {
    string name;
    size_t id;
    VarType type;
    variable(string _name = "", size_t _id = 0, VarType _type=VNOT);
    string getTypeName();
};

class Scope {
    vector<variable> vars;
    static size_t globalId;
public:
    void addVar(string varName, VarType varType=VNOT);
    variable getVar(string varName);
    const size_t getGlobalId();
};


class ScopeStack {
    vector<Scope> scopes;
public:
    void enterScope();
    void exitScope();
    void addVar(string varName, VarType varType=VNOT);
    variable getVar(string varName);
    size_t size();
};

#endif
