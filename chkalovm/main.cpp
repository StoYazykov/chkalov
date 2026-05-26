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
    cout << "t: " << hex << t << '!' << endl;
    unsigned char o,ty;
    int64_t v;
    i=0;
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
                string m, l, r=pool[v].value; // stdcon.println
                cout << r << endl;
                m=r.substr(0, r.find("."));
                l=r.substr(r.find("."));
                string s=env; // /root/chk/
                s+="/"+m+"/lib"+m+".so";
                cout << "====LIB CALLED: " << s << endl;
                break;
                //void *h=dlopen()
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
                vars.push_back(pop(stack));
                cout << "Store; istored to " << hex << vars.size()-1 << " , type=" << (int)vars.back().type
                << ", value " << (int)vars.back().value << dec << endl;
                break;
            }
        }
    }
    return 0;
}
