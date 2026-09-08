#include "ExprAST.h"

int main() {
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40;

	getNextToken();
	if (Curtok == static_cast<int>(Token::tok_eof))
		return 0;

	if (auto Expression = ParseExpression()) {
		std::cout << Expression->dump() << '\n';
		return Curtok == static_cast<int>(Token::tok_eof) ? 0 : 1;
	}
	return 1;
}
