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


class Node
{
public:
    char c;
    unsigned int count;
    bool actual;
    bool composite;
    Node *left, *right;
    Node *nextNode;

    Node()
    {
        this->c = 0;
        this->count = 0;
        this->actual = true;
        this->composite = false;
        this->left = this->right = NULL;
        this->nextNode = NULL;
    }
    Node(char c)
    {
        this->c = c;
        this->count = 1;
        this->actual = true;
        this->composite = false;
        this->left = this->right = NULL;
        this->nextNode = NULL;
    }    
    static Node* Max(Node *node1, Node *node2)
    {
        if ((int)node1->c <= (int)node2->c)
            return node1;
        else return node2;
    }
    static Node* Min(Node *node1, Node *node2)
    {
        if ((int)node1->c > (int)node2->c)
            return node1;
        else return node2;
    }
    static Node* AddNewNode(Node *node1, Node *node2)
    {
        Node *newNode = new Node();
        newNode->composite = true;
        newNode->count = node1->count + node2->count;        
        while (true)
        {
            if (node1->composite && node2->composite)
            {
                newNode->c = (int)node1->c + (int)node2->c;
                newNode->left = Min(node1, node2);      // Left leaf - min. It have a code 1.
                newNode->right = Max(node1, node2);     // Right leaf - max. It have a code 0.
                break;
            }
            if (node1->composite)
            {
                newNode->c = (int)node1->c + 1;
                newNode->left = node1;
                newNode->right = node2;
                break;
            }
            if (node2->composite)
            {
                newNode->c = (int)node2->c + 1;
                newNode->left = node2;
                newNode->right = node1;
                break;
            }
            else
            {
                newNode->c = 1;
                newNode->left = node1;
                newNode->right = node2;
                break;
            }
        }
        return newNode;
    }
};
//

class Haffman
{
public:
    Haffman(string fileName);
    ~Haffman();

    int Encode(bool isShowInfo);
    int Decode(bool isShowInfo);

    bool statusOk;
    string fileName;
    int inputFileSize;  // in Byte.
    int outputFileSize;    // in Byte.

private:
    char* ReadFromFile(int count);
    int WriteToFile(char *outputString, int count);


    void AddSymbolToMap(char c, map<char, unsigned int> *symbolsMap);
    map<char, unsigned int>* GetSymbolsMap();
    Node* GetNodeList(map<char, unsigned int> *symbolsMap);
    Node* GetLastNode(Node *firstNode);
    Node* FindLeastNode(Node *firstNode);
    Node* GetHaffmanTree(map<char, unsigned int> *symbolsMap);
    void GetSymbolsCode(Node *nodeTree, map<char, string> *codesMap, string code);
    string NormalizeCode(string code, byte maxCodeSize);
    int EncodeTree(char *data, map<char, string> *codesMap);
    int EncodeData();
    

    int GetOutputFileSize();
    string GetBitsString(char *data, int dataSize);
    string NormalizeCode(string code);
    int DecodeTree(char *data, map<string, char> *charMap);
    int DecodeData();


    ifstream inputFile;
    ofstream outputFile;

    char* data;
    map<char, unsigned int> *symbolsMap;
    map<char, string> *codesMap;
    map<string, char> *charMap;
    Node *treeNode;
};

#endif