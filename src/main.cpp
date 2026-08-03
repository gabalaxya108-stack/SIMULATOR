#include <iostream>
#include <string>

#include "ExpressionConverter.h"
#include "ExpressionValidator.h"

// Entry point for the console application.
// Reads one infix expression, validates it, and prints the postfix result.
int main() {
    std::string expression;

    std::cout << "Enter an arithmetic expression: ";
    std::getline(std::cin, expression);

    if (!ExpressionValidator::isValidExpression(expression)) {
        std::cout << "Invalid expression. Please use only lowercase operands, + - * /, and balanced parentheses." << std::endl;
        return 1;
    }

    std::cout << "Original Expression: " << expression << std::endl;
    std::cout << "Postfix Expression: " << ExpressionConverter::toPostfix(expression) << std::endl;

    return 0;
}
