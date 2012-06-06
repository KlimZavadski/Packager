#include <stdlib.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Unpackager.h"


void main(int argc, char *argv[])
{
    if (!argc)
    {
        cout << "\n\tError! Command line is empty." << endl;
        return;
    }
    if (argc == 1)
    {
        Unpackager unpackager(argv[0]);
        if (unpackager.Unpackage(true))
        {
            unpackager.ShowTimeTable();
            unpackager.StartProcess();
        }
    }
    else cout << "\n\tError! Incorrect count parametrs in command line." << endl;
    getch();
}
