// XMLite.h: interface for the XMLite class.
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Public Domain (CPOL) 2020  Thomas Mielke
// 
// Ursprünglich wurde dieser Code von Code Project übernommen:
// Artikel: https://www.codeproject.com/Articles/3426/XMLite-simple-XML-parser
//
// Dies ist public domain Software; Sie dürfen sie unter den Bedingungen der 
// Code Project Open License (CPOL) verwenden.
// CPOL: https://www.codeproject.com/info/cpol10.aspx
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
//
// XMLite : XML Lite Parser Library
// by bro ( Cho,Kyung Min: bro@shinbiro.com ) 2002-10-30
// Microsoft MVP (Visual C++) bro@msmvp.com
// 
// History.
// 2002-10-29 : First Coded. Parsing XMLElelement and Attributes.
//              get xml parsed string ( looks good )
// 2002-10-30 : Get Node Functions, error handling ( not completed )
// 2002-12-06 : Helper Funtion string to long
// 2002-12-12 : Entity Helper Support
// 2003-04-08 : Close, 
// 2003-07-23 : add property escape_value. (now no escape on default)
// 2003-10-24 : bugfix) attribute parsing <tag a='1' \r\n/> is now ok
// 2004-03-05 : add branch copy functions
// 2004-06-14 : add _tcseistr/_tcsenistr/_tcsenicmp functions
// 2004-06-14 : now support, XML Document and PI, Comment, CDATA node
// 2004-06-15 : add GetText()/ Find() functions
// 2004-06-15 : add force_parse : now can parse HTML (not-welformed xml)
// 
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMLITE_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_)
#define AFX_XMLITE_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <deque>

struct _tagXMLAttr;
typedef _tagXMLAttr XAttr, *LPXAttr;
typedef std::vector<LPXAttr> XAttrs;

struct _tagXMLNode;
typedef _tagXMLNode XNode, *LPXNode;
typedef std::vector<LPXNode> XNodes, *LPXNodes;

struct _tagXMLDocument;
typedef struct _tagXMLDocument XDoc, *LPXDoc;

// Entity Encode/Decode Support
typedef struct _tagXmlEntity
{
	TCHAR entity;					// entity ( & " ' < > )
	TCHAR ref[10];					// entity reference ( &amp; &quot; etc )
	int ref_len;					// entity reference length
}XENTITY,*LPXENTITY;

typedef struct _tagXMLEntitys : public std::vector<XENTITY>
{
	AFX_EXT_API LPXENTITY GetEntity( int entity );
	AFX_EXT_API LPXENTITY GetEntity( LPTSTR entity );	
	AFX_EXT_API int GetEntityCount( LPCTSTR str );
	AFX_EXT_API int Ref2Entity( LPCTSTR estr, LPTSTR str, int strlen );
	AFX_EXT_API int Entity2Ref( LPCTSTR str, LPTSTR estr, int estrlen );
	AFX_EXT_API CString Ref2Entity( LPCTSTR estr );
	AFX_EXT_API CString Entity2Ref( LPCTSTR str );	

	_tagXMLEntitys(){};
	AFX_EXT_API _tagXMLEntitys( LPXENTITY entities, int count );
}XENTITYS,*LPXENTITYS;
//extern XENTITYS entityDefault;
static const XENTITY x_EntityTable[] = {
		{ '&', _T("&amp;"), 5 } ,
		{ '\"', _T("&quot;"), 6 } ,
		{ '\'', _T("&apos;"), 6 } ,
		{ '<', _T("&lt;"), 4 } ,
		{ '>', _T("&gt;"), 4 } 
	};
static XENTITYS entityDefault((LPXENTITY)x_EntityTable, sizeof(x_EntityTable)/sizeof(x_EntityTable[0]) ); // aus cpp
AFX_EXT_API CString XRef2Entity( LPCTSTR estr );
AFX_EXT_API CString XEntity2Ref( LPCTSTR str );	

typedef enum 
{
	PIE_PARSE_WELFORMED	= 0,
	PIE_ALONE_NOT_CLOSED,
	PIE_NOT_CLOSED,
	PIE_NOT_NESTED,
	PIE_ATTR_NO_VALUE
}PCODE;

// Parse info.
typedef struct _tagParseInfo
{
	bool		trim_value;			// [set] do trim when parse?
	bool		entity_value;		// [set] do convert from reference to entity? ( &lt; -> < )
	LPXENTITYS	entitys;			// [set] entity table for entity decode
	TCHAR		escape_value;		// [set] escape value (default '\\')
	bool		force_parse;		// [set] force parse even if xml is not welformed

	LPTSTR		xml;				// [get] xml source
	bool		erorr_occur;		// [get] is occurance of error?
	LPTSTR		error_pointer;		// [get] error position of xml source
	PCODE		error_code;			// [get] error code
	CString		error_string;		// [get] error string

	LPXDoc		doc;
	_tagParseInfo() { trim_value = false; entity_value = true; force_parse = false; entitys = &entityDefault; xml = NULL; erorr_occur = false; error_pointer = NULL; error_code = PIE_PARSE_WELFORMED; escape_value = '\\'; }
}PARSEINFO,*LPPARSEINFO;
extern PARSEINFO piDefault;

// display optional environment
typedef struct _tagDispOption
{
	bool newline;			// newline when new tag
	bool reference_value;	// do convert from entity to reference ( < -> &lt; )
	char value_quotation_mark;	// val="" (default value quotation mark "
	LPXENTITYS	entitys;	// entity table for entity encode

	int tab_base;			// internal usage
	_tagDispOption() { newline = true; reference_value = true; entitys = &entityDefault; tab_base = 0; value_quotation_mark = '"'; }
}DISP_OPT, *LPDISP_OPT;
extern DISP_OPT optDefault;

// XAttr : Attribute Implementation
typedef struct _tagXMLAttr
{
	CString name;
	CString	value;
	
	_tagXMLNode*	parent;

	AFX_EXT_API CString GetXML( LPDISP_OPT opt = &optDefault );
}XAttr, *LPXAttr;

typedef enum
{
	XNODE_ELEMENT,				// general node '<element>...</element>' or <element/>
	XNODE_PI,					// <?xml version="1.0" ?>
	XNODE_COMMENT,				// <!-- comment -->
	XNODE_CDATA,				// <![CDATA[ cdata ]]>
	XNODE_DOC,					// internal virtual root
}NODE_TYPE;

// XMLNode structure
typedef struct _tagXMLNode
{
	// name and value
	CString name;
	CString	value;

	// internal variables
	LPXNode	parent;		// parent node
	XNodes	childs;		// child node
	XAttrs	attrs;		// attributes
	NODE_TYPE type;		// node type 
	LPXDoc	doc;		// document

	// Load/Save XML
	AFX_EXT_API LPTSTR	Load( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );
	AFX_EXT_API CString GetXML( LPDISP_OPT opt = &optDefault );
	AFX_EXT_API CString GetText( LPDISP_OPT opt = &optDefault );

	// internal load functions
	AFX_EXT_API LPTSTR	LoadAttributes( LPCTSTR pszAttrs, LPPARSEINFO pi = &piDefault );
	AFX_EXT_API LPTSTR	LoadAttributes( LPCTSTR pszAttrs, LPCTSTR pszEnd, LPPARSEINFO pi = &piDefault );
	AFX_EXT_API LPTSTR	LoadProcessingInstrunction( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault );
	AFX_EXT_API LPTSTR	LoadComment( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault ); 
	AFX_EXT_API LPTSTR	LoadCDATA( LPCTSTR pszXml, LPPARSEINFO pi = &piDefault ); 

	// in own attribute list
	AFX_EXT_API LPXAttr	GetAttr( LPCTSTR attrname ); 
	AFX_EXT_API LPCTSTR	GetAttrValue( LPCTSTR attrname ); 
	AFX_EXT_API XAttrs	GetAttrs( LPCTSTR name ); 

	// in one level child nodes
	AFX_EXT_API LPXNode	GetChild( LPCTSTR name ); 
	AFX_EXT_API LPCTSTR	GetChildValue( LPCTSTR name ); 
	AFX_EXT_API CString	GetChildText( LPCTSTR name, LPDISP_OPT opt = &optDefault );
	AFX_EXT_API XNodes	GetChilds( LPCTSTR name ); 
	AFX_EXT_API XNodes	GetChilds(); 

	AFX_EXT_API LPXAttr GetChildAttr( LPCTSTR name, LPCTSTR attrname );
	AFX_EXT_API LPCTSTR GetChildAttrValue( LPCTSTR name, LPCTSTR attrname );
	
	// search node
	AFX_EXT_API LPXNode	Find( LPCTSTR name );

	// modify DOM 
	AFX_EXT_API int		GetChildCount();
	AFX_EXT_API LPXNode GetChild( int i );
	AFX_EXT_API XNodes::iterator GetChildIterator( LPXNode node );
	AFX_EXT_API LPXNode CreateNode( LPCTSTR name = NULL, LPCTSTR value = NULL );
	AFX_EXT_API LPXNode	AppendChild( LPCTSTR name = NULL, LPCTSTR value = NULL );
	AFX_EXT_API LPXNode	AppendChild( LPXNode node );
	AFX_EXT_API bool	RemoveChild( LPXNode node );
	AFX_EXT_API LPXNode DetachChild( LPXNode node );

	// node/branch copy
	AFX_EXT_API void	CopyNode( LPXNode node );
	AFX_EXT_API void	CopyBranch( LPXNode branch );
	AFX_EXT_API void	_CopyBranch( LPXNode node );
	AFX_EXT_API LPXNode	AppendChildBranch( LPXNode node );

	// modify attribute
	AFX_EXT_API LPXAttr GetAttr( int i );
	AFX_EXT_API XAttrs::iterator GetAttrIterator( LPXAttr node );
	AFX_EXT_API LPXAttr CreateAttr( LPCTSTR anem = NULL, LPCTSTR value = NULL );
	AFX_EXT_API LPXAttr AppendAttr( LPCTSTR name = NULL, LPCTSTR value = NULL );
	AFX_EXT_API LPXAttr	AppendAttr( LPXAttr attr );
	AFX_EXT_API bool	RemoveAttr( LPXAttr attr );
	AFX_EXT_API LPXAttr DetachAttr( LPXAttr attr );

	// operator overloads
	AFX_EXT_API LPXNode operator [] ( int i ) { return GetChild(i); }
	AFX_EXT_API XNode& operator = ( XNode& node ) { CopyBranch(&node); return *this; }

	AFX_EXT_API _tagXMLNode() { parent = NULL; doc = NULL; type = XNODE_ELEMENT; }
	AFX_EXT_API ~_tagXMLNode();

	void Close();
}XNode, *LPXNode;

// XMLDocument structure
typedef struct _tagXMLDocument : public XNode
{
	PARSEINFO	parse_info;

	_tagXMLDocument() { parent = NULL; doc = this; type = XNODE_DOC; }
	
	AFX_EXT_API LPTSTR	Load( LPCTSTR pszXml, LPPARSEINFO pi = NULL );
	AFX_EXT_API LPTSTR	LoadFile( LPCTSTR pszFilename, LPPARSEINFO pi = NULL );
	AFX_EXT_API LPXNode	GetRoot();
	AFX_EXT_API BOOL	SaveFile(LPCTSTR pszFilename, LPDISP_OPT opt = &optDefault );

}XDoc, *LPXDoc;

// Helper Funtion
inline long XStr2Int( LPCTSTR str, long default_value = 0 )
{
	return ( str && *str ) ? _ttol(str) : default_value;
}

inline bool XIsEmptyString( LPCTSTR str )
{
	CString s(str);
	s.TrimLeft();
	s.TrimRight();

	return ( s.IsEmpty() || s == _T("") );
}

#endif // !defined(AFX_XMLITE_H__786258A5_8360_4AE4_BDAF_2A52F8E1B877__INCLUDED_)
