#include <stdlib.h>
#include <conio.h>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

#include "Haffman.h"


string GetFileName(string fileName)
{
    int pos = -1, i = 0, len = fileName.length();
    while (i < fileName.length())
    {
        if (fileName[i] == '\\') pos = i;
        i++;
    }
    return fileName.substr(pos + 1, len - pos);
}

void StartProcess(string path)
{    
    wchar_t *pathW = new wchar_t[500];
    mbstowcs(pathW, path.c_str(), path.length());
    pathW[path.length()] = '\0';

    STARTUPINFO startupInfo = {0};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInformation;
    if (CreateProcess(pathW, NULL, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInformation))
    {
        WaitForSingleObject(processInformation.hProcess, INFINITE);
    }
    else
    {
        cout << "\n\tError! Invalid file. " << GetLastError() << endl;
    }
}


void main(int argc, char *argv[])
{
    if (!argc)
    {
        cout << "\n\tError! Command line is empty." << endl;
        return;
    }
    Haffman haffman(GetFileName(argv[0]));
    if (haffman.Decode(true))
    {
        haffman.ShowDecodeTimeTable();
        string path = argv[0];
        path = path.substr(0, path.length() - 9)  + "_unpack.exe";
        StartProcess(path);
        remove(path.c_str());
    
    }
    getch();
}
