#include "ExpressionValidator.h"

bool ExpressionValidator::isOperand(char character) {
	return character >= 'a' && character <= 'z';
}

bool ExpressionValidator::isOperator(char character) {
	return character == '+' || character == '-' || character == '*' || character == '/';
}

bool ExpressionValidator::isValidExpression(const std::string& expression) {
	if (expression.empty()) {
		return false;
	}

	int parenthesisDepth = 0;

	enum class PreviousToken {
		Start,
		Operand,
		Operator,
		OpenParenthesis,
		CloseParenthesis
	};

	PreviousToken previousToken = PreviousToken::Start;

	for (char character : expression) {
		if (isOperand(character)) {
			if (previousToken == PreviousToken::Operand || previousToken == PreviousToken::CloseParenthesis) {
				return false;
			}
			previousToken = PreviousToken::Operand;
		} else if (isOperator(character)) {
			if (previousToken != PreviousToken::Operand && previousToken != PreviousToken::CloseParenthesis) {
				return false;
			}
			previousToken = PreviousToken::Operator;
		} else if (character == '(') {
			if (previousToken == PreviousToken::Operand || previousToken == PreviousToken::CloseParenthesis) {
				return false;
			}
			++parenthesisDepth;
			previousToken = PreviousToken::OpenParenthesis;
		} else if (character == ')') {
			if (parenthesisDepth == 0 || previousToken == PreviousToken::Operator || previousToken == PreviousToken::OpenParenthesis || previousToken == PreviousToken::Start) {
				return false;
			}
			--parenthesisDepth;
			previousToken = PreviousToken::CloseParenthesis;
		} else {
			return false;
		}
	}

	return parenthesisDepth == 0 && previousToken != PreviousToken::Operator && previousToken != PreviousToken::OpenParenthesis;
}
