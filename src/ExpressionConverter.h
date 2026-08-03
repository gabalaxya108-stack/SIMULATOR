#pragma once

#include <string>

// Converts a validated infix expression into postfix form using the Shunting Yard algorithm.
class ExpressionConverter {
public:
    // Converts a single infix expression into postfix notation.
    static std::string toPostfix(const std::string& infixExpression);

private:
    // Returns true when the character is one of the supported arithmetic operators.
    static bool isOperator(char character);

    // Returns a higher number for operators with higher precedence.
    static int precedence(char character);
};
