#include "ExpressionConverter.h"

#include <stack>

bool ExpressionConverter::isOperator(char character) {
	return character == '+' || character == '-' || character == '*' || character == '/';
}

int ExpressionConverter::precedence(char character) {
	if (character == '+' || character == '-') {
		return 1;
	}

	if (character == '*' || character == '/') {
		return 2;
	}

	return 0;
}

std::string ExpressionConverter::toPostfix(const std::string& infixExpression) {
	std::string postfixExpression;
	std::stack<char> operatorStack;

	for (char character : infixExpression) {
		if (character >= 'a' && character <= 'z') {
			postfixExpression += character;
		} else if (character == '(') {
			operatorStack.push(character);
		} else if (character == ')') {
			while (!operatorStack.empty() && operatorStack.top() != '(') {
				postfixExpression += operatorStack.top();
				operatorStack.pop();
			}

			if (!operatorStack.empty() && operatorStack.top() == '(') {
				operatorStack.pop();
			}
		} else if (isOperator(character)) {
			while (!operatorStack.empty() && isOperator(operatorStack.top()) && precedence(operatorStack.top()) >= precedence(character)) {
				postfixExpression += operatorStack.top();
				operatorStack.pop();
			}

			operatorStack.push(character);
		}
	}

	while (!operatorStack.empty()) {
		if (operatorStack.top() != '(') {
			postfixExpression += operatorStack.top();
		}

		operatorStack.pop();
	}

	return postfixExpression;
}
