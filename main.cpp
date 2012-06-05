#include <stdlib.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Haffman.h"


void Encode(string encodeFileName)
{
    Haffman haffman(encodeFileName, true);
    haffman.Encode(true);
    haffman.ShowEncodeTimeTable();
}

void Decode(string decodeFileName)
{
    Haffman haffman(decodeFileName, false);
    haffman.Decode(true);
    haffman.ShowDecodeTimeTable();
}

void main(int argc, char *argv[])
{
    if (!argc)
    {
        cout << "\n\tError! Command line is empty." << endl;
        return;
    }
    if (argc == 3)
    {
        string fileName = argv[2];
        switch (argv[1][1])
        {
            case 'e': Encode(fileName);
                break;
            case 'd': Decode(fileName);
                break;
        }
    }
    else cout << "\n\tError! Less than 2 parametr in command line." << endl;
    getch();
}
