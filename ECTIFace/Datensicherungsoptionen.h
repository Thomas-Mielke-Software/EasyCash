#pragma once
#include "afxwin.h"

// Datensicherungsoptionen dialog : header file
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

class CDatensicherungsoptionen : public CDialog
{
	DECLARE_DYNAMIC(CDatensicherungsoptionen)

public:
	CDatensicherungsoptionen(int *npNachfrageIntervall, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDatensicherungsoptionen();

// Dialog Data
	enum { IDD = IDD_DATENSICHERUNGSOPTIONEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL PreTranslateMessage(MSG* pMsg);
	BOOL OnInitDialog();	
	void Nachfrageintervall();

	int *m_npNachfrageIntervall;
	BOOL bNachfrageIntervallGeaendert;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedKeine();
	afx_msg void OnBnClickedIntern();
	afx_msg void OnBnClickedExtern();
	afx_msg void OnBnClickedDisk();
	afx_msg void OnBnClickedZeit();
	afx_msg void OnBnClickedCancel();
	CButton m_DiskBtn;
	CButton m_ExternBtn;
	CButton m_InternBtn;
	CButton m_KeineBtn;
	CButton m_ZeitBtn;
	CButton m_DiskBtn2;
	CButton m_ExternBtn2;
	CButton m_InternBtn2;
	CButton m_KeineBtn2;
	CButton m_ZeitBtn2;
	CStatic m_IntervallStatic;
	CToolTipCtrl m_ToolTip;
};
