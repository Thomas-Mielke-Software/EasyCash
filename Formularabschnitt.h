#pragma once


// CFormularabschnitt dialog

class CFormularabschnitt : public CDialog
{
	DECLARE_DYNAMIC(CFormularabschnitt)

public:
	CFormularabschnitt(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFormularabschnitt();

// Dialog Data
	enum { IDD = IDD_FORMULARABSCHNITT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Abschnittsname
	CString m_name;
	int m_seite;
	int m_vertikal;
};
