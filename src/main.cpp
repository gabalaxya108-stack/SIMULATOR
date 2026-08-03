#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "ExpressionConverter.h"
#include "ExpressionValidator.h"
#include "InstructionGenerator.h"
#include "Simulator.h"

// Entry point for the console application.
// Reads one infix expression, validates it, and prints postfix, three-address code, two-address code, one-address code, zero-address code, and simulation output.
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

    std::vector<std::string> threeAddressInstructions = InstructionGenerator::generateThreeAddressCode(postfixExpression);
    std::cout << "Three Address Code:" << std::endl;
    for (const std::string& instruction : threeAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    Simulator::simulateThreeAddressCode(threeAddressInstructions);

    std::vector<std::string> twoAddressInstructions = InstructionGenerator::generateTwoAddressCode(postfixExpression);
    std::cout << "Two Address Code:" << std::endl;
    for (const std::string& instruction : twoAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    Simulator::simulateTwoAddressCode(twoAddressInstructions);

    std::vector<std::string> oneAddressInstructions = InstructionGenerator::generateOneAddressCode(postfixExpression);
    std::cout << "One Address Code:" << std::endl;
    for (const std::string& instruction : oneAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    Simulator::simulateOneAddressCode(oneAddressInstructions);

    std::vector<std::string> zeroAddressInstructions = InstructionGenerator::generateZeroAddressCode(postfixExpression);
    std::cout << "Zero Address Code:" << std::endl;
    for (const std::string& instruction : zeroAddressInstructions) {
        std::cout << instruction << std::endl;
    }

    Simulator::simulateZeroAddressCode(zeroAddressInstructions);

    auto collectTemporaryVariables = [](const std::vector<std::string>& instructions) {
        std::set<std::string> temporaryVariables;

        for (const std::string& instruction : instructions) {
            std::size_t firstTemporaryPosition = instruction.find('t');
            while (firstTemporaryPosition != std::string::npos) {
                std::size_t nextPosition = firstTemporaryPosition + 1;
                while (nextPosition < instruction.size() && instruction[nextPosition] >= '0' && instruction[nextPosition] <= '9') {
                    ++nextPosition;
                }

                if (nextPosition > firstTemporaryPosition + 1) {
                    temporaryVariables.insert(instruction.substr(firstTemporaryPosition, nextPosition - firstTemporaryPosition));
                }

                firstTemporaryPosition = instruction.find('t', nextPosition);
            }
        }

        return temporaryVariables;
    };

    std::cout << "Instruction Format Comparison:" << std::endl;
    std::cout << "Format | Number of Instructions | Temporary Variables | Registers Used | Execution Steps" << std::endl;

    std::set<std::string> threeAddressTemps = collectTemporaryVariables(threeAddressInstructions);
    std::set<std::string> twoAddressTemps = collectTemporaryVariables(twoAddressInstructions);
    std::set<std::string> oneAddressTemps = collectTemporaryVariables(oneAddressInstructions);
    std::set<std::string> zeroAddressTemps = collectTemporaryVariables(zeroAddressInstructions);

    std::cout << "Three Address | " << threeAddressInstructions.size() << " | ";
    if (threeAddressTemps.empty()) {
        std::cout << "None";
    } else {
        bool first = true;
        for (const std::string& temporaryVariable : threeAddressTemps) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << temporaryVariable;
            first = false;
        }
    }
    std::cout << " | None | " << threeAddressInstructions.size() << std::endl;

    std::cout << "Two Address | " << twoAddressInstructions.size() << " | ";
    if (twoAddressTemps.empty()) {
        std::cout << "None";
    } else {
        bool first = true;
        for (const std::string& temporaryVariable : twoAddressTemps) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << temporaryVariable;
            first = false;
        }
    }
    std::cout << " | R1 | " << twoAddressInstructions.size() << std::endl;

    std::cout << "One Address | " << oneAddressInstructions.size() << " | ";
    if (oneAddressTemps.empty()) {
        std::cout << "None";
    } else {
        bool first = true;
        for (const std::string& temporaryVariable : oneAddressTemps) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << temporaryVariable;
            first = false;
        }
    }
    std::cout << " | AC | " << oneAddressInstructions.size() << std::endl;

    std::cout << "Zero Address | " << zeroAddressInstructions.size() << " | ";
    if (zeroAddressTemps.empty()) {
        std::cout << "None";
    } else {
        bool first = true;
        for (const std::string& temporaryVariable : zeroAddressTemps) {
            if (!first) {
                std::cout << ", ";
            }
            std::cout << temporaryVariable;
            first = false;
        }
    }
    std::cout << " | Stack | " << zeroAddressInstructions.size() << std::endl;

    return 0;
}
