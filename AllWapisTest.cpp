// AllWapisTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "MainHeader.h"
#include <iostream>

using namespace std;
int main()
{
    cout << ">>> Start\n";
    cout << "\n>>> Processes\n"; processes();
    //cout << "\n>>> Network\n"; network();
    cout << "\n>>> Registry\n"; registry();
    cout << "\n>>> Mutexes\n"; mutexes();
    cout << "\n>>> Filesystem\n"; filesystem();
    cout << "\n>>> Service\n"; service();
    cout << "\n<<< End\n";
    cin.get();
}

