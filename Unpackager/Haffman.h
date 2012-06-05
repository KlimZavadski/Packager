#ifndef HAFFMAN_H
#define HAFFMAN_H

#include "Node.h"

#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <map>
#include <bitset>


class Haffman
{
public:
    Haffman(string fileName);
    ~Haffman();

    unsigned int Decode(bool isShowInfo);
    void ShowDecodeTimeTable();

    bool status;
    string fileName;
    unsigned int inputFileSize;     // in Bytes.
    unsigned int outputFileSize;    // in Bytes.
    byte maxCodeSize;               // in bits.
    unsigned int treeSize;          // in Bytes.

private:
    char* ReadFromFile(unsigned long count);
    int WriteToFile(char *outputString, unsigned long count);    

    unsigned int GetOutputFileSize();
    char* GetBitsString(char *inputData, unsigned long inputSize, unsigned long &outputSize);
    char* NormalizeCode(char *bitsString, byte symbol, byte codeSize);
    unsigned int GetHaffmanTree(int);
    void DecodeTree();
    unsigned int DecodeData();


    ifstream inputFile;
    ofstream outputFile;

    Node **symbolsMap;
    int countSymbols;
    Node *treeRoot;
    long clocks[10];
    byte clocksCount;
};

#endif