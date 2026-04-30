#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include "../chkalov/chkalov.h"


using namespace std;

const char* env = getenv("CHKALOV");

int main(int argc, char **argv) {
    bool debug;
    string abcdef;
    if(argc<2) error("Usage: chkvm program.cvm (-d)");
    cout << "Chkalov Virtual Machine version 1.00." << endl;
    if (!env) error("Not setted environment variable $CHKALOV!");
    if(argc==3) abcdef=argv[2];
    if(abcdef=="-d") debug=true;
    else debug=false;
    ifstream a(argv[1], ios::binary);
    if(!a) error("Not found CVM file!");
    uint32_t magic;
    uint64_t t, b;
    a.read(_TPL magic, sz(uint32_t));
    if(magic!=0x05020100) {
        cout << "Error! Hex magic number is: "<<magic<<", expected: "<<0x05020100<<endl;
        return 0;
    }
    if(debug) cout << "this" << endl;
    Vm c;
    vector<unsigned char> vm;
    vector<Pool> pool;
    vector<int64_t> stack, vars;
    uint64_t size;
    //stack.push_back(8);
    a.read(_TPL t, sz(uint64_t));
    pool.resize(t);
    for(uint64_t i=0; i<t; i++) {
        a.read(_TPL pool[i].type, sizeof(unsigned char));
        a.read(_TPL b, sz(b));
        pool[i].value.resize(b);
        a.read(pool[i].value.data(), b);
        if(debug) cout << "Readed: pool [" << i << "] as value " << pool[i].value.c_str() << '!' << endl;
    }
    if(debug) cout << "this" << endl;
    a.read(_TPL size, sz(size));
    vm.resize(size);
    a.read(_CP vm.data(), size);
    a.close();
    if(vm.size()!=size) error("Incorrect size!");
    if(debug) cout << "size: " << size << " bytes, " << size/sz(Vm) << " Vm objects" << '!' << endl;
    uint64_t q, w;
    for(uint_fast64_t i=0; i<size/sz(Vm); i++) {
        if(debug) cout << "Iteration: " << i << '!' << endl;
        if(debug) cout << "Opcode: " << _TP(void)vm[i].opcode << " value: " << vm[i].value << endl;
        switch(vm[i].opcode) {
            case IPUSH: {
                if(debug) cout << "ipushed " << vm[i].value << endl;
                stack.push_back(vm[i].value);
                break;
            }
            case ISTORE: {
                int64_t v=pop(stack);
                if(debug) cout << "A; Istored: " << v << " from-to " << vars.size() << '!' << endl;
                vars.push_back(v);
                break;
            }
            case ILOAD: {
                int64_t v=vars[vm[i].value];
                if(debug) cout << "A; Iloaded! Variable № " << vm[i].value << " as " << v << '!' << endl; //: " << v << " from-to " << vars.size() << '!' << endl;
                stack.push_back(v);
                break;
            }
            case IINC: {
                stack.back()++;
                break;
            }
            case IDEC: {
                stack.back()--;
                break;
            }
            case IADD: {
                stack.push_back(stack.pop()+stack.pop());
                break;
            }
            case ISUB: {
                stack.push_back(stack.pop()-stack.pop());
                break;
            }
            case IMUL: {
                stack.push_back(stack.pop()*stack.pop());
                break;
            }
            case IDIV: {
                stack.push_back(stack.pop()/stack.pop());
                break;
            }
            case CALL: {
                string s=env;
                string e=pool[vm[i].value].value;
                string z=e.substr(0, e.find('.'));
                if(debug) cout << pool[vm[i].value].value << endl;
                string y=pool[vm[i].value].value.substr(pool[vm[i].value].value.find('.')+1);
                s+=z+"/"+"lib"+z+".so";
                if(debug) cout << "Called! Index= " << vm[i].value << " , value= " << y.c_str() << endl;
                if(debug) cout << s << endl;
                void *handle = dlopen(s.c_str(), RTLD_LAZY);
                if(!handle) cout << "error!" << endl;
                int64_t (*f1)(int64_t);
                f1 = (int64_t (*)(int64_t))dlsym(handle, y.c_str());
                if(!f1) cout << "error! not f1! f1 is : " << y << endl;
                if(stack.empty()) error("Stack empty before CALL!");
                int64_t d=pop(stack);
                if(debug) cout << "Stack.pop(): " << d << '!' << endl;
                f1((int64_t)pool[d].value.c_str());
                dlclose(handle);
                break;
            }
            case IFE:
                w=pop(stack);
                q=pop(stack);
                if(debug) cout << "IFE: comparing " << q << " and " << w << endl;
                if(q==w) {
                    i=vm[i].value-1;
                    if(debug) cout << "Jumping to " << i << endl;
                }
                break;

            case IFNE:
                w=pop(stack);
                q=pop(stack);
                if(debug) cout << "IFNE: comparing " << q << " and " << w << endl;
                if(q!=w) {
                    i=vm[i].value-1;
                    if(debug) cout << "Jumping to " << i << endl;
                }
                break;
        }
    }
    return 0;
}
