#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BigInt {

private:
  vector<int> num;

public:
  BigInt(string val);

  ~BigInt();

  vector<int> GetNum();

  vector<int> MultiplyNum(vector<int> toMult);

  string Exp(int power);
};
