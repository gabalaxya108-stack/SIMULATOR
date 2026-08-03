#pragma once

#include <string>

// Validates arithmetic expressions before later processing stages.
class ExpressionValidator {
public:
    static bool isValidExpression(const std::string& expression);

private:
    static bool isOperand(char character);
    static bool isOperator(char character);
};
