/*
* This is a calculator object that allows you to perform arithmetic operations on very large
* numbers by using a string form of the values and calculating on a digit-by-digit basis.
* Currently only supports calculating the exponential of large numbers, but further functionality
* will be added soon.
*/

#include "calculator.h"
#include <algorithm>
#include <iostream>
#include <string>

// Constructor
Calculator::Calculator() : B(0) {}

// Destructor
Calculator::~Calculator() {}

// Calculate main function
void Calculator::Start() {
    string inputVal;
    
    cout << "Calculator started in exponential mode (A ^ B)" << endl;

    cout << "Please enter an integer (0 - 99999) for value of A: ";
    cin >> inputVal;

    inputVal.clear();

    cout << "Please enter an integer (0 - 99999) for value of B: ";
    cin >> inputVal;

    this->A = new BigInt(inputVal);

    this->B = stoi(inputVal);

    // Call exponential function to calculate value
    string multipliedVal = this->A->Exp(this->B);

    // Reverse inputval for output string
    reverse(inputVal.begin(), inputVal.end());

    cout << "Result is: " << multipliedVal << endl;
}
