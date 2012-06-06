// Unpackager.cpp
#include "Unpackager.h"

#define blockSize 1048576   // = 1MB.
#include <time.h>


/// Constructor.
Unpackager::Unpackager(string filePath)
{
    this->filePath = filePath;
    this->fileName = GetFileName(filePath);
    this->inputFile.open(this->fileName.c_str(), ios::in | ios::binary);
    this->inputFileSize = 0;
    if (!inputFile)
    {
        this->status = false;
        int code = GetLastError();
        return;
    }
    int size = 18432;   // 58368; 18432;
    this->inputFileSize = GetFileSize(&inputFile) - size;
    this->inputFile.seekg(size);
    
    this->fileName = this->fileName.substr(0, this->fileName.length() - 9)  + "_unpack.exe";
    this->outputFile.open(this->fileName.c_str(), ios::out | ios::binary);
    this->outputFileSize = 0;
    if (!outputFile)
    {
        this->status = false;
        return;
    }

    this->maxCodeSize = 0;
    this->symbolsMap = (Node**)calloc(256, sizeof(Node*));
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
Unpackager::~Unpackager()
{
    if (this->inputFileSize && this->outputFileSize)
    {
        for (int i = 0; i < 256; i++)
        {
            if (this->symbolsMap[i])
            {
                delete this->symbolsMap[i];
            }
        }
        delete this->symbolsMap;
    }
}

//
/// Read count bytes from fileIn.
char* Unpackager::ReadFromFile(unsigned long count)
{
    char *buffer = (char*)calloc(count, 1);
    this->inputFile.read(buffer, count);
    return buffer;
}

/// Write count bytes to fileOut.
unsigned int Unpackager::WriteToFile(char *buffer, unsigned long count)
{
    this->outputFile.write(buffer, count);
    return count;
}

/// Return file size.
unsigned int Unpackager::GetFileSize(ifstream *file)
{
    unsigned int pos = file->tellg();
    file->seekg(0, ios_base::end);
    unsigned int size = file->tellg();
    file->clear();
    file->seekg(pos);
    return size;
}

/// Get file name from path of file.
string Unpackager::GetFileName(string filePath)
{
    int pos = -1, i = 0, len = filePath.length();
    while (i < len)
    {
        if (filePath[i] == '\\') pos = i;
        i++;
    }
    return filePath.substr(pos + 1, len - pos);
}
//
//Decode. Version 4. Ok.


/// Return size of output file.
unsigned int Unpackager::ReadOutputFileSize()
{
    unsigned int size = 0;
    for (byte i = 0; i < 4; i++)
    {
        size += ((unsigned int)this->inputFile.get()) << (24 - i * 8);
    }
    return size;
}

/// Convert bytes string to bits string (decode).
char* Unpackager::GetBitsString(char *inputData, unsigned long inputSize, unsigned long &outputSize)
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
char* Unpackager::NormalizeCode(char *bitsString, byte symbol, byte maxCodeSize)
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
unsigned int Unpackager::GetTree(int)
{
    this->outputFileSize = ReadOutputFileSize();
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

/// Recursive decode Unpackager tree by symbols code.
void Unpackager::UnpackageTree()
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
unsigned int Unpackager::UnpackageData()
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
void Unpackager::ShowTimeTable()
{
    puts("\n\n\t\tTime table:");
    printf("\n\tGet Unpackager tree\t%d ms", this->clocks[1] - this->clocks[0]);
    printf("\n\tDecode tree\t\t%d ms", this->clocks[2] - this->clocks[1]);
    printf("\n\tDecode data\t\t%d ms", this->clocks[3] - this->clocks[2]);
    printf("\n\tWrite data to file\t%d ms", this->clocks[4] - this->clocks[3]);
    printf("\n\n\t\tTotal time = %d ms\n\n", this->clocks[4] - this->clocks[0]);
}

/// Decode function.
unsigned int Unpackager::Unpackage(bool isShowInfo)
{
    if (status)
    {
        this->clocks[this->clocksCount++] = clock();
        this->treeSize = GetTree(0);
        this->clocks[this->clocksCount++] = clock();
        UnpackageTree();
        this->clocks[this->clocksCount++] = clock();
        this->outputFileSize = UnpackageData();
        if (isShowInfo)
        {
            cout << "\n Data was decode:";
            printf("\n\tSize of tree = %d bytes", this->treeSize);
            printf("\n\tDecode file size =  %d bytes\n\n", this->outputFileSize);
        }
        this->clocks[this->clocksCount++] = clock();
        this->inputFile.close();
        this->outputFile.close();
        return outputFileSize;
    }
    else
    {
        cout << "\n\n\tIncorrect input parametrs.\n\n";
        return 0;
    }
}

/// Start in new process unpack file.
void Unpackager::StartProcess()
{
    if (!status) return;

    wchar_t *path = new wchar_t[500];
    mbstowcs(path, this->fileName.c_str(), this->fileName.length());
    path[this->fileName.length()] = '\0';

    STARTUPINFO startupInfo = {0};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInformation;
    if (CreateProcess(path, NULL, NULL, NULL, true, NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInformation))
    {
        WaitForSingleObject(processInformation.hProcess, INFINITE);
    }
    else
    {
        cout << "\n\tError [" << GetLastError() << "]" << endl;
    }
    remove(this->fileName.c_str());
}
