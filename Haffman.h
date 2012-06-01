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


class Node
{
public:
    unsigned long count;
    string code;
    bool actual;
    Node *left, *right;

    Node()
    {
        this->count = 0;
        this->actual = false;
        this->left = this->right = NULL;
    }
    Node(unsigned long count)
    {
        this->count = count;
        this->actual = false;
        this->left = this->right = NULL;
    }
    Node(char *code)
    {
        this->count = 0;
        this->code = code;
        this->actual = false;
        this->left = this->right = NULL;
    }
};

static Node* GetNewNode(Node *node1, Node *node2)
{
    Node *newNode = new Node();
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


    void GetSymbolsMap();
    Node* FindLeastNode(int lastNodeIndex);
    Node* GetHaffmanTree();
    void GetSymbolsCode(Node *node, string code);
    char* GetBytesString(char *inputData, unsigned long inputSize, unsigned long &outputSize);
    string NormalizeCode(string code, byte maxCodeSize);
    unsigned int EncodeTree();
    unsigned int EncodeData();
    

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