// VCalcParserTypes.h: VisualCalc Parser Types 
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

#if !defined(__AFX_VCALCPARSERTYPES_H_INCLUDED__)
#define		 __AFX_VCALCPARSERTYPES_H_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif


#include "VCalcParserException.h"


// Type for the handled operands and results
typedef	long double VALUES_TYPE;


// Type for recognized separation tokens
typedef enum {
	TV_NUMBER,
	TV_IDENTIFIER,
	TV_END,
	TV_SEQ		= ',',
	TV_PLUS		= '+',
	TV_MINUS	= '-',
	TV_MUL		= '*',
	TV_DIV		= '/',
	TV_POW		= '^',
	TV_MOD		= '%',
	TV_FACT		= '!',
	TV_DEG		= '°',
	TV_LP		= '(',
	TV_RP		= ')',
	TV_ASSIGN	= '='
} TokenValue;


// Type for an entry in the answers history
typedef struct {
	std::string	m_strFormula;
	VALUES_TYPE	m_valResult;
} AnswerItem;


#endif // !defined(__AFX_VCALCPARSERTYPES_H_INCLUDED__)
