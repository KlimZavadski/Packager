#ifndef HAFFMAN_H
#define HAFFMAN_H

#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <map>
#include <bitset>


struct SmallNode
{
    int symbol;
    SmallNode *left, *right;
};


struct Node
{
    unsigned long count;
    string code;
    bool actual;
    Node *left, *right;
};

static Node* GetNewNode(Node *node1, Node *node2)
{
    Node *newNode = new Node;
    newNode->count = node1->count + node2->count;
    newNode->actual = true;

    if (node1->right && node2->right)
    {
        if (node1->count > node2->count)
        {
            newNode->right = node1;     // Right leaf - max. It have a code 0.
            newNode->left = node2;      // Left leaf - min. It have a code 1.
        }
        else
        {
            newNode->right = node2;
            newNode->left = node1;
        }    
        return newNode;
    }
    if (node1->right)
    {
        newNode->right = node2;
        newNode->left = node1;
        return newNode;
    }
    if (node2->right)
    {
        newNode->right = node1;
        newNode->left = node2;
        return newNode;
    }
    newNode->right = node2;
    newNode->left = node1;
    return newNode;
}
//
//

class Haffman
{
public:
    Haffman(string fileName, bool isEncode);
    ~Haffman();

    unsigned int Encode(bool isShowInfo);
    unsigned int Decode(bool isShowInfo);
    void ShowEncodeTimeTable();

    bool status;
    string fileName;
    unsigned int inputFileSize;     // in Bytes.
    unsigned int outputFileSize;    // in Bytes.
    byte maxCodeSize;                  // in bits.
    unsigned int sizeOfTree;        // in Bytes.

private:
    char* ReadFromFile(unsigned long count);
    int WriteToFile(char *outputString, unsigned long count);


    void GetSymbolsMap();
    Node* FindLeastNode(int lastNodeIndex);
    Node* GetHaffmanTree();
    void GetSymbolsCode(Node *node, string code);
    char* GetBytesString(char *inputData, unsigned long dataSize, unsigned long &outputSize);
    string NormalizeCode(string code, byte maxCodeSize);
    unsigned int EncodeTree();
    unsigned int EncodeData();
    

    unsigned int GetOutputFileSize();
    char* GetBitsString(char *data, unsigned int dataSize);
    string NormalizeCode(char *bitsString, byte symbol, byte codeSize);
    unsigned int DecodeTree(char *data, map<string, char> *charsMap);
    SmallNode* DecodeHaffmanTree(map<string, char> *charsMap, string code);
    unsigned int DecodeData();


    ifstream inputFile;
    ofstream outputFile;

    Node **symbolsMap;
    int countSymbols;
    Node *rootTree;
    long clocks[10];
    byte clocksCount;
};

#endif