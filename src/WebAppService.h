#pragma once

#include <map>
#include <string>
#include <vector>

struct SimulationStep {
    std::string instruction;
    std::string result;
    std::vector<std::pair<std::string, int>> state;
};

struct InstructionFormatResult {
    std::string name;
    std::vector<std::string> instructions;
    std::vector<SimulationStep> steps;
    std::size_t instructionCount = 0;
    std::vector<std::string> temporaryVariables;
    std::string registers;
    std::string finalResult;
};

struct AnalysisResult {
    bool success = false;
    std::string error;
    std::string expression;
    std::string postfix;
    std::vector<std::string> variables;
    std::vector<InstructionFormatResult> formats;
    std::vector<std::pair<std::string, std::string>> comparisonRows;
};

class WebAppService {
public:
    static AnalysisResult processExpression(const std::string& expression, const std::map<std::string, int>& values);
    static std::string toJson(const AnalysisResult& result);
    static void runServer(int port);
};
