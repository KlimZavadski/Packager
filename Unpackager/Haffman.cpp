// Haffman.cpp
#include "Haffman.h"

#define blockSize 1048576   // = 1MB.
#include <time.h>
#include <math.h>

clock_t getBitsString, getBitsString_e;
clock_t convert, convert_e;


/// Constructor.
Haffman::Haffman(string fileName)
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
    this->inputFile.clear();
    this->inputFile.seekg(18944);
    
    this->outputFile.open((fileName.substr(0, fileName.length() - 9)  + "_unpack.exe").c_str(), ios::out | ios::binary);
    if (!outputFile)
    {
        this->status = false;
        return;
    }
    this->outputFileSize = 0;
    this->maxCodeSize = 0;
    this->treeSize = 0;

    this->symbolsMap = (Node**)calloc(511, sizeof(Node*));  // Sum all Nodes up to root.
    byte symbol = 0;
    do 
    {
        this->symbolsMap[(byte)symbol] = new Node();
    } while (++symbol);
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
//Decode. Version 3. Ok.


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
char* Haffman::NormalizeCode(char *bitsString, byte symbol, byte maxCodeSize)
{
    char *code = new char[maxCodeSize];
    code[0] = 0;
    maxCodeSize++;  // For '1' in start.
    byte i = 0;
    while (i < maxCodeSize - 1)
    {
        if (bitsString[symbol * maxCodeSize + i] != '1')    // While not found '1'. It means that Code is started.
        {
            i++;
        }
        else
        {
            i++;    // Next position after '1'.
            strncpy(code, &(bitsString[symbol * maxCodeSize + i]), maxCodeSize - i);   // Copy the Code.
            code[maxCodeSize - i] = '\0';
            break;
        }
    }
    return code;
}

/// Get decode tree from file and decode it.
unsigned int Haffman::GetHaffmanTree(int)
{
    this->outputFileSize = GetOutputFileSize();
    this->maxCodeSize = this->inputFile.get();
    int sizeOfTree = (this->maxCodeSize + 1) * 256 / 8;      // Size in bytes.
    
    unsigned long n;
    char *encodeTree = ReadFromFile(sizeOfTree);
    char *bitsString = GetBitsString(encodeTree, sizeOfTree, n);
    delete encodeTree;

    // Convert bits string to codes.
    byte symbol = 0;
    do
    {
        char *code = NormalizeCode(bitsString, symbol, this->maxCodeSize);
        if (*code)     // Jump over absent code.
        {
            this->symbolsMap[symbol] = new Node(code);
            this->symbolsMap[symbol]->count = symbol;
            this->symbolsMap[symbol]->actual = true;
            this->countSymbols++;
        }
    } while (++symbol);
    delete bitsString;
    return sizeOfTree + 5;
}

/// Recursive decode haffman tree by symbols code.
void Haffman::DecodeTree()
{
    int lastIndex = 256;
    this->treeRoot = new Node();
    byte symbol = 0;
    do
    {
        if (this->symbolsMap[symbol] == NULL) continue;

        Node *currentNode = this->symbolsMap[symbol];
        Node *node = this->treeRoot;        
        byte len = currentNode->code.length();
        for (int i = 0; i < len; i++)
        {
            if (currentNode->code[i] == '0')    // Go to right.
            {
                if (i == len - 1)   // If find place for Current Node.
                {
                    node->right = currentNode;
                }
                else    // Continue.
                {
                    if (node->right == NULL)    // Create new Node if apsent.
                    {
                        node->right = new Node();
                    }
                    node = node->right;
                }
            }
            else    // Go to left.
            {
                if (i == len - 1)   // If find place for Current Node.
                {
                    node->left = currentNode;
                }
                else    // Continue.
                {
                    if (node->left == NULL)    // Create new Node if apsent.
                    {
                        node->left = new Node();
                    }
                    node = node->left;
                }
            }
        }
    } while (++symbol);
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
            while (!node->actual)
            {
                if (bitsString[indexReadSymbol + i] == '0') node = node->right;
                if (bitsString[indexReadSymbol + i] == '1') node = node->left;
                i++;
            }
            data[indexWriteSymbol] = (char)node->count;
            indexWriteSymbol++;
            indexReadSymbol += i;
        }
        delete bitsString;
    }
    this->clocks[this->clocksCount++] = clock();
    return WriteToFile(data, indexWriteSymbol);
}

/// Show how long algorithm done.
void Haffman::ShowDecodeTimeTable()
{
    puts("\n\n\t\tTime table:");
    printf("\n\tGet haffman tree\t%d ms", this->clocks[1] - this->clocks[0]);
    printf("\n\tDecode tree\t\t%d ms", this->clocks[2] - this->clocks[1]);
    printf("\n\tDecode data\t\t%d ms", this->clocks[3] - this->clocks[2]);
    printf("\n\tWrite data to file\t%d ms", this->clocks[4] - this->clocks[3]);
    printf("\n\n\t\tTotal time = %d ms\n\n", this->clocks[4] - this->clocks[0]);
}

/// Decode function.
unsigned int Haffman::Decode(bool isShowInfo)
{
    if (status)
    {
        this->clocks[this->clocksCount++] = clock();
        this->treeSize = GetHaffmanTree(0);
        this->clocks[this->clocksCount++] = clock();
        DecodeTree();
        this->clocks[this->clocksCount++] = clock();
        this->outputFileSize = DecodeData();
        this->outputFile.close();
        if (isShowInfo)
        {
            cout << "\n Data was decode:";
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
