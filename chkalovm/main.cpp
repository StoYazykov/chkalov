#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include "../chkalov/chkalov.h"


using namespace std;

const char* env=getenv("CHKALOV");

int main(int argc, char **argv) {
    bool debug;
    string abcdef;
    cout << "\nChkalov Virtual Machine version 1.00." << endl;
    if(argc<2) {
        cout << "Usage: " << argv[0] << " program.cvm (-d for debug mode)\n";
        return 0;
    }
    if (!getenv("CHKALOV")) {
        cout << "Not setted environment variable CHKALOV!\n";
        return 0;
    }
    cout << "Executing " << argv[1] << "...\n";
    if(argc==3) abcdef=argv[2];
    if(abcdef=="-d") debug=true;
    else debug=false;
    ifstream a(argv[1], ios::binary);
    if(!a) error("Not found CVM file!");
    uint32_t magic;
    uint64_t t;
    a.read(_TPL magic, sz(uint32_t));
    if(magic!=0x05020100) {
        cout << "Error! Hex magic number is: "<<magic<<", expected: "<<0x05020100<<endl;
        return 0;
    }
    if(debug) cout << "this" << endl;

    vector<unsigned char> vm;

    uint64_t i,b;
    vector<Pool> pool;
    vector<Slot> stack, vars;
    a.read(_TPL t, sz(uint64_t));
    pool.resize(t);
    for(i=0; i<t; i++) {
        a.read(_TPL pool[i].type, sizeof(unsigned char));
        a.read(_TPL b, sz(b));
        pool[i].value.resize(b);
        a.read(pool[i].value.data(), b);
        if(debug) cout << "Readed: pool [" << i << "] as value " << pool[i].value.c_str() << '!' << endl;
    }
    a.read(_TPL t, sz(t));
    vm.resize(t);
    a.read((char*)vm.data(), t);
    unsigned char opcode, siza;
    int64_t tmp;
    if(debug) cout << "t: " << hex << t << '!' << endl;
    unsigned char o,ty;
    int64_t v;
    i=0;
    Slot g, h;
    while(i<t) {
        o=vm[i++];
        ty=vm[i++];
        v=0;
        memcpy(&v, &vm[i], st[ty]);
        i+=st[ty];
        switch(o) {
            case PUSH: {
                stack.push_back({ty,v});
                if(debug) cout << "PUSH; " << hex << " type: " << ty << " size: " << st[ty] <<
                " value: " << v << '!' << endl;
                break;
            }
            case CALL: {
                if(debug) cout << "Stack size: " << stack.size() << endl;
                for(int i = 0; i < stack.size(); i++) {
                    if(debug) cout << "  [" << i << "] type=" << (int)stack[i].type << " value=" << stack[i].value << endl;
                }
                string m, l, r=pool[v].value; // stdcon.println
                if(debug) cout << r << endl;
                m=r.substr(0, r.find("."));
                l=r.substr(r.find(".")+1);
                string s=env; // /root/chk/
                s+="/"+m+"/lib"+m+".so";
                if(debug) cout << "====LIB CALLED: " << s << endl;
                void *h=dlopen(s.c_str(), RTLD_LAZY);
                typedef Slot (*CFUNC)(size_t argc, Slot *argp);
                CFUNC func = (CFUNC)dlsym(h, l.c_str());
                if(!func) {
                    cerr << "dlsym failed for '" << l << "': " << dlerror() << endl;
                    dlclose(h);
                    break;
                }
                Slot a=pop(stack);
                if(debug) cout << "calling... a.value=" << a.value << " \n";
                if(a.type>=STR1&&a.type<=STR8) a.value=(int64_t)pool[a.value].value.c_str();
                if(debug) cout << "calling... a.value (converted!)=" << a.value << " \n";
                func(1, &a);
                dlclose(h);
                break;
            }
            case IFNE: {
                int64_t a=pop(stack).value, b=pop(stack).value;
                cout << "IFNE; " << a << " and " << b << '!' << endl;
                if(a!=b) {
                    cout << hex << "!=. Jumping to " << v << "..." << '!' << dec << endl;
                    i=v;
                }
                break;
            }
            case STORE: {
                if(stack.empty()) {
                    error("STORE: stack empty!");
                    break;
                }
                Slot val=pop(stack);
                if(v>=vars.size()) {
                    vars.resize(v+1);
                }
                vars[v]=val;
                if(debug) cout << hex << "Store; stored to " << v << " , type=" << (int)val.type
                    << ", value " << (int)val.value << dec << endl;
                break;
            }
            case ADD: {
                g=pop(stack);
                h=pop(stack);
                stack.push_back({g.type, g.value+h.value});
                break;
            }
            case SUB: {
                g=pop(stack);
                h=pop(stack);
                stack.push_back({g.type, h.value-g.value});
                break;
            }
            case MUL: {
                g=pop(stack);
                h=pop(stack);
                stack.push_back({g.type, g.value*h.value});
                break;
            }
            case DIV: {
                g=pop(stack);
                h=pop(stack);
                stack.push_back({g.type, h.value/g.value});
                break;
            }
            case LOAD: {
                if(v>=vars.size()) {
                    cerr << "LOAD: variable index " << v << " out of bounds!" << endl;
                    break;
                }
                stack.push_back(vars[v]);
                if(debug) cout << "LOAD: var[" << v << "] type=" << (int)vars[v].type
                            << " value=" << vars[v].value << endl;
                break;
            }
        }
    }
    return 0;
}
