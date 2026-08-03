#include <iostream>
#include <cstdlib>
#include <set>
#include <iomanip>
#include <string>
#include <vector>

#include "ExpressionConverter.h"
#include "ExpressionValidator.h"
#include "InstructionGenerator.h"
#include "Simulator.h"

namespace {

// Returns true when color accents should be enabled for table headers.
bool useColorOutput() {
    return std::getenv("NO_COLOR") == nullptr;
}

const char* accent() {
    return useColorOutput() ? "\033[35m" : "";
}

const char* reset() {
    return useColorOutput() ? "\033[0m" : "";
}

// Prints a table separator line.
void printSeparator(const std::vector<std::size_t>& widths) {
    std::cout << "+";
    for (std::size_t width : widths) {
        std::cout << std::string(width + 2, '-') << "+";
    }
    std::cout << std::endl;
}

// Prints one table row using fixed widths.
void printRow(const std::vector<std::string>& cells, const std::vector<std::size_t>& widths) {
    std::cout << "|";
    for (std::size_t index = 0; index < cells.size(); ++index) {
        std::cout << ' ' << std::left << std::setw(static_cast<int>(widths[index])) << cells[index] << " |";
    }
    std::cout << std::endl;
}

// Prints a section title with a subtle accent when supported.
void printSectionTitle(const std::string& title) {
    std::cout << accent() << title << reset() << std::endl;
}

// Prints a table for a list of instructions.
void printInstructionTable(const std::string& title, const std::vector<std::string>& instructions) {
    printSectionTitle(title);
    std::vector<std::size_t> widths = {6, 18};
    printSeparator(widths);
    printRow({"#", "Instruction"}, widths);
    printSeparator(widths);

    for (std::size_t index = 0; index < instructions.size(); ++index) {
        printRow({std::to_string(index + 1), instructions[index]}, widths);
    }

    printSeparator(widths);
}

// Extracts temporary variable names from an instruction list.
std::set<std::string> collectTemporaryVariables(const std::vector<std::string>& instructions) {
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
}

// Prints a comparison table for all formats.
void printComparisonTable(
    std::size_t threeAddressCount,
    const std::set<std::string>& threeAddressTemps,
    std::size_t twoAddressCount,
    const std::set<std::string>& twoAddressTemps,
    std::size_t oneAddressCount,
    const std::set<std::string>& oneAddressTemps,
    std::size_t zeroAddressCount,
    const std::set<std::string>& zeroAddressTemps) {
    printSectionTitle("Instruction Format Comparison");

    std::vector<std::size_t> widths = {14, 6, 18, 10, 8};
    printSeparator(widths);
    printRow({"Format", "Count", "Temporary Vars", "Registers", "Steps"}, widths);
    printSeparator(widths);

    auto renderTemporaryList = [](const std::set<std::string>& temporaries) {
        if (temporaries.empty()) {
            return std::string("None");
        }

        std::string rendered;
        bool first = true;
        for (const std::string& temporaryVariable : temporaries) {
            if (!first) {
                rendered += ", ";
            }
            rendered += temporaryVariable;
            first = false;
        }
        return rendered;
    };

    printRow({"Three Addr", std::to_string(threeAddressCount), renderTemporaryList(threeAddressTemps), "None", std::to_string(threeAddressCount)}, widths);
    printRow({"Two Addr", std::to_string(twoAddressCount), renderTemporaryList(twoAddressTemps), "R1", std::to_string(twoAddressCount)}, widths);
    printRow({"One Addr", std::to_string(oneAddressCount), renderTemporaryList(oneAddressTemps), "AC", std::to_string(oneAddressCount)}, widths);
    printRow({"Zero Addr", std::to_string(zeroAddressCount), renderTemporaryList(zeroAddressTemps), "Stack", std::to_string(zeroAddressCount)}, widths);
    printSeparator(widths);
}

} // namespace

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

    std::string postfixExpression = ExpressionConverter::toPostfix(expression);
    printSectionTitle("Expression Summary");
    printSeparator({20, 22});
    printRow({"Original", expression}, {20, 22});
    printRow({"Postfix", postfixExpression}, {20, 22});
    printSeparator({20, 22});

    std::vector<std::string> threeAddressInstructions = InstructionGenerator::generateThreeAddressCode(postfixExpression);
    printInstructionTable("Three Address Code", threeAddressInstructions);

    Simulator::simulateThreeAddressCode(threeAddressInstructions);

    std::vector<std::string> twoAddressInstructions = InstructionGenerator::generateTwoAddressCode(postfixExpression);
    printInstructionTable("Two Address Code", twoAddressInstructions);

    Simulator::simulateTwoAddressCode(twoAddressInstructions);

    std::vector<std::string> oneAddressInstructions = InstructionGenerator::generateOneAddressCode(postfixExpression);
    printInstructionTable("One Address Code", oneAddressInstructions);

    Simulator::simulateOneAddressCode(oneAddressInstructions);

    std::vector<std::string> zeroAddressInstructions = InstructionGenerator::generateZeroAddressCode(postfixExpression);
    printInstructionTable("Zero Address Code", zeroAddressInstructions);

    Simulator::simulateZeroAddressCode(zeroAddressInstructions);

    printComparisonTable(
        threeAddressInstructions.size(),
        collectTemporaryVariables(threeAddressInstructions),
        twoAddressInstructions.size(),
        collectTemporaryVariables(twoAddressInstructions),
        oneAddressInstructions.size(),
        collectTemporaryVariables(oneAddressInstructions),
        zeroAddressInstructions.size(),
        collectTemporaryVariables(zeroAddressInstructions));

    return 0;
}
