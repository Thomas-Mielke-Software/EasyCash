// VCalcParser.cpp: implementation file
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
// 
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 

#include "stdafx.h"
#include "VCalcParser.h"



// VisualCalc Parser Constructor
CVCalcParser::CVCalcParser()
		: m_PI(3.1415926535897932384626433832795),
		  m_E (2.7182818284590452353602874713527),
		  m_strParserVersion("2.0") {
	this->ResetParserMembers("");
	this->ResetFunctions();
	this->ResetVariables();
	this->ResetAnswersHistory();
}


// VisualCalc Parser Destructor
CVCalcParser::~CVCalcParser() {
	this->ResetParserMembers("");
	this->m_lstFunctions.clear();
	this->m_mapVariables.clear();
	this->m_dqeAnswersHistory.clear();
}


// Resets the parser members state to default
void CVCalcParser::ResetParserMembers(const std::string strSource) {
	this->m_tokCurrentToken		= TV_END;
	this->m_strSource			= strSource;
	this->m_strIdentifierValue	= "";
	this->m_strWarningMsg		= "";
	this->m_valNumberValue		= 0.0;
	this->m_bWarningFlag		= false;
	this->m_bEndEncountered		= false;
	this->m_iCurrentIndex		= 0;
}


// Resets the functions list to its default state
void CVCalcParser::ResetFunctions() {
	this->m_lstFunctions.clear();
	this->m_lstFunctions.push_back("abs");
	this->m_lstFunctions.push_back("Acos");
	this->m_lstFunctions.push_back("Ans");
	this->m_lstFunctions.push_back("Asin");
	this->m_lstFunctions.push_back("Atan");
	this->m_lstFunctions.push_back("cos");
	this->m_lstFunctions.push_back("cosh");
	this->m_lstFunctions.push_back("deg");
	this->m_lstFunctions.push_back("exp");
	this->m_lstFunctions.push_back("ln");
	this->m_lstFunctions.push_back("log");
	this->m_lstFunctions.push_back("logn");
	this->m_lstFunctions.push_back("nAp");
	this->m_lstFunctions.push_back("nCp");
	this->m_lstFunctions.push_back("product");
	this->m_lstFunctions.push_back("rad");
	this->m_lstFunctions.push_back("sin");
	this->m_lstFunctions.push_back("sinh");
	this->m_lstFunctions.push_back("sqrt");
	this->m_lstFunctions.push_back("sum");
	this->m_lstFunctions.push_back("tan");
	this->m_lstFunctions.push_back("tanh");
}


// Resets the variables list to its default state
void CVCalcParser::ResetVariables() {
	this->m_mapVariables.clear();
	this->m_mapVariables["e"]  = m_E;
	this->m_mapVariables["pi"] = m_PI;
}


// Resets the answers history list to its default state
void CVCalcParser::ResetAnswersHistory() {
	this->m_dqeAnswersHistory.clear();
}


// Returns the functions list
const std::list<std::string>& CVCalcParser::GetFunctions() {
	return this->m_lstFunctions;
}


// Returns the variables list
const std::map<std::string, VALUES_TYPE>& CVCalcParser::GetVariables() {
	return this->m_mapVariables;
}


// Returns the Answers history list
const std::deque<AnswerItem>& CVCalcParser::GetAnswersHistory() {
	return this->m_dqeAnswersHistory;
}


// Tells if the parsing throws a warning
bool CVCalcParser::HasWarning() {
	return this->m_bWarningFlag;
}


// Returns the warning message
std::string CVCalcParser::GetWarningMsg() {
	return this->m_strWarningMsg;
}


// Returns the version of the Parser
std::string CVCalcParser::GetVersion() {
	return this->m_strParserVersion;
}


// Returns the factorial of the operand
VALUES_TYPE CVCalcParser::ffactor(VALUES_TYPE valOperand) {
	if (valOperand < 0) {
		throw CFactorialDefForPositiveIntsException(this->m_iCurrentIndex);
	}
	else if (valOperand != ::floor(valOperand)) {
		throw CMathParameterMustBeAnIntegerException(this->m_iCurrentIndex);
	}
	else if (valOperand == 0) {
		valOperand = 1;
	}
	else if (valOperand > 0) {
		for (long i = (long)valOperand-1; i > 1; i--) {
			valOperand *= i;
		}
	}
	return valOperand;
}


// Returns the n combinations of p
VALUES_TYPE CVCalcParser::nCp(VALUES_TYPE n, VALUES_TYPE p) {
	if (n < 0) {
		throw CNMustBePositiveException(this->m_iCurrentIndex);
	}
	else if (p < 0) {
		throw CPMustBePositiveException(this->m_iCurrentIndex);
	}
	else if (n != ::floor(n)) {
		throw CMathParameterMustBeAnIntegerException(this->m_iCurrentIndex);
	}
	else if (p != ::floor(p)) {
		throw CMathParameterMustBeAnIntegerException(this->m_iCurrentIndex);
	}
	else if (p > n) {
		throw CPMustBeSmallerThanNException(this->m_iCurrentIndex);
	}
	//  n! / ((n-p)! * p!)
	return (this->ffactor(n)) / ( this->ffactor(n - p) * this->ffactor(p) );
}


// Returns the n arrangements of p
VALUES_TYPE CVCalcParser::nAp(VALUES_TYPE n, VALUES_TYPE p) {
	if (n < 0) {
		throw CNMustBePositiveException(this->m_iCurrentIndex);
	}
	else if (p < 0) {
		throw CPMustBePositiveException(this->m_iCurrentIndex);
	}
	else if (n != ::floor(n)) {
		throw CMathParameterMustBeAnIntegerException(this->m_iCurrentIndex);
	}
	else if (p != ::floor(p)) {
		throw CMathParameterMustBeAnIntegerException(this->m_iCurrentIndex);
	}
	else if (p > n) {
		throw CPMustBeSmallerThanNException(this->m_iCurrentIndex);
	}
	//  n! / (n-p)!
	return (this->ffactor(n)) / ( this->ffactor(n - p) );
}


// Returns the required Answer in the history
AnswerItem CVCalcParser::Ans(VALUES_TYPE valIndex) {
	valIndex--;
	if (valIndex < 0) {
		throw CDomParameterMustBePositiveException(this->m_iCurrentIndex);
	}
	else if (valIndex != ::floor(valIndex)) {
		throw CDomParameterMustBeAnIntegerException(this->m_iCurrentIndex);
	}
	else if (valIndex >= this->m_dqeAnswersHistory.size()) {
		throw CAnswerRequestedNotFoundException(this->m_iCurrentIndex);
	}
	return m_dqeAnswersHistory.at((int)valIndex);
}


// Returns the absolute value of the operand
VALUES_TYPE CVCalcParser::abs(VALUES_TYPE valOperand) {
	return ::fabs(valOperand);
}


// Returns the cosine of the operand (radians)
VALUES_TYPE CVCalcParser::cos(VALUES_TYPE valOperand) {
	return ::cos(valOperand);
}


// Returns the sine of the operand (radians)
VALUES_TYPE CVCalcParser::sin(VALUES_TYPE valOperand) {
	return ::sin(valOperand);
}


// Returns the tangent of the operand (radians)
VALUES_TYPE CVCalcParser::tan(VALUES_TYPE valOperand) {
	VALUES_TYPE valResult;
	try {
		valResult = ::tan(valOperand);
	}
	catch (...) {
		throw CParameterOutOfRangeException(this->m_iCurrentIndex);
	}
	return valResult;
}


// Returns the hyperbolic cosine of the operand (radians)
VALUES_TYPE CVCalcParser::cosh(VALUES_TYPE valOperand) {
	return ::cosh(valOperand);
}


// Returns the hyperbolic sine of the operand (radians)
VALUES_TYPE CVCalcParser::sinh(VALUES_TYPE valOperand) {
	return ::sinh(valOperand);
}


// Returns the hyperbolic tangent of the operand (radians)
VALUES_TYPE CVCalcParser::tanh(VALUES_TYPE valOperand) {
	VALUES_TYPE valResult;
	try {
		valResult = ::tanh(valOperand);
	}
	catch (...) {
		throw CParameterOutOfRangeException(this->m_iCurrentIndex);
	}
	return valResult;
}


// Returns the arccosine of the operand ( [-1 ; +1] )
VALUES_TYPE CVCalcParser::Acos(VALUES_TYPE valOperand) {
	if ((valOperand < -1) || (valOperand > 1)) {
		throw CParameterOutOfRangeException(this->m_iCurrentIndex);
	}
	return ::acos(valOperand);
}


// Returns the arcsine of the operand ( [-1 ; +1] )
VALUES_TYPE CVCalcParser::Asin(VALUES_TYPE valOperand) {
	if ((valOperand < -1) || (valOperand > 1)) {
		throw CParameterOutOfRangeException(this->m_iCurrentIndex);
	}
	return ::asin(valOperand);
}


// Returns the arctangent of the operand ( [-1 ; +1] )
VALUES_TYPE CVCalcParser::Atan(VALUES_TYPE valOperand) {
	if ((valOperand < -1) || (valOperand > 1)) {
		throw CParameterOutOfRangeException(this->m_iCurrentIndex);
	}
	return ::atan(valOperand);
}


// Returns a radian angle converted into degrees
VALUES_TYPE CVCalcParser::deg(VALUES_TYPE valOperand) {
	return ( (180 * valOperand) / this->m_PI );
}


// Returns a degree angle converted into radians
VALUES_TYPE CVCalcParser::rad(VALUES_TYPE valOperand) {
	return ( (this->m_PI * valOperand) / 180 );
}


// Returns the exponential value of the operand
VALUES_TYPE CVCalcParser::exp(VALUES_TYPE valOperand) {
	return ::exp(valOperand);
}


// Returns the natural logarithm of the operand
VALUES_TYPE CVCalcParser::ln(VALUES_TYPE valOperand) {
	if (valOperand < 0) {
		throw CMathParameterMustBePositiveException(this->m_iCurrentIndex);
	}
	return ::log(valOperand);
}


// Returns the decimal logarithm of the operand
VALUES_TYPE CVCalcParser::log(VALUES_TYPE valOperand) {
	if (valOperand < 0) {
		throw CMathParameterMustBePositiveException(this->m_iCurrentIndex);
	}
	return ::log10(valOperand);
}


// Returns the base-n logarithm of the operand
VALUES_TYPE CVCalcParser::logn(VALUES_TYPE valOperand, VALUES_TYPE valBase) {
	if (valOperand < 0) {
		throw CMathParameterMustBePositiveException(this->m_iCurrentIndex);
	}
	if (valBase == 0) {
		throw CDivisionByZeroException(this->m_iCurrentIndex);
	}
	if (valBase < 0) {
		throw CMathParameterMustBePositiveException(this->m_iCurrentIndex);
	}
	return ( ::log(valOperand) / ::log(valBase) );
}


// Returns the squared root of the operand
VALUES_TYPE CVCalcParser::sqrt(VALUES_TYPE valOperand) {
	if (valOperand < 0) {
		throw CMathParameterMustBePositiveException(this->m_iCurrentIndex);
	}
	return ::sqrt(valOperand);
}


// Returns the power of the first operand by the second one
VALUES_TYPE CVCalcParser::pow(VALUES_TYPE valLeftOperand, VALUES_TYPE valRightOperand) {
	if ((valLeftOperand == 0.0) && (valRightOperand == 0.0)) {
		this->m_strWarningMsg = "Warning: 0^0 replaced by 1";
		this->m_bWarningFlag  = true;
		return 1.;
	}
	return ::pow(valLeftOperand, valRightOperand);
}


// Returns the Modulus of the first operand by the second one
VALUES_TYPE CVCalcParser::mod(VALUES_TYPE valLeftOperand, VALUES_TYPE valRightOperand) {
	if (valRightOperand == 0.0) {
		throw CDivisionByZeroException(this->m_iCurrentIndex);
	}
	return ::fmod(valLeftOperand, valRightOperand);
}


// Returns the squared root of the operand
VALUES_TYPE CVCalcParser::sum(std::string expr, std::string var, VALUES_TYPE low, VALUES_TYPE high) {
	throw CFunctionNotSupportedException("sum", this->m_iCurrentIndex);
	// Supposed behavior :
	//    VALUES_TYPE vResult = 0;
	//    for (var = low; var < high; var++) {
	//        vResult += expr(var);
	//    }
	return (VALUES_TYPE)0;
}


// Returns the squared root of the operand
VALUES_TYPE CVCalcParser::product(std::string expr, std::string var, VALUES_TYPE low, VALUES_TYPE high) {
	throw CFunctionNotSupportedException("product", this->m_iCurrentIndex);
	// Supposed behavior :
	//    VALUES_TYPE vResult = 0;
	//    for (var = low; var < high; var++) {
	//        vResult *= expr(var);
	//    }
	return (VALUES_TYPE)0;
}


// Returns the result of the parsed formula
VALUES_TYPE CVCalcParser::Evaluate(const std::string& Source) {
	this->ResetParserMembers(Source);
	try {
		VALUES_TYPE valResult = this->Level_1();
		AnswerItem AnsItem;
		AnsItem.m_strFormula = this->m_strSource;
		AnsItem.m_valResult  = valResult;
		this->m_dqeAnswersHistory.push_front(AnsItem);
		return valResult;
	}
	catch (...) {
		throw;
	}
}


// Operates on 'addition' and 'subtraction' operators
VALUES_TYPE CVCalcParser::Level_1(void) {
	VALUES_TYPE valLeftOperand = this->Level_2();
	while (true) {
		switch (this->m_tokCurrentToken) {
		case TV_PLUS:
			// Addition : 'expr + expr'
			valLeftOperand += this->Level_2();
			break;

		case TV_MINUS:
			// Subtraction : 'expr - expr'
			valLeftOperand -= this->Level_2();
			break;

		case TV_ASSIGN:
			// Assignation : 'expr = expr'
			throw CLiteralAssignationException(this->m_iCurrentIndex);

		default:
			return valLeftOperand;
		}
	}
}


// Operates on 'multiplication' and 'division' operators
VALUES_TYPE CVCalcParser::Level_2(void) {
	VALUES_TYPE valLeftOperand = this->Level_3();
	while (true) {
		switch (this->m_tokCurrentToken) {
		case TV_MUL:
			// Multiplication : 'expr * expr'
			valLeftOperand *= this->Level_2();
			break;

		case TV_DIV:
			// Division : 'expr / expr'
			{
				VALUES_TYPE valRightOperand = this->Level_2();
				if (valRightOperand != 0.0) {
					valLeftOperand /= valRightOperand;
				}
				else {
					throw CDivisionByZeroException(this->m_iCurrentIndex);
				}
			}
			break;

		default:
			return valLeftOperand;
		}
	}
}


// Operates on 'power' operator
VALUES_TYPE CVCalcParser::Level_3(void) {
	VALUES_TYPE valLeftOperand = this->Level_4();
	while (true) {
		switch(this->m_tokCurrentToken) {
		case TV_POW:
			// Power : 'expr ^ expr'
			valLeftOperand = this->pow(valLeftOperand, this->Level_3());
			break;

		default:
			return valLeftOperand;
		}
	}
}


// Operates on 'modulus' operator
VALUES_TYPE CVCalcParser::Level_4(void) {
	VALUES_TYPE valLeftOperand = this->Level_5();
	while (true) {
		switch (this->m_tokCurrentToken) {
		case TV_MOD:
			// Modulus : 'expr % expr'
			valLeftOperand = this->mod(valLeftOperand, this->Level_3());
			break;

		default:
			return valLeftOperand;
		}
	}
}


// Operates on 'factorial' and 'degree to radian' operators
VALUES_TYPE CVCalcParser::Level_5(void) {
	VALUES_TYPE valLeftOperand = this->Primary();
	while (true) {
		switch (this->m_tokCurrentToken) {
		case TV_NUMBER:
			this->GetToken();
			break;

		case TV_FACT:
			// Factorial : 'expr !'
			this->GetToken();
			valLeftOperand = this->ffactor(valLeftOperand);
			break;

		case TV_DEG:
			// Degree to radian : 'expr°'
			this->GetToken();
			valLeftOperand = this->rad(valLeftOperand);
			break;

		default:
			return valLeftOperand;
		}
	}
}


// Operates on 'parenthesis', 'Unary +' and 'Unary -' operators
VALUES_TYPE CVCalcParser::Primary(void) {
	this->GetToken();
	switch (this->m_tokCurrentToken) {
	case TV_NUMBER:
		return this->m_valNumberValue;

	case TV_IDENTIFIER:
		{
			std::string strIdentifierName = this->m_strIdentifierValue;
			std::list<std::string>::iterator IterList =
				std::find(this->m_lstFunctions.begin(),
						  this->m_lstFunctions.end(),
						  strIdentifierName
						 );
			if (IterList != this->m_lstFunctions.end()) {
				// The identifier is a function name
				if (this->GetToken() != TV_LP) {
					throw CIllegalUseOfFonctionNameException(strIdentifierName, this->m_iCurrentIndex);
				}
				else {
					VALUES_TYPE valFunctionResult = 0.0;
					if (strIdentifierName == "abs") {
						valFunctionResult = this->abs(this->Level_1());
					}
					else if (strIdentifierName == "Acos") {
						valFunctionResult = this->Acos(this->Level_1());
					}
					else if (strIdentifierName == "Ans") {
						valFunctionResult = this->Ans(this->Level_1()).m_valResult;
					}
					else if (strIdentifierName == "Asin") {
						valFunctionResult = this->Asin(this->Level_1());
					}
					else if (strIdentifierName == "Atan") {
						valFunctionResult = this->Atan(this->Level_1());
					}
					else if (strIdentifierName == "cos") {
						valFunctionResult = this->cos(this->Level_1());
					}
					else if (strIdentifierName == "cosh") {
						valFunctionResult = this->cosh(this->Level_1());
					}
					else if (strIdentifierName == "deg") {
						valFunctionResult = this->deg(this->Level_1());
					}
					else if (strIdentifierName == "exp") {
						valFunctionResult = this->exp(this->Level_1());
					}
					else if (strIdentifierName == "ln") {
						valFunctionResult = this->ln(this->Level_1());
					}
					else if (strIdentifierName == "log") {
						valFunctionResult = this->log(this->Level_1());
					}
					else if (strIdentifierName == "logn") {
						VALUES_TYPE valFirstParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						valFunctionResult = this->logn(valFirstParameter, this->Level_1());
					}
					else if (strIdentifierName == "nAp") {
						VALUES_TYPE valFirstParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						valFunctionResult = this->nAp(valFirstParameter, this->Level_1());
					}
					else if (strIdentifierName == "nCp") {
						VALUES_TYPE valFirstParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						valFunctionResult = this->nCp(valFirstParameter, this->Level_1());
					}
					else if (strIdentifierName == "product") {
						VALUES_TYPE valFirstParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						VALUES_TYPE valSecondParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						VALUES_TYPE valThirdParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						
						valFunctionResult = this->product("",
														  "",
														  valThirdParameter,
														  this->Level_1());
					}
					else if (strIdentifierName == "rad") {
						valFunctionResult = this->rad(this->Level_1());
					}
					else if (strIdentifierName == "sin") {
						valFunctionResult = this->sin(this->Level_1());
					}
					else if (strIdentifierName == "sinh") {
						valFunctionResult = this->sinh(this->Level_1());
					}
					else if (strIdentifierName == "sqrt") {
						valFunctionResult = this->sqrt(this->Level_1());
					}
					else if (strIdentifierName == "sum") {
						VALUES_TYPE valFirstParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						VALUES_TYPE valSecondParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						VALUES_TYPE valThirdParameter = this->Level_1();
						if (this->m_tokCurrentToken != TV_SEQ) {
							throw CTooFewParametersException(this->m_iCurrentIndex);
						}
						
						valFunctionResult = this->sum("",
													  "",
													  valThirdParameter,
													  this->Level_1());
					}
					else if (strIdentifierName == "tan") {
						valFunctionResult = this->tan(this->Level_1());
					}
					else if (strIdentifierName == "tanh") {
						valFunctionResult = this->tanh(this->Level_1());
					}
					else {
						throw CUnknownFunctionCalledException(strIdentifierName, this->m_iCurrentIndex);
					}
					if (m_tokCurrentToken != TV_RP) {
						if (m_tokCurrentToken == TV_SEQ) {
							throw CTooManyParametersException(this->m_iCurrentIndex);
						}
						else {
							throw CClosingParenthesisExpectedException(this->m_iCurrentIndex);
						}
					}
					this->GetToken();
					return valFunctionResult;
				}
			}
			else {
				// The identifier is not a function name but a user-defined variable
				this->GetToken();
				if (this->m_tokCurrentToken == TV_LP) {
					throw CImplicitMultiplicationException(this->m_iCurrentIndex);
				}
				
				VALUES_TYPE valResult = 0.0;
				// The user assigns a value to the variable
				if (this->m_tokCurrentToken == TV_ASSIGN) {
					if ((strIdentifierName == "pi") || (strIdentifierName == "e")) {
						throw CConstantAssignationException(strIdentifierName, this->m_iCurrentIndex);
					}
					valResult = this->Level_1();
					// Adds the variable in the map if not existing yet
					this->m_mapVariables[strIdentifierName] = valResult;
				}
				else {
					// The identifier is neither a function nor a variable
					if (this->m_mapVariables.find(strIdentifierName) == this->m_mapVariables.end()) {
						throw CUndefinedVariableException(strIdentifierName, this->m_iCurrentIndex);
					}
					valResult = this->m_mapVariables[strIdentifierName];
				}
				return valResult;
			}
		}
		break;

	case TV_PLUS:
		// Unary Plus '+expr'
		return this->Primary();

	case TV_MINUS:
		// Unary Minus '-expr'
		return -1 * this->Primary();

	case TV_LP:
		// Opening parenthesis '('
		{
			VALUES_TYPE valExpression = this->Level_1();
			if (this->m_tokCurrentToken != TV_RP) {
				throw CClosingParenthesisExpectedException(this->m_iCurrentIndex);
			}
			this->GetToken();
			return valExpression;
		}

	default:
		throw CMathematicExpressionExpectedException(this->m_iCurrentIndex);
	}
}


// Operates on 'addition' and 'subtraction' operations
TokenValue CVCalcParser::GetToken(void) {
	if (this->m_strSource.empty()) {
		return this->m_tokCurrentToken = TV_END;
	}
	if (this->m_bEndEncountered) {
		return this->m_tokCurrentToken = TV_END;
	}
	// Eats the white spaces...
	while (::isspace(this->m_strSource[this->m_iCurrentIndex])) {
		if (this->m_iCurrentIndex <= this->m_strSource.length()-1) {
			this->m_iCurrentIndex++;
			if (this->m_iCurrentIndex > this->m_strSource.length()-1) {
				//this->m_bEndEncountered;
				return this->m_tokCurrentToken = TV_END;
			}
		}
		/*
		else {
			return m_tokCurrentToken = TV_END;
		}
		*/
	}
	char ch = this->m_strSource[m_iCurrentIndex++];
	if (this->m_iCurrentIndex > this->m_strSource.length()-1) {
		this->m_bEndEncountered = true;
	}
	switch (ch) {
	case '\0':
	case ';':
		// End of the expression
		return this->m_tokCurrentToken = TV_END;

	case ',':
	case '*':
	case '/':
	case '+':
	case '-':
	case '^':
	case '%':
	case '!':
	case '°':
	case '(':
	case ')':
	case '=':
		// A token is found on the input stream
		return this->m_tokCurrentToken = (TokenValue)ch;
		
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		// A digit is found on the input stream...
		{
			int iStartPos = this->m_iCurrentIndex - 1;
			if (!this->m_bEndEncountered) {
				// A valid number must contain at most one period ('.')
				bool bDotAlreadyThere = ( (this->m_strSource[iStartPos] == '.') ? true : false );
				while ( (!this->m_bEndEncountered) &&
						((::isdigit(this->m_strSource[this->m_iCurrentIndex])) ||
						 (this->m_strSource[this->m_iCurrentIndex] == '.')) ) {
					if (this->m_strSource[m_iCurrentIndex] == '.') {
						if (bDotAlreadyThere) {
							throw CDigitExpectedException(this->m_iCurrentIndex);
						}
						else {
							bDotAlreadyThere = true;
						}
					}
					this->m_iCurrentIndex++;
					if (this->m_iCurrentIndex > this->m_strSource.length()-1) {
						this->m_bEndEncountered = true;
						break;
					}
				}
			}
			// Extracts all the digits which compound the number
			const std::string strNumber = this->m_strSource.substr  (iStartPos,
																	 (this->m_iCurrentIndex - iStartPos)
																	).c_str();
			const char* pszStrNumber = strNumber.c_str();
			this->m_valNumberValue = ::atof(pszStrNumber);
			return this->m_tokCurrentToken = TV_NUMBER;
		}

	case '_':
	case 'a':	case 'A':
	case 'b':	case 'B':
	case 'c':	case 'C':
	case 'd':	case 'D':
	case 'e':	case 'E':
	case 'f':	case 'F':
	case 'g':	case 'G':
	case 'h':	case 'H':
	case 'i':	case 'I':
	case 'j':	case 'J':
	case 'k':	case 'K':
	case 'l':	case 'L':
	case 'm':	case 'M':
	case 'n':	case 'N':
	case 'o':	case 'O':
	case 'p':	case 'P':
	case 'q':	case 'Q':
	case 'r':	case 'R':
	case 's':	case 'S':
	case 't':	case 'T':
	case 'u':	case 'U':
	case 'v':	case 'V':
	case 'w':	case 'W':
	case 'x':	case 'X':
	case 'y':	case 'Y':
	case 'z':	case 'Z':
		// A letter is found on the input stream...
		{
			int iStartPos = this->m_iCurrentIndex - 1;
			if (!this->m_bEndEncountered) {
				while ( (!this->m_bEndEncountered) &&
						((::isalnum(this->m_strSource[this->m_iCurrentIndex])) ||
						 (this->m_strSource[this->m_iCurrentIndex] == '_')) ) {
					this->m_iCurrentIndex++;
					if (this->m_iCurrentIndex > this->m_strSource.length()-1) {
						this->m_bEndEncountered = true;
						break;
					}
				}
			}
			// Extracts all the characters which compound the identifier
			this->m_strIdentifierValue = this->m_strSource.substr(iStartPos,
															   (this->m_iCurrentIndex - iStartPos)
															  );
			return this->m_tokCurrentToken = TV_IDENTIFIER;
		}

	default:
		this->m_tokCurrentToken = TV_END;
		throw CUnexpectedCharacterException(ch, this->m_iCurrentIndex);
	}
}
