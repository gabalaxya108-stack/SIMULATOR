#include "Simulator.h"

#include <iostream>
#include <sstream>

namespace {

// Finds a stored value by name in the simulation state.
bool findValue(const std::vector<std::pair<std::string, int>>& values, const std::string& name, int& outValue) {
	for (const auto& entry : values) {
		if (entry.first == name) {
			outValue = entry.second;
			return true;
		}
	}

	return false;
}

// Stores or updates a value in the simulation state.
void setValue(std::vector<std::pair<std::string, int>>& values, const std::string& name, int value) {
	for (auto& entry : values) {
		if (entry.first == name) {
			entry.second = value;
			return;
		}
	}

	values.emplace_back(name, value);
}

// Prints the current state after each instruction.
void printState(const std::vector<std::pair<std::string, int>>& values) {
	std::cout << "Current Values: ";

	for (std::size_t index = 0; index < values.size(); ++index) {
		std::cout << values[index].first << '=' << values[index].second;
		if (index + 1 < values.size()) {
			std::cout << " ";
		}
	}

	std::cout << std::endl;
}

// Prints the current register and temporary state after each two-address instruction.
void printTwoAddressState(int registerValue, const std::vector<std::pair<std::string, int>>& values) {
	std::cout << "Current R1: " << registerValue;

	if (!values.empty()) {
		std::cout << " | Temps: ";
		for (std::size_t index = 0; index < values.size(); ++index) {
			std::cout << values[index].first << '=' << values[index].second;
			if (index + 1 < values.size()) {
				std::cout << " ";
			}
		}
	}

	std::cout << std::endl;
}

// Prints the current accumulator and temporary state after each one-address instruction.
void printOneAddressState(int accumulatorValue, const std::vector<std::pair<std::string, int>>& values) {
	std::cout << "Current AC: " << accumulatorValue;

	if (!values.empty()) {
		std::cout << " | Temps: ";
		for (std::size_t index = 0; index < values.size(); ++index) {
			std::cout << values[index].first << '=' << values[index].second;
			if (index + 1 < values.size()) {
				std::cout << " ";
			}
		}
	}

	std::cout << std::endl;
}

// Resolves a token for the two-address simulator.
int resolveTwoAddressToken(const std::string& token, int registerValue, const std::vector<std::pair<std::string, int>>& values) {
	if (token == "R1") {
		return registerValue;
	}

	int storedValue = 0;
	if (findValue(values, token, storedValue)) {
		return storedValue;
	}

	if (token.size() == 1 && token[0] >= 'a' && token[0] <= 'z') {
		return (token[0] - 'a') + 1;
	}

	return 0;
}

// Resolves a token for the one-address simulator.
int resolveOneAddressToken(const std::string& token, int accumulatorValue, const std::vector<std::pair<std::string, int>>& values) {
	if (token == "AC") {
		return accumulatorValue;
	}

	int storedValue = 0;
	if (findValue(values, token, storedValue)) {
		return storedValue;
	}

	if (token.size() == 1 && token[0] >= 'a' && token[0] <= 'z') {
		return (token[0] - 'a') + 1;
	}

	return 0;
}

} // namespace

int Simulator::defaultValueForVariable(char variableName) {
	return (variableName - 'a') + 1;
}

bool Simulator::isTemporaryName(const std::string& name) {
	return name.size() >= 2 && name[0] == 't';
}

int Simulator::resolveValue(const std::string& token, const std::vector<std::pair<std::string, int>>& values) {
	int storedValue = 0;
	if (findValue(values, token, storedValue)) {
		return storedValue;
	}

	if (token.size() == 1 && token[0] >= 'a' && token[0] <= 'z') {
		return defaultValueForVariable(token[0]);
	}

	return 0;
}

int Simulator::applyOperation(int leftOperand, int rightOperand, char operation) {
	switch (operation) {
	case '+':
		return leftOperand + rightOperand;
	case '-':
		return leftOperand - rightOperand;
	case '*':
		return leftOperand * rightOperand;
	case '/':
		return rightOperand == 0 ? 0 : leftOperand / rightOperand;
	default:
		return 0;
	}
}

void Simulator::simulateThreeAddressCode(const std::vector<std::string>& instructions) {
	std::vector<std::pair<std::string, int>> values;

	std::cout << "Three Address Simulation:" << std::endl;

	for (const std::string& instruction : instructions) {
		std::size_t equalsPosition = instruction.find('=');
		if (equalsPosition == std::string::npos) {
			continue;
		}

		std::string destination = instruction.substr(0, equalsPosition);
		std::string expression = instruction.substr(equalsPosition + 1);

		if (expression.size() < 3) {
			continue;
		}

		std::string leftToken(1, expression[0]);
		char operation = expression[1];
		std::string rightToken = expression.substr(2);

		int leftValue = resolveValue(leftToken, values);
		int rightValue = resolveValue(rightToken, values);
		int resultValue = applyOperation(leftValue, rightValue, operation);

		setValue(values, destination, resultValue);

		std::cout << instruction << std::endl;
		std::cout << "  " << destination << " = " << resultValue << std::endl;
		printState(values);
	}
}

void Simulator::simulateTwoAddressCode(const std::vector<std::string>& instructions) {
	std::vector<std::pair<std::string, int>> values;
	int registerValue = 0;

	std::cout << "Two Address Simulation:" << std::endl;

	for (const std::string& instruction : instructions) {
		std::istringstream stream(instruction);
		std::string operation;
		std::string destination;
		std::string source;

		stream >> operation >> destination;

		if (operation == "MOV") {
			std::size_t commaPosition = destination.find(',');
			if (commaPosition == std::string::npos) {
				continue;
			}

			std::string target = destination.substr(0, commaPosition);
			source = destination.substr(commaPosition + 1);

			int sourceValue = resolveTwoAddressToken(source, registerValue, values);

			if (target == "R1") {
				registerValue = sourceValue;
			} else {
				setValue(values, target, sourceValue);
			}
		} else if (operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV") {
			std::size_t commaPosition = destination.find(',');
			if (commaPosition == std::string::npos) {
				continue;
			}

			source = destination.substr(commaPosition + 1);
			int sourceValue = resolveTwoAddressToken(source, registerValue, values);

			switch (operation[0]) {
			case 'A':
				registerValue += sourceValue;
				break;
			case 'S':
				registerValue -= sourceValue;
				break;
			case 'M':
				registerValue *= sourceValue;
				break;
			case 'D':
				registerValue = sourceValue == 0 ? 0 : registerValue / sourceValue;
				break;
			default:
				break;
			}
		} else {
			continue;
		}

		std::cout << instruction << std::endl;
		printTwoAddressState(registerValue, values);
	}
}

void Simulator::simulateOneAddressCode(const std::vector<std::string>& instructions) {
	std::vector<std::pair<std::string, int>> values;
	int accumulatorValue = 0;

	std::cout << "One Address Simulation:" << std::endl;

	for (const std::string& instruction : instructions) {
		std::istringstream stream(instruction);
		std::string operation;
		std::string operand;

		stream >> operation >> operand;

		if (operation == "LOAD") {
			accumulatorValue = resolveOneAddressToken(operand, accumulatorValue, values);
		} else if (operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV") {
			int operandValue = resolveOneAddressToken(operand, accumulatorValue, values);

			switch (operation[0]) {
			case 'A':
				accumulatorValue += operandValue;
				break;
			case 'S':
				accumulatorValue -= operandValue;
				break;
			case 'M':
				accumulatorValue *= operandValue;
				break;
			case 'D':
				accumulatorValue = operandValue == 0 ? 0 : accumulatorValue / operandValue;
				break;
			default:
				break;
			}
		} else if (operation == "STORE") {
			setValue(values, operand, accumulatorValue);
		} else {
			continue;
		}

		std::cout << instruction << std::endl;
		printOneAddressState(accumulatorValue, values);
	}
}
