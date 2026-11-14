#include "mini/ir/Operation.h"
#include "mini/ir/Value.h"
#include "mini/ir/Verify.h"

#include <cassert>
#include <string>

int main() {
  using mini::ir::Operation;
  using mini::ir::Type;
  using mini::ir::Value;

  const Type tensor4 = Type::tensor({4});
  const Type tensor8 = Type::tensor({8});
  const Type tensor6 = Type::tensor({6});
  Value left(tensor4);
  Value right(tensor4);
  Value wrongShape(tensor8);
  Value unalignedLength(tensor6);
  const Type matrix16x32 = Type::tensor({16, 32});
  const Type matrix32x16 = Type::tensor({32, 16});
  const Type matrix16x16 = Type::tensor({16, 16});
  Value matrixA(matrix16x32);
  Value matrixB(matrix32x16);
  Value matrixWrongK(matrix16x16);
  std::string error;

  {
    Operation valid("mini.add", {&left, &right}, {tensor4});
    assert(mini::ir::verifyOperation(valid, error));
    assert(error.empty());
  }

  {
    Operation mismatchedInputs("mini.add", {&left, &wrongShape}, {tensor4});
    assert(!mini::ir::verifyOperation(mismatchedInputs, error));
    assert(!error.empty());
  }

  {
    Operation wrongResult("mini.add", {&left, &right}, {tensor8});
    assert(!mini::ir::verifyOperation(wrongResult, error));
    assert(!error.empty());
  }

  {
    Operation missingOperand("mini.add", {&left}, {tensor4});
    assert(!mini::ir::verifyOperation(missingOperand, error));
    assert(!error.empty());
  }

  {
    Operation noResult("mini.add", {&left, &right}, {});
    assert(!mini::ir::verifyOperation(noResult, error));
    assert(!error.empty());
  }

  {
    Operation badLength("mini.add", {&unalignedLength, &unalignedLength},
                        {tensor6});
    assert(!mini::ir::verifyOperation(badLength, error));
    assert(!error.empty());
  }

  {
    Operation validRelu("mini.relu", {&left}, {tensor4});
    assert(mini::ir::verifyOperation(validRelu, error));
    assert(error.empty());
  }

  {
    Operation invalidRelu("mini.relu", {&left}, {tensor8});
    assert(!mini::ir::verifyOperation(invalidRelu, error));
    assert(error.find("mini.relu") != std::string::npos);
  }

  {
    Operation validMatmul("mini.matmul", {&matrixA, &matrixB},
                          {matrix16x16});
    assert(mini::ir::verifyOperation(validMatmul, error));
    assert(error.empty());
  }

  {
    Operation invalidMatmul("mini.matmul", {&matrixA, &matrixWrongK},
                            {matrix16x16});
    assert(!mini::ir::verifyOperation(invalidMatmul, error));
    assert(error.find("mini.matmul") != std::string::npos);
  }
}
