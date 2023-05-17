#include "bigInt.h"

class Calculator {

private:
  BigInt *A;
  int B;

  void CalculateValue();

public:
  Calculator();

  ~Calculator();

  void Start();
};
