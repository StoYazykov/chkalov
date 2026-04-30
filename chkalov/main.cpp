#include <iostream>
#include <fstream>
#include "parser.h"

using namespace std;

int main(int argc, char** argv) {
	if(argc < 3) {
        cout << "Usage: chkalovc input.chk output.cvm (--debug)" << endl;
        return 1;
    }
    string argi=argv[1], argo=argv[2], argd;
    if(argc==4) argd=argv[3];
    else argd="";
    cout << "In main\n";
    Parser parser(argi, argo, (argd=="-d")?true:false);
    parser.parseFile();
    parser.finally();

    return 0;
}
