#include "WebAppService.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

#include "ExpressionConverter.h"
#include "ExpressionValidator.h"
#include "InstructionGenerator.h"
#include "Simulator.h"

namespace {

std::string trim(const std::string& input) {
    std::size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    std::size_t end = input.find_last_not_of(" \t\r\n");
    return input.substr(start, end - start + 1);
}

std::string escapeJson(const std::string& value) {
    std::string output;
    output.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
        case '"': output += "\\\""; break;
        case '\\': output += "\\\\"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\t': output += "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                std::ostringstream stream;
                stream << "\\u" << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(ch));
                output += stream.str();
            } else {
                output += ch;
            }
        }
    }
    return output;
}

std::string decodeUrl(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for (std::size_t index = 0; index < input.size(); ++index) {
        char ch = input[index];
        if (ch == '+') {
            output.push_back(' ');
        } else if (ch == '%' && index + 2 < input.size()) {
            char first = input[index + 1];
            char second = input[index + 2];
            if (std::isxdigit(static_cast<unsigned char>(first)) && std::isxdigit(static_cast<unsigned char>(second))) {
                std::string hex = input.substr(index + 1, 2);
                char decoded = static_cast<char>(std::stoi(hex, nullptr, 16));
                output.push_back(decoded);
                index += 2;
            } else {
                output.push_back(ch);
            }
        } else {
            output.push_back(ch);
        }
    }
    return output;
}

std::set<std::string> collectTemporaryVariables(const std::vector<std::string>& instructions) {
    std::set<std::string> temporaryVariables;
    for (const std::string& instruction : instructions) {
        std::size_t position = instruction.find('t');
        while (position != std::string::npos) {
            std::size_t end = position + 1;
            while (end < instruction.size() && std::isdigit(static_cast<unsigned char>(instruction[end]))) {
                ++end;
            }
            if (end > position + 1) {
                temporaryVariables.insert(instruction.substr(position, end - position));
            }
            position = instruction.find('t', end);
        }
    }
    return temporaryVariables;
}

std::vector<std::string> collectVariables(const std::string& expression) {
    std::set<std::string> variables;
    for (char ch : expression) {
        if (ch >= 'a' && ch <= 'z') {
            variables.insert(std::string(1, ch));
        }
    }
    return std::vector<std::string>(variables.begin(), variables.end());
}

std::string join(const std::vector<std::string>& values, const std::string& separator) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            stream << separator;
        }
        stream << values[index];
    }
    return stream.str();
}

std::string formatValueMap(const std::map<std::string, int>& values) {
    std::ostringstream stream;
    for (const auto& entry : values) {
        stream << entry.first << "=" << entry.second << ",";
    }
    return stream.str();
}

std::string contentTypeForPath(const std::string& path) {
    if (path == "/styles.css") {
        return "text/css; charset=utf-8";
    }
    if (path == "/app.js") {
        return "application/javascript; charset=utf-8";
    }
    return "text/html; charset=utf-8";
}

std::vector<std::pair<std::string, int>> buildStateFromValues(const std::map<std::string, int>& values) {
    std::vector<std::pair<std::string, int>> state;
    for (const auto& entry : values) {
        state.emplace_back(entry.first, entry.second);
    }
    std::sort(state.begin(), state.end(), [](const auto& left, const auto& right) {
        return left.first < right.first;
    });
    return state;
}

std::string renderInstructionList(const std::vector<std::string>& instructions) {
    std::ostringstream stream;
    for (std::size_t index = 0; index < instructions.size(); ++index) {
        stream << (index + 1) << ". " << instructions[index];
        if (index + 1 != instructions.size()) {
            stream << "\n";
        }
    }
    return stream.str();
}

InstructionFormatResult buildFormatResult(const std::string& name, const std::vector<std::string>& instructions, const std::map<std::string, int>& values) {
    InstructionFormatResult result;
    result.name = name;
    result.instructions = instructions;
    result.instructionCount = instructions.size();

    std::set<std::string> temporaryVariables = collectTemporaryVariables(instructions);
    result.temporaryVariables.assign(temporaryVariables.begin(), temporaryVariables.end());
    result.registers = (name == "Two Address") ? "R1" : (name == "One Address") ? "AC" : (name == "Zero Address") ? "Stack" : "None";

    if (name == "Three Address") {
        result.steps.reserve(instructions.size());
        std::vector<std::pair<std::string, int>> state;
        for (const std::string& instruction : instructions) {
            std::size_t equals = instruction.find('=');
            if (equals == std::string::npos) {
                continue;
            }
            std::string destination = instruction.substr(0, equals);
            std::string expression = instruction.substr(equals + 1);
            if (expression.size() < 3) {
                continue;
            }
            std::string leftToken(1, expression[0]);
            char operation = expression[1];
            std::string rightToken = expression.substr(2);
            int leftValue = 0;
            int rightValue = 0;
            auto findLeft = values.find(leftToken);
            auto findRight = values.find(rightToken);
            if (findLeft != values.end()) {
                leftValue = findLeft->second;
            } else if (leftToken.size() == 1 && leftToken[0] >= 'a' && leftToken[0] <= 'z') {
                leftValue = static_cast<int>(leftToken[0] - 'a' + 1);
            }
            if (findRight != values.end()) {
                rightValue = findRight->second;
            } else if (rightToken.size() == 1 && rightToken[0] >= 'a' && rightToken[0] <= 'z') {
                rightValue = static_cast<int>(rightToken[0] - 'a' + 1);
            }
            int resultValue = 0;
            switch (operation) {
            case '+': resultValue = leftValue + rightValue; break;
            case '-': resultValue = leftValue - rightValue; break;
            case '*': resultValue = leftValue * rightValue; break;
            case '/': resultValue = rightValue == 0 ? 0 : leftValue / rightValue; break;
            }
            state.emplace_back(destination, resultValue);
            std::sort(state.begin(), state.end(), [](const auto& left, const auto& right) {
                return left.first < right.first;
            });
            result.steps.push_back({instruction, destination + " = " + std::to_string(resultValue), state});
            if (destination.rfind("t", 0) == 0) {
                result.finalResult = destination + " = " + std::to_string(resultValue);
            }
        }
    } else if (name == "Two Address") {
        int registerValue = 0;
        std::vector<std::pair<std::string, int>> state;
        for (const std::string& instruction : instructions) {
            std::istringstream stream(instruction);
            std::string operation;
            std::string operandLine;
            stream >> operation >> operandLine;
            if (operation == "MOV") {
                std::size_t comma = operandLine.find(',');
                if (comma == std::string::npos) {
                    continue;
                }
                std::string target = operandLine.substr(0, comma);
                std::string source = operandLine.substr(comma + 1);
                int sourceValue = 0;
                auto findSource = values.find(source);
                if (findSource != values.end()) {
                    sourceValue = findSource->second;
                } else if (source.size() == 1 && source[0] >= 'a' && source[0] <= 'z') {
                    sourceValue = static_cast<int>(source[0] - 'a' + 1);
                }
                if (target == "R1") {
                    registerValue = sourceValue;
                } else {
                    state.emplace_back(target, sourceValue);
                }
            } else if (operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV") {
                std::size_t comma = operandLine.find(',');
                if (comma == std::string::npos) {
                    continue;
                }
                std::string source = operandLine.substr(comma + 1);
                int sourceValue = 0;
                auto findSource = values.find(source);
                if (findSource != values.end()) {
                    sourceValue = findSource->second;
                } else if (source.size() == 1 && source[0] >= 'a' && source[0] <= 'z') {
                    sourceValue = static_cast<int>(source[0] - 'a' + 1);
                }
                switch (operation[0]) {
                case 'A': registerValue += sourceValue; break;
                case 'S': registerValue -= sourceValue; break;
                case 'M': registerValue *= sourceValue; break;
                case 'D': registerValue = sourceValue == 0 ? 0 : registerValue / sourceValue; break;
                }
            }
            std::sort(state.begin(), state.end(), [](const auto& left, const auto& right) {
                return left.first < right.first;
            });
            result.steps.push_back({instruction, "R1 = " + std::to_string(registerValue), state});
        }
        if (!result.steps.empty()) {
            result.finalResult = "R1 = " + std::to_string(registerValue);
        }
    } else if (name == "One Address") {
        int accumulatorValue = 0;
        std::vector<std::pair<std::string, int>> state;
        for (const std::string& instruction : instructions) {
            std::istringstream stream(instruction);
            std::string operation;
            std::string operandLine;
            stream >> operation >> operandLine;
            if (operation == "LOAD") {
                auto find = values.find(operandLine);
                if (find != values.end()) {
                    accumulatorValue = find->second;
                } else if (operandLine.size() == 1 && operandLine[0] >= 'a' && operandLine[0] <= 'z') {
                    accumulatorValue = static_cast<int>(operandLine[0] - 'a' + 1);
                }
            } else if (operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV") {
                int operandValue = 0;
                auto find = values.find(operandLine);
                if (find != values.end()) {
                    operandValue = find->second;
                } else if (operandLine.size() == 1 && operandLine[0] >= 'a' && operandLine[0] <= 'z') {
                    operandValue = static_cast<int>(operandLine[0] - 'a' + 1);
                }
                switch (operation[0]) {
                case 'A': accumulatorValue += operandValue; break;
                case 'S': accumulatorValue -= operandValue; break;
                case 'M': accumulatorValue *= operandValue; break;
                case 'D': accumulatorValue = operandValue == 0 ? 0 : accumulatorValue / operandValue; break;
                }
            } else if (operation == "STORE") {
                state.emplace_back(operandLine, accumulatorValue);
            }
            std::sort(state.begin(), state.end(), [](const auto& left, const auto& right) {
                return left.first < right.first;
            });
            result.steps.push_back({instruction, "AC = " + std::to_string(accumulatorValue), state});
        }
        if (!result.steps.empty()) {
            result.finalResult = "AC = " + std::to_string(accumulatorValue);
        }
    } else if (name == "Zero Address") {
        std::vector<int> stackValues;
        std::vector<std::pair<std::string, int>> state;
        for (const std::string& instruction : instructions) {
            std::istringstream stream(instruction);
            std::string operation;
            std::string operandLine;
            stream >> operation >> operandLine;
            if (operation == "PUSH") {
                int value = 0;
                auto find = values.find(operandLine);
                if (find != values.end()) {
                    value = find->second;
                } else if (operandLine.size() == 1 && operandLine[0] >= 'a' && operandLine[0] <= 'z') {
                    value = static_cast<int>(operandLine[0] - 'a' + 1);
                }
                stackValues.push_back(value);
            } else if (operation == "POP") {
                if (!stackValues.empty()) {
                    int topValue = stackValues.back();
                    stackValues.pop_back();
                    state.emplace_back(operandLine, topValue);
                }
            } else if (operation == "ADD" || operation == "SUB" || operation == "MUL" || operation == "DIV") {
                if (stackValues.size() >= 2) {
                    int rightValue = stackValues.back();
                    stackValues.pop_back();
                    int leftValue = stackValues.back();
                    stackValues.pop_back();
                    int resultValue = 0;
                    switch (operation[0]) {
                    case 'A': resultValue = leftValue + rightValue; break;
                    case 'S': resultValue = leftValue - rightValue; break;
                    case 'M': resultValue = leftValue * rightValue; break;
                    case 'D': resultValue = rightValue == 0 ? 0 : leftValue / rightValue; break;
                    }
                    stackValues.push_back(resultValue);
                }
            }
            std::sort(state.begin(), state.end(), [](const auto& left, const auto& right) {
                return left.first < right.first;
            });
            result.steps.push_back({instruction, "Stack updated", state});
        }
        if (!result.steps.empty()) {
            result.finalResult = "Stack top = " + (stackValues.empty() ? "-" : std::to_string(stackValues.back()));
        }
    }

    if (result.temporaryVariables.empty()) {
        result.temporaryVariables.push_back("None");
    }

    return result;
}

} // namespace

AnalysisResult WebAppService::processExpression(const std::string& expression, const std::map<std::string, int>& values) {
    AnalysisResult result;
    result.expression = trim(expression);

    if (result.expression.empty()) {
        result.error = "Please enter an expression.";
        return result;
    }

    if (!ExpressionValidator::isValidExpression(result.expression)) {
        result.error = "Invalid expression. Use lowercase operands a-z, operators + - * /, and balanced parentheses.";
        return result;
    }

    result.success = true;
    result.postfix = ExpressionConverter::toPostfix(result.expression);
    result.variables = collectVariables(result.expression);

    std::vector<std::string> threeAddress = InstructionGenerator::generateThreeAddressCode(result.postfix);
    std::vector<std::string> twoAddress = InstructionGenerator::generateTwoAddressCode(result.postfix);
    std::vector<std::string> oneAddress = InstructionGenerator::generateOneAddressCode(result.postfix);
    std::vector<std::string> zeroAddress = InstructionGenerator::generateZeroAddressCode(result.postfix);

    result.formats.push_back(buildFormatResult("Three Address", threeAddress, values));
    result.formats.push_back(buildFormatResult("Two Address", twoAddress, values));
    result.formats.push_back(buildFormatResult("One Address", oneAddress, values));
    result.formats.push_back(buildFormatResult("Zero Address", zeroAddress, values));

    result.comparisonRows.push_back({"Three Address", std::to_string(threeAddress.size())});
    result.comparisonRows.push_back({"Two Address", std::to_string(twoAddress.size())});
    result.comparisonRows.push_back({"One Address", std::to_string(oneAddress.size())});
    result.comparisonRows.push_back({"Zero Address", std::to_string(zeroAddress.size())});

    return result;
}

std::string WebAppService::toJson(const AnalysisResult& result) {
    std::ostringstream stream;
    stream << "{\"success\":" << (result.success ? "true" : "false") << ",";
    stream << "\"error\":" << "\"" << escapeJson(result.error) << "\"" << ",";
    stream << "\"expression\":" << "\"" << escapeJson(result.expression) << "\"" << ",";
    stream << "\"postfix\":" << "\"" << escapeJson(result.postfix) << "\"" << ",";
    stream << "\"variables\":[";
    for (std::size_t index = 0; index < result.variables.size(); ++index) {
        if (index != 0) {
            stream << ",";
        }
        stream << "\"" << escapeJson(result.variables[index]) << "\"";
    }
    stream << "],\"formats\":[";
    for (std::size_t formatIndex = 0; formatIndex < result.formats.size(); ++formatIndex) {
        if (formatIndex != 0) {
            stream << ",";
        }
        const InstructionFormatResult& format = result.formats[formatIndex];
        stream << "{\"name\":\"" << escapeJson(format.name) << "\",\"instructions\":[";
        for (std::size_t index = 0; index < format.instructions.size(); ++index) {
            if (index != 0) {
                stream << ",";
            }
            stream << "\"" << escapeJson(format.instructions[index]) << "\"";
        }
        stream << "],\"steps\":[";
        for (std::size_t stepIndex = 0; stepIndex < format.steps.size(); ++stepIndex) {
            if (stepIndex != 0) {
                stream << ",";
            }
            const SimulationStep& step = format.steps[stepIndex];
            stream << "{\"instruction\":\"" << escapeJson(step.instruction) << "\",\"result\":\"" << escapeJson(step.result) << "\",\"state\":[";
            for (std::size_t stateIndex = 0; stateIndex < step.state.size(); ++stateIndex) {
                if (stateIndex != 0) {
                    stream << ",";
                }
                const auto& stateEntry = step.state[stateIndex];
                stream << "{\"name\":\"" << escapeJson(stateEntry.first) << "\",\"value\":" << stateEntry.second << "}";
            }
            stream << "]}";
        }
        stream << "],\"instructionCount\":" << format.instructionCount << ",\"temporaryVariables\":[";
        for (std::size_t index = 0; index < format.temporaryVariables.size(); ++index) {
            if (index != 0) {
                stream << ",";
            }
            stream << "\"" << escapeJson(format.temporaryVariables[index]) << "\"";
        }
        stream << "],\"registers\":\"" << escapeJson(format.registers) << "\",\"finalResult\":\"" << escapeJson(format.finalResult) << "\"}";
    }
    stream << "],\"comparisonRows\":[";
    for (std::size_t index = 0; index < result.comparisonRows.size(); ++index) {
        if (index != 0) {
            stream << ",";
        }
        const auto& row = result.comparisonRows[index];
        stream << "{\"name\":\"" << escapeJson(row.first) << "\",\"value\":\"" << escapeJson(row.second) << "\"}";
    }
    stream << "]}";
    return stream.str();
}

void WebAppService::runServer(int port) {
    int serverSocket = -1;
    int selectedPort = port;

    for (int candidatePort = port; candidatePort < port + 20; ++candidatePort) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            continue;
        }

        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(candidatePort);

        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0) {
            selectedPort = candidatePort;
            break;
        }

        close(serverSocket);
        serverSocket = -1;
    }

    if (serverSocket < 0) {
        std::cerr << "Unable to open a local port for the COA web server." << std::endl;
        return;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Failed to listen on port " << selectedPort << std::endl;
        close(serverSocket);
        return;
    }

    std::cout << "COA Web Simulator listening on http://127.0.0.1:" << selectedPort << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            continue;
        }

        std::array<char, 4096> buffer{};
        std::string request;
        ssize_t received = recv(clientSocket, buffer.data(), buffer.size(), 0);
        if (received > 0) {
            request.assign(buffer.data(), static_cast<std::size_t>(received));
        }

        std::string responseBody;
        std::string path = "/";
        std::size_t pathPosition = request.find("GET ");
        if (pathPosition != std::string::npos) {
            std::size_t end = request.find(' ', pathPosition + 4);
            if (end != std::string::npos) {
                std::string rawPath = request.substr(pathPosition + 4, end - pathPosition - 4);
                std::size_t queryPosition = rawPath.find('?');
                if (queryPosition != std::string::npos) {
                    path = rawPath.substr(0, queryPosition);
                } else {
                    path = rawPath;
                }
            }
        }

        if (path == "/api/validate") {
            std::string expression = "";
            std::size_t queryPosition = request.find("expression=");
            if (queryPosition != std::string::npos) {
                std::string query = request.substr(queryPosition + 11);
                std::size_t amp = query.find('&');
                if (amp != std::string::npos) {
                    query = query.substr(0, amp);
                }
                expression = trim(decodeUrl(query));
            }
            std::map<std::string, int> values;
            std::string valuesPart = "";
            std::size_t valuesPosition = request.find("values=");
            if (valuesPosition != std::string::npos) {
                valuesPart = request.substr(valuesPosition + 7);
                std::size_t amp = valuesPart.find('&');
                if (amp != std::string::npos) {
                    valuesPart = valuesPart.substr(0, amp);
                }
                std::string decoded = decodeUrl(valuesPart);
                std::stringstream stream(decoded);
                std::string item;
                while (std::getline(stream, item, ',')) {
                    if (item.empty()) {
                        continue;
                    }
                    std::size_t separator = item.find(':');
                    if (separator != std::string::npos) {
                        std::string key = trim(item.substr(0, separator));
                        std::string value = trim(item.substr(separator + 1));
                        if (!key.empty()) {
                            try {
                                values[key] = std::stoi(value);
                            } catch (...) {
                                values[key] = 0;
                            }
                        }
                    }
                }
            }
            AnalysisResult analysis = processExpression(expression, values);
            responseBody = toJson(analysis);
        } else {
            std::string filePath = "./public/index.html";
            if (path == "/") {
                filePath = "./public/index.html";
            } else if (path == "/styles.css") {
                filePath = "./public/styles.css";
            } else if (path == "/app.js") {
                filePath = "./public/app.js";
            } else if (path.find("/favicon") == 0) {
                filePath = "./public/favicon.ico";
            } else {
                filePath = "./public" + path;
            }

            std::ifstream file(filePath, std::ios::binary);
            if (!file.is_open()) {
                responseBody = "Not found";
            } else {
                std::ostringstream bufferStream;
                bufferStream << file.rdbuf();
                responseBody = bufferStream.str();
            }
        }

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: " << ((path == "/api/validate") ? "application/json" : contentTypeForPath(path)) << "\r\n";
        response << "Content-Length: " << responseBody.size() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << responseBody;
        send(clientSocket, response.str().c_str(), static_cast<int>(response.str().size()), 0);
        close(clientSocket);
    }
}
