#ifndef PACKAGER_H
#define PACKAGER_H

#include "Node.h"

#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <map>
#include <bitset>


class Packager
{
public:
    Packager(string fileName);
    ~Packager();

    unsigned int Package(bool isShowInfo);
    void ShowTimeTable();

    bool status;
    string fileName;
    unsigned int inputFileSize;     // in Bytes.
    unsigned int outputFileSize;    // in Bytes.
    byte maxCodeSize;               // in bits.
    unsigned int treeSize;          // in Bytes.

private:
    char* ReadFromFile(unsigned long count);
    unsigned int WriteToFile(char *outputString, unsigned long count);
    unsigned int GetFileSize(ifstream *file);

    void WriteUnpackageCode();
    void GetSymbolsMap();
    Node* FindLeastNode(int lastNodeIndex);
    Node* GetTree();
    void GetSymbolsCode(Node *node, string code);
    char* GetBytesString(char *inputData, unsigned long inputSize, unsigned long &outputSize);
    string NormalizeCode(string code, byte maxCodeSize);
    unsigned int PackageTree();
    unsigned int PackageData();

    ifstream inputFile;
    ofstream outputFile;

    Node **symbolsMap;
    int countSymbols;
    Node *treeRoot;
    long clocks[10];
    byte clocksCount;
};

#endif