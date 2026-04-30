#include "scope.h"
#include <iostream>

size_t Scope::globalId = 0;  // определение статика!

static variable nullvar;

variable::variable(string _name, size_t _id, VarType _type) : name(_name), id(_id), type(_type){};


void Scope::addVar(string varName, VarType varType) {
    vars.push_back(variable(varName, globalId++, varType));
}

string variable::getTypeName() {
	switch(type) {
		case VINT: return "Int";
		case VSTRING: return "String";
		default: return "Not";
	}
}

variable Scope::getVar(string varName) {
    for(size_t i = 0; i < vars.size(); i++) {
        if(vars[i].name == varName) return vars[i];
    }
    return nullvar;
}

void ScopeStack::enterScope() {
    scopes.push_back(Scope());
}

void ScopeStack::exitScope() {
    scopes.pop_back();
}

void ScopeStack::addVar(string varName, VarType varType) {
    if(scopes.empty()) enterScope();
    scopes.back().addVar(varName, varType);
}

variable ScopeStack::getVar(string varName) {
    for(int i = scopes.size() - 1; i >= 0; i--) {
        variable var = scopes[i].getVar(varName);
        if(var.name != "") return var;
    }
    return nullvar;
}

size_t ScopeStack::size() {
	return scopes.size();
}

const size_t Scope::getGlobalId() {
	return (globalId?globalId-1:0);
}
