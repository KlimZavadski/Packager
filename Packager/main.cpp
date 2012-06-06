#include <stdlib.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Packager.h"


void main(int argc, char *argv[])
{
    if (!argc)
    {
        cout << "\n\tError! Command line is empty." << endl;
        return;
    }
    if (argc == 2)
    {
        Packager packager(argv[1]);
        if (packager.Package(true))
        {
            packager.ShowTimeTable();
        }
    }
    else cout << "\n\tError! Incorrect count parametrs in command line." << endl;
    getch();
}
