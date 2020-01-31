// VCalcParser.h: interface for the CVCalcParser class.
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

#if !defined(__AFX_VCALCPARSER_H_INCLUDED__)
#define		 __AFX_VCALCPARSER_H_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif

#pragma warning (disable: 4786)	//identifier was truncated to '255' characters in the debug information

#include <ALGORITHM>// std::find()
#include <STRING>	// std::string
#include <LIST>		// std::list<>
#include <MAP>		// std::map<>
#include <DEQUE>	// std::deque<>
#include <CMATH>	// ::abs(), ::acos(), ::asin(), ::atan(), ::cos(),   ::cosh()
					// ::exp(), ::fabs(), ::floor() ::log(),  ::log10(), ::fmod()
					// ::pow(), ::sin(),  ::sinh(), ::sqrt(), ::tan(),   ::tanh()
#include "VCalcParserTypes.h"


////////////////////////////////////////////////////////////////
// VisualCalc 2.20 Parser...
class CVCalcParser {
private:
	// Mathematic constants
	const VALUES_TYPE	m_PI;	// 3.1415926535897932384626433832795
	const VALUES_TYPE	m_E;	// 2.7182818284590452353602874713527

	// Members for parsing treatment
	TokenValue					m_tokCurrentToken;		// Current token extracted from the input stream
	std::string					m_strSource;			// Reference to the input stream
	std::string					m_strIdentifierValue;	// Identifier extracted as an IDENTIFIER token
	std::string					m_strWarningMsg;		// To describe the warning
	VALUES_TYPE					m_valNumberValue;		// Number extracted as a NUMBER token
	bool						m_bWarningFlag;			// Non-interrupting low level message
	bool						m_bEndEncountered;		// The end of the input stream is reached
	int							m_iCurrentIndex;		// Index in the input stream
	std::list<std::string>		m_lstFunctions;			// Supported functions
	std::map<std::string, VALUES_TYPE>
								m_mapVariables;			// User defined variables table mapping the identifier with a value
	std::deque<AnswerItem>		m_dqeAnswersHistory;	// Answers History mapping a formula with a result
	std::string					m_strParserVersion;		// Version of the Parser

	// Locally defined/redefined mathematic functions
	VALUES_TYPE	ffactor	(VALUES_TYPE);
	VALUES_TYPE	nCp		(VALUES_TYPE, VALUES_TYPE);
	VALUES_TYPE	nAp		(VALUES_TYPE, VALUES_TYPE);
	AnswerItem	Ans		(VALUES_TYPE valIndex);

	VALUES_TYPE abs		(VALUES_TYPE);
	VALUES_TYPE cos		(VALUES_TYPE);
	VALUES_TYPE sin		(VALUES_TYPE);
	VALUES_TYPE tan		(VALUES_TYPE);
	VALUES_TYPE cosh	(VALUES_TYPE);
	VALUES_TYPE sinh	(VALUES_TYPE);
	VALUES_TYPE tanh	(VALUES_TYPE);
	VALUES_TYPE Acos	(VALUES_TYPE);
	VALUES_TYPE Asin	(VALUES_TYPE);
	VALUES_TYPE Atan	(VALUES_TYPE);
	VALUES_TYPE deg		(VALUES_TYPE);
	VALUES_TYPE rad		(VALUES_TYPE);
	VALUES_TYPE exp		(VALUES_TYPE);
	VALUES_TYPE ln		(VALUES_TYPE);
	VALUES_TYPE log		(VALUES_TYPE);
	VALUES_TYPE logn	(VALUES_TYPE, VALUES_TYPE);
	VALUES_TYPE sqrt	(VALUES_TYPE);
	VALUES_TYPE pow		(VALUES_TYPE, VALUES_TYPE);
	VALUES_TYPE mod		(VALUES_TYPE, VALUES_TYPE);
	VALUES_TYPE sum		(std::string expr, std::string var, VALUES_TYPE low, VALUES_TYPE high);
	VALUES_TYPE product	(std::string expr, std::string var, VALUES_TYPE low, VALUES_TYPE high);

public:
	CVCalcParser();
	virtual ~CVCalcParser();
	void ResetParserMembers(const std::string);
	void ResetFunctions();
	void ResetVariables();
	void ResetAnswersHistory();
	const std::list<std::string>&				GetFunctions();
	const std::map<std::string, VALUES_TYPE>&	GetVariables();
	const std::deque<AnswerItem>&				GetAnswersHistory();
	bool HasWarning();
	std::string GetWarningMsg();
	std::string GetVersion();

	//Parsing functions following (in the recursive-descending order)...
	VALUES_TYPE Evaluate(const std::string& Source);

private:
	VALUES_TYPE	Level_1	(void);		// + , -
	VALUES_TYPE	Level_2	(void);		// * , /
	VALUES_TYPE	Level_3	(void);		// ^
	VALUES_TYPE	Level_4	(void);		// %
	VALUES_TYPE	Level_5	(void);		// ! , °
	VALUES_TYPE	Primary	(void);		// ( ) , Unary + , Unary -
	TokenValue	GetToken(void);
};


#endif // !defined(__AFX_VCALCPARSER_H_INCLUDED__)
