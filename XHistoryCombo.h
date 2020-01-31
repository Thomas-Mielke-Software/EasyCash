////////////////////////////////////////////////////////////////////////////
// File:	HistoryCombo.h
// Version:	2
// Created:	10-May-2002
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
// Das Entfernen des Namens des ursprünglichen Autors Paul S. Vickery ist
// jedoch ausgeschlossen (siehe vorhergehende Release-Info weiter unten).
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 
//
// Author:	Paul S. Vickery
// E-mail:	paul@vickeryhome.freeserve.co.uk
//
// Implementation of CHistoryCombo which incorporates functionality to help
// with Loading and Saving of history in a combo box
//
// You are free to use or modify this code, with no restrictions, other than
// you continue to acknowledge me as the original author in this source code,
// or any code derived from it.
//
// If you use this code, or use it as a base for your own code, it would be 
// nice to hear from you simply so I know it's not been a waste of time!
//
// Copyright (c) 2001-2002 Paul S. Vickery
//
////////////////////////////////////////////////////////////////////////////
// Version History:
//
// Version 2 - 01-May-2002
// =======================
// Produced new version with changes as below:
// * removed CBS_SORT on creation if specified
// * added option to allow the sort style to be set if required
// * fixed SetMaxHistoryItems, so it removes old entries from the list to 
//   ensure that there are no more than the maximum. Also made SaveHistory
//   remove redundant profile entries above the maximum.
// * use WriteProfileString to remove profile entries rather than CRegKey.
//
// Version 1 - 12-Apr-2001
// =======================
// Initial version
// 
////////////////////////////////////////////////////////////////////////////
// PLEASE LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////
//
// Modified 20-Jun-2003 by Hans Dietrich - see "+++hd" lines for details.  
// Changed name to XHistoryCombo to avoid confusion with original.
//
////////////////////////////////////////////////////////////////////////////

#ifndef HISTORYCOMBO_H
#define HISTORYCOMBO_H

#ifndef __AFXADV_H__
#include "afxadv.h" // needed for CRecentFileList
#endif // ! __AFXADV_H__

/////////////////////////////////////////////////////////////////////////////
// CHistoryCombo window

class CHistoryCombo : public CComboBox
{
// Construction
public:
	CHistoryCombo(BOOL bAllowSortStyle = FALSE);

// Attributes
public:

// Operations
public:
	int AddString(LPCTSTR lpszString);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistoryCombo)
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	void	StoreValue(BOOL bIgnoreIfEmpty = TRUE);
	void	ClearHistory(BOOL bDeleteRegistryEntries = TRUE);
	void	SetMaxHistoryItems(int nMaxItems);
	void	SaveHistory(BOOL bAddCurrentItemtoHistory = TRUE);
	CString	LoadHistory(LPCTSTR lpszSection, 
						LPCTSTR lpszKeyPrefix, 
						BOOL bSaveRestoreLastCurrent = TRUE, 
						LPCTSTR lpszKeyCurItem = NULL);
	CString	LoadHistory(CRecentFileList* pListMRU, BOOL bSelectMostRecent = TRUE);

	int		GetMaxHistoryItems() { return m_nMaxHistoryItems; }						//+++hd
	void	SetCheckAccess(BOOL bCheckAccess) { m_bCheckAccess = bCheckAccess; }	//+++hd
	BOOL	GetCheckAccess() { return m_bCheckAccess; }								//+++hd
	void	SetDropSize(int nDropSize) { m_nDropSize = nDropSize; }					//+++hd
	int		GetDropSize() { return m_nDropSize; }									//+++hd

	// Generated message map functions
protected:
	CString m_sSection;
	CString m_sKeyPrefix;
	CString m_sKeyCurItem;
	BOOL m_bSaveRestoreLastCurrent;
	int m_nMaxHistoryItems;
	BOOL m_bAllowSortStyle;
	BOOL m_bCheckAccess;	// TRUE = check if the strings are file paths	//+++hd
	int m_nDropSize;		// no. of items in dropdown list				//+++hd

	//{{AFX_MSG(CHistoryCombo)
	afx_msg void OnDropdown();		//+++hd
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //HISTORYCOMBO_H
