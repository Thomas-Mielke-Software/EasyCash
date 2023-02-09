// Formularabschnitt.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCash.h"
#include "Formularabschnitt.h"


// CFormularabschnitt dialog

IMPLEMENT_DYNAMIC(CFormularabschnitt, CDialog)

CFormularabschnitt::CFormularabschnitt(CWnd* pParent /*=NULL*/)
	: CDialog(CFormularabschnitt::IDD, pParent)
	, m_name(_T(""))
	, m_seite(0)
	, m_vertikal(0)
{

}

CFormularabschnitt::~CFormularabschnitt()
{
}

void CFormularabschnitt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormularabschnitt)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_SEITE, m_seite);
	DDV_MinMaxInt(pDX, m_seite, 1, 10000);
	DDX_Text(pDX, IDC_VERTIKAL, m_vertikal);
	DDV_MinMaxInt(pDX, m_vertikal, 0, 1413);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFormularabschnitt, CDialog)
END_MESSAGE_MAP()


// CFormularabschnitt message handlers


