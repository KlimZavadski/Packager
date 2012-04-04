#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <map>
#include <vector>
#include <bitset>


class Node
{
public:
    char c;
    unsigned int count;
    bool actual;
    Node *left, *right;

    Node()
    {
        this->c = 0;
        this->count = 0;
        this->actual = true;
        this->left = this->right = NULL;
    }
    Node(char c)
    {
        this->c = c;
        this->count = 1;
        this->actual = true;
        this->left = this->right = NULL;
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

/// Return list of a ... from map.
vector<Node>* ConvertMapToVector(map<char, unsigned int> *symbolsMap)
{
    vector<Node> *nodeVector = new vector<Node>;
    map<char, unsigned int>::iterator constIt;
    for (constIt = symbolsMap->begin(); constIt != symbolsMap->end(); constIt++)
    {
        Node *node = new Node(constIt->first);
        node->count = constIt->second;
        nodeVector->push_back(*node);
    }
    return nodeVector;
}

/// Looking for a node with a minimum content of the node.
vector<Node>::iterator FindLeastNode(vector<Node> *nodeVector)
{
    vector<Node>::iterator it, minIt;
    bool flag = true;
    for (it = nodeVector->begin(); it != nodeVector->end(); it++)
    {
        if (it->actual)
        {
            if (flag)
            {
                minIt = it;
                flag = false;
            }
            else
            {
                minIt = (it->count < minIt->count) ? it : minIt; 
            }
        }
    }
    return minIt;
}

//////////////////////////////////////////////////////////////////////////
void main()
{
    map<char, unsigned int> *symbolsMap;
    symbolsMap = GetSymbolMap("text.txt");
    if (symbolsMap)
    {
        cout << symbolsMap->size() << endl;
        Node *nodeTree = GetHaffmanTree(symbolsMap);
        symbolsMap->clear();
        GetSymbolsCode(nodeTree, symbolsMap, 0);
        int i = 100000;
        while (i)
        {
            i--;
        }
        int p = 0;
    }
}