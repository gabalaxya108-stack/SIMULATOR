#include <iostream>
#include <string>
#include <vector>

#include "ExpressionConverter.h"
#include "ExpressionValidator.h"
#include "InstructionGenerator.h"

// Entry point for the console application.
// Reads one infix expression, validates it, and prints postfix and three-address code output.
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

    std::vector<std::string> instructions = InstructionGenerator::generateThreeAddressCode(postfixExpression);
    for (const std::string& instruction : instructions) {
        std::cout << instruction << std::endl;
    }

    return 0;
}
