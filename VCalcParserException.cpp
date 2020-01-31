// VCalcParserException.cpp: implementation file
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
#include "VCalcParserException.h"


//////////////////////////////////////////////////////
//class CVCalcParserException
CVCalcParserException::CVCalcParserException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos) {
	this->m_enExceptionNumber = enExceptionNumber;
	this->m_strExceptionMsg	  = strExceptionMsg;
	this->m_iErrorPos		  = iErrorPos;
}

CVCalcParserException::~CVCalcParserException() {
	this->m_enExceptionNumber = ENB_NONE;
	this->m_strExceptionMsg	  = "";
	this->m_iErrorPos		  = -1;
}

CVCalcParserException::ExceptionNumbers CVCalcParserException::GetExceptionNumber() {
	return this->m_enExceptionNumber;
}

std::string CVCalcParserException::GetMessage() {
	return this->m_strExceptionMsg;
}

long CVCalcParserException::GetErrorPos() {
	return this->m_iErrorPos;
}



/************************************************************************/
/* CSyntaxException derived classes                                     */
/************************************************************************/

/////////////////////////////////////////////////////
// class CSyntaxException
CSyntaxException::CSyntaxException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CSyntaxException::~CSyntaxException() {
}


// class CUnexpectedCharacterException
CUnexpectedCharacterException::CUnexpectedCharacterException(char cCharacter, int iErrorPos) 
		: CSyntaxException(ENB_SYN_UNEXPECTED_CHARACTER,
						   std::string("Unexpected character '") + cCharacter + "'",
						   iErrorPos) {
}

CUnexpectedCharacterException::~CUnexpectedCharacterException() {
}


// class CDigitExpectedException
CDigitExpectedException::CDigitExpectedException(int iErrorPos)
		: CSyntaxException(ENB_SYN_DIGIT_EXPECTED,
						   "Digit expected",
						   iErrorPos) {
}

CDigitExpectedException::~CDigitExpectedException() {
}


// class CClosingParenthesisExpectedException
CClosingParenthesisExpectedException::CClosingParenthesisExpectedException(int iErrorPos)
		: CSyntaxException(ENB_SYN_CLOSING_PARENTHESIS_EXPECTED,
						   "')' expected",
						   iErrorPos) {
}

CClosingParenthesisExpectedException::~CClosingParenthesisExpectedException() {
}


// class CMathematicExpressionExpectedException
CMathematicExpressionExpectedException::CMathematicExpressionExpectedException(int iErrorPos)
		: CSyntaxException(ENB_SYN_MATHEMATIC_EXPRESSION_EXPECTED,
						   "Mathematic expression expected",
						   iErrorPos) {
}

CMathematicExpressionExpectedException::~CMathematicExpressionExpectedException() {
}


/************************************************************************/
/* CMathematicException derived classes                                 */
/************************************************************************/

//////////////////////////////////////////////////////
// class CMathematicException
CMathematicException::CMathematicException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CMathematicException::~CMathematicException() {
}


// class CDivisionByZeroException
CDivisionByZeroException::CDivisionByZeroException(int iErrorPos)
		: CMathematicException(ENB_MTH_DIVISION_BY_ZERO,
							   "Division by 0",
							   iErrorPos) {
}

CDivisionByZeroException::~CDivisionByZeroException() {
}


// class CNMustBePositiveException
CNMustBePositiveException::CNMustBePositiveException(int iErrorPos)
		: CMathematicException(ENB_MTH_N_MUST_BE_POSITIVE,
							   "'n' must be positive",
							   iErrorPos) {
}

CNMustBePositiveException::~CNMustBePositiveException() {
}


// class CPMustBePositiveException
CPMustBePositiveException::CPMustBePositiveException(int iErrorPos)
		: CMathematicException(ENB_MTH_P_MUST_BE_POSITIVE,
							   "'p' must be positive",
							   iErrorPos) {
}

CPMustBePositiveException::~CPMustBePositiveException() {
}


// class CPMustBeSmallerThanNException
CPMustBeSmallerThanNException::CPMustBeSmallerThanNException(int iErrorPos)
		: CMathematicException(ENB_MTH_P_MUST_BE_SMALLER_THAN_N,
							   "'p' must be smaller than 'n'",
							   iErrorPos) {
}

CPMustBeSmallerThanNException::~CPMustBeSmallerThanNException() {
}


// class CFactorialDefForPositiveIntsException
CFactorialDefForPositiveIntsException::CFactorialDefForPositiveIntsException(int iErrorPos)
		: CMathematicException(ENB_MTH_FACTORIAL_DEF_FOR_POSITIVE_INTS,
							   "A factorial is defined for positive integers",
							   iErrorPos) {
}

CFactorialDefForPositiveIntsException::~CFactorialDefForPositiveIntsException() {
}


// class CParameterOutOfRangeException
CParameterOutOfRangeException::CParameterOutOfRangeException(int iErrorPos)
		: CMathematicException(ENB_MTH_PARAMETER_OUT_OF_RANGE,
							   "Parameter out of range",
							   iErrorPos) {
}

CParameterOutOfRangeException::~CParameterOutOfRangeException() {
}


// class CMathParameterMustBePositiveException
CMathParameterMustBePositiveException::CMathParameterMustBePositiveException(int iErrorPos)
		: CMathematicException(ENB_MTH_PARAMETER_MUST_BE_POSITIVE,
							   "Parameter must be positive",
							   iErrorPos) {
}

CMathParameterMustBePositiveException::~CMathParameterMustBePositiveException() {
}


// class CMathParameterMustBeAnIntegerException
CMathParameterMustBeAnIntegerException::CMathParameterMustBeAnIntegerException(int iErrorPos)
		: CMathematicException(ENB_MTH_PARAMETER_MUST_BE_AN_INTEGER,
							   "Parameter must be an integer",
							   iErrorPos) {
}

CMathParameterMustBeAnIntegerException::~CMathParameterMustBeAnIntegerException() {
}



/************************************************************************/
/* CFunctionException derived classes                                   */
/************************************************************************/

//////////////////////////////////////////////////////
// class CFunctionException
CFunctionException::CFunctionException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CFunctionException::~CFunctionException() {
}


// class CUnknownFunctionCalledException
CUnknownFunctionCalledException::CUnknownFunctionCalledException(const std::string& strFunctionName, int iErrorPos)
		: CFunctionException(ENB_FCT_UNKNOWN_FUNCTION_CALLED,
							 "Unknown function called ('" + strFunctionName + "')",
							 iErrorPos) {
}

CUnknownFunctionCalledException::~CUnknownFunctionCalledException() {
}


// class CImplicitMultiplicationException
CImplicitMultiplicationException::CImplicitMultiplicationException(int iErrorPos)
		: CFunctionException(ENB_FCT_IMPLICIT_MULTIPLICATION,
							 "Implicit multiplication not supported",
							 iErrorPos) {
}

CImplicitMultiplicationException::~CImplicitMultiplicationException() {
}


// class CIllegalUseOfFonctionNameException
CIllegalUseOfFonctionNameException::CIllegalUseOfFonctionNameException(const std::string& strFunctionName, int iErrorPos)
		: CFunctionException(ENB_FCT_ILLEGAL_USE_OF_FUNCTION_NAME,
							 "Illegal use of function name ('" + strFunctionName + "')",
							 iErrorPos) {
}

CIllegalUseOfFonctionNameException::~CIllegalUseOfFonctionNameException() {
}



/************************************************************************/
/* CParameterException derived classes                                  */
/************************************************************************/

//////////////////////////////////////////////////////
// class CParameterException
CParameterException::CParameterException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CParameterException::~CParameterException() {
}


// class CTooFewParametersException
CTooFewParametersException::CTooFewParametersException(int iErrorPos)
		: CParameterException(ENB_PRM_TOO_FEW_PARAMETERS,
							  "Too few parameters. ',' expected",
							  iErrorPos) {
}

CTooFewParametersException::~CTooFewParametersException() {
}


// class CTooFewParametersException
CTooManyParametersException::CTooManyParametersException(int iErrorPos)
		: CParameterException(ENB_PRM_TOO_MANY_PARAMETERS,
							  "Too many parameters. ')' expected",
							  iErrorPos) {
}

CTooManyParametersException::~CTooManyParametersException() {
}



/************************************************************************/
/* CVariableException derived classes                                   */
/************************************************************************/

//////////////////////////////////////////////////////
// class CVariableException
CVariableException::CVariableException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CVariableException::~CVariableException() {
}


// class CConstantAssignationException
CConstantAssignationException::CConstantAssignationException(const std::string& strConstantName, int iErrorPos)
		: CVariableException(ENB_VAR_CONSTANTASSIGNATION,
							 "Cannot assign a constant ('" + strConstantName + "')",
							 iErrorPos) {
}

CConstantAssignationException::~CConstantAssignationException() {
}


// class CLiteralAssignationException
CLiteralAssignationException::CLiteralAssignationException(int iErrorPos)
		: CVariableException(ENB_VAR_LITERALASSIGNATION,
							 "Cannot assign a literal",
							 iErrorPos) {
}

CLiteralAssignationException::~CLiteralAssignationException() {
}


// class CUndefinedVariableException
CUndefinedVariableException::CUndefinedVariableException(const std::string& strVariableName, int iErrorPos)
		: CVariableException(ENB_VAR_UNDEFINED_VARIABLE,
							 "Undefined variable '" + strVariableName + "'",
							 iErrorPos) {
}

CUndefinedVariableException::~CUndefinedVariableException() {
}



/************************************************************************/
/* CDomainException derived classes                                     */
/************************************************************************/

//////////////////////////////////////////////////////
// class CDomainException
CDomainException::CDomainException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CDomainException::~CDomainException() {
}


// class CAnswerRequestedNotFoundException
CAnswerRequestedNotFoundException::CAnswerRequestedNotFoundException(int iErrorPos)
		: CDomainException(ENB_DMN_ANSWER_REQUESTED_NOT_FOUND,
						   "The answer requested was not found",
						   iErrorPos) {
}

CAnswerRequestedNotFoundException::~CAnswerRequestedNotFoundException() {
}


// class CDomParameterMustBePositiveException
CDomParameterMustBePositiveException::CDomParameterMustBePositiveException(int iErrorPos)
		: CDomainException(ENB_DMN_PARAMETER_MUST_BE_POSITIVE,
						   "The answer requested was not found",
						   iErrorPos) {
}

CDomParameterMustBePositiveException::~CDomParameterMustBePositiveException() {
}


// class CDomParameterMustBeAnIntegerException
CDomParameterMustBeAnIntegerException::CDomParameterMustBeAnIntegerException(int iErrorPos)
		: CDomainException(ENB_DMN_PARAMETER_MUST_BE_AN_INTEGER,
						   "The answer requested was not found",
						   iErrorPos) {
}

CDomParameterMustBeAnIntegerException::~CDomParameterMustBeAnIntegerException() {
}


// class CDomParametersMustBePositiveException
CDomParametersMustBePositiveException::CDomParametersMustBePositiveException(int iErrorPos)
		: CDomainException(ENB_DMN_PARAMETERS_MUST_BE_POSITIVE,
						   "The answer requested was not found",
						   iErrorPos) {
}

CDomParametersMustBePositiveException::~CDomParametersMustBePositiveException() {
}


// class CDomParametersMustBeIntegersException
CDomParametersMustBeIntegersException::CDomParametersMustBeIntegersException(int iErrorPos)
		: CDomainException(ENB_DMN_PARAMETERS_MUST_BE_INTEGERS,
						   "The answer requested was not found",
						   iErrorPos) {
}

CDomParametersMustBeIntegersException::~CDomParametersMustBeIntegersException() {
}



/************************************************************************/
/* CParserException derived classes                                     */
/************************************************************************/

//////////////////////////////////////////////////////
// class CParserException
CParserException::CParserException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos)
		: CVCalcParserException(enExceptionNumber, strExceptionMsg, iErrorPos) {
}

CParserException::~CParserException() {
}


// class CFunctionNotSupportedException
CFunctionNotSupportedException::CFunctionNotSupportedException(const std::string& strFunctionName, int iErrorPos)
		: CParserException(ENB_PRS_FUNCTION_NOT_SUPPORTED,
						   "Warning: Function not supported yet ('" + strFunctionName + "')",
						   iErrorPos) {
}

CFunctionNotSupportedException::~CFunctionNotSupportedException() {
}


// class CUnknownException
CUnknownException::CUnknownException(int iErrorPos)
		: CParserException(ENB_PRS_UNKNOWN_EXCEPTION,
						   "Unknown exception",
						   iErrorPos) {
}

CUnknownException::~CUnknownException() {
}
