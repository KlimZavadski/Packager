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
    this->inputFile.clear();
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
    /*for (int i = 0; i < 256; i++)
    {
        this->symbolsMap[i] = new Node();
    }*/
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
// Encode. Version 3. Ok.


/// Return a map that include all characters from file.
void Haffman::GetSymbolsMap()
{
    // Open input file.
    unsigned int positionInFile = 0;
    while (!this->inputFile.eof())
    {
        unsigned int i = 0;
        char *buffer = (char*)calloc(blockSize, 1);
        this->inputFile.read(buffer, blockSize);
        while (i < blockSize && positionInFile < inputFileSize)
        {
            if (this->symbolsMap[(byte)buffer[i]] == NULL)
            {
                this->symbolsMap[(byte)buffer[i]] = new Node();
            }
            this->symbolsMap[(byte)buffer[i]]->count++;
            i++;
            positionInFile++;
        }
        delete[] buffer;
    }
    this->inputFile.clear();
    this->inputFile.seekg(0);
    
    // Search initialize nodes.
    byte symbol = 0;
    do 
    {
        if (this->symbolsMap[symbol]->count)
        {
            this->symbolsMap[symbol]->actual = true;
            this->countSymbols++;
        }
    } while (++symbol);
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
    else
    {
        node->code = code;
        byte len = code.length();
        if (len > this->maxCodeSize)
        {
            maxCodeSize = len;
        }
    }
}

/// Convert bits string to bytes string (encode).
char* Haffman::GetBytesString(char *inputData, unsigned long inputSize, unsigned long &outputSize)
{
    unsigned long inputIndex = 0, outputIndex = 0;
    char *outputData = (char*)calloc(inputSize / 8 + 1, 1);
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
    byte len = code.length();
    char nul[] = "0000000000000000";
    byte pos = maxCodeSize - len;
    nul[pos] = '1';
    nul[pos + 1] = '\0';
    return nul + code;
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
    data[4] = this->maxCodeSize;    // Max length of code without added '1'.
    WriteToFile(data, 5);

    // Create bits string.
    unsigned int outputIndex = 0;
    byte symbol = 0;
    do 
    {
        string code = NormalizeCode(this->symbolsMap[symbol]->code, this->maxCodeSize);
        byte len = code.length();
        strncpy(&(data[outputIndex]), code.c_str(), len);
        outputIndex += len;
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
    unsigned int totalSize = this->treeSize;

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
            byte len = code.length();
            strncpy(&(data[outputIndex]), code.c_str(), len);
            outputIndex += len;
            
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
    printf("\n\tGet symbols map\t\t%d ms", this->clocks[1] - this->clocks[0]);
    printf("\n\tBuild tree\t\t%d ms", this->clocks[2] - this->clocks[1]);
    printf("\n\tCreate symbols code\t%d ms", this->clocks[3] - this->clocks[2]);
    printf("\n\tEncode and write tree\t%d ms", this->clocks[4] - this->clocks[3]);
    printf("\n\tEncode and write data\t%d ms", this->clocks[5] - this->clocks[4]);
    printf("\n\n\t\tTotal time = %d ms\n\n", this->clocks[5] - this->clocks[0]);
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
    return this->outputFileSize;
}
//
//Decode. Version 2. Ok.


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


