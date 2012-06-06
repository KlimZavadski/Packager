// Packager.cpp
#include "Packager.h"

#define blockSize 1048576   // = 1MB.
#include <time.h>


/// Constructor.
Packager::Packager(string fileName)
{
    this->inputFile.open(fileName.c_str(), ios::in | ios::binary);
    this->inputFileSize = 0;
    if (!inputFile)
    {
        this->status = false;
        return;
    }
    this->inputFileSize = GetFileSize(&inputFile);
    
    this->fileName = fileName.substr(0, fileName.length() - 4) + "_pack.exe";
    this->outputFile.open(this->fileName.c_str(), ios::out | ios::binary);
    this->outputFileSize = 0;
    if (!outputFile)
    {
        this->status = false;
        return;
    }
    
    this->maxCodeSize = 0;
    this->symbolsMap = (Node**)calloc(511, sizeof(Node*));  // Sum all Nodes up to root.
    byte symbol = 0;
    do 
    {
        this->symbolsMap[(byte)symbol] = new Node();
    } while (++symbol);
    this->countSymbols = 0;
    this->treeSize = 0;
    this->treeRoot = NULL;
    this->clocksCount = 0;
}

/// Destructor.
Packager::~Packager()
{
    if (this->inputFileSize && this->outputFileSize)
    {
        for (int i = 0; i < 511; i++)
        {
            if (this->symbolsMap[i])
            {
                delete this->symbolsMap[i];
            }
        }
    }
    delete this->symbolsMap;
}

//
/// Read count bytes from fileIn.
char* Packager::ReadFromFile(unsigned long count)
{
    char *buffer = (char*)calloc(count, 1);
    this->inputFile.read(buffer, count);
    return buffer;
}

/// Write count bytes to fileOut.
unsigned int Packager::WriteToFile(char *buffer, unsigned long count)
{
    this->outputFile.write(buffer, count);
    return count;
}

/// Return file size.
unsigned int Packager::GetFileSize(ifstream *file)
{
    unsigned int pos = file->tellg();
    file->seekg(0, ios_base::end);
    unsigned int size = file->tellg();
    file->clear();
    file->seekg(pos);
    return size;
}
//
// Encode. Version 5. Ok.


/// Write unpackage Code into output file.
void Packager::WriteUnpackageCode()
{
    ifstream unpackageFile;
    unpackageFile.open("Unpackager.exe", ios_base::binary);
    unsigned int unpackageSize = GetFileSize(&unpackageFile);
    char *buffer = new char[unpackageSize];
    unpackageFile.read(buffer, unpackageSize);
    WriteToFile(buffer, unpackageSize);
    delete buffer;
}

/// Return a map that include all characters from file.
void Packager::GetSymbolsMap()
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
Node* Packager::FindLeastNode(int lastNodeIndex)
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
Node* Packager::GetTree()
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

        this->symbolsMap[lastNodeIndex] = Node::GetNewNode(node1, node2);
        actualCount--;
    }
    return this->symbolsMap[lastNodeIndex];
}

/// Create a binary code for all coding symbols.
void Packager::GetSymbolsCode(Node *node, string code)
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
char* Packager::GetBytesString(char *inputData, unsigned long inputSize, unsigned long &outputSize)
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
string Packager::NormalizeCode(string code, byte maxCodeSize)
{
    byte len = code.length();
    char nul[] = "0000000000000000";
    byte pos = maxCodeSize - len;
    nul[pos] = '1';
    nul[pos + 1] = '\0';
    return nul + code;
}

/// Package tree.
unsigned int Packager::PackageTree()
{
    // Create Packager data.
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

/// Package data and return it size.
unsigned int Packager::PackageData()
{
    this->treeSize = PackageTree();
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
void Packager::ShowTimeTable()
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
unsigned int Packager::Package(bool isShowInfo)
{
    if (status)
    {
        WriteUnpackageCode();
        this->clocks[this->clocksCount++] = clock();
        GetSymbolsMap();
        if (isShowInfo) cout << "\n Count different symbols = " << this->countSymbols;
        this->clocks[this->clocksCount++] = clock();
        this->treeRoot = GetTree();
        if (isShowInfo) cout << "\n Tree build.";
        this->clocks[this->clocksCount++] = clock();
        GetSymbolsCode(this->treeRoot, "");
        if (isShowInfo) cout << "\n Create symbols code.";
        this->clocks[this->clocksCount++] = clock();
        this->outputFileSize = PackageData();
        if (isShowInfo)
        {
            printf("\n Data was encode:\n\tSize of tree = %d bytes", this->treeSize);
            printf("\n\tEncode file size =  %d bytes\n\n", this->outputFileSize);
        }
        this->clocks[this->clocksCount++] = clock();
        this->inputFile.close();
        this->outputFile.close();
        return this->outputFileSize;
    }
    else
    {
        cout << "\n\n\tIncorrect input parametrs.\n\n";
        return 0;
    }    
}

