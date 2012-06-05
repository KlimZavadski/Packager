#ifndef NODE_H
#define NODE_H

#include <string>
#include <iomanip>
using namespace std;


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

#endif
