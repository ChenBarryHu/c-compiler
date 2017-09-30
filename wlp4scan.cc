#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
// Use only the neeeded aspects of each namespace
using std::string;
using std::vector;
using std::endl;
using std::cerr;
using std::cin;
using std::getline;
using std::istringstream;
namespace{
	// The different kinds of Tokens that are part of different wlp4 instructions
	// Used for determining the correct Token to create in the wlp4 recognizer

	enum Kind {
		KEYWORD_OR_ID,
		NUM,
		LPAREN,
		RPAREN,
		LBRACE,
		RBRACE,
		BECOMES,
		EQ,
		NE,
		LT,
		GT,
		LE,
		GE,
		PLUS,
		MINUS,
		STAR,
		SLASH,
		PCT,
		COMMA,
		SEMI,
		LBRACK,
		RBRACK,
		AMP,
		ERR,
		WHITESPACE,
		ID,
		IF,
		RETURN,
		ELSE,
		WHILE,
		PRINTLN,
		WAIN,
		INT,
		NEW,
		DELETE,
		Null
	};
	Kind seperateKinds[] = {ID, NUM, RETURN, IF, ELSE, WHILE, PRINTLN, WAIN, 
		INT, NEW, Null, DELETE,EQ, NE, LT, LE, GT, GE, BECOMES};
	std::set<Kind> seperateSet1(seperateKinds, seperateKinds + 12);
	std::set<Kind> seperateSet2(seperateKinds +12, seperateKinds + 19);

	enum State {
		ST_ERR,
		ST_START,
		ST_KEYWORD_OR_ID,          // 
		ST_NUM,                    // NUMS
		ST_LPAREN,                 // (
		ST_RPAREN,                 // )
		ST_LBRACE,                 // {
		ST_RBRACE,                 // }
		ST_BECOMES,                // =
		ST_EQ,                     // ==
		ST_NOT,                    // !
		ST_NE,                     // !=
		ST_LT,                     // <
		ST_GT,                     // >
		ST_LE,                     // <=
		ST_GE,                     // >=
		ST_PLUS,                   // +
		ST_MINUS,                   // -
		ST_STAR,                   // *
		ST_SLASH,                  // /
		ST_PCT,                    // %
		ST_COMMA,                  // ,
		ST_SEMI,                   // ;
		ST_LBRACK,                 // [
		ST_RBRACK,                 // ]
		ST_AMP,                    // &
		ST_WHITESPACE,              // WHITE SPACES
		ST_COMMENT                 // comment
	};

	Kind stateKinds[] = {
		ERR,            // ST_ERR
		ERR,            // ST_START
		KEYWORD_OR_ID,  // ST_KEYWORD_OR_ID
		NUM,            // ST_NUM
		LPAREN,         // ST_LPAREN
		RPAREN,         // ST_RPAREN
		LBRACE,         // ST_LBRACE
		RBRACE,         // ST_RBRACE
		BECOMES,        // ST_BECOMES
		EQ,             // ST_EQ
		ERR,            // ST_NOT
		NE,             // ST_NE
		LT,             // ST_LT
		GT,             // ST_GT
		LE,             // ST_LE
		GE,             // ST_GE
		PLUS,           // ST_PLUS           
		MINUS,          // ST_MINUS
		STAR,           // ST_STAR
		SLASH,          // ST_SLASH
		PCT,            // ST_PCT
		COMMA,          // ST_COMMA
		SEMI,           // ST_SEMI
		LBRACK,         // ST_LBRACK
		RBRACK,         // ST_RBRACK
		AMP,            // ST_AMP
		WHITESPACE,     // ST_WHITESPACE
		WHITESPACE,     // ST_COMMENT
	};

	// the kindstrings for kinds above
	const string kindStrings[] = {
		"KEYWORD_OR_ID",          // Opcode or identifier (e.g. label use without colon)
		"NUM",
		"LPAREN",
		"RPAREN",
		"LBRACE",
		"RBRACE",
		"BECOMES",
		"EQ",
		"NE",
		"LT",
		"GT",
		"LE",
		"GE",
		"PLUS",
		"MINUS",
		"STAR",
		"SLASH",
		"PCT",
		"COMMA",
		"SEMI",
		"LBRACK",
		"RBRACK",
		"AMP",
		"ERR",         // ERR
		"WHITESPACE",
		"ID",
		"IF",
		"RETURN",
		"ELSE",
		"WHILE",
		"PRINTLN",
		"WAIN",
		"INT",
		"NEW",
		"DELETE",
		"NULL"
	};

	// A Token class representing the concrete functions we
	// might want to apply to a wlp4 Token
	

	class Token {
	protected:
		// The kind of the Token
		Kind kind;
		// The actual string representing the Token
		std::string lexeme;
	public:
		static Token* makeToken(Kind kind, std::string lexeme);
		Token(Kind kind, std::string lexeme) : kind(kind), lexeme(lexeme) {}
		// Convenience functions for operations we might like to
		// use on a Token
		std::string toString() const {
			return kindStrings[kind];
		}
		std::string getLexeme() const {
			return lexeme;
		}
		Kind getKind() const {
			return kind;
		}
	};

	class NumToken : public Token{
	  public:
	    NumToken(Kind kind, std::string lexeme);
	};

	Token* Token::makeToken(Kind kind, std::string lexeme) {
			switch (kind) {
			case NUM:
				return new NumToken(kind, lexeme);
			default:
				return new Token(kind, lexeme);
			}
	}

	template <typename T>
	T fromString(const string& s, bool hex = false) {
		istringstream iss (s);
		T n;
		if (hex)
			iss >> std::hex;
		if (iss >> n)
			return n;
		else
			throw string("ERROR: Type not convertible from string.");
	}

	NumToken::NumToken(Kind kind, std::string lexeme) : Token(kind, lexeme) {
		if(lexeme.length() > 1 && lexeme[0]=='0') throw "ERROR: illegal number: "+ lexeme;
		long long l = fromString<long long>(lexeme);
		if (l > 2147483647)
			throw string("ERROR: Numeric literal out of range: " + lexeme);
	}

	std::ostream& operator<<(std::ostream& out, const Token& t) {
		out << t.toString() << " " << t.getLexeme();
		return out;
	}

	class Lexer {
		std::map<std::string, Kind> keyWordKinds;
		const string whitespace = "\t\n ";
		const string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		const string digits = "0123456789";
		const string oneToNine = "123456789";
		// At most 21 states and 256 transitions (max number of characters in ASCII)
		static const int maxStates = 28;
		static const int maxTrans = 256;
		// Transition function
		State delta[maxStates][maxTrans];
		// Private method to set the transitions based upon characters in the
		// given string
		void setTrans(State from, const std::string& chars, State to) {
			for (string::const_iterator it = chars.begin(); it != chars.end(); ++it)
				delta[from][static_cast<unsigned int>(*it)] = to;
		}
	public:
		Lexer() {
			keyWordKinds["if"] = IF;
			keyWordKinds["return"] = RETURN;
			keyWordKinds["else"] = ELSE;
			keyWordKinds["while"] = WHILE;
			keyWordKinds["println"] = PRINTLN;
			keyWordKinds["wain"] = WAIN;
			keyWordKinds["int"] = INT;
			keyWordKinds["new"] = NEW;
			keyWordKinds["delete"] = DELETE;
			keyWordKinds["NULL"] = Null;
			// Set default transitions to the Error state
			for (int i = 0; i < maxStates; ++i) {
				for (int j = 0; j < maxTrans; ++j) {
					delta[i][j] = ST_ERR;
				}
			}
			// Change transitions as appropriate for the MIPS recognizer
			setTrans(ST_START, whitespace, ST_WHITESPACE);
			setTrans(ST_WHITESPACE, whitespace, ST_WHITESPACE);
			setTrans(ST_START, digits, ST_NUM);
			setTrans(ST_NUM, digits, ST_NUM);
			setTrans(ST_START, letters, ST_KEYWORD_OR_ID);
			setTrans(ST_KEYWORD_OR_ID, letters + digits, ST_KEYWORD_OR_ID);
			setTrans(ST_START, "(", ST_LPAREN);
			setTrans(ST_START, ")", ST_RPAREN);
			setTrans(ST_START, "{", ST_LBRACE);
			setTrans(ST_START, "}", ST_RBRACE);
			setTrans(ST_START, "=", ST_BECOMES);
			setTrans(ST_BECOMES, "=", ST_EQ);
			setTrans(ST_START, "!", ST_NOT);
			setTrans(ST_NOT, "=", ST_NE);
			setTrans(ST_START, "<", ST_LT);
			setTrans(ST_LT, "=", ST_LE);
			setTrans(ST_START, ">", ST_GT);
			setTrans(ST_GT, "=", ST_GE);
			setTrans(ST_START, "+", ST_PLUS);
			setTrans(ST_START, "-", ST_MINUS);
			setTrans(ST_START, "*", ST_STAR);
			setTrans(ST_START, "/", ST_SLASH);
			setTrans(ST_SLASH, "/", ST_COMMENT);
			setTrans(ST_START, "%", ST_PCT);
			setTrans(ST_START, ",", ST_COMMA);
			setTrans(ST_START, ";", ST_SEMI);
			setTrans(ST_START, "[", ST_LBRACK);
			setTrans(ST_START, "]", ST_RBRACK);
			setTrans(ST_START, "&", ST_AMP);

			// A comment can only ever lead to the comment state
			for (int j = 0; j < maxTrans; ++j) delta[ST_COMMENT][j] = ST_COMMENT;
		}
			// Output a vector of Tokens representing the Tokens present in the
			// given line
		std::vector<Token*> scan(const std::string& line);
	};

	vector<Token*> Lexer::scan(const string& line) {
		int attention1 = 0;
		int attention2 = 0;
		// Return vector
		vector<Token*> ret;
		if (line.size() == 0) return ret;
		// Always begin at the start state
		State currState = ST_START;
		// startIter represents the beginning of the next Token
		// that is to be recognized. Initially, this is the beginning
		// of the line.
		// Use a const_iterator since we cannot change the input line
		string::const_iterator startIter = line.begin();
		// Loop over the the line
		for (string::const_iterator it = line.begin();;) {
			// Assume the next state is the error state
			State nextState = ST_ERR;
			// If we aren't done then get the transition from the current
			// state to the next state based upon the current character of
			//input
			if (it != line.end())
				nextState = delta[currState][static_cast<unsigned int>(*it)];
			// If there is no valid transition then we have reach then end of a
			// Token and can add a new Token to the return vector
			if (ST_ERR == nextState) {
				// Get the kind corresponding to the current state
				Kind currKind = stateKinds[currState];
				// If we are in an Error state then we have reached an invalid
				// Token - so we throw and error and delete the Tokens parsed
				// thus far
				if (ERR == currKind) {
					vector<Token*>::iterator vit;
					for (vit = ret.begin(); vit != ret.end(); ++vit)
						delete *vit;
					throw string("ERROR in lexing after reading " + string(line.begin(), it));
				}
				// If we are not in Whitespace then we push back a new token
				// based upon the kind of the State we end in
				// Whitespace is ignored for practical purposes
				if (WHITESPACE != currKind) {
					
					if (currKind == KEYWORD_OR_ID) {
						attention1 += 1;
						attention2 -= 1;
						string tokenString = string(startIter, it);
						std::map<std::string, Kind>::iterator itr = keyWordKinds.find(tokenString);
						if (itr != keyWordKinds.end()) {

							ret.push_back(Token::makeToken(itr->second, tokenString));
						}
						else {
							ret.push_back(Token::makeToken(ID, tokenString));
						}
					}
					else {
						if(seperateSet2.find(currKind) != seperateSet2.end()) {attention2 += 1; attention1 -= 1;}
						else {
							attention2 -= 1;
							if(seperateSet1.find(currKind) != seperateSet1.end()) attention1+=1;
							else attention1 -=1;
						}
						ret.push_back(Token::makeToken(currKind, string(startIter, it)));
					}
				}else{
					attention1 = 0;
					attention2 = 0;
				}
				if(attention1 < 0) attention1 =0;
				if(attention2 < 0) attention2 =0;
				if(attention1 > 1 || attention2 > 1) throw string("ERROR: two token should be seperated, but together now");
				// Start of next Token begins here
				startIter = it;
				// Revert to start state to begin recognizing next token
				currState = ST_START;
				if (it == line.end()) break;
			}
			else {
				// Otherwise we proceed to the next state and increment the iterator
				currState = nextState;
				++it;
			}
		}
		return ret;
	}
}

int main(int argc, char* argv[]) {
	// Nested vector representing lines of Tokens
	// Needs to be used here to cleanup in the case
	// of an exception
	vector< vector<Token*> > tokLines;
	try {
		// Create a MIPS recognizer to tokenize
		// the input lines
		Lexer lexer;
		// Tokenize each line of the input
		string line;
		while (getline(cin, line)) {
			tokLines.push_back(lexer.scan(line));
		}

		// Iterate over the lines of tokens and print them
		// to standard error
		vector<vector<Token*> >::iterator it;
		for (it = tokLines.begin(); it != tokLines.end(); ++it) {
			vector<Token*>::iterator it2;
			for (it2 = it->begin(); it2 != it->end(); ++it2) {
				std::cout << *(*it2) << endl;
			}
		}
	}
	catch (const string& msg) {
		// If an exception occurs print the message and end the program
		std::cerr << msg << endl;
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


