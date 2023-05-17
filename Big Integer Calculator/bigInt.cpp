#include "bigInt.h"

using namespace std;

// Constructor
BigInt::BigInt(string val) {
  // Initialise local num vector with 0s, same length as input val
  this->num = vector<int>(val.size(), 0);

  // Populate vector
  for (int i = 0; i < val.size(); i++) {
    this->num[i] = val[i] - '0';
  }
}

// Destructor
BigInt::~BigInt() {}

// Multiplication function
vector<int> BigInt::MultiplyNum(vector<int> num2) {
  // t1 and t2 are placeholders for the index from num and num2 values
  // carry holds the carryover value used in multiplication
  int t1, t2, carry = 0;

  // Initialise results vector with 0s, max length is both vals' lengths added
  // up
  vector<int> result(this->num.size() + num2.size(), 0);

  t1 = 0;

  // Loop through both num and num2 and multiply the numbers by their base
  for (int i = this->num.size() - 1; i >= 0; i--) {
    t2 = 0;
    for (int j = num2.size() - 1; j >= 0; j--) {
      int val = (this->num[i] * num2[j]) + result[t1 + t2] + carry;
      carry = val / 10;
      result[t1 + t2] = val % 10;
      t2++;
    }
    if (carry > 0) {
      result[t1 + t2] += carry;
      carry = 0;
    }
    t1++;
  }

  // The results value is stored with bases inverted, so ignore redundant 0s at
  // the end of the vector (0s in front of the highest base of the results
  // value)
  int i = result.size() - 1;
  while (result[i] == 0) {
    i--;
  }
  vector<int>::const_iterator s = result.begin();
  vector<int>::const_iterator e = result.begin() + i + 1;

  vector<int> retVector(s, e);
  reverse(retVector.begin(), retVector.end());

  this->num = retVector;

  return retVector;
}

// Exponential function
string BigInt::Exp(int power) {
  // Keep placeholder of original value
  vector<int> temp1 = this->num;

  // Check power values
  if (power == 0) {
    return "1";
  } else if (power >= 2) {
    // Loop multiply the number with itself for the exponential function
    for (int i = 0; i < power - 1; i++) {
      this->MultiplyNum(temp1);
    }
  }

  // Return the number as a string
  string ret;

  for (int i = 0; i < this->num.size(); i++) {
    ret += to_string(this->num[i]);
  }
  return ret;
}

// Getter function for the num vector
vector<int> BigInt::GetNum() { return this->num; }
