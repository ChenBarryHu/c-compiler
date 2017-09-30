#include "kind.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <algorithm>    // std::find

// Use only the needed aspects of each namespace
using std::string;
using std::vector;
using std::endl;
using std::cerr;
using std::cin;
using std::getline;
using ASM::Token;
using ASM::Lexer;

void printOutAllTokens(vector<Token*> &allTokens);
void assembleTokens(vector<Token *> &tokens);
void binaryGenerator(std::map<string, int> &st, vector< vector<Token *> > &correctInstr);
void outputWord(unsigned int operandValue);
void outputSymbleTable(std::map<string, int> &st);
void printOperands(vector<vector<Token *> > &correctInstr);
void checkComponentPosition(vector<Token *> &tokens);
bool checkLegalInstructionName(string instruName);
void analyzeLine(vector< Token * > &tokens, vector< unsigned int > &intermediateResult, std::map< int, string > &indexLinesNeedLabel,
	std::map<string, int> &st);
unsigned int assemblyAddType(vector< Token * > &tokens, vector< Token * >::iterator it2);
unsigned int assemblyMultType(vector< Token * > &tokens, vector< Token * >::iterator it2);
unsigned int assemblyMfhiType(vector< Token * > &tokens, vector< Token * >::iterator it2);
unsigned int assemblyLwType(vector< Token * > &tokens, vector< Token * >::iterator it2);
unsigned int assemblyBeqType(vector< Token * > &tokens, vector< Token * >::iterator it2, bool & labelUsed, string &potentialLabel);
unsigned int assemblyJrType(vector< Token * > &tokens, vector< Token * >::iterator it2);
unsigned int assemblyWordType(vector< Token * > &tokens, vector< Token * >::iterator it2, bool &labelUsed, string &potentialLabel);

namespace {
	// legal string names for instructions
	const string instructionNames[] = {
		"add",       
		"sub",       
		"slt",       
		"sltu",
		"mult", 
		"multu", 
		"div",
		"divu",
		"mfhi", 
		"mflo", 
		"lis",
		"lw",
		"sw",
		"beq",
		"bne",
		"jr",
		"jalr",
		".word"
	};
}

int main(int argc, char* argv[]) {
	// Nested vector representing lines of Tokens
	// Needs to be used here to cleanup in the case
	// of an exception
	// st store the symble table;
	std::map<string, int>st;
	// each line is a correct instruction
	vector< vector<Token*> > correctInstr;
	vector< vector<Token*> > tokLines;
	std::map< int , string > indexLinesNeedLabel;
	vector<Token*> allTokens;
	vector< unsigned int > intermediateResult;
	bool waitingForOp = true;
	try {
		// Create a MIPS recognizer to tokenize
		// the input lines
		Lexer lexer;
		// Tokenize each line of the input
		string line;
		while (getline(cin, line)) {
			tokLines.push_back(lexer.scan(line));
		}

		vector<vector<Token*> >::iterator it;
		for (it = tokLines.begin(); it != tokLines.end(); ++it) {
			// scan each line
			analyzeLine(*it, intermediateResult, indexLinesNeedLabel, st );
			//checkComponentPosition(*it);
		}
		// after analyze every line, we got a intermediate result and a symbol table and set of instructions 
		// that need to add address
		// the following code add address to the instructions:
		for (std::map<int, string>::iterator it = indexLinesNeedLabel.begin(); it != indexLinesNeedLabel.end(); ++it) {
			unsigned int Instr = intermediateResult.at(it->first);
			string labelNeeded = it->second;
			if (st.find(labelNeeded) == st.end()) {
				// not found
				string errorMsg = "ERROR: label {" + labelNeeded;
				errorMsg = errorMsg + "} not found";
				throw string(errorMsg);
			}
			if (Instr == 0) {
				// this is a word instruction
				// we simply store the value in it
				intermediateResult[it->first] = st[labelNeeded];
			}
			else {
				// this is a beq or bne instruction
				int encodedValue = (st[labelNeeded] - (it->first) * 4 - 4) / 4;
				if (encodedValue > 32767 || 
					encodedValue < -32768) {
					throw ("ERROR: label value is out of range");
				}
				encodedValue = encodedValue & 65535;
				Instr = Instr | encodedValue;
				intermediateResult[it->first] = Instr;
			}
		}
		// our instructions are ready to be printed out...
		for (vector< unsigned int >::iterator it3 = intermediateResult.begin(); it3 != intermediateResult.end(); it3++) {
			outputWord(*it3);
		}
		outputSymbleTable(st);
	}
	catch (const string& msg) {
		// If an exception occurs print the message and end the program
		cerr << msg << endl;
	}
	// Delete the Tokens that have been made
	vector<vector<Token*> >::iterator it;
	for (it = tokLines.begin(); it != tokLines.end(); ++it) {
		vector<Token*>::iterator it2;
		for (it2 = it->begin(); it2 != it->end(); ++it2) {
			delete *it2;
		}
	}
}

void analyzeLine(vector< Token * > &tokens, vector< unsigned int > &intermediateResult, std::map< int , string > &indexLinesNeedLabel,
	std::map<string, int> &st) {
	vector< Token* >::iterator it2;
	// we must first deal with labels because they are the first couple items
	// in each line
	for (it2 = tokens.begin(); it2 != tokens.end(); it2++) {
		if (((*it2)->toString()) != "LABEL") {
			break;
		}
		string lexeme = (*(*it2)).getLexeme();
		string labelString = lexeme.substr(0, lexeme.size() - 1);
		if (st.find(labelString) == st.end()) {
			// the label hasnt been defined
			int address = intermediateResult.size() * 4;
			st[labelString] = address;
		}
		else {
			string errorMsg = "ERROR: label {" + labelString;
			errorMsg = errorMsg + "} redefine";
			throw string(errorMsg);
		}
	}
	
	// if there is no instruction in this line
	// then null instruction, return
	if (it2 == tokens.end()) {
		return;
	}
	// if there is a instruction,
	// we start to analyze instructions:
	if (((*it2)->toString() == "ID" || (*it2)->toString() == "DOTWORD" )&&
		checkLegalInstructionName((*it2)->getLexeme())) {
		// this is a legel instruction name
		//we need to get parameters next

		// 1. add, sub, slt, sltu
		// add $1, $2, $3
		if ((*it2)->getLexeme() == "add" ||
			(*it2)->getLexeme() == "sub" ||
			(*it2)->getLexeme() == "slt" ||
			(*it2)->getLexeme() == "sltu"){
				unsigned int mipsCode = assemblyAddType(tokens, it2);
				intermediateResult.push_back(mipsCode);
		}
		// 2. mult, multu, div, divu
		// mult $4, $5
		if ((*it2)->getLexeme() == "mult" ||
			(*it2)->getLexeme() == "multu" ||
			(*it2)->getLexeme() == "div" ||
			(*it2)->getLexeme() == "divu") {
				// correct number of COMMA, REGISTER, ID
				unsigned int mipsCode = assemblyMultType(tokens, it2);
				intermediateResult.push_back(mipsCode);
		}

		// 3. mfhi, mflo, lis
		// These opcodes have a single register operand, $d.
		if ((*it2)->getLexeme() == "mfhi" ||
			(*it2)->getLexeme() == "mflo" ||
			(*it2)->getLexeme() == "lis") {
			// correct number of COMMA, REGISTER, ID
			unsigned int mipsCode = assemblyMfhiType(tokens, it2);
			intermediateResult.push_back(mipsCode);
		}

		// 4. lw, sw
		// opcode $t, i($s) 
		if ((*it2)->getLexeme() == "lw" ||
			(*it2)->getLexeme() == "sw") {
			// correct number of COMMA, REGISTER, ID
			unsigned int mipsCode = assemblyLwType(tokens, it2);
			intermediateResult.push_back(mipsCode);
		}

		// 5. beq, bne
		// opcode $t, i($s) 
		if ((*it2)->getLexeme() == "beq" ||
			(*it2)->getLexeme() == "bne") {
			// correct number of COMMA, REGISTER, ID
			bool labelUsed = false;
			string potentialLabel = "";
			unsigned int mipsCode = assemblyBeqType(tokens, it2, labelUsed, potentialLabel);
			intermediateResult.push_back(mipsCode);
			if (labelUsed == true) {
				// we need to add location for it;
				// we store the index of it in intermediateResult in indexLInesNeedLabel
				indexLinesNeedLabel.insert(std::pair<int, string>(intermediateResult.size() - 1, potentialLabel));
			}
		}

		// 6. jr, jalr
		// opcode $t, i($s) 
		if ((*it2)->getLexeme() == "jr" ||
			(*it2)->getLexeme() == "jalr") {
			// correct number of COMMA, REGISTER, ID
			unsigned int mipsCode = assemblyJrType(tokens, it2);
			intermediateResult.push_back(mipsCode);
		}

		// 7. .word
		// opcode $t, i($s) 
		if ((*it2)->getLexeme() == ".word") {
			// correct number of COMMA, REGISTER, ID
			
			bool labelUsed = false;
			string potentialLabel = "";
			unsigned int mipsCode = assemblyWordType(tokens, it2, labelUsed, potentialLabel);
			intermediateResult.push_back(mipsCode);
			if (labelUsed == true) {
				// we need to add location for it;
				// we store the index of it in intermediateResult in indexLInesNeedLabel
				indexLinesNeedLabel.insert(std::pair< int , string >(intermediateResult.size() - 1, potentialLabel));
			}
		}
	}
	else {
		// throw errorMsg
		string errorMsg = "ERROR: expected an instruction name, but ";
		errorMsg = errorMsg + (*it2)->getLexeme();
		errorMsg = errorMsg + " was found";
		throw (errorMsg);
	}
}

bool checkLegalInstructionName(string instruName) {
	const string * p = std::find(instructionNames, instructionNames + 18, instruName);
	if (p != instructionNames + 18)
		return true;
	else
		return false;
}

unsigned int assemblyAddType(vector< Token * > &tokens, vector< Token * >::iterator it2) {
	// 1. add sub, slt, sltu
	// add $1, $2, $3
	if ((tokens.end() - it2) != 6) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int op = 0;
	string instructionName = (*(it2))->getLexeme();
	if (instructionName == "add") {
		op = 32;
	}
	else if (instructionName == "sub") {
		op = 34;
	}
	else if (instructionName == "slt") {
		op = 42;
	}
	else if (instructionName == "sltu") {
		op = 43;
	}
	if ((*(it2 + 1))->toString() == "REGISTER" &&
		(*(it2 + 2))->toString() == "COMMA" &&
		(*(it2 + 3))->toString() == "REGISTER" &&
		(*(it2 + 4))->toString() == "COMMA" &&
		(*(it2 + 5))->toString() == "REGISTER") {
		unsigned int d = (*(it2 + 1))->toInt();
		unsigned int s = (*(it2 + 3))->toInt();
		unsigned int t = (*(it2 + 5))->toInt();
		unsigned int word = (0 << 26) | (s << 21) | (t << 16) | (d << 11) | op;
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

unsigned int assemblyMultType(vector< Token * > &tokens, vector< Token * >::iterator it2) {
	// 2. mult $s, $t
	if ((tokens.end() - it2) != 4) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int op=0;
	string instructionName = (*(it2))->getLexeme();
	if (instructionName == "mult") {
		op = 24;
	}
	else if (instructionName == "multu"){
		op = 25;
	}
	else if (instructionName == "div") {
		op = 26;
	}
	else if (instructionName == "divu") {
		op = 27;
	}

	if ((*(it2 + 1))->toString() == "REGISTER" &&
		(*(it2 + 2))->toString() == "COMMA" &&
		(*(it2 + 3))->toString() == "REGISTER") {
		unsigned int s = (*(it2 + 1))->toInt();
		unsigned int t = (*(it2 + 3))->toInt();
		unsigned int word = (0 << 26) | (s << 21) | (t << 16)  | op;
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

unsigned int assemblyMfhiType(vector< Token * > &tokens, vector< Token * >::iterator it2) {
	// 3. mfhi $d
	//    mflo $d
	//    lis $d
	if ((tokens.end() - it2) != 2) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int op = 0;
	string instructionName = (*(it2))->getLexeme();
	if (instructionName == "mfhi") {
		op = 16;
	}
	else if (instructionName == "mflo") {
		op = 18;
	}
	else if (instructionName == "lis") {
		op = 20;
	}

	if ((*(it2 + 1))->toString() == "REGISTER") {
		unsigned int d = (*(it2 + 1))->toInt();
		unsigned int word = (0 << 26) | (d << 11) | op;
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

unsigned int assemblyLwType(vector< Token * > &tokens, vector< Token * >::iterator it2) {
	// opcode $t, i($s) 
	if ((tokens.end() - it2) != 7) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int func = 0;
	string instructionName = (*(it2))->getLexeme();
	if (instructionName == "lw") {
		// 100011
		func = 35;
	}
	else if (instructionName == "sw") {
		// 101011
		func = 43;
	}
	
	if ((*(it2 + 1))->toString() == "REGISTER"&&
		(*(it2 + 2))->toString() == "COMMA"&&
		((*(it2 + 3))->toString() == "INT" ||
			(*(it2 + 3))->toString() == "HEXINT")&&
		(*(it2 + 4))->toString() == "LPAREN"&&
		(*(it2 + 5))->toString() == "REGISTER"&&
		(*(it2 + 6))->toString() == "RPAREN") {
		unsigned int t = (*(it2 + 1))->toInt();
		int i = (*(it2 + 3))->toInt();
		unsigned int s = (*(it2 + 5))->toInt();
		if ((*(it2 + 3))->toString() == "INT") {
			if (i > 32767 ||
				i < -32768) {
				string errorMsg = "ERROR: The i value for ";
				errorMsg = errorMsg + instructionName;
				errorMsg = errorMsg + " is out of range";
				throw (errorMsg);
			}
		}
		if ((*(it2 + 3))->toString() == "HEXINT") {
			int hignBitsI = (i >> 16) & 65535;
			if (hignBitsI != 0) {
				string errorMsg = "ERROR: The i value for ";
				errorMsg = errorMsg + instructionName;
				errorMsg = errorMsg + " is out of range";
				throw (errorMsg);
			}
		}
		i = i & 65535;
		unsigned int word = (func << 26) | (s << 21) | (t << 16) |i;
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

unsigned int assemblyBeqType(vector< Token * > &tokens, vector< Token * >::iterator it2, bool & labelUsed, string &potentialLabel) {
	// beq $s, $t, i
	// 
	bool labelAsParam = false;
	if ((tokens.end() - it2) != 6) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int func = 0;
	string instructionName = (*(it2))->getLexeme();
	if (instructionName == "beq") {
		// 100011
		func = 4;
	}
	else if (instructionName == "bne") {
		// 101011
		func = 5;
	}


	if ((*(it2 + 1))->toString() == "REGISTER" &&
		(*(it2 + 2))->toString() == "COMMA" &&
		(*(it2 + 3))->toString() == "REGISTER" &&
		(*(it2 + 4))->toString() == "COMMA" &&
		((*(it2 + 5))->toString() == "INT" ||
		(*(it2 + 5))->toString() == "HEXINT" ||
		(*(it2 + 5))->toString() == "ID")) {
		unsigned int s = (*(it2 + 1))->toInt();
		unsigned int t = (*(it2 + 3))->toInt();
		int i = 0;
		if ((*(it2 + 5))->toString() == "INT" ) {
			i = (*(it2 + 5))->toInt();
			
			if (i > 32767 || i < -32768) {
				string errorMsg = "ERROR: The i value for ";
				errorMsg = errorMsg + instructionName;
				errorMsg = errorMsg + " is out of range";
				throw (errorMsg);
			}
			i = i & 65535;
		}
		else if ((*(it2 + 5))->toString() == "HEXINT" ) {
			i = (*(it2 + 5))->toInt();
			unsigned int unsignedI = i;
			int hignBitsOfI = (i >> 16) & 65535;
			if (hignBitsOfI != 0) {
				string errorMsg = "ERROR: The hex i value for ";
				errorMsg = errorMsg + instructionName;
				errorMsg = errorMsg + " is out of range";
				throw (errorMsg);
			}
			i = i & 65535;
		} else {
			labelUsed = true;
			potentialLabel = (*(it2 + 5))->getLexeme();
		}
		unsigned int word = (func << 26) | (s << 21) | (t << 16) | i;
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

unsigned int assemblyJrType(vector< Token * > &tokens, vector< Token * >::iterator it2) {
	// jr $s
	if ((tokens.end() - it2) != 2) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int op = 0;
	string instructionName = (*(it2))->getLexeme();
	if (instructionName == "jr") {
		// 1000
		op = 8;
	}
	else if (instructionName == "jalr") {
		// 1001
		op = 9;
	}

	if ((*(it2 + 1))->toString() == "REGISTER") {
		unsigned int s = (*(it2 + 1))->toInt();
		unsigned int word = (s << 21) | op;
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

unsigned int assemblyWordType(vector< Token * > &tokens, vector< Token * >::iterator it2, bool &labelUsed, string &potentialLabel) {
	// jr $s
	if ((tokens.end() - it2) != 2) {
		string errorMsg = "ERROR:wrong number of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
	unsigned int word = 0;
	if ((*(it2 + 1))->toString() == "INT" || 
		(*(it2 + 1))->toString() == "HEXINT") {
		unsigned int word = (*(it2 + 1))->toInt();
		return word;
	}
	else if((*(it2 + 1))->toString() == "ID"){
		labelUsed = true;
		potentialLabel = (*(it2 + 1))->getLexeme();
		return word;
	}
	else {
		string errorMsg = "ERROR: wrong type of tokens after";
		errorMsg = errorMsg + (*it2)->getLexeme();
		throw (errorMsg);
	}
}

void outputWord(unsigned int operandValue) {
	// std::cout << operandValue << endl;
	std::cout << char(operandValue >> 24)
		<< char(operandValue >> 16)
		<< char(operandValue >> 8)
		<< char(operandValue);
	// std::cout << endl;
}

void outputSymbleTable(std::map<string, int> &st) {
	for (std::map<string, int>::iterator it = st.begin(); it != st.end(); ++it) {
		std::cerr << it->first << " " << it->second << '\n';
	}
}
