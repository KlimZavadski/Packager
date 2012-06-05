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

    unsigned int Encode(bool isShowInfo);
    void ShowEncodeTimeTable();

    bool status;
    string fileName;
    unsigned int inputFileSize;     // in Bytes.
    unsigned int outputFileSize;    // in Bytes.
    byte maxCodeSize;               // in bits.
    unsigned int treeSize;          // in Bytes.

private:
    char* ReadFromFile(unsigned long count);
    int WriteToFile(char *outputString, unsigned long count);

    void WriteUnpackager();
    void GetSymbolsMap();
    Node* FindLeastNode(int lastNodeIndex);
    Node* GetHaffmanTree();
    void GetSymbolsCode(Node *node, string code);
    char* GetBytesString(char *inputData, unsigned long inputSize, unsigned long &outputSize);
    string NormalizeCode(string code, byte maxCodeSize);
    unsigned int EncodeTree();
    unsigned int EncodeData();

    ifstream inputFile;
    ofstream outputFile;

    Node **symbolsMap;
    int countSymbols;
    Node *treeRoot;
    long clocks[10];
    byte clocksCount;
};

#endif