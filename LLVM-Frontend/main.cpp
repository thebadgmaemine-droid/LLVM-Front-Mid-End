#include "parser.h"

using namespace llvm_frontend;

int main() {
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40;

	getNextToken();
	return 1;
}
