#include "InstructionGenerator.h"

#include <stack>

bool InstructionGenerator::isOperator(char character) {
	return character == '+' || character == '-' || character == '*' || character == '/';
}

std::string InstructionGenerator::createTemporaryName(int temporaryIndex) {
	return "t" + std::to_string(temporaryIndex);
}

std::string InstructionGenerator::instructionMnemonic(char character) {
	switch (character) {
	case '+':
		return "ADD";
	case '-':
		return "SUB";
	case '*':
		return "MUL";
	case '/':
		return "DIV";
	default:
		return "";
	}
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

std::vector<std::string> InstructionGenerator::generateTwoAddressCode(const std::string& postfixExpression) {
	std::vector<std::string> instructions;
	std::stack<std::string> operandStack;
	int temporaryIndex = 1;

	// Build two-address instructions one postfix token at a time.
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

			instructions.push_back("MOV R1," + leftOperand);
			instructions.push_back(instructionMnemonic(character) + " R1," + rightOperand);

			std::string temporaryName = createTemporaryName(temporaryIndex++);
			instructions.push_back("MOV " + temporaryName + ",R1");
			operandStack.push(temporaryName);
		}
	}

	return instructions;
}

std::vector<std::string> InstructionGenerator::generateOneAddressCode(const std::string& postfixExpression) {
	std::vector<std::string> instructions;
	std::stack<std::string> operandStack;
	int temporaryIndex = 1;

	// Build one-address accumulator instructions one postfix token at a time.
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
			instructions.push_back("LOAD " + leftOperand);
			instructions.push_back(instructionMnemonic(character) + " " + rightOperand);
			instructions.push_back("STORE " + temporaryName);
			operandStack.push(temporaryName);
		}
	}

	return instructions;
}

std::vector<std::string> InstructionGenerator::generateZeroAddressCode(const std::string& postfixExpression) {
	std::vector<std::string> instructions;
	int temporaryIndex = 1;

	// Translate postfix tokens directly into stack-machine instructions.
	for (char character : postfixExpression) {
		if (character >= 'a' && character <= 'z') {
			instructions.push_back("PUSH " + std::string(1, character));
		} else if (isOperator(character)) {
			instructions.push_back(instructionMnemonic(character));
		}
	}

	// Store the final stack result in a temporary so the output is explicit.
	instructions.push_back("POP " + createTemporaryName(temporaryIndex));

	return instructions;
}
