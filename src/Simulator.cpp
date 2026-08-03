#include "Simulator.h"

#include <iomanip>
#include <iostream>

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
