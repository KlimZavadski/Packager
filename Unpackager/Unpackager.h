#ifndef UNPACKAGER_H
#define UNPACKAGER_H

#include "Node.h"

#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <map>
#include <bitset>


class Unpackager
{
public:
    Unpackager(string fileName);
    ~Unpackager();

    unsigned int Unpackage(bool isShowInfo);
    void ShowTimeTable();
    void StartProcess();

    bool status;
    string fileName;
    string filePath;
    unsigned int inputFileSize;     // in Bytes.
    unsigned int outputFileSize;    // in Bytes.
    byte maxCodeSize;               // in bits.
    unsigned int treeSize;          // in Bytes.

private:
    char* ReadFromFile(unsigned long count);
    unsigned int WriteToFile(char *outputString, unsigned long count);
    unsigned int GetFileSize(ifstream *file);
    string GetFileName(string filePath);

    unsigned int ReadOutputFileSize();
    char* GetBitsString(char *inputData, unsigned long inputSize, unsigned long &outputSize);
    char* NormalizeCode(char *bitsString, byte symbol, byte codeSize);
    unsigned int GetTree(int);
    void UnpackageTree();
    unsigned int UnpackageData();

    ifstream inputFile;
    ofstream outputFile;

    Node **symbolsMap;
    int countSymbols;
    Node *treeRoot;
    long clocks[10];
    byte clocksCount;
};

#endif