#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

//#include <Windows.h>
//#include <time.h>
//#include <math.h>

#include <map>
#include <vector>
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


/// Add symbol to map.
void AddSymbolToMap(char c, map<char, unsigned int> *symbolsMap)
{
    map<char, unsigned int>::iterator it = symbolsMap->find(c);
    if (it == symbolsMap->end())        // Element not found. Create new record.
    {
        map<char, unsigned int>::value_type *valueType =
            new map<char, unsigned int>::value_type(c, 1);
        symbolsMap->insert(it, *valueType);
    }
    else            // Element found. Counter increment.
    {
        it->second++;
    }
}

/// Return a map that include all characters from file.
map<char, unsigned int>* GetSymbolMap(char *fileName)
{
    map<char, unsigned int> *symbolsMap = NULL;
    ifstream file;
    file.open(fileName, ios::in | ios::binary);
    if (file)
    {
        symbolsMap = new map<char, unsigned int>;
        while (!file.eof())
        {
            char *s = new char[];
            int i = 0;
            file.getline(s, 10240);
            while (s[i] != '\0')
            {
                AddSymbolToMap(s[i], symbolsMap);
                i++;
            }
        }
    }
    file.close();
    return symbolsMap;
}

/// Build a list of Node from symbols map.
Node* GetNodeList(map<char, unsigned int> *symbolsMap)
{
    Node *firstNode = NULL;
    Node *currentNode = NULL;
    map<char, unsigned int>::iterator constIt;
    for (constIt = symbolsMap->begin(); constIt != symbolsMap->end(); constIt++)
    {
        Node *node = new Node(constIt->first);
        node->count = constIt->second;
        if (!firstNode)
        {
            firstNode = node;
            currentNode = firstNode;
        }
        else
        {
            currentNode->nextNode = node;
            currentNode = node;
        }
    }
    return firstNode;
}

/// Return the last Node of the list.
Node* GetLastNode(Node *firstNode)
{
    Node *lastNode;
    for (lastNode = firstNode; lastNode->nextNode != NULL; lastNode = lastNode->nextNode);
    return lastNode;
}

/// Looking for a Node with a minimum content of the node.
Node* FindLeastNode(Node *firstNode)
{
    Node *node, *minNode = NULL;
    for (node = firstNode; node != NULL; node = node->nextNode)
    {
        if (node->actual)
        {
            if (!minNode)
            {
                minNode = node;
            }
            else
            {
                if (node->count < minNode->count)
                {
                    minNode = node;
                }
            }
        }
    }
    return minNode;
}

/// Return a tree that code symbols in map.
Node* GetHaffmanTree(map<char, unsigned int> *symbolsMap)
{
    Node *firstNode = GetNodeList(symbolsMap);
    Node *lastNode = GetLastNode(firstNode);
    int actualCount = symbolsMap->size();
    while (actualCount > 1)
    {
        Node *node1 = FindLeastNode(firstNode);    // min_1.
        node1->actual = false;
        Node *node2 = FindLeastNode(firstNode);    // min_2 >= min_1.
        node2->actual = false;
        
        Node *newNode = Node::AddNewNode(node1, node2);
        lastNode->nextNode = newNode;
        lastNode = newNode;

        actualCount--;
    }
    return lastNode;
}

/// Craete a binary code for all coding symbols.
void GetSymbolsCode(Node *nodeTree, map<char, unsigned int> *symbolsMap, unsigned int currentCode)
{
    unsigned int code = currentCode;
    code <<= 1;
    if (nodeTree->right != NULL)
    {                
        GetSymbolsCode(nodeTree->right, symbolsMap, code);      // Right code 0.
        puts("Ok. Right.");
    }
    if (nodeTree->left != NULL)
    {
        code += 1;
        GetSymbolsCode(nodeTree->left, symbolsMap, code);       // Left code 1.
        puts("Ok. Left.");
    }
    if (!nodeTree->composite)
    {
        map<char, unsigned int>::value_type *valueType =
            new map<char, unsigned int>::value_type(nodeTree->c, currentCode);
        symbolsMap->insert(*valueType);
    }
}


//////////////////////////////////////////////////////////////////////////
void main()
{
    map<char, unsigned int> *symbolsMap;
    symbolsMap = GetSymbolMap("text.txt");
    if (symbolsMap)
    {
        Node *nodeTree = GetHaffmanTree(symbolsMap);
        symbolsMap->clear();
        GetSymbolsCode(nodeTree, symbolsMap, 0);
    }
}