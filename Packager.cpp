#include <stdlib.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "Haffman.h"


void Encode(string encodeFileName)
{
    Haffman haffman(encodeFileName);
    if (haffman.statusOk)
    {
        cout << "\n\n\tEncode file size = " << haffman.Encode(true) << endl;
    }
}

void Decode(string decodeFileName)
{
    Haffman haffman(decodeFileName);
    if (haffman.statusOk)
    {
        cout << "\n\n\tDecode file size = " << haffman.Decode(true) << endl;
    }
}

void main(int argc, char *argv[])
{
    if (argc > 1)
    {
        string fileName = argv[argc - 1];
        if (2 < argc && argc < 4)     // If command line contain key as "-e" or some and fileName
        {
            switch (argv[1][1])
            {
                case 'e': Encode(fileName);
                    break;
                case 'd': Decode(fileName);
                    break;
            }
        }
        else
        {
            // Package.
        }
    }
    else cout << "\n\tError! Command line is empty." << endl;
    getch();
}


/*
#include "LZMA.h"

void fun(char *inputName, char *outputName)
{
	CFileSeqInStream inStream;
	CFileOutStream outStream;
	char c;
	int res;
	char rs[800] = { 0 };

	FileSeqInStream_CreateVTable(&inStream);
	File_Construct(&inStream.file);

	FileOutStream_CreateVTable(&outStream);
	File_Construct(&outStream.file);

	{
		size_t t4 = sizeof(UInt32);
		size_t t8 = sizeof(UInt64);
		if (t4 != 4 || t8 != 8) cout << "Incorrect UInt32 or UInt64" << endl;
	}

	if (InFile_Open(&inStream.file, inputName) != 0)
		cout << "Can not open input file" << endl;

	if (OutFile_Open(&outStream.file, outputName) != 0)
		cout << "Can not open output file" << endl;

	UInt64 fileSize;
	File_GetLength(&inStream.file, &fileSize);
	res = Encode(&outStream.s, &inStream.s, fileSize, rs);
	
	res = Decode(&outStream.s, &inStream.s);

	File_Close(&inStream.file);
	File_Close(&outStream.file);
}


void main(int argc, char *argv[])
{
	string *commandKey = NULL;
	string packageFileName = argv[argc - 1];

	if (argc > 2)				// If command line contain key as "-e" or some
	{
		commandKey = new string[argc - 2];
		for (int i = 1; i < argc - 1; i++)
		{
			commandKey[i-1] = argv[i];
		}
	}

	fun("input.txt", "output.txt");
}*/