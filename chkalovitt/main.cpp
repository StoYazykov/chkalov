#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char **argv) {
    if(argc<2) {
        cout << "Chkalov Imports-Table Toolkit 1.00.\n"
        " *** USAGE: *** \n"
        "* chkitt mksect <section_name> *\n"
        "* chkitt mkfunc <section_name> <function_name> *\n"
        "* chkitt rmsect <section_name> *\n"
        "* chkitt rmfunc <function_name> *\n"
        "* chkitt expsect <section_name> <file_name> *\n"
        "* chkitt impsect <file_name> *\n"
        "* chkitt lstall *\n"
        "* chkitt lstsect <section_name> *\n"
        "* chkitt chksect <section_name> *\n"
        "* chkitt chkfunc <section_name> <function_name> *\n";
        return 0;
    }
    string a, b, c;
    a=argv[1];
    b=(argc==3)?argv[2]:"";
    c=(argc==4)?argv[3]:"";

    ifstream file("/root/chk/imports-table.txt");
    if(!file) {
        cout << "Error: imports-table.txt not found!\n";
        return 1;
    }

    string content((istreambuf_iterator<char>(file)),
                istreambuf_iterator<char>());
    file.close();

    cout << content << endl;

    if(a=="mksect") {
        cout << "mksect!\n";
        content+="\n%"+b;
    } else if(a=="mkfunc") {
        cout << "mkfunc!\n";
        size_t sec = content.find("%" + b);
        if(sec!=string::npos) {
            size_t eol=content.find('\n', sec);
            if(eol!=string::npos) {
                content.insert(eol+1, "+"+c+"\n");
            }
        }
    }

    cout << content << endl;
}
