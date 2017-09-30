#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;
namespace {
	unordered_set<string> nonLeafNodeRule = {
		"procedures main",
		"params paramlist",
		"paramlist dcl",
		"type INT",
		"expr term",
		"term factor",
		"factor ID"  ,
		"factor NUM",
		"factor NULL" ,
		"arglist expr",
		"lvalue ID"
	};
	unordered_set<string> terminals = {
		"AMP",
		"BECOMES",
		"BOF",
		"COMMA",
		"DELETE",
		"ELSE",
		"EOF",
		"EQ",
		"GE",
		"GT",
		"ID",
		"IF",
		"INT",
		"LBRACE",
		"LBRACK",
		"LE",
		"LPAREN",
		"LT",
		"MINUS",
		"NE",
		"NEW",
		"NULL",
		"NUM",
		"PCT",
		"PLUS",
		"PRINTLN",
		"RBRACE",
		"RBRACK",
		"RETURN",
		"RPAREN",
		"SEMI",
		"SLASH",
		"STAR",
		"WAIN",
		"WHILE"
	};
	unordered_set<string> nonTerminals = {
		"start",
		"dcl",
		"dcls",
		"expr",
		"factor",
		"lvalue",
		"procedure",
		"procedures",
		"main",
		"params",
		"paramlist",
		"statement",
		"statements",
		"term",
		"test",
		"type",
		"arglist",
		"start"
	};
	unordered_set<string> procedureRules = {
		"procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE",
		"main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE"
	};
	unordered_set<string> idRules = {
		"dcl type ID",
		"factor ID",
		"lvalue ID",
		"factor ID LPAREN RPAREN",
		"factor ID LPAREN arglist RPAREN"
	};
}

class Node {
public:
	int numOfChildren;
	string type;
	string lexeme;
	string reduceRule;
	vector<string> childrenTypes;
	vector<Node *> children;
	Node(string reduceRule);
	// FindProcedure(vector<Procedure *> &procedures);
	// AnalyzeProcedure(Procedure & procedure);
	void addChild(Node * child);
	~Node();
	void print();
};

Node::Node(string reduceRule) : reduceRule{ reduceRule } {
	numOfChildren = 0;
	type = "";
	lexeme = "";
	int n = std::count(reduceRule.begin(), reduceRule.end(), ' ');
	if (n == 0) {
		// disappear reduceRule
		this->type = reduceRule;
		this->lexeme = "";
		this->numOfChildren = 0;
		return;
	}

	if (n == 1) {
		// first check if that is part of the rule 
		if (nonLeafNodeRule.find(reduceRule) != nonLeafNodeRule.end()) {
			// part of the rule, not a leaf
			this->type = reduceRule.substr(0, reduceRule.find(" "));
			this->lexeme = "";
			this->numOfChildren = 1;
			return;
		}
		// no kids, this is a leaf
		this->type = reduceRule.substr(0, reduceRule.find(" "));
		this->lexeme = reduceRule.substr(reduceRule.find(" ") + 1, reduceRule.length() - reduceRule.find(" ") - 1);
		this->numOfChildren = 0;
		return;
	}
	// now we need to seperate those children types
	std::istringstream iss(reduceRule);
	iss >> this->type;
	this->lexeme = "";
	this->numOfChildren = n;
	for (int i = 0; i < numOfChildren; i++) {
		string sub;
		iss >> sub;
		childrenTypes.push_back(sub);
	}
}

void Node::addChild(Node * child) {
	this->children.push_back(child);
};

Node::~Node() {
	for (vector<Node *>::iterator it = children.begin(); it != children.end(); it++) {
		delete *it;
	}
}

void Node::print() {
	cout << this->reduceRule << endl;
	for (vector<Node *>::iterator it = this->children.begin(); it != this->children.end(); ++it) {
		(*it)->print();
	}
}
class Procedure;
class ConstructionStack {
public:
	vector<Node *> nodeStack;
	vector<Procedure *> procedures;
	Procedure *main;
	static int ifCount;
	static int whileCount;
	vector<string> mipsCodePool;
	ConstructionStack() {
		main = nullptr;
	};
	void mipsGen();
	~ConstructionStack();
	void putOnNode(Node *);
	void print();
	void outputMips();
	void analyzeNodeAfterConstruct();
	void findProcedures(Node * startNode);
	void printProcedures();
};

class Procedure {
public:
	Node *rootNode;
	string name;
	string returnType;
	unordered_map<string, string> localVariables;
	// first is type, second is id
	vector<std::pair<string, string>> parameterList;
	vector<Procedure *> &procedures;
	vector<string> &mipsCodePool;
	vector<vector<string>> symbolTable;
	Procedure(Node *rootNode, vector<Procedure *> &procedures, vector<string> &mipsCodePool);
	~Procedure();
	void buildSymbolTable();
	void storeLocalVariableOnRAM();
	void print();
	void processProcedure();
	void processParams(Node *params);
	void processParamlist(Node *paramlist);
	void processDcl(Node * dcl, bool param);
	void processDcls(Node * dcls);
	void processStatements(Node *statements);
	void processStatement(Node *statement);
	void processLvalue(Node *lvalue);
	void processExpr(Node *Expr);
	void processTest(Node *test);
	void processTerm(Node *term);
	void processFactor(Node *factor);
	void processID(Node *ID);
	void processNum(Node *NUM);
	void typeCheckStatements(Node *statements);
	void typeCheckStatement(Node *statement);
	void typeCheckTest(Node *test);
	void typeCheckDCLS(Node *dcls);
	string typeDcl(Node *dcl);
	string typeExpr(Node *expr);
	string typeTerm(Node *term);
	string typeFactor(Node *factor);
	string typeID(Node *ID);
	string typeLvalue(Node *lvalue);
	vector<string> processArglist(Node *argList);

	// mips code generation part:
	void generateCode();
	void generateDcls(Node *dcls);
	void generateStatements(Node *statements);
	void generateStatement(Node *statement);
	void generateTest(Node *test);
	void generateExpr(Node * expr);
	void generateTerm(Node *term);
	void generateFactor(Node *factor);
	void generateID(Node *ID);
	int generateArgList(Node *arglist);
	Node *generateLvalue(Node *lvalue);
	void generateReturn();
};

void Procedure::generateTest(Node *test) {
	generateExpr(test->children[0]);
	this->mipsCodePool.push_back("sw $3, -4($30)");
	this->mipsCodePool.push_back("sub $30, $30, $4");
	generateExpr(test->children[2]);
	string typeE1 = typeExpr(test->children[0]);
	
	// we pop the value of expr from RAM,store to $5
	this->mipsCodePool.push_back("add $30, $30, $4");
	this->mipsCodePool.push_back("lw $5, -4($30)");

	if (typeE1 == "int") {
		if (test->reduceRule == "test expr EQ expr") {
			this->mipsCodePool.push_back("slt $6, $3, $5");
			this->mipsCodePool.push_back("slt $7, $5, $3");
			this->mipsCodePool.push_back("add $3, $6, $7");
			this->mipsCodePool.push_back("sub $3, $11, $3");
		}
		else if (test->reduceRule == "test expr NE expr") {
			this->mipsCodePool.push_back("slt $6, $3, $5");
			this->mipsCodePool.push_back("slt $7, $5, $3");
			this->mipsCodePool.push_back("add $3, $6, $7");
		}
		else if (test->reduceRule == "test expr LT expr") {
			this->mipsCodePool.push_back("slt $3, $5, $3");
		}
		else if (test->reduceRule == "test expr LE expr") {
			this->mipsCodePool.push_back("slt $3, $3, $5");
			this->mipsCodePool.push_back("sub $3, $11, $3");
		}
		else if (test->reduceRule == "test expr GE expr") {
			this->mipsCodePool.push_back("slt $3, $5, $3");
			this->mipsCodePool.push_back("sub $3, $11, $3");
		}
		else if (test->reduceRule == "test expr GT expr") {
			this->mipsCodePool.push_back("slt $3, $3, $5");
		}
	}
	else {
		if (test->reduceRule == "test expr EQ expr") {
			this->mipsCodePool.push_back("sltu $6, $3, $5");
			this->mipsCodePool.push_back("sltu $7, $5, $3");
			this->mipsCodePool.push_back("add $3, $6, $7");
			this->mipsCodePool.push_back("sub $3, $11, $3");
		}
		else if (test->reduceRule == "test expr NE expr") {
			this->mipsCodePool.push_back("sltu $6, $3, $5");
			this->mipsCodePool.push_back("sltu $7, $5, $3");
			this->mipsCodePool.push_back("add $3, $6, $7");
		}
		else if (test->reduceRule == "test expr LT expr") {
			this->mipsCodePool.push_back("sltu $3, $5, $3");
		}
		else if (test->reduceRule == "test expr LE expr") {
			this->mipsCodePool.push_back("sltu $3, $3, $5");
			this->mipsCodePool.push_back("sub $3, $11, $3");
		}
		else if (test->reduceRule == "test expr GE expr") {
			this->mipsCodePool.push_back("sltu $3, $5, $3");
			this->mipsCodePool.push_back("sub $3, $11, $3");
		}
		else if (test->reduceRule == "test expr GT expr") {
			this->mipsCodePool.push_back("sltu $3, $3, $5");
		}
	}
}

Node *Procedure::generateLvalue(Node *lvalue) {
	if ((lvalue->reduceRule == "lvalue ID") || 
		(lvalue->reduceRule == "lvalue STAR factor")) {
		
		return lvalue;
	}
	//if (lvalue->reduceRule == "lvalue STAR factor") {
	//	// this is something aboubt pointer, we think of it later

	//}
	if (lvalue->reduceRule == "lvalue LPAREN lvalue RPAREN") {
		return generateLvalue(lvalue->children[1]);
	}
}

void Procedure::generateID(Node *ID) {
	string nameID = ID->lexeme;
	for (int i = 0; i < symbolTable.size(); i++) {
		if (symbolTable[i][0] == nameID) {
			string location = symbolTable[i][2];
			string loadStr = "lw $3, " + location;
			loadStr = loadStr + "($29)";
			this->mipsCodePool.push_back(loadStr);
			return;
		}
	}
}

void Procedure::generateFactor(Node *factor) {
	// if the id is a variable
	if (factor->reduceRule == "factor ID") {
		generateID(factor->children[0]);
		return;
	}
	// for rule: factor -> NUM, NULL, do nothing for now
	if (factor->reduceRule == "factor NUM") {
		string numStr = factor->children[0]->lexeme;
		this->mipsCodePool.push_back("lis $3");
		string wordStr = ".word " + numStr;
		this->mipsCodePool.push_back(wordStr);
		return;
	}

	if (factor->reduceRule == "factor NULL") {
		// this is null
		this->mipsCodePool.push_back("add $3, $0, $11");
		return;
	}

	// for rule: factor -> STAR factor
	if (factor->reduceRule == "factor STAR factor") {
		// this is about pointer, we think aboubt it later
		generateFactor(factor->children[1]);
		this->mipsCodePool.push_back("lw $3, 0($3)");
		return;
	}
	// for rule: factor -> LPAREN expr RPAREN
	if (factor->reduceRule == "factor LPAREN expr RPAREN") {
		generateExpr(factor->children[1]);
		return;
	}
	// for rule: factor -> AMP lvalue
	if (factor->reduceRule == "factor AMP lvalue") {
		// this is about pointer, we think aboubt it later
		Node *lvalue = generateLvalue(factor->children[1]);
		if (lvalue->reduceRule == "lvalue ID") {
			string IDName = lvalue->children[0]->lexeme;
			string location = "";
			for (int i = 0; i < symbolTable.size(); i++) {
				if (symbolTable[i][0] == IDName) {
					location = symbolTable[i][2];
				}
			}
			this->mipsCodePool.push_back("lis $3");
			string wordStr = ".word " + location;
			this->mipsCodePool.push_back(wordStr);
			this->mipsCodePool.push_back("add $3, $3, $29");
		}
		else if (lvalue->reduceRule == "lvalue STAR factor") {
			generateFactor(lvalue->children[1]);
		}
		return;
	}
	// for rule: factor -> NEW INT LBRACK expr RBRACK
	if (factor->reduceRule == "factor NEW INT LBRACK expr RBRACK") {
		// this is about memory , we think about it later
		generateExpr(factor->children[3]);
		this->mipsCodePool.push_back("add $1, $3, $0");
		this->mipsCodePool.push_back("lis $10");
		this->mipsCodePool.push_back(".word new");
		this->mipsCodePool.push_back("jalr $10");
		this->mipsCodePool.push_back("bne $0, $3, 1");
		this->mipsCodePool.push_back("add $3, $0, $11");
		return;
	}
	// for rule: factor -> ID LPAREN RPAREN
	if (factor->reduceRule == "factor ID LPAREN RPAREN"
		|| factor->reduceRule == "factor ID LPAREN arglist RPAREN") {
		// this is function call, we think about it later
		string functionName = "F"+factor->children[0]->lexeme;
		string wordFunc = ".word " + functionName;
		int paramNum = 0;
		// first store the $29 and $30
		this->mipsCodePool.push_back("sw $29, -4($30)");
		this->mipsCodePool.push_back("sub $30, $30, $4");
		if (this->name != "wain") {
			this->mipsCodePool.push_back("sw $31, -4($30)");
			this->mipsCodePool.push_back("sub $30, $30, $4");
		}
		if (factor->reduceRule == "factor ID LPAREN arglist RPAREN") {
			paramNum = generateArgList(factor->children[2]);
		}
		this->mipsCodePool.push_back("lis $5");
		this->mipsCodePool.push_back(wordFunc);
		this->mipsCodePool.push_back("jalr $5");
		if (factor->reduceRule == "factor ID LPAREN arglist RPAREN") {
			// we need to pop off the parameters we just pushed
			int addUp = paramNum * 4;
			stringstream ss;
			ss << addUp;
			string strOffset = ss.str();
			string strWordOffset = ".word " + strOffset;
			mipsCodePool.push_back("lis $5");
			mipsCodePool.push_back(strWordOffset);
			mipsCodePool.push_back("add $30, $30, $5");
		}
		if (this->name != "wain") {
			this->mipsCodePool.push_back("add $30, $30, $4");
			this->mipsCodePool.push_back("lw $31, -4($30)");
		}
		this->mipsCodePool.push_back("add $30, $30, $4");
		this->mipsCodePool.push_back("lw $29, -4($30)");
		return;
	}
}

int Procedure::generateArgList(Node *arglist) {
	int answer = 0;
	while (arglist->numOfChildren == 3) {
		answer++;
		generateExpr(arglist->children[0]);
		this->mipsCodePool.push_back("sw $3, -4($30)");
		this->mipsCodePool.push_back("sub $30, $30, $4");
		arglist = arglist->children[2];
	}
	generateExpr(arglist->children[0]);
	this->mipsCodePool.push_back("sw $3, -4($30)");
	this->mipsCodePool.push_back("sub $30, $30, $4");
	return answer + 1;
}

void Procedure::generateTerm(Node *term) {
	if (term->reduceRule == "term factor") {
		generateFactor(term->children[0]);
	}
	else {
		generateTerm(term->children[0]);
		// push the result of term to RAM
		this->mipsCodePool.push_back("sw $3, -4($30)");
		this->mipsCodePool.push_back("sub $30, $30, $4");
		generateFactor(term->children[2]);
		// we pop the value of expr from RAM,store to $5
		this->mipsCodePool.push_back("add $30, $30, $4");
		this->mipsCodePool.push_back("lw $5, -4($30)");
		if (term->reduceRule == "term term STAR factor") {
			// then multiplication
			this->mipsCodePool.push_back("mult $5, $3");
			this->mipsCodePool.push_back("mflo $3");
		}
		else if (term->reduceRule == "term term SLASH factor") {
			// then division
			this->mipsCodePool.push_back("div $5, $3");
			this->mipsCodePool.push_back("mflo $3");
		}
		else if (term->reduceRule == "term term PCT factor") {
			// then division
			this->mipsCodePool.push_back("div $5, $3");
			this->mipsCodePool.push_back("mfhi $3");
		}
	}
}

void Procedure::generateExpr(Node *expr) {
	if (expr->reduceRule == "expr term") {
		generateTerm(expr->children[0]);
	}
	else {
		generateExpr(expr->children[0]);
		// we push the result of expr to RAM
		this->mipsCodePool.push_back("sw $3, -4($30)");
		this->mipsCodePool.push_back("sub $30, $30, $4");
		generateTerm(expr->children[2]);
		string typeE = this->typeExpr(expr->children[0]);
		string typeT = this->typeTerm(expr->children[2]);
		// we pop the value of expr from RAM,store to $5
		this->mipsCodePool.push_back("add $30, $30, $4");
		this->mipsCodePool.push_back("lw $5, -4($30)");
		if (expr->reduceRule == "expr expr PLUS term") {
			if (typeE == "int" && typeT == "int") {
				this->mipsCodePool.push_back("add $3, $5, $3");
				return;
			}
			if (typeE == "int*" && typeT == "int") {
				this->mipsCodePool.push_back("mult $3, $4");
				this->mipsCodePool.push_back("mflo $3");
				this->mipsCodePool.push_back("add $3, $5, $3");
				return;
			}
			if (typeE == "int" && typeT == "int*") {
				this->mipsCodePool.push_back("mult $5, $4");
				this->mipsCodePool.push_back("mflo $5");
				this->mipsCodePool.push_back("add $3, $5, $3");
				return;
			}
		}
		else if (expr->reduceRule == "expr expr MINUS term") {
			if (typeE == "int" && typeT == "int") {
				this->mipsCodePool.push_back("sub $3, $5, $3");
				return;
			}
			if (typeE == "int*" && typeT == "int") {
				this->mipsCodePool.push_back("mult $3, $4");
				this->mipsCodePool.push_back("mflo $3");
				this->mipsCodePool.push_back("sub $3, $5, $3");
				return;
			}
			if (typeE == "int*" && typeT == "int*") {
				this->mipsCodePool.push_back("sub $3, $5, $3");
				this->mipsCodePool.push_back("divu $3, $4");
				this->mipsCodePool.push_back("mflo $3");
			}
		}
	}
}

void Procedure::generateStatement(Node *statement) {
	if (statement->reduceRule == "statement lvalue BECOMES expr SEMI") {
		mipsCodePool.push_back(";; statement lvalue BECOMES expr SEMI");
		Node *lvalue = generateLvalue(statement->children[0]);
		Node *expr = statement->children[2];
		if (lvalue->reduceRule == "lvalue ID") {
			generateExpr(expr);
			string nameID = lvalue->children[0]->lexeme;
			string location = "";
			for (int i = 0; i < symbolTable.size(); i++) {
				if (symbolTable[i][0] == nameID) {
					location = symbolTable[i][2];
				}
			}
			string swStr = "sw $3, " + location;
			swStr = swStr + "($29)";
			this->mipsCodePool.push_back(swStr);
		}
		else if (lvalue->reduceRule == "lvalue STAR factor") {
			// first put the value of expr on $3
			generateExpr(expr);
			this->mipsCodePool.push_back("sw $3, -4($30)");
			this->mipsCodePool.push_back("sub $30, $30, $4");
			// now we start to process lvalue
			generateFactor(lvalue->children[1]);
			// if it is a pointer, we already load the content of the pointer which is an address in $3
			// then we need to store the value of expr to this address
			this->mipsCodePool.push_back("add $30, $30, $4");
			this->mipsCodePool.push_back("lw $5, -4($30)");
			this->mipsCodePool.push_back("sw $5, 0($3)");
		}
	}
	if (statement->reduceRule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		mipsCodePool.push_back(";; statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE");
		Node *test = statement->children[2];
		Node *firstStatements = statement->children[5];
		Node *secondStatements = statement->children[9];
		ConstructionStack::ifCount++;
		stringstream ss;
		ss << ConstructionStack::ifCount;
		string count = ss.str();
		string se = "se" + count;
		string ee = "ee" + count;
		generateTest(test);
		this->mipsCodePool.push_back("beq $3, $0, "+ se);
		generateStatements(firstStatements);
		this->mipsCodePool.push_back("beq $0, $0, "+ ee);
		this->mipsCodePool.push_back(se+":");
		generateStatements(secondStatements);
		this->mipsCodePool.push_back(ee+":");
	}
	if (statement->reduceRule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		mipsCodePool.push_back(";; statement WHILE LPAREN test RPAREN LBRACE statements RBRACE");
		Node *test = statement->children[2];
		Node *statements = statement->children[5];
		stringstream ss;
		ConstructionStack::whileCount++;
		ss << ConstructionStack::whileCount;
		string count = ss.str();
		string sw = "sw" + count;
		string ew = "ew" + count;
		this->mipsCodePool.push_back(sw+":");
		generateTest(test);
		this->mipsCodePool.push_back("beq $3, $0, " + ew);
		generateStatements(statements);
		this->mipsCodePool.push_back("beq $0, $0, " + sw);
		this->mipsCodePool.push_back(ew + ":");
		
	}
	if (statement->reduceRule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		mipsCodePool.push_back(";; statement PRINTLN LPAREN expr RPAREN SEMI");
		generateExpr(statement->children[2]);
		this->mipsCodePool.push_back("add $1, $3, $0");
		this->mipsCodePool.push_back("add $18, $31, $0");
		
		this->mipsCodePool.push_back("lis $10");
		this->mipsCodePool.push_back(".word print");
		this->mipsCodePool.push_back("jalr $10");
		//this->mipsCodePool.push_back("add $17, $1, $0");
		this->mipsCodePool.push_back("add $31, $18, $0");
	}
	if (statement->reduceRule == "statement DELETE LBRACK RBRACK expr SEMI") {
		mipsCodePool.push_back(";; statement DELETE LBRACK RBRACK expr SEMI");
		//this is about memory, think about it later
		generateExpr(statement->children[3]);
		this->mipsCodePool.push_back("add $1, $3, $0");
		this->mipsCodePool.push_back("beq $1, $11, 3");
		this->mipsCodePool.push_back("lis $10");
		this->mipsCodePool.push_back(".word delete");
		this->mipsCodePool.push_back("jalr $10");
	}
}

void Procedure::generateStatements(Node *statements) {
	if (statements->numOfChildren != 0) {
		generateStatements(statements->children[0]);
		generateStatement(statements->children[1]);
	}
	return;
}
void Procedure::generateCode() {
	if (this->name == "wain") {
		// for now we only generate code for wain
		// first we try to ignore procedures, just focus on main
		// setup prolog:
		string firstParamType = typeDcl(rootNode->children[3]);
		bool pointerType = firstParamType == "int*";
		mipsCodePool.push_back(";; wain:");
		mipsCodePool.push_back(";; prolog");
		mipsCodePool.push_back("lis $4");
		mipsCodePool.push_back(".word 4");
		mipsCodePool.push_back("lis $11");
		mipsCodePool.push_back(".word 1");
		mipsCodePool.push_back("sw $31, -4($30)");
		mipsCodePool.push_back("sub $30, $30, $4");
		this->mipsCodePool.push_back("sub $29, $30, $4");
		if (pointerType) {
			this->mipsCodePool.push_back("lis $10");
			this->mipsCodePool.push_back(".word init");
			this->mipsCodePool.push_back("jalr $10");
		}
		else {
			this->mipsCodePool.push_back("sw $2, -4($30)");
			this->mipsCodePool.push_back("sub $30, $30, $4");
			this->mipsCodePool.push_back("add $2, $0, $0");
			this->mipsCodePool.push_back("lis $10");
			this->mipsCodePool.push_back(".word init");
			this->mipsCodePool.push_back("jalr $10");
			this->mipsCodePool.push_back("add $30, $30, $4");
			this->mipsCodePool.push_back("lw $2, -4($30)");
		}
		//body part:
		mipsCodePool.push_back(";; body");

		this->buildSymbolTable();
		// we print out symbol table as a comment
		mipsCodePool.push_back(";; Symbol Table Generated");
		for (int i = 0; i < symbolTable.size(); i++) {
			string singleItem = ";;  ";
			singleItem = singleItem + symbolTable[i][0] + "   " + symbolTable[i][1] + "   " + symbolTable[i][2];
			this->mipsCodePool.push_back(singleItem); 
		}
		mipsCodePool.push_back(";; store local variables on RAM");
		this->storeLocalVariableOnRAM();
		mipsCodePool.push_back(";; start processing statement");
		generateStatements(this->rootNode->children[9]);
		mipsCodePool.push_back(";; start processing return");
		generateExpr(this->rootNode->children[11]);
		//Epilog:
		mipsCodePool.push_back(";; epilog");
		mipsCodePool.push_back("add $30, $29, $4");
		mipsCodePool.push_back("add $30, $30, $4");
		mipsCodePool.push_back("lw $31, -4($30)");
		mipsCodePool.push_back("jr $31");
	}
	else {
		// this part is for procedures
		// first put a lable for the start of the function
		string lableStr = "F" + this->name + ":";
		mipsCodePool.push_back(lableStr);
		// set $29
		mipsCodePool.push_back("sub $29, $30, $4");

		this->buildSymbolTable();
		mipsCodePool.push_back(";; Symbol Table Generated");
		for (int i = 0; i < symbolTable.size(); i++) {
			string singleItem = ";;  ";
			singleItem = singleItem + symbolTable[i][0] + "   " + symbolTable[i][1] + "   " + symbolTable[i][2];
			this->mipsCodePool.push_back(singleItem); 
		}
		this->storeLocalVariableOnRAM();
		mipsCodePool.push_back(";; start processing statement");
		generateStatements(this->rootNode->children[7]);
		mipsCodePool.push_back(";; start processing return");
		generateExpr(this->rootNode->children[9]);
		// ;; epilog
		mipsCodePool.push_back("add $30, $29, $4");
		mipsCodePool.push_back("jr $31");
	}
}

void Procedure::buildSymbolTable() {
	if (this->name == "wain") {
		int i = 0;
		for (auto it = localVariables.begin(); it != localVariables.end(); it++, i = i - 4) {
			vector<string> tempSymbolTableItem;
			tempSymbolTableItem.push_back(it->first);
			tempSymbolTableItem.push_back(it->second);
			stringstream ss;
			ss << i;
			string strLocation = ss.str();
			tempSymbolTableItem.push_back(strLocation);
			this->symbolTable.push_back(tempSymbolTableItem);
		}
	}
	else {
		int numOfParam = this->parameterList.size();
		int totalSize = numOfParam * 4;
		for (int i = 0; i < this->parameterList.size(); i++) {
			vector<string> tempSymbolTableItem;
			tempSymbolTableItem.push_back(parameterList[i].first);
			tempSymbolTableItem.push_back(parameterList[i].second);
			stringstream ss;
			ss << (totalSize - 4 * i);
			string strLocation = ss.str();
			tempSymbolTableItem.push_back(strLocation);
			this->symbolTable.push_back(tempSymbolTableItem);
		}
		// now we need to build localVars
		int i = 0;
		for (auto it = localVariables.begin(); it != localVariables.end(); it++) {
			string localVarName = it->first;
			bool param = false;
			for (int j = 0; j < parameterList.size(); j++) {
				if (parameterList[j].first == localVarName) {
					param = true; break;
				}
			}
			if (param) continue;
			vector<string> tempSymbolTableItem;
			tempSymbolTableItem.push_back(it->first);
			tempSymbolTableItem.push_back(it->second);
			stringstream ss;
			ss << i;
			string strLocation = ss.str();
			tempSymbolTableItem.push_back(strLocation);
			this->symbolTable.push_back(tempSymbolTableItem);
			i = i - 4;
		}
	}
}

void Procedure::storeLocalVariableOnRAM() {
	
	// if this procedure is a wain
	if (this->name == "wain") {
		// set up the frame pointer
		
		// calculate the offset of $30
		int sizeOfSymbolTable = symbolTable.size();
		stringstream ss;
		ss << sizeOfSymbolTable * 4;
		string offset = ss.str();
		this->mipsCodePool.push_back("lis $3");
		string offsetWordString = ".word " + offset;
		this->mipsCodePool.push_back(offsetWordString);
		this->mipsCodePool.push_back("sub $30, $30, $3");

		Node *firstDcl = rootNode->children[3];
		string firstParameterName = firstDcl->children[1]->lexeme;
		Node *secondDcl = rootNode->children[5];
		string secondParameterName = secondDcl->children[1]->lexeme;
		for (int i = 0; i < symbolTable.size(); i++) {
			if (symbolTable[i][0] == firstParameterName) {
				string location = symbolTable[i][2];
				string storeStr = "sw $1, " + location;
				storeStr = storeStr + "($29)";
				this->mipsCodePool.push_back(storeStr);
				continue;
			}
			if (symbolTable[i][0] == secondParameterName) {
				string location = symbolTable[i][2];
				string storeStr = "sw $2, " + location;
				storeStr = storeStr + "($29)";
				this->mipsCodePool.push_back(storeStr);
				continue;
			}
		}
		this->generateDcls(this->rootNode->children[8]);
	}
	else {
		// we need to update stack pointer
		int NumOflocalNonParamVar = this->localVariables.size() - this->parameterList.size();
		stringstream ss;
		ss << NumOflocalNonParamVar * 4;
		string offset = ss.str();
		this->mipsCodePool.push_back("lis $3");
		string offsetWordString = ".word " + offset;
		this->mipsCodePool.push_back(offsetWordString);
		this->mipsCodePool.push_back("sub $30, $30, $3");
		// if this is a procedure other than wain
		this->generateDcls(this->rootNode->children[6]);
	}
}
Procedure::~Procedure() {};
void Procedure::typeCheckStatements(Node *statements) {
	if (statements->reduceRule == "statements") {
		return;
	}
	else if (statements->reduceRule == "statements statements statement") {
		typeCheckStatements(statements->children[0]);
		typeCheckStatement(statements->children[1]);
	}
}

void Procedure::typeCheckStatement(Node * statement) {
	if (statement->reduceRule == "statement lvalue BECOMES expr SEMI") {
		string typeL = typeLvalue(statement->children[0]);
		string typeE = typeExpr(statement->children[2]);
		if (typeL != typeE) {
			throw string("ERROR: a <statement lvalue BECOMES expr SEMI> statement, left right diff type");
		}
	}
	if (statement->reduceRule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		Node *test = statement->children[2];
		Node *firstStatements = statement->children[5];
		Node *secondStatements = statement->children[9];
		typeCheckTest(test);
		typeCheckStatements(firstStatements);
		typeCheckStatements(secondStatements);
	}
	if (statement->reduceRule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		Node *test = statement->children[2];
		Node *statements = statement->children[5];
		typeCheckTest(test);
		typeCheckStatements(statements);
	}
	if (statement->reduceRule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		string typeE = typeExpr(statement->children[2]);
		if (typeE != "int") {
			throw string("ERROR: try to print a non int");
		}
	}
	if (statement->reduceRule == "statement DELETE LBRACK RBRACK expr SEMI") {
		string typeE = typeExpr(statement->children[3]);
		if (typeE != "int*") {
			throw string("ERROR: try to delete a non pointer");
		}
	}
}

void Procedure::typeCheckTest(Node * test) {
	string typeExp1 = typeExpr(test->children[0]);
	string typeExp2 = typeExpr(test->children[2]);
	if (typeExp1 != typeExp2) {
		throw string("ERROR: type diff in a test");
	}
	return;
}

// return a string containing the type of a expr		
string Procedure::typeExpr(Node *expr) {
	if (expr->reduceRule == "expr term") {
		return typeTerm(expr->children[0]);
	}
	else if (expr->reduceRule == "expr expr PLUS term") {
		string typeEx = typeExpr(expr->children[0]);
		string typeT = typeTerm(expr->children[2]);
		if (typeEx == "int" && typeT == "int") {
			return "int";
		}
		else if ((typeEx == "int" && typeT == "int*") ||
			(typeEx == "int*" && typeT == "int")) {
			return "int*";
		}
		else {
			throw string("ERROR: wrong type arround addition, maybe it's two pointer");
		}
	}
	else if (expr->reduceRule == "expr expr MINUS term") {
		string typeEx = typeExpr(expr->children[0]);
		string typeT = typeTerm(expr->children[2]);
		if ((typeEx == "int" && typeT == "int") ||
			(typeEx == "int*" && typeT == "int*")) {
			return "int";
		}
		else if (typeEx == "int*" && typeT == "int") {
			return "int*";
		}
		else {
			throw string("ERROR: wrong type arround subtraction, subtract pointer from a int");
		}
	}
}

// return a string containing the type of a term	
string Procedure::typeTerm(Node *term) {
	if (term->reduceRule == "term factor") {
		return typeFactor(term->children[0]);
	}
	else if (term->reduceRule == "term term STAR factor") {
		string typeT = typeTerm(term->children[0]);
		string typeF = typeFactor(term->children[2]);
		if (typeT == "int" && typeF == "int") {
			return "int";
		}
		else {
			throw string("ERROR: wrong operand type around multiple");
		}
	}
	else if (term->reduceRule == "term term SLASH factor") {
		string typeT = typeTerm(term->children[0]);
		string typeF = typeFactor(term->children[2]);
		if (typeT == "int" && typeF == "int") {
			return "int";
		}
		else {
			throw string("ERROR: wrong operand type arround slash");
		}
	}
	else if (term->reduceRule == "term term PCT factor") {
		string typeT = typeTerm(term->children[0]);
		string typeF = typeFactor(term->children[2]);
		if (typeT == "int" && typeF == "int") {
			return "int";
		}
		else {
			throw string("ERROR: wrong operand type arround percentage");
		}
	}
}

// return a string containing the type of a factor	
string Procedure::typeFactor(Node *factor) {
	// if the id is a variable
	if (factor->reduceRule == "factor ID") {
		return typeID(factor->children[0]);
	}
	// for rule: factor -> NUM, NULL, do nothing for now
	if (factor->reduceRule == "factor NUM") {
		return "int";
	}

	if (factor->reduceRule == "factor NULL") {
		return "int*";
	}

	// for rule: factor -> STAR factor
	if (factor->reduceRule == "factor STAR factor") {
		string typeFac = typeFactor(factor->children[1]);
		if (typeFac == "int*") {
			return "int";
		}
		else {
			throw string("ERROR: dereference on a non pointer");
		}
	}

	// for rule: factor -> LPAREN expr RPAREN
	if (factor->reduceRule == "factor LPAREN expr RPAREN") {
		return typeExpr(factor->children[1]);
	}

	// for rule: factor -> AMP lvalue
	if (factor->reduceRule == "factor AMP lvalue") {
		string typelv = typeLvalue(factor->children[1]);
		if (typelv == "int") {
			return "int*";
		}
		else {
			throw string("ERROR: reference on a non int");
		}
	}

	// for rule: factor -> NEW INT LBRACK expr RBRACK
	if (factor->reduceRule == "factor NEW INT LBRACK expr RBRACK") {
		string typeEx = typeExpr(factor->children[3]);
		if (typeEx == "int") {
			return "int*";
		}
		else {
			throw string("ERROR: try to allocate an array of a length of <non int>");
		}
	}

	// for rule: factor -> ID LPAREN RPAREN
	if (factor->reduceRule == "factor ID LPAREN RPAREN" ||
		factor->reduceRule == "factor ID LPAREN arglist RPAREN") {
		// we need to check if we have this function with the same parameters
		return "int";
	}
}

// return a string containing the type of a ID	
string Procedure::typeID(Node *ID) {
	unordered_map<string, string>::iterator itr = this->localVariables.find(ID->lexeme);
	if (itr != localVariables.end()) {
		string type = itr->second;
		return type;
	}
	else {
		throw string("ERROR: check type before define");
	}
}

// return a string containing the type of a lvalue	
string Procedure::typeLvalue(Node *lvalue) {
	if (lvalue->reduceRule == "lvalue ID") {
		return typeID(lvalue->children[0]);
	}
	else if (lvalue->reduceRule == "lvalue STAR factor") {
		string typeF = typeFactor(lvalue->children[1]);
		if (typeF == "int*") {
			return "int";
		}
		else {
			throw string("ERROR: dereference on a non pointer");
		}
	}
	else if (lvalue->reduceRule == "lvalue LPAREN lvalue RPAREN") {
		return typeLvalue(lvalue->children[1]);
	}
}

//INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
// main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
void Procedure::processProcedure() {
	if (this->name != "wain") {
		processDcls(rootNode->children[6]);
		processStatements(rootNode->children[7]);
		processExpr(rootNode->children[9]);
		typeCheckDCLS(rootNode->children[6]);
		typeCheckStatements(rootNode->children[7]);
		if ("int" != typeExpr(rootNode->children[9])) {
			string errMsg = "ERROR: return type of funtion ";
			errMsg = errMsg + this->name;
			errMsg = errMsg + "is wrong";
			throw errMsg;
		}
		
	}
	else {
		processDcls(rootNode->children[8]);
		processStatements(rootNode->children[9]);
		processExpr(rootNode->children[11]);
		if (typeDcl(rootNode->children[5]) != "int") {
			throw string("ERROR: the second parameter of main is wrong");
		}
		typeCheckDCLS(rootNode->children[8]);
		typeCheckStatements(rootNode->children[9]);
		if ("int" != typeExpr(rootNode->children[11])) {
			string errMsg = "ERROR: return type of funtion wain is wrong";
			throw errMsg;
		}
	}
	//this->print();
}

void Procedure::print() {
	cerr << name << " ";
	for (int i = 0; i < parameterList.size(); i++) {
		cerr << parameterList[i].second;
		if (i != parameterList.size() - 1) {
			cerr << " ";
		}
	}
	cerr << endl;
	for (auto it = localVariables.begin(); it != localVariables.end(); it++) {
		cerr << it->first << " " << it->second << endl;
	}
	// print symbol table for test:
	// TEST_CODE_BEGIN
	/*cerr << "Symbol table for " << this->name << ":" << endl;
	for (int i = 0; i < symbolTable.size(); i++) {
		cerr << symbolTable[i][0] << " " << symbolTable[i][1] << " " << symbolTable[i][2] << endl;
	}*/
	// TEST_CODE_END
}

void Procedure::processLvalue(Node *lvalue) {
	if (lvalue->reduceRule == "lvalue ID") {
		// we need to check if the id is defined
		unordered_map<string, string>::const_iterator got = localVariables.find(lvalue->children[0]->lexeme);
		if (got == localVariables.end()) {
			// if the variable is not found
			string errorMsg = "ERROR: variable " + lvalue->children[0]->lexeme;
			errorMsg = errorMsg + " is not defined";
			throw errorMsg;
		}
		// if the id is defined, then we do other things
	}
	if (lvalue->reduceRule == "lvalue STAR factor") {
		processFactor(lvalue->children[1]);
	}
	if (lvalue->reduceRule == "lvalue LPAREN lvalue RPAREN") {
		processLvalue(lvalue->children[1]);
	}
}

vector<string> Procedure::processArglist(Node *argList) {
	vector<string> answer;
	while (argList->numOfChildren == 3) {
		processExpr(argList->children[0]);
		answer.push_back(typeExpr(argList->children[0]));
		argList = argList->children[2];
	}
	processExpr(argList->children[0]);
	answer.push_back(typeExpr(argList->children[0]));
	return answer;
}

void Procedure::processFactor(Node *factor) {
	// if the id is a variable
	if (factor->reduceRule == "factor ID") {
		unordered_map<string, string>::const_iterator got = localVariables.find(factor->children[0]->lexeme);
		if (got == localVariables.end()) {
			// if the variable is not found
			string errorMsg = "ERROR: variable " + factor->children[0]->lexeme;
			errorMsg = errorMsg + " is not defined";
			throw errorMsg;
		}
		// if the variable is found do nothing for now
	}
	// for rule: factor -> NUM, NULL, do nothing for now

	// for rule: factor -> STAR factor
	if (factor->reduceRule == "factor STAR factor") {
		processFactor(factor->children[1]);
		return;
	}
	// for rule: factor -> LPAREN expr RPAREN
	if (factor->reduceRule == "factor LPAREN expr RPAREN") {
		processExpr(factor->children[1]);
		return;
	}
	// for rule: factor -> AMP lvalue
	if (factor->reduceRule == "factor AMP lvalue") {
		processLvalue(factor->children[1]);
		return;
	}
	// for rule: factor -> NEW INT LBRACK expr RBRACK
	if (factor->reduceRule == "factor NEW INT LBRACK expr RBRACK") {
		processExpr(factor->children[3]);
		return;
	}
	// for rule: factor -> ID LPAREN RPAREN
	if (factor->reduceRule == "factor ID LPAREN RPAREN") {
		// we need to check if we have this function with the same parameters
		Node *ID = factor->children[0];
		string funcName = ID->lexeme;
		bool exist = false;
		for (int i = 0; i < procedures.size(); i++) {
			if (procedures[i]->name == funcName) {
				exist = true;
				if (procedures[i]->parameterList.size() != 0) {
					string errorMsg = "ERROR: function " + funcName;
					errorMsg = errorMsg + " is called with wrong number of arguments";
					throw errorMsg;
				}
			}
		}
		if (!exist) {
			string errorMsg = "ERROR: function " + funcName;
			errorMsg = errorMsg + " is undefined";
			throw errorMsg;
		}
		return;
	}
	// for rule: factor -> ID LPAREN arglist RPAREN
	if (factor->reduceRule == "factor ID LPAREN arglist RPAREN") {
		// we need to check if this function exists and the parameters are correct
		Node *ID = factor->children[0];
		Node *argList = factor->children[2];
		// a vector of argments' types
		vector<string> arglist = processArglist(argList);
		string funcName = ID->lexeme;
		if (funcName == "wain") {
			throw string("ERROR wain cannot be called within a function");
		}
		bool exist = false;
		bool typeCorrect = false;
		for (int i = 0; i < procedures.size(); i++) {
			if (procedures[i]->name == funcName) {
				exist = true;
				// get the correst argument list		
				vector<std::pair<string, string>> &matchProcedureParameterList = procedures[i]->parameterList;
				if (matchProcedureParameterList.size() == arglist.size()) {
					for (int i = 0; i < arglist.size(); i++) {
						if (arglist[i] != matchProcedureParameterList[i].second) {
							// there is a argument with wrong type
							string errorMsg = "ERROR: wrong type of arguments for function: ";
							errorMsg = errorMsg + funcName;
							throw errorMsg;
						}
					}
				}
				else {
					string errorMsg = "ERROR: wrong number of arguments for function: ";
					errorMsg = errorMsg + funcName;
					throw errorMsg;
				}
			}
		}
		if (!exist) {
			string errorMsg = "ERROR: function " + funcName;
			errorMsg = errorMsg + " is undefined";
			throw errorMsg;
		}
		return;
	}
}

void Procedure::processTerm(Node *term) {
	if (term->numOfChildren == 1) {
		processFactor(term->children[0]);
	}
	else {
		processTerm(term->children[0]);
		processFactor(term->children[2]);
	}
}

void Procedure::processExpr(Node *Expr) {
	if (Expr->reduceRule == "expr term") {
		processTerm(Expr->children[0]);
	}
	else {
		processExpr(Expr->children[0]);
		processTerm(Expr->children[2]);
	}
}

void Procedure::processTest(Node *test) {
	processExpr(test->children[0]);
	processExpr(test->children[2]);
}

void Procedure::processStatement(Node *statement) {
	if (statement->reduceRule == "statement lvalue BECOMES expr SEMI") {
		Node *lvalue = statement->children[0];
		Node *expr = statement->children[2];
		processLvalue(lvalue);
		processExpr(expr);
	}
	if (statement->reduceRule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		Node *test = statement->children[2];
		Node *firstStatements = statement->children[5];
		Node *secondStatements = statement->children[9];
		processTest(test);
		processStatements(firstStatements);
		processStatements(secondStatements);
	}
	if (statement->reduceRule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		Node *test = statement->children[2];
		Node *statements = statement->children[5];
		processTest(test);
		processStatements(statements);
	}
	if (statement->reduceRule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		Node *expr = statement->children[2];
		processExpr(expr);
	}
	if (statement->reduceRule == "statement DELETE LBRACK RBRACK expr SEMI") {
		Node *expr = statement->children[3];
		processExpr(expr);
	}
}

void Procedure::processStatements(Node *statements) {
	while (statements->numOfChildren != 0) {
		processStatement(statements->children[1]);
		statements = statements->children[0];
	}
	return;
}

void Procedure::generateDcls(Node *dcls) {
	while (dcls->numOfChildren != 0) {
		string typeOfRvalue = dcls->children[3]->type;
		string nameOfVar = dcls->children[1]->children[1]->lexeme;
		string location = "";
		for (int i = 0; i < symbolTable.size(); i++) {
			if (symbolTable[i][0] == nameOfVar) location = symbolTable[i][2];
		}
		if (typeOfRvalue == "NUM") {
			//process number declaration
			string numStr = dcls->children[3]->lexeme;
			// load the value of this dcl to $3
			this->mipsCodePool.push_back("lis $3");
			string wordStr = ".word " + numStr;
			this->mipsCodePool.push_back(wordStr);
		}
		else if(typeOfRvalue == "NULL"){
			//process NULL declaration
			this->mipsCodePool.push_back("add $3, $0, $11");
		}
		string saveOnRAMStr = "sw $3, ";
		saveOnRAMStr = saveOnRAMStr + location;
		saveOnRAMStr = saveOnRAMStr + "($29)";
		this->mipsCodePool.push_back(saveOnRAMStr);
		dcls = dcls->children[0];
	}
	return;
}
void Procedure::processDcls(Node *dcls) {
	while (dcls->numOfChildren != 0) {
		this->processDcl(dcls->children[1], false);
		dcls = dcls->children[0];
	}
	return;
}

void Procedure::typeCheckDCLS(Node * dcls) {
	while (dcls->numOfChildren != 0) {
		string typeOfRvalue = dcls->children[3]->type;
		string typeOfLvalue = typeDcl(dcls->children[1]);
		if ((typeOfLvalue == "int" && typeOfRvalue == "NUM") ||
			(typeOfLvalue == "int*" && typeOfRvalue == "NULL")) {
		}
		else {
			throw string("ERROR: declaration type wrong");
		}
		dcls = dcls->children[0];
	}
	return;
}

string Procedure::typeDcl(Node *dcl) {
	Node * type = dcl->children[0];
	return type->numOfChildren == 1 ? "int" : "int*";
}

void Procedure::processDcl(Node * dcl, bool param) {
	Node * type = dcl->children[0];
	Node *ID = dcl->children[1];
	string typeString;
	if (type->reduceRule == "type INT STAR") {
		typeString = "int*";
	}
	else {
		typeString = "int";
	}
	string idString = ID->lexeme;
	if (localVariables.find(idString) != localVariables.end()) {
		string error = "ERROR parameter redefined in function:" + this->name;
		throw error;
	}
	//if (this->parameterList[i].first == typeString &&
	//	this->parameterList[i].second == idString) {
	//	string error = "ERROR parameter redefined in function:" + this->name;
	//	throw error;
	//}
	if (param) {
		this->parameterList.push_back(std::make_pair(idString, typeString));
	}
	localVariables[idString] = typeString;
}

void Procedure::processParamlist(Node *paramlist) {
	while (paramlist->numOfChildren == 3) {
		Node * dclNode = paramlist->children[0];
		processDcl(dclNode, true);
		paramlist = paramlist->children[2];
	}

	// we still need to add one more
	Node *dclNode = paramlist->children[0];
	processDcl(dclNode, true);
}

void Procedure::processParams(Node *params) {
	if (params->numOfChildren == 0) return;
	if (params->numOfChildren == 1) {
		params = params->children[0];
		processParamlist(params);
	}
}

Procedure::Procedure(Node *rootNode, vector<Procedure *>&procedures, vector<string> &mipsCodePool) : rootNode{ rootNode }, procedures{ procedures }, returnType{ "int" }, 
name{ rootNode->children[1]->lexeme }, mipsCodePool{ mipsCodePool } {
	localVariables = unordered_map<string, string>();
	parameterList = vector<std::pair<string, string>>();
	// if this procedure is a "procedure"
	for (int i = 0; i < procedures.size(); i++) {
		if (procedures[i]->name == name) {
			string errorMsg = "ERROR: function " + name;
			errorMsg = errorMsg + " is already defined";
			throw errorMsg;
		}
	}
	if (name != "wain") {
		Node *params = rootNode->children[3];
		processParams(params);
		return;
	}
	else {
		// if this is actually a main
		Node * firstDcl = rootNode->children[3];
		Node * secondDcl = rootNode->children[5];
		processDcl(firstDcl, true);
		processDcl(secondDcl, true);
	}
}


int ConstructionStack::ifCount = 0;
int ConstructionStack::whileCount = 0;

void ConstructionStack::outputMips() {
	//cerr << "Mips code generated:" << endl;
	for (int i = 0; i < mipsCodePool.size(); i++) {
		cout << mipsCodePool[i] << endl;
	}
}

void ConstructionStack::findProcedures(Node * startNode) {
	mipsCodePool.push_back(".import print");
	mipsCodePool.push_back(".import new");
	mipsCodePool.push_back(".import delete");
	mipsCodePool.push_back(".import init");
	Node * proceduresNode = startNode->children[1];
	while (proceduresNode->numOfChildren == 2) {
		procedures.push_back(new Procedure{ proceduresNode->children[0] , this->procedures , this->mipsCodePool});
		procedures.back()->processProcedure();
		proceduresNode = proceduresNode->children[1];
	}
	main = new Procedure{ proceduresNode->children[0] , this->procedures, this->mipsCodePool };
	main->processProcedure();
	main->generateCode();
	for (int i = 0; i < this->procedures.size(); i++) {
		this->procedures[i]->generateCode();
	}
}

void ConstructionStack::printProcedures() {
	for (int i = 0; i < this->procedures.size(); i++) {
		procedures[i]->print();
		cerr << endl;
	}
	this->main->print();
}

void ConstructionStack::analyzeNodeAfterConstruct() {
	for (int i = 0; i < nodeStack.size(); i++) {
		findProcedures(nodeStack[i]);
	}
	//printProcedures();
}

ConstructionStack::~ConstructionStack() {
	for (vector<Node *>::iterator it = nodeStack.begin(); it != nodeStack.end(); it++) {
		delete *it;
	}
	//vector<Procedure *> procedures;
	for (vector<Procedure *>::iterator it = procedures.begin(); it != procedures.end(); it++) {
		delete *it;
	}
	delete this->main;
}

void ConstructionStack::print() {
	for (vector<Node *>::reverse_iterator rit = this->nodeStack.rbegin(); rit != this->nodeStack.rend(); rit++) {
		(*rit)->print();
	}
}

void ConstructionStack::putOnNode(Node * node) {
	int sizeOfStack = this->nodeStack.size();
	int numOfChildNode = node->numOfChildren;
	if (sizeOfStack == 0 || numOfChildNode == 0) {
		this->nodeStack.push_back(node);
		return;
	}
	vector<Node *>::reverse_iterator rbg = this->nodeStack.rbegin();
	for (vector<Node *>::reverse_iterator rit = rbg; rit < rbg + numOfChildNode; rit++) {
		node->addChild(*rit);
	}
	for (int i = numOfChildNode; i >= 1; i--) {
		this->nodeStack.pop_back();
	}
	nodeStack.push_back(node);
}
int main()
{
	ConstructionStack * stack = new ConstructionStack();
	string line;
	vector<string> inputRules;
	// first try to get reversed input 
	while (getline(cin, line)) {
		inputRules.push_back(line);
	}
	for (vector<string>::reverse_iterator rit = inputRules.rbegin(); rit != inputRules.rend(); rit++) {
		Node *temp = new Node{ *rit };
		stack->putOnNode(temp);
	}
	//stack->print();
	// for(int  i = 0; i < stack->nodeStack.size(); i++){
	// 	cout << stack->nodeStack[i]->type<<endl;
	// }
	//stack->nodeStack[stack->nodeStack.size()  -1]->print();
	try {
		stack->analyzeNodeAfterConstruct();
		stack->outputMips();
	}
	catch (string e) {
		cerr << e << endl;
		delete stack;
		return 0;
	}
	delete stack;
	return 0;
}

