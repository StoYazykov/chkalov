#include <iostream>
#include <fstream>
#include "parser.h"

using namespace std;

int main(int argc, char** argv) {
	if(argc < 3) {
        cout << "Usage: chkalovc input.chk output.cvm (-d)" << endl;
        return 1;
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
    cout << "In main\n";
    Parser parser(argi, argo, (argd=="-d")?true:false);
    parser.parseFile();
    parser.finally();

    return 0;
}
