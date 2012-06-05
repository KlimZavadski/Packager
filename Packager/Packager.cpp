#include <stdlib.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Haffman.h"


void main(int argc, char *argv[])
{
    if (!argc)
    {
        cout << "\n\tError! Command line is empty." << endl;
        return;
    }
    if (argc == 2)
    {
        Haffman haffman(argv[1]);
        if (haffman.Encode(true))
        {
            haffman.ShowEncodeTimeTable();
        }
    }
    else cout << "\n\tError! More than 2 parametr in command line." << endl;
    getch();
}
