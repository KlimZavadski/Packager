// Haffman.cpp
#include "Haffman.h"

#define blockSize 1048576   // = 1MB.
#include <time.h>
#include <math.h>

clock_t getBitsString, getBitsString_e;
clock_t convert, convert_e;


/// Constructor.
Haffman::Haffman(string fileName, bool isEncode)
{
    this->fileName = fileName;
    this->inputFile.open(fileName.c_str(), ios::in | ios::binary);
    if (!inputFile)
    {
        this->status = false;
        return;
    }
    this->inputFile.seekg(0, ios_base::end);
    this->inputFileSize = this->inputFile.tellg();
    this->inputFile.seekg(0);
    if (isEncode)
    {
        this->outputFile.open((fileName + ".pac").c_str(), ios::out | ios::binary);
    }
    else
    {
        this->outputFile.open(fileName.substr(0, fileName.length() - 4).c_str(), ios::out | ios::binary);
    }
    if (!outputFile)
    {
        this->status = false;
        return;
    }

    this->outputFileSize = 0;
    this->maxCodeSize = 0;
    this->treeSize = 0;

    this->symbolsMap = (Node**)calloc(511, sizeof(Node*));  // Sum all Nodes up to root.
    for (int i = 0; i < 256; i++)
    {
        this->symbolsMap[i] = new Node;
        this->symbolsMap[i]->count = 0;
        this->symbolsMap[i]->actual = false;
        this->symbolsMap[i]->left = NULL;
        this->symbolsMap[i]->right = NULL;
    }
    this->countSymbols = 0;
    this->treeRoot = NULL;
    this->clocksCount = 0;
}

/// Destructor.
Haffman::~Haffman()
{
    this->inputFile.close();
    this->outputFile.close();

    for (int i = 0; i < 511; i++)
    {
        if (this->symbolsMap[i])
        {
            delete this->symbolsMap[i];
        }
    }
    delete this->symbolsMap;
}

//
/// Read count bytes from fileIn.
char* Haffman::ReadFromFile(unsigned long count)
{
    char *buffer = (char*)calloc(count, 1);
    this->inputFile.read(buffer, count);
    return buffer;
}

/// Write count bytes to fileOut.
int Haffman::WriteToFile(char *buffer, unsigned long count)
{
    this->outputFile.write(buffer, count);
    return count;
}
//
// Encode.


/// Return a map that include all characters from file.
void Haffman::GetSymbolsMap()
{
    // Open input file.
    ifstream file;
    unsigned int positionInFile = 0;
    file.open(fileName.c_str(), ios::in | ios::binary);
    if (file)   // If opened.
    {
        while (!file.eof())
        {
            unsigned int i = 0;
            char *buffer = (char*)calloc(blockSize, 1);
            file.read(buffer, blockSize);
            while (i < blockSize && positionInFile < inputFileSize)
            {
                this->symbolsMap[(byte)buffer[i]]->count++;
                i++;
                positionInFile++;
            }
            delete[] buffer;
        }
        file.close();
    }
    // Search initialize nodes.
    for (int i = 0; i < 256; i++)
    {
        if (this->symbolsMap[i]->count)
        {
            this->symbolsMap[i]->actual = true;
            this->countSymbols++;
        }
    }
}

/// Looking for a Node with a minimum count and return it index.
Node* Haffman::FindLeastNode(int lastNodeIndex)
{
    int minIndex = -1;
    for (int i = 0; i < lastNodeIndex; i++)
    {
        if (this->symbolsMap[i]->actual)
        {
            if (minIndex < 0)
            {
                minIndex = i;
            }
            else
            {
                if (this->symbolsMap[i]->count < this->symbolsMap[minIndex]->count)
                {
                    minIndex = i;
                }
            }
        }
    }
    return this->symbolsMap[minIndex];
}

/// Return a tree that code symbols in map.
Node* Haffman::GetHaffmanTree()
{
    int lastNodeIndex = 255;    // Last used index.
    int actualCount = this->countSymbols;
    while (actualCount > 1)
    {
        lastNodeIndex++;
        Node *node1 = FindLeastNode(lastNodeIndex);  // min_1.
        node1->actual = false;
        Node *node2 = FindLeastNode(lastNodeIndex);  // min_2 >= min_1.
        node2->actual = false;

        this->symbolsMap[lastNodeIndex] = GetNewNode(node1, node2);
        actualCount--;
    }
    return this->symbolsMap[lastNodeIndex];
}

/// Create a binary code for all coding symbols.
void Haffman::GetSymbolsCode(Node *node, string code)
{
    if (node->right != NULL)
    {
        GetSymbolsCode(node->right, code + "0");      // Right code 0.
    }
    if (node->left != NULL)
    {
        GetSymbolsCode(node->left, code + "1");       // Left code 1.
    }
    node->code = code;
    if (node->code.length() > this->maxCodeSize)
    {
        maxCodeSize = node->code.length();
    }
}

/// Convert bits string to bytes string (encode).
char* Haffman::GetBytesString(char *inputData, unsigned long inputSize, unsigned long &outputSize)
{
    unsigned long inputIndex, outputIndex = 0;
    char *outputData = (char*)calloc(inputSize / 8, 1);
    bitset<8> oneByte;
    do 
    {
        for (byte bit = 0; bit < 8; bit++)
        {
            if (inputIndex + bit == inputSize)
            {
                break;
            }
            oneByte.set(7 - bit, (bool)(inputData[inputIndex + bit] - 48));     // Revers, becouse it is bitset.
        }
        inputIndex += 8;
        outputData[outputIndex] = (char)oneByte.to_ulong();
        oneByte.reset();
        outputIndex++;
    } while (inputIndex < inputSize);
    outputSize = outputIndex;
    return outputData;
}

/// Add apsent null, for normalize to maxCodeSize lenght.
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
unsigned int Haffman::EncodeTree()
{
    // Create haffman data.
    char *data = new char[256 * (this->maxCodeSize + 1)];
    data[0] = (char)(this->inputFileSize >> 24);
    data[1] = (char)(this->inputFileSize >> 16);
    data[2] = (char)(this->inputFileSize >> 8);
    data[3] = (char)this->inputFileSize;
    data[4] = this->maxCodeSize;    // Max length of code without '1'.
    WriteToFile(data, 5);

    // Create bits string.
    unsigned int outputIndex = 0;
    byte symbol = 0;
    do 
    {
        string code = NormalizeCode(this->symbolsMap[symbol]->code, this->maxCodeSize);
        for (byte i = 0; i < code.length(); i++)
        {
            data[outputIndex] = code[i];
            outputIndex++;
        }
    } while (++symbol);
    unsigned long outputSize;
    char *compresedData = GetBytesString(data, outputIndex, outputSize);
    WriteToFile(compresedData, outputSize);
    delete compresedData;
    return outputSize + 5;
}

/// Encode data and return it size.
unsigned int Haffman::EncodeData()
{
    this->treeSize = EncodeTree();
    this->clocks[this->clocksCount++] = clock();

    char *data = (char*)calloc(this->maxCodeSize * blockSize, 1);
    char *inputString = NULL;
    unsigned long outputIndex, inputIndex, leftSize = this->inputFileSize;
    unsigned long totalSize = this->treeSize;

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
        inputIndex = 0;
        outputIndex = 0;
        while (inputIndex < blockSize && leftSize)
        {
            string code = this->symbolsMap[(byte)inputString[inputIndex]]->code;
            for (byte i = 0; i < code.length(); i++)
            {
                data[outputIndex] = code[i];
                outputIndex++;
            }
            inputIndex++;
            leftSize--;
        }
        char *compresedData = GetBytesString(data, outputIndex, inputIndex);
        WriteToFile(compresedData, inputIndex);
        totalSize += inputIndex;
        delete compresedData;
        delete inputString;
    }
    return totalSize;
}

/// Show how long algorithm done.
void Haffman::ShowEncodeTimeTable()
{
    puts("\n\n\t\tTime table:");
    printf("\n\tGet symbols map\t%d ms", this->clocks[1] - this->clocks[0]);
    printf("\n\tBuild tree\t\t%d ms", this->clocks[2] - this->clocks[1]);
    printf("\n\tCreate symbols code\t%d ms", this->clocks[3] - this->clocks[2]);
    printf("\n\tEncode and write tree\t%d ms", this->clocks[4] - this->clocks[3]);
    printf("\n\tEncode and write data\t%d ms", this->clocks[5] - this->clocks[4]);
    printf("\n\n\t\tTotal time = %d ms", this->clocks[5] - this->clocks[0]);
}

/// Encode function.
unsigned int Haffman::Encode(bool isShowInfo)
{
    if (status)
    {
        this->clocks[this->clocksCount++] = clock();
        GetSymbolsMap();
        if (isShowInfo) cout << "\n Count different symbols = " << this->countSymbols;
        this->clocks[this->clocksCount++] = clock();
        this->treeRoot = GetHaffmanTree();
        if (isShowInfo) cout << "\n Tree build.";
        this->clocks[this->clocksCount++] = clock();
        GetSymbolsCode(this->treeRoot, "");
        if (isShowInfo) cout << "\n Create symbols code.";
        this->clocks[this->clocksCount++] = clock();
        this->outputFileSize = EncodeData();
        if (isShowInfo)
        {
            printf("\n Data was encode:\n\tSize of tree = %d bytes", this->treeSize);
            printf("\n\tEncode file size =  %d bytes\n\n", this->outputFileSize);
        }
        this->clocks[this->clocksCount++] = clock();
    }
    else
    {
        cout << "\n\n\tIncorrect input parametrs.\n\n";
    }
    return outputFileSize;
}
//
//Decode.


/// Get size of output file.
unsigned int Haffman::GetOutputFileSize()
{
    unsigned int size = 0;
    for (byte i = 0; i < 4; i++)
    {
        size += ((unsigned int)this->inputFile.get()) << (24 - i * 8);
    }
    return size;
}

/// Convert bytes string to bits string (decode).
char* Haffman::GetBitsString(char *inputData, unsigned long inputSize, unsigned long &outputSize)
{
    char *outputData = (char*)calloc(inputSize * 8, 1);
    int i, j;
    for (i = 0; i < inputSize; i++)
    {
        bitset<8> oneByte(inputData[i]);
        for (j = 0; j < 8; j++)
        {
            outputData[i * 8 + j] = oneByte[7 - j] + 48;
        }
    }
    outputSize = inputSize * 8;
    return outputData;
}

/// Delete excess null.
string Haffman::NormalizeCode(char *bitsString, byte symbol, byte codeSize)
{
    bool flag = false;
    byte i = 0;
    string code = "";
    while (i < codeSize)
    {
        if (flag)
        {
            code += bitsString[symbol * codeSize + i];
        }
        if (!flag && bitsString[symbol * codeSize + i] == '1')  // Remove first 1.
        {
            flag = true;
        }
        i++;
    }
    return code;
}

/// Get decode tree from file and decode it.
unsigned int Haffman::DecodeTree()
{
    this->outputFileSize = GetOutputFileSize();
    byte codeSize = this->inputFile.get();
    this->maxCodeSize = codeSize;
    int sizeOfTree = codeSize * 256 / 8;      // Size in bytes.
    unsigned long n;
    char *encodeTree = ReadFromFile(sizeOfTree);
    char *bitsString = GetBitsString(encodeTree, sizeOfTree, n);

    // Convert bits string to codes.
    byte symbol = 0;
    do
    {
        string code = NormalizeCode(bitsString, symbol, codeSize);
        if (code != "")     // Jump over absent code.
        {
            this->symbolsMap[symbol]->code = code;
            this->symbolsMap[symbol]->actual = true;
            this->countSymbols++;
        }
    } while (++symbol);
    delete encodeTree;
    delete bitsString;
    return sizeOfTree + 5;
}

/////
//map<string, char>* Create(Node **symbolsMap)
//{
//    map<string, char> *charsMap = new map<string, char>;
//    byte symbol = 0;
//    do 
//    {
//        charsMap->insert(map<string, char>::value_type(symbolsMap[symbol]->code, symbol));
//    } while (++symbol);
//    return charsMap;
//}

int* FindNode(Node **symbolsMap, int lastNodeIndex)
{
    bool isFirst = true;
    int j = 0;
    string code;
    int *indexes = new int[2];
    indexes[1] = -1;
    for (int index = 0; index < lastNodeIndex; index++)
    {
        if (symbolsMap[index]->actual)
        {
            if (isFirst)
            {
                symbolsMap[index]->actual = false;
                int last = symbolsMap[index]->code.length() - 1;
                if (symbolsMap[index]->code[last] == '0')
                {
                    code = symbolsMap[index]->code.substr(0, last) + "1";
                    indexes[1] = index;
                    j = 0;
                }
                else
                {
                    code = symbolsMap[index]->code.substr(0, last) + "0";
                    indexes[0] = index;
                    j = 1;
                }
                continue;
            }
            else
            {
                if (symbolsMap[index]->code == code)
                {
                    symbolsMap[index]->actual = false;
                    indexes[j] = index;
                    break;            
                }
            }
        }
    }
    return indexes;
}

/// Recursive decode haffman tree by symbols code.
Node* Haffman::DecodeHaffmanTree()
{
    //Node *node = new Node;
    ////map<string, char>::const_iterator constIt = charsMap->find(code);
    //if (constIt == charsMap->end())
    //{
    //    if (code.length() < this->maxCodeSize)
    //    {
    //        node->symbol = 256;
    //        node->right = DecodeHaffmanTree(charsMap, code + "0");
    //        node->left = DecodeHaffmanTree(charsMap, code + "1");
    //    }
    //    else        // If find node
    //    {
    //        delete node;
    //        return NULL;
    //    }
    //}
    //else
    //{
    //    node->symbol = constIt->second;
    //    node->right = node->left = NULL;
    //}
    //return node;

    int len, lastNodeIndex = 255;    // Last used index.
    int *indexes;
    int count = 0;
    while (true)
    {
        indexes = FindNode(this->symbolsMap, lastNodeIndex);
        if (indexes[1] == -1)   // if second node index contained -1 than first is root.
        {
            break;
        }
        symbolsMap[lastNodeIndex] = new Node;
        symbolsMap[lastNodeIndex]->actual = true;
        symbolsMap[lastNodeIndex]->left = indexes[0];
        symbolsMap[lastNodeIndex]->right = indexes[1];
        len = symbolsMap[indexes[0]]->code.length() - 1;
        symbolsMap[lastNodeIndex]->code = symbolsMap[indexes[0]]->code.substr(0, len);

        delete indexes;
        lastNodeIndex++;
        count++;
    }
    return symbolsMap[indexes[0]];
}

/// Decode data and return it size.
unsigned int Haffman::DecodeData()
{
    char *data = (char*)calloc(this->maxCodeSize * blockSize, 1);
    char *bitsString = NULL;
    unsigned long indexReadSymbol, indexWriteSymbol = 0;
    unsigned long bitsStringSize, leftSize = this->inputFileSize - this->treeSize;

    while (leftSize)
    {
        if (this->inputFile.eof())
        {
            break;
        }
        if (leftSize < blockSize)
        {
            bitsString = GetBitsString(ReadFromFile(leftSize), leftSize, bitsStringSize);
            leftSize = 0;
        }
        else
        {
            bitsString = GetBitsString(ReadFromFile(blockSize), blockSize, bitsStringSize);
            leftSize -= blockSize;
        }
        indexReadSymbol = 0;
        while (indexReadSymbol < bitsStringSize && indexWriteSymbol < this->outputFileSize)
        {
            byte i = 0;
            Node *node = this->treeRoot;
            while (true)
            {
                if (smallNode->symbol != 256) break;
                else
                {
                    if (bitsString[indexReadSymbol + i] == '0') smallNode = smallNode->right;
                    if (bitsString[indexReadSymbol + i] == '1') smallNode = smallNode->left;
                    i++;
                }
            }
            //this->data[indexWriteSymbol] = (char)smallNode->symbol;
            indexWriteSymbol++;
            indexReadSymbol += i;
        }
        delete inputString;
    }
    delete bitsString;
    delete treeRoot;
    return indexWriteSymbol;
}

/// Show how long algorithm done.
void Haffman::ShowDecodeTimeTable()
{
}

/// Decode function.
unsigned int Haffman::Decode(bool isShowInfo)
{
    if (status)
    {
        this->clocks[this->clocksCount++] = clock();
        this->treeSize = DecodeTree();
        this->clocks[this->clocksCount++] = clock();
        //this->treeRoot = DecodeHaffmanTree(Create(this->symbolsMap), "");
        this->treeRoot = DecodeHaffmanTree();
        this->clocks[this->clocksCount++] = clock();
        this->outputFileSize = DecodeData();
        if (isShowInfo) cout << "\n Data was decode:";
        this->clocks[this->clocksCount++] = clock();
        //WriteToFile(this->data, this->outputFileSize);
        if (isShowInfo)
        {
            printf("\n\tSize of tree = %d bytes", this->treeSize);
            printf("\n\tDecode file size =  %d bytes\n\n", this->outputFileSize);
        }
        this->clocks[this->clocksCount++] = clock();
    }
    else
    {
        cout << "\n\n\tIncorrect input parametrs.\n\n";
    }
    return outputFileSize;
}


