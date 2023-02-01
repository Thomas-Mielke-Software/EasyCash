#pragma once
#include "afxwin.h"


// CStartoptionen dialog : header file
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

class CStartoptionen : public CDialog
{
	DECLARE_DYNAMIC(CStartoptionen)

public:
	CStartoptionen(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStartoptionen();

// Dialog Data
	enum { IDD = IDD_STARTOPTIONEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();	

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOeffnen();
	afx_msg void OnBnClickedDatenverzeichnis();
	afx_msg void OnBnClickedLeer();
	afx_msg void OnBnClickedOhne();
	afx_msg void OnBnClickedExit();
	CButton m_OeffnenBn;
	CButton m_Oeffnen2Bn;
	CButton m_DatenverzeichnisBn;
	CButton m_Datenverzeichnis2Bn;
	CButton m_LeerBn;
	CButton m_Leer2Bn;
	CButton m_OhneBn;
	CButton m_Ohne2Bn;
	CButton m_ExitBn;
	CButton m_Exit2Bn;
};
