#include "Simulator.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>

namespace {

// Returns true when color output should be enabled for the current terminal.
bool useColorOutput() {
	return std::getenv("NO_COLOR") == nullptr && isatty(STDOUT_FILENO) != 0;
}

const char* colorCyan() {
	return useColorOutput() ? "\033[36m" : "";
}

const char* colorGreen() {
	return useColorOutput() ? "\033[32m" : "";
}

const char* colorYellow() {
	return useColorOutput() ? "\033[33m" : "";
}

const char* colorReset() {
	return useColorOutput() ? "\033[0m" : "";
}

// Prints a single table row with fixed column widths.
void printTableRow(const std::vector<std::string>& cells, const std::vector<std::size_t>& widths) {
	std::cout << "|";
	for (std::size_t index = 0; index < cells.size(); ++index) {
		std::cout << ' ' << std::left << std::setw(static_cast<int>(widths[index])) << cells[index] << " |";
	}
	std::cout << std::endl;
}

// Prints a separator row for a table.
void printTableSeparator(const std::vector<std::size_t>& widths) {
	std::cout << "+";
	for (std::size_t width : widths) {
		std::cout << std::string(width + 2, '-') << "+";
	}
	std::cout << std::endl;
}

// Prints a titled table.
void printTableTitle(const std::string& title) {
	std::cout << colorCyan() << title << colorReset() << std::endl;
}

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

// Prints the current state after each instruction as a table.
void printState(const std::vector<std::pair<std::string, int>>& values) {
	std::vector<std::size_t> widths = {12, 10};
	printTableSeparator(widths);
	printTableRow({"Variable", "Value"}, widths);
	printTableSeparator(widths);
	for (const auto& entry : values) {
		printTableRow({entry.first, std::to_string(entry.second)}, widths);
	}
	printTableSeparator(widths);
}

// Prints the current register and temporary state after each two-address instruction as a table.
void printTwoAddressState(int registerValue, const std::vector<std::pair<std::string, int>>& values) {
	std::vector<std::size_t> widths = {10, 8};
	printTableSeparator(widths);
	printTableRow({"Register", "Value"}, widths);
	printTableSeparator(widths);
	printTableRow({"R1", std::to_string(registerValue)}, widths);
	printTableSeparator(widths);
	for (const auto& entry : values) {
		printTableRow({entry.first, std::to_string(entry.second)}, widths);
	}
	if (!values.empty()) {
		printTableSeparator(widths);
	}
}

// Prints the current accumulator and temporary state after each one-address instruction as a table.
void printOneAddressState(int accumulatorValue, const std::vector<std::pair<std::string, int>>& values) {
	std::vector<std::size_t> widths = {10, 8};
	printTableSeparator(widths);
	printTableRow({"Register", "Value"}, widths);
	printTableSeparator(widths);
	printTableRow({"AC", std::to_string(accumulatorValue)}, widths);
	printTableSeparator(widths);
	for (const auto& entry : values) {
		printTableRow({entry.first, std::to_string(entry.second)}, widths);
	}
	if (!values.empty()) {
		printTableSeparator(widths);
	}
}

// Prints the current stack and temporary state after each zero-address instruction as a table.
void printZeroAddressState(const std::vector<int>& stackValues, const std::vector<std::pair<std::string, int>>& values) {
	std::vector<std::size_t> widths = {12, 16};
	printTableSeparator(widths);
	printTableRow({"Stack Position", "Value"}, widths);
	printTableSeparator(widths);
	for (std::size_t index = 0; index < stackValues.size(); ++index) {
		printTableRow({"S" + std::to_string(index), std::to_string(stackValues[index])}, widths);
	}
	if (stackValues.empty()) {
		printTableRow({"(empty)", "-"}, widths);
	}
	printTableSeparator(widths);
	for (const auto& entry : values) {
		printTableRow({entry.first, std::to_string(entry.second)}, widths);
	}
	if (!values.empty()) {
		printTableSeparator(widths);
	}
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

// Resolves a token for the zero-address simulator.
int resolveZeroAddressToken(const std::string& token, const std::vector<int>& stackValues, const std::vector<std::pair<std::string, int>>& values) {
	if (token.size() == 1 && token[0] >= 'a' && token[0] <= 'z') {
		return (token[0] - 'a') + 1;
	}

	int storedValue = 0;
	if (findValue(values, token, storedValue)) {
		return storedValue;
	}

	(void)stackValues;
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

	printTableTitle("Three Address Simulation");

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

		printTableSeparator({18, 14});
		printTableRow({"Instruction", "Result"}, {18, 14});
		printTableSeparator({18, 14});
		printTableRow({instruction, destination + " = " + std::to_string(resultValue)}, {18, 14});
		printState(values);
	}
}

void Simulator::simulateTwoAddressCode(const std::vector<std::string>& instructions) {
	std::vector<std::pair<std::string, int>> values;
	int registerValue = 0;

	printTableTitle("Two Address Simulation");

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

		printTableSeparator({18, 14});
		printTableRow({"Instruction", "Result"}, {18, 14});
		printTableSeparator({18, 14});
		printTableRow({instruction, "R1 = " + std::to_string(registerValue)}, {18, 14});
		printTwoAddressState(registerValue, values);
	}
}

void Simulator::simulateOneAddressCode(const std::vector<std::string>& instructions) {
	std::vector<std::pair<std::string, int>> values;
	int accumulatorValue = 0;

	printTableTitle("One Address Simulation");

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

		printTableSeparator({18, 14});
		printTableRow({"Instruction", "Result"}, {18, 14});
		printTableSeparator({18, 14});
		printTableRow({instruction, "AC = " + std::to_string(accumulatorValue)}, {18, 14});
		printOneAddressState(accumulatorValue, values);
	}
}

void Simulator::simulateZeroAddressCode(const std::vector<std::string>& instructions) {
	std::vector<std::pair<std::string, int>> values;
	std::vector<int> stackValues;

	printTableTitle("Zero Address Simulation");

	for (const std::string& instruction : instructions) {
		std::istringstream stream(instruction);
		std::string operation;
		std::string operand;

		stream >> operation;

		if (operation == "PUSH") {
			stream >> operand;
			stackValues.push_back(resolveZeroAddressToken(operand, stackValues, values));
		} else if (operation == "POP") {
			stream >> operand;
			if (!stackValues.empty()) {
				int topValue = stackValues.back();
				stackValues.pop_back();
				setValue(values, operand, topValue);
			}
		} else if (operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV") {
			if (stackValues.size() < 2) {
				continue;
			}

			int rightOperand = stackValues.back();
			stackValues.pop_back();
			int leftOperand = stackValues.back();
			stackValues.pop_back();

			int resultValue = 0;
			switch (operation[0]) {
			case 'A':
				resultValue = leftOperand + rightOperand;
				break;
			case 'S':
				resultValue = leftOperand - rightOperand;
				break;
			case 'M':
				resultValue = leftOperand * rightOperand;
				break;
			case 'D':
				resultValue = rightOperand == 0 ? 0 : leftOperand / rightOperand;
				break;
			default:
				break;
			}

			stackValues.push_back(resultValue);
		} else {
			continue;
		}

		printTableSeparator({18, 14});
		printTableRow({"Instruction", "Result"}, {18, 14});
		printTableSeparator({18, 14});
		printTableRow({instruction, "Stack updated"}, {18, 14});
		printZeroAddressState(stackValues, values);
	}
}
