#pragma once

#include <string>
#include <vector>

// Generates three-address code instructions from postfix expressions.
class InstructionGenerator {
public:
    // Converts a postfix expression into a list of three-address code statements.
    static std::vector<std::string> generateThreeAddressCode(const std::string& postfixExpression);

    // Converts a postfix expression into a list of two-address instructions.
    static std::vector<std::string> generateTwoAddressCode(const std::string& postfixExpression);

    // Converts a postfix expression into a list of one-address accumulator instructions.
    static std::vector<std::string> generateOneAddressCode(const std::string& postfixExpression);

    // Converts a postfix expression into a list of zero-address stack instructions.
    static std::vector<std::string> generateZeroAddressCode(const std::string& postfixExpression);

private:
    // Returns true when the character is one of the supported arithmetic operators.
    static bool isOperator(char character);

    // Creates the next temporary variable name in sequence, such as t1, t2, and so on.
    static std::string createTemporaryName(int temporaryIndex);

    // Returns the mnemonic used by two-address code for an operator.
    static std::string instructionMnemonic(char character);
};
