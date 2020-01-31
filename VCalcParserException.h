// VCalcParserException.h: header file
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

#if !defined(__AFX_VCALCPARSEREXCEPTION_H_INCLUDED__)
#define		 __AFX_VCALCPARSEREXCEPTION_H_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif


/***************************************************************/
/*     VisualCalc Parser Exceptions hierarchy                  */
/* ----------------------------------------------------------- */
/*  - CVCalcParserException                                    */
/*      - CSyntaxException                                     */
/*          - CUnexpectedCharacterException                    */
/*          - CDigitExpectedException                          */
/*          - CClosingParenthesisExpectedException             */
/*          - CMathematicExpressionExpectedException           */
/*      - CMathematicException                                 */
/*          - CDivisionByZeroException                         */
/*          - CNMustBePositiveException                        */
/*          - CPMustBePositiveException                        */
/*          - CPMustBeSmallerThanNException                    */
/*          - CFactorialDefForPositiveIntsException            */
/*          - CParameterOutOfRangeException                    */
/*          - CMathParameterMustBePositiveException            */
/*          - CMathParameterMustBeAnIntegerException           */
/*      - CFunctionException                                   */
/*          - CUnknownFunctionCalledException                  */
/*          - CImplicitMultiplicationException                 */
/*          - CIllegalUseOfFonctionNameException               */
/*      - CParameterException                                  */
/*          - CTooFewParametersException                       */
/*          - CTooManyParametersException                      */
/*      - CVariableException                                   */
/*          - CConstantAssignationException                    */
/*          - CLiteralAssignationException                     */
/*          - CUndefinedVariableException                      */
/*      - CDomainException                                     */
/*          - CAnswerRequestedNotFoundException                */
/*          - CDomParameterMustBePositiveException             */
/*          - CDomParameterMustBeAnIntegerException            */
/*          - CDomParametersMustBePositiveException            */
/*          - CDomParametersMustBeIntegersException            */
/*      - CParserException                                     */
/*          - CFunctionNotSupportedException                   */
/*          - CUnknownException                                */
/***************************************************************/


#include <STRING>	// std::string


//////////////////////////////////////////////////////
// class CVCalcParserException
//   Base class for VisualCalc Parser Exceptions
//   See 'Exception definitions.txt' for more details
//     about the exceptions Code Numbers and Messages.
class CVCalcParserException {
protected:
	enum ExceptionNumbers {			// Used by the derived classes
		ENB_SYN_UNEXPECTED_CHARACTER			= 1001,
		ENB_SYN_DIGIT_EXPECTED					= 1002,
		ENB_SYN_CLOSING_PARENTHESIS_EXPECTED	= 1003,
		ENB_SYN_MATHEMATIC_EXPRESSION_EXPECTED	= 1004,
		ENB_MTH_DIVISION_BY_ZERO				= 2001,
		ENB_MTH_N_MUST_BE_POSITIVE				= 2002,
		ENB_MTH_P_MUST_BE_POSITIVE				= 2003,
		ENB_MTH_P_MUST_BE_SMALLER_THAN_N		= 2004,
		ENB_MTH_FACTORIAL_DEF_FOR_POSITIVE_INTS	= 2005,
		ENB_MTH_PARAMETER_OUT_OF_RANGE			= 2006,
		ENB_MTH_PARAMETER_MUST_BE_POSITIVE		= 2007,
		ENB_MTH_PARAMETER_MUST_BE_AN_INTEGER	= 2008,
		ENB_FCT_UNKNOWN_FUNCTION_CALLED			= 3001,
		ENB_FCT_IMPLICIT_MULTIPLICATION			= 3002,
		ENB_FCT_ILLEGAL_USE_OF_FUNCTION_NAME	= 3003,
		ENB_PRM_TOO_FEW_PARAMETERS				= 4001,
		ENB_PRM_TOO_MANY_PARAMETERS				= 4002,
		ENB_VAR_CONSTANTASSIGNATION				= 5001,
		ENB_VAR_LITERALASSIGNATION				= 5002,
		ENB_VAR_UNDEFINED_VARIABLE				= 5003,
		ENB_DMN_ANSWER_REQUESTED_NOT_FOUND		= 6001,
		ENB_DMN_PARAMETER_MUST_BE_POSITIVE		= 6002,
		ENB_DMN_PARAMETER_MUST_BE_AN_INTEGER	= 6003,
		ENB_DMN_PARAMETERS_MUST_BE_POSITIVE		= 6004,
		ENB_DMN_PARAMETERS_MUST_BE_INTEGERS		= 6005,
		ENB_PRS_FUNCTION_NOT_SUPPORTED			= 9001,
		ENB_PRS_UNKNOWN_EXCEPTION				= 9009,
		ENB_NONE								= -1
	};

	ExceptionNumbers	m_enExceptionNumber;	// Exception Number
	std::string			m_strExceptionMsg;		// Exception message
	long				m_iErrorPos;			// Position of the error in the source stream

	// Protected to make the class abstract. It must not be possible to instantiate the class
	CVCalcParserException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);
	
public:
	virtual ~CVCalcParserException();
	ExceptionNumbers GetExceptionNumber();	// Returns the exception number
	std::string		 GetMessage();			// Returns the exception description message
	long			 GetErrorPos();			// Returns the Position of the error in the input stream
};



/************************************************************************/
/* CSyntaxException derived classes                                     */
/************************************************************************/

//////////////////////////////////////////////////////
// class CSyntaxException
//   Child class for VisualCalc Syntax Exceptions
class CSyntaxException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CSyntaxException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);
	
public:
	virtual ~CSyntaxException();
};


// class CUnexpectedCharacterException
//   Child class for VisualCalc Syntax Exceptions
class CUnexpectedCharacterException : public CSyntaxException {
public:
	CUnexpectedCharacterException(char cCharacter, int iErrorPos);
	~CUnexpectedCharacterException();
};


// class CDigitExpectedException
//   Child class for VisualCalc Syntax Exceptions
class CDigitExpectedException : public CSyntaxException {
public:
	CDigitExpectedException(int iErrorPos);
	~CDigitExpectedException();
};


// class CClosingParenthesisExpectedException
//   Child class for VisualCalc Syntax Exceptions
class CClosingParenthesisExpectedException : public CSyntaxException {
public:
	CClosingParenthesisExpectedException(int iErrorPos);
	~CClosingParenthesisExpectedException();
};


// class CMathematicExpressionExpectedException
//   Child class for VisualCalc Syntax Exceptions
class CMathematicExpressionExpectedException : public CSyntaxException {
public:
	CMathematicExpressionExpectedException(int iErrorPos);
	~CMathematicExpressionExpectedException();
};


/************************************************************************/
/* CMathematicException derived classes                                 */
/************************************************************************/

//////////////////////////////////////////////////////
// class CMathematicException
//   Child class for VisualCalc Mathematic Exceptions
class CMathematicException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CMathematicException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);

public:
	virtual ~CMathematicException();
};


// class CDivisionByZeroException
//   Child class for VisualCalc Mathematic Exceptions
class CDivisionByZeroException : public CMathematicException {
public:
	CDivisionByZeroException(int iErrorPos);
	virtual ~CDivisionByZeroException();
};


// class CNMustBePositiveException
//   Child class for VisualCalc Mathematic Exceptions
class CNMustBePositiveException : public CMathematicException {
public:
	CNMustBePositiveException(int iErrorPos);
	virtual ~CNMustBePositiveException();
};


// class CPMustBePositiveException
//   Child class for VisualCalc Mathematic Exceptions
class CPMustBePositiveException : public CMathematicException {
public:
	CPMustBePositiveException(int iErrorPos);
	virtual ~CPMustBePositiveException();
};


// class CPMustBeSmallerThanNException
//   Child class for VisualCalc Mathematic Exceptions
class CPMustBeSmallerThanNException : public CMathematicException {
public:
	CPMustBeSmallerThanNException(int iErrorPos);
	virtual ~CPMustBeSmallerThanNException();
};


// class CFactorialDefForPositiveIntsException
//   Child class for VisualCalc Mathematic Exceptions
class CFactorialDefForPositiveIntsException : public CMathematicException {
public:
	CFactorialDefForPositiveIntsException(int iErrorPos);
	virtual ~CFactorialDefForPositiveIntsException();
};


// class CParameterOutOfRangeException
//   Child class for VisualCalc Mathematic Exceptions
class CParameterOutOfRangeException : public CMathematicException {
public:
	CParameterOutOfRangeException(int iErrorPos);
	virtual ~CParameterOutOfRangeException();
};


// class CMathParameterMustBePositiveException
//   Child class for VisualCalc Mathematic Exceptions
class CMathParameterMustBePositiveException : public CMathematicException {
public:
	CMathParameterMustBePositiveException(int iErrorPos);
	virtual ~CMathParameterMustBePositiveException();
};


// class CMathParameterMustBeAnIntegerException
//   Child class for VisualCalc Mathematic Exceptions
class CMathParameterMustBeAnIntegerException : public CMathematicException {
public:
	CMathParameterMustBeAnIntegerException(int iErrorPos);
	virtual ~CMathParameterMustBeAnIntegerException();
};



/************************************************************************/
/* CFunctionException derived classes                                   */
/************************************************************************/

//////////////////////////////////////////////////////
// class CFunctionException
//   Child class for VisualCalc Function Exceptions
class CFunctionException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CFunctionException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);
	
public:
	virtual ~CFunctionException();
};

// class CUnknownFunctionCalledException
//   Child class for VisualCalc Function Exceptions
class CUnknownFunctionCalledException : public CFunctionException {
public:
	CUnknownFunctionCalledException(const std::string& strFunctionName, int iErrorPos);
	virtual ~CUnknownFunctionCalledException();
};

// class CImplicitMultiplicationException
//   Child class for VisualCalc Function Exceptions
class CImplicitMultiplicationException : public CFunctionException {
public:
	CImplicitMultiplicationException(int iErrorPos);
	virtual ~CImplicitMultiplicationException();
};

// class CIllegalUseOfFonctionNameException
//   Child class for VisualCalc Function Exceptions
class CIllegalUseOfFonctionNameException : public CFunctionException {
public:
	CIllegalUseOfFonctionNameException(const std::string& strFunctionName, int iErrorPos);
	virtual ~CIllegalUseOfFonctionNameException();
};



/************************************************************************/
/* CParameterException derived classes                                  */
/************************************************************************/

//////////////////////////////////////////////////////
// class CParameterException
//   Child class for VisualCalc Parameter Exceptions
class CParameterException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CParameterException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);

public:
	virtual ~CParameterException();
};

// class CTooFewParametersException
//   Child class for VisualCalc Parameter Exceptions
class CTooFewParametersException : public CParameterException {
public:
	CTooFewParametersException(int iErrorPos);
	virtual ~CTooFewParametersException();
};

// class CTooManyParametersException
//   Child class for VisualCalc Parameter Exceptions
class CTooManyParametersException : public CParameterException {
public:
	CTooManyParametersException(int iErrorPos);
	virtual ~CTooManyParametersException();
};



/************************************************************************/
/* CVariableException derived classes                                   */
/************************************************************************/

//////////////////////////////////////////////////////
// class CVariableException
//   Child class for VisualCalc Variable Exceptions
class CVariableException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CVariableException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);

public:
	virtual ~CVariableException();
};

// class CConstantAssignationException
//   Child class for VisualCalc Variable Exceptions
class CConstantAssignationException : public CVariableException {
public:
	CConstantAssignationException(const std::string& strConstantName, int iErrorPos);
	virtual ~CConstantAssignationException();
};

// class CLiteralAssignationException
//   Child class for VisualCalc Variable Exceptions
class CLiteralAssignationException : public CVariableException {
public:
	CLiteralAssignationException(int iErrorPos);
	virtual ~CLiteralAssignationException();
};

// class CUndefinedVariableException
//   Child class for VisualCalc Variable Exceptions
class CUndefinedVariableException : public CVariableException {
public:
	CUndefinedVariableException(const std::string& strVariableName, int iErrorPos);
	virtual ~CUndefinedVariableException();
};



/************************************************************************/
/* CDomainException derived classes                                     */
/************************************************************************/

//////////////////////////////////////////////////////
// class CDomainException
//   Child class for VisualCalc Domain Exceptions
class CDomainException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CDomainException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);

public:
	virtual ~CDomainException();
};

// class CAnswerRequestedNotFoundException
//   Child class for VisualCalc Domain Exceptions
class CAnswerRequestedNotFoundException : public CDomainException {
public:
	CAnswerRequestedNotFoundException(int iErrorPos);
	virtual ~CAnswerRequestedNotFoundException();
};

// class CDomParameterMustBePositiveException
//   Child class for VisualCalc Domain Exceptions
class CDomParameterMustBePositiveException : public CDomainException {
public:
	CDomParameterMustBePositiveException(int iErrorPos);
	virtual ~CDomParameterMustBePositiveException();
};

// class CDomParameterMustBeAnIntegerException
//   Child class for VisualCalc Domain Exceptions
class CDomParameterMustBeAnIntegerException : public CDomainException {
public:
	CDomParameterMustBeAnIntegerException(int iErrorPos);
	virtual ~CDomParameterMustBeAnIntegerException();
};

// class CDomParametersMustBePositiveException
//   Child class for VisualCalc Domain Exceptions
class CDomParametersMustBePositiveException : public CDomainException {
public:
	CDomParametersMustBePositiveException(int iErrorPos);
	virtual ~CDomParametersMustBePositiveException();
};

// class CDomParametersMustBeIntegersException
//   Child class for VisualCalc Domain Exceptions
class CDomParametersMustBeIntegersException : public CDomainException {
public:
	CDomParametersMustBeIntegersException(int iErrorPos);
	virtual ~CDomParametersMustBeIntegersException();
};



/************************************************************************/
/* CParserException derived classes                                     */
/************************************************************************/

//////////////////////////////////////////////////////
// class CParserException
//   Child class for VisualCalc general exceptions
class CParserException : public CVCalcParserException {
protected:
	// Protected to make the class abstract. It must not be possible to instantiate the class
	CParserException(ExceptionNumbers enExceptionNumber, const std::string& strExceptionMsg, int iErrorPos);

public:
	virtual ~CParserException();
};

// class CFunctionNotSupportedException
//   Child class for VisualCalc general exceptions
class CFunctionNotSupportedException : public CParserException {
public:
	CFunctionNotSupportedException(const std::string& strFunctionName, int iErrorPos);
	virtual ~CFunctionNotSupportedException();
};

// class CUnknownException
//   Child class for VisualCalc general exceptions
class CUnknownException : public CParserException {
public:
	CUnknownException(int iErrorPos);
	virtual ~CUnknownException();
};


#endif // !defined(__AFX_VCALCPARSEREXCEPTION_H_INCLUDED__)
