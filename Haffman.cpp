// Haffman.cpp
#include "Haffman.h"

#define blockSize 1048576   // = 1MB.
#include <time.h>

/// Constructor.
Haffman::Haffman(string fileName)
{
    this->fileName = fileName;

    this->inputFile.open(fileName.c_str(), ios::in | ios::binary);
    this->inputFile.seekg(0, ios_base::end);
    this->inputFileSize = this->inputFile.tellg();
    this->inputFile.seekg(0);

    fileName += ".pac";
    this->outputFile.open(fileName.c_str(), ios::out | ios::binary);
    if (!inputFile || !outputFile)
    {
        this->statusOk = false;
    }
    this->outputFileSize = 0;
    this->data = NULL;

    this->symbolsMap = new map<char, unsigned int>;
    this->codesMap = new map<char, string>;
    this->charMap = new map<string, char>;
    this->treeNode = NULL;
}

/// Destructor.
Haffman::~Haffman()
{
    inputFile.close();
    outputFile.close();

    delete this->symbolsMap;
    delete this->codesMap;
    delete this->treeNode;
    delete this->charMap;
    delete this->data;
}

//
/// Read count bytes from fileIn.
char* Haffman::ReadFromFile(int count)
{
    char *buffer = (char*)calloc(count, 1);
    inputFile.read(buffer, count);
    return buffer;
}

/// Write count bytes to fileOut.
int Haffman::WriteToFile(char *buffer, int count)
{
    outputFile.write(buffer, count);
    return count;
}
//
// Encode.

/// Add symbol to map.
void Haffman::AddSymbolToMap(char c, map<char, unsigned int> *symbolsMap)
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
map<char, unsigned int>* Haffman::GetSymbolsMap()
{
    map<char, unsigned int> *symbolsMap = new map<char, unsigned int>;
    ifstream file;
    int positionInFile = 0, i;
    file.open(fileName.c_str(), ios::in | ios::binary);
    if (file)
    {
        while (!file.eof())
        {
            i = 0;
            char *buffer = (char*)calloc(blockSize, 1);
            file.read(buffer, blockSize);
            while (i < blockSize && positionInFile < inputFileSize)
            {
                AddSymbolToMap(buffer[i], symbolsMap);
                i++;
                positionInFile++;
            }
            delete[] buffer;
        }
        file.close();
    }
    return symbolsMap;
}

/// Build a list of Node from symbols map.
Node* Haffman::GetNodeList(map<char, unsigned int> *symbolsMap)
{
    Node *firstNode = NULL;
    Node *currentNode = NULL;
    map<char, unsigned int>::const_iterator constIt;
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
Node* Haffman::GetLastNode(Node *firstNode)
{
    Node *lastNode;
    for (lastNode = firstNode; lastNode->nextNode != NULL; lastNode = lastNode->nextNode);
    return lastNode;
}

/// Looking for a Node with a minimum content of the node.
Node* Haffman::FindLeastNode(Node *firstNode)
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
Node* Haffman::GetHaffmanTree(map<char, unsigned int> *symbolsMap)
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
void Haffman::GetSymbolsCode(Node *nodeTree, map<char, string> *codesMap, string code)
{
    if (nodeTree->right != NULL)
    {
        GetSymbolsCode(nodeTree->right, codesMap, code + "0");      // Right code 0.
    }
    if (nodeTree->left != NULL)
    {
        GetSymbolsCode(nodeTree->left, codesMap, code + "1");       // Left code 1.
    }
    if (!nodeTree->composite)
    {
        map<char, string>::value_type *valueType =
            new map<char, string>::value_type(nodeTree->c, code);
        codesMap->insert(*valueType);
    }
}

/// Add apsent null.
string Haffman::NormalizeCode(string code, byte maxCodeSize)
{
    string nul = "";
    for (byte i = 0; i < maxCodeSize - code.length(); i++)
    {
        nul += "0";
    }
    return nul + "1" + code;
}

/// Encode tree.
int Haffman::EncodeTree(char *data, map<char, string> *codesMap)
{
    // Search max length of code.
    byte maxCodeSize = 1;
    map<char, string>::const_iterator constIt;
    for (constIt = codesMap->begin(); constIt != codesMap->end(); constIt++)
    {
        if (maxCodeSize < constIt->second.length())
        {
            maxCodeSize = constIt->second.length();
        }
    }
    data[0] = (char)this->inputFileSize >> 24;
    data[1] = (char)this->inputFileSize >> 16;
    data[2] = (char)this->inputFileSize >> 8;
    data[3] = (char)this->inputFileSize;
    data[4] = maxCodeSize + 1;
    
    // Create code string.
    int outputIndex = 5;
    byte inputIndex = 0, index, bit = 7;
    bitset<8> oneByte;
    char *s = (char*)calloc(256, 1);
    string code;
    string nullCode = NormalizeCode("", maxCodeSize);

    do
    {
        map<char, string>::const_iterator constIt = codesMap->find((char)inputIndex);
        if (constIt == codesMap->end())
        {
            code = nullCode;
        }
        else
        {
            code = NormalizeCode(constIt->second, maxCodeSize);
        }
        index = 0;
        while (index < maxCodeSize + 1)     // all simbils.!!!!!!
        {
            if (code[index] == '1')
            {
                oneByte.set(bit);
            }
            bit--;
            index++;      // Reverse, because bitset.
            if (bit == 255)       // Byte is full.
            {
                data[outputIndex] = (char)oneByte.to_ulong();
                oneByte.reset();
                outputIndex++;
                bit = 7;
            }                
        }
    } while (inputIndex++ < 255);
    if (bit != 7)       // If last symbol in fileIn and bitset not reset.
    {
        data[outputIndex] = (char)oneByte.to_ulong();
        outputIndex++;
    }
    return outputIndex;
}

/// Encode data and return it size.
int Haffman::EncodeData()
{
    byte index, bit = 7;
    bitset<8> oneByte;      // One byte.
    oneByte.reset();
    char *inputString = NULL;
    this->data = (char*)calloc(100 + inputFileSize * 1.5, 1);
    int indexReadSymbol, indexWriteSymbol = EncodeTree(this->data, this->codesMap);
    int leftSize = this->inputFileSize;

    while (leftSize)
    {
        if (this->inputFile.eof())
        {
            break;
        }
        if (leftSize < blockSize)
        {
            inputString = ReadFromFile(leftSize);
        }
        else
        {
            inputString = ReadFromFile(blockSize);
        }
        indexReadSymbol = 0;
        while (indexReadSymbol < blockSize && leftSize)
        {
            index = 0;
            string code = (*codesMap)[inputString[indexReadSymbol]];
            while (index < code.length())
            {
                if (code[index] == '1')
                {
                    oneByte.set(bit);
                }
                bit--;
                index++;      // Reverse, because bitset.
                if (bit == 255)       // Byte is full.
                {
                    this->data[indexWriteSymbol] = (char)oneByte.to_ulong();
                    oneByte.reset();
                    indexWriteSymbol++;
                    bit = 7;
                }                
            }
            indexReadSymbol++;
            leftSize--;
        }
        delete inputString;
    }
    if (bit != 7)       // If last symbol in fileIn and bitset not reset.
    {
        this->data[indexWriteSymbol] = (char)oneByte.to_ulong();
        indexWriteSymbol++;
    }
    return indexWriteSymbol;
}

/// Encode function.
int Haffman::Encode(bool isShowInfo)
{
    if (statusOk)
    {
        this->symbolsMap = GetSymbolsMap();
        if (isShowInfo) cout << "\n Count different symbols = " << symbolsMap->size();
        this->treeNode = GetHaffmanTree(this->symbolsMap);
        if (isShowInfo) cout << "\n Count level of tree = " << (int)treeNode->c;
        GetSymbolsCode(this->treeNode, this->codesMap, "");
        if (isShowInfo) cout << "\n Create symbols code.";
        this->outputFileSize = EncodeData();
        if (isShowInfo) cout << "\n Data was encode.";
        WriteToFile(this->data, this->outputFileSize);
        if (isShowInfo) cout << "\n Data was write to output file.";
    }
    return outputFileSize;
}
//
//Decode.


/// Get size of output file.
int Haffman::GetOutputFileSize()
{
    int size = 0;
    for (byte i = 0; i < 4; i++)
    {
        size += ((int)this->inputFile.get()) << (24 - i * 8);
    }
    return size;
}

/// Get bits string
string Haffman::GetBitsString(char *data, int dataSize)
{
    string bitsString = "";
    for (int i = 0; i < dataSize; i++)
    {
        bitset<8> oneByte(data[i]);
        bitsString += oneByte.to_string();
    }
    return bitsString;
}

/// Delete excess null.
string Haffman::NormalizeCode(string code)
{
    byte i = 0;
    while (code[i++] != '1');
    return code.substr(i, code.length() - i);
}

/// Get encode tree from file and decode it.
int Haffman::DecodeTree(char *data, map<string, char> *charMap)
{
    this->outputFileSize = GetOutputFileSize();
    byte codeSize = this->inputFile.get();
    int treeSize = 256 * codeSize / 8;      // Size in bytes.
    char *tree = ReadFromFile(treeSize);
    string bitsString = GetBitsString(tree, treeSize);

    // Convert bits string to codes.
    byte i = 0;
    string code;
    do
    {
        code = bitsString.substr(i * codeSize, codeSize);
        code = NormalizeCode(code);
        if (code != "")
        {
            map<string, char>::value_type *valueType =
                 new map<string, char>::value_type(code, (char)i);
            charMap->insert(*valueType);
        }
    } while (i++ < 255);
    return treeSize + 5;
}

/// Decode data and return it size.
int Haffman::DecodeData()
{
    char *inputString = NULL;
    string bitsString;
    this->data = (char*)calloc(inputFileSize * 1.5, 1);
    int indexReadSymbol, indexWriteSymbol = 0;
    int leftSize = this->inputFileSize - DecodeTree(this->data, this->charMap);

    while (leftSize)
    {
        if (this->inputFile.eof())
        {
            break;
        }
        if (leftSize < blockSize)
        {
            inputString = ReadFromFile(leftSize);
            bitsString =  GetBitsString(inputString, leftSize);
            leftSize = 0;
        }
        else
        {
            inputString = ReadFromFile(blockSize);
            bitsString =  GetBitsString(inputString, blockSize);
            leftSize -= blockSize;
        }
        indexReadSymbol = 0;
        while (indexReadSymbol < bitsString.length() && indexWriteSymbol < this->outputFileSize)
        {
            byte i = 1;
            map<string, char>::const_iterator constIt;
            do
            {
                constIt = charMap->find(bitsString.substr(indexReadSymbol, i));
                i++;
            } while (constIt == charMap->end());
            this->data[indexWriteSymbol] = constIt->second;
            indexWriteSymbol++;
            indexReadSymbol += i - 1;
        }
    }
    return indexWriteSymbol;
}

/// Decode function.
int Haffman::Decode(bool isShowInfo)
{
    if (statusOk)
    {
        this->outputFileSize = DecodeData();
        if (isShowInfo) cout << "\n Data was decode.";
        WriteToFile(this->data, this->outputFileSize);
        if (isShowInfo) cout << "\n Data was write to output file.";
    }
    return outputFileSize;
}
