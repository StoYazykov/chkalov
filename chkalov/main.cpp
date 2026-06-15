#include <iostream>
#include <fstream>
#include "parser.h"

using namespace std;

int main(int argc, char** argv) {
    cout << "\nChkalov Compiler version 1.00\n";
	if(argc<3) {
        cout << "Usage: " << (const char*)argv[0] << " input.chk output.cvm (-d for debug mode)\n";
        return 0;
    }
    if (!getenv("CHKALOV")) {
        cout << "Not setted environment variable CHKALOV!\n";
        return 0;
    }
    /*
    123.45
    i=123
    d=45
    r=i
    r+=d/(d/10)
*/

    string argi=argv[1], argo=argv[2], argd;
    if(argc==4) argd=argv[3];
    else argd="";
    try {
        Parser parser(argi, argo, (argd=="-d")?true:false);
        parser.parseFile();
        parser.finally();
    }
    catch(const ChkalovError& e) {
        cerr << "Compilation Error: " << e.what() << endl;
        return 0x08;
    }
    catch(const exception& e) {
        cerr << "System Error: " << e.what() << endl;
        return 0x09;
    }
    cout << "Succesfully compiled " << argv[1] << " to " << argv[2] << "!\n";
    return 0;
}
