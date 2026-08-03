#pragma once

#include <string>
#include <vector>

// Simulates generated three-address instructions and prints each intermediate result.
class Simulator {
public:
    // Executes three-address instructions using a simple default variable table.
    static void simulateThreeAddressCode(const std::vector<std::string>& instructions);

    // Executes two-address instructions using a simple R1-based register model.
    static void simulateTwoAddressCode(const std::vector<std::string>& instructions);

    // Executes one-address instructions using AC as the accumulator.
    static void simulateOneAddressCode(const std::vector<std::string>& instructions);

private:
    // Returns a default value for a variable when no explicit value is provided.
    static int defaultValueForVariable(char variableName);

    // Returns true if the token is a temporary name such as t1 or t2.
    static bool isTemporaryName(const std::string& name);

    // Resolves a variable or temporary to its current numeric value.
    static int resolveValue(const std::string& token, const std::vector<std::pair<std::string, int>>& values);

    // Applies an arithmetic operation to two operands.
    static int applyOperation(int leftOperand, int rightOperand, char operation);
};
