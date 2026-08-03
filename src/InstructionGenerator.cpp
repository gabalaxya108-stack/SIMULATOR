#include "InstructionGenerator.h"

#include <stack>

bool InstructionGenerator::isOperator(char character) {
	return character == '+' || character == '-' || character == '*' || character == '/';
}

std::string InstructionGenerator::createTemporaryName(int temporaryIndex) {
	return "t" + std::to_string(temporaryIndex);
}

std::vector<std::string> InstructionGenerator::generateThreeAddressCode(const std::string& postfixExpression) {
	std::vector<std::string> instructions;
	std::stack<std::string> operandStack;
	int temporaryIndex = 1;

	// Build TAC one postfix token at a time.
	for (char character : postfixExpression) {
		if (character >= 'a' && character <= 'z') {
			operandStack.push(std::string(1, character));
		} else if (isOperator(character)) {
			if (operandStack.size() < 2) {
				continue;
			}

			std::string rightOperand = operandStack.top();
			operandStack.pop();

			std::string leftOperand = operandStack.top();
			operandStack.pop();

			std::string temporaryName = createTemporaryName(temporaryIndex++);
			instructions.push_back(temporaryName + "=" + leftOperand + character + rightOperand);
			operandStack.push(temporaryName);
		}
	}

	return instructions;
}
