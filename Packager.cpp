#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;


void main(int argc, char *argv[])
{
	string *commandKey = NULL;
	string packageFileName = argv[argc - 1];

	if (argc > 2)
	{
		commandKey = new string[argc - 2];
		for (int i = 1; i < argc - 1; i++)
		{
			commandKey[i-1] = argv[i];
		}
	}
}