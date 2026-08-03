#include <iostream>
#include <string>
#include <vector>

#include "ExpressionConverter.h"
#include "ExpressionValidator.h"
#include "InstructionGenerator.h"
#include "Simulator.h"

// Entry point for the console application.
// Reads one infix expression, validates it, and prints postfix, three-address code, two-address code, one-address code, and zero-address code output.
int main() {
    std::string expression;

    std::cout << "Enter an arithmetic expression: ";
    std::getline(std::cin, expression);

    if (!ExpressionValidator::isValidExpression(expression)) {
        std::cout << "Invalid expression. Please use only lowercase operands, + - * /, and balanced parentheses." << std::endl;
        return 1;
    }

    std::cout << "Original Expression: " << expression << std::endl;

    std::string postfixExpression = ExpressionConverter::toPostfix(expression);
    std::cout << "Postfix Expression: " << postfixExpression << std::endl;

    std::vector<std::string> threeAddressInstructions = InstructionGenerator::generateThreeAddressCode(postfixExpression);
    std::cout << "Three Address Code:" << std::endl;
    for (const std::string& instruction : threeAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    Simulator::simulateThreeAddressCode(threeAddressInstructions);

    std::vector<std::string> twoAddressInstructions = InstructionGenerator::generateTwoAddressCode(postfixExpression);
    std::cout << "Two Address Code:" << std::endl;
    for (const std::string& instruction : twoAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    std::vector<std::string> oneAddressInstructions = InstructionGenerator::generateOneAddressCode(postfixExpression);
    std::cout << "One Address Code:" << std::endl;
    for (const std::string& instruction : oneAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    std::vector<std::string> zeroAddressInstructions = InstructionGenerator::generateZeroAddressCode(postfixExpression);
    std::cout << "Zero Address Code:" << std::endl;
    for (const std::string& instruction : zeroAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    return 0;
}
