// NeuesFormular.cpp : implementation file
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
#include "easycash.h"
#include "NeuesFormular.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNeuesFormular dialog


CNeuesFormular::CNeuesFormular(CWnd* pParent /*=NULL*/)
	: CDialog(CNeuesFormular::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNeuesFormular)
	m_name = _T("");
	m_seiten = 0;
	m_dateiname = _T("");
	m_grafikdateien = _T("");
	m_anzeigename = _T("");
	m_schriftart = _T("");
	m_schriftgroesse = 0;
	//}}AFX_DATA_INIT
}


void CNeuesFormular::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNeuesFormular)
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Text(pDX, IDC_SEITEN, m_seiten);
	DDV_MinMaxUInt(pDX, m_seiten, 1, 10000);
	DDX_Text(pDX, IDC_DATEINAME, m_dateiname);
	DDX_Text(pDX, IDC_GRAFIKDATEIEN, m_grafikdateien);
	DDX_Text(pDX, IDC_ANZEIGENAME, m_anzeigename);
	DDX_Text(pDX, IDC_SCHRIFTART, m_schriftart);
	DDX_Text(pDX, IDC_SCHRIFTGROESSE, m_schriftgroesse);
	DDV_MinMaxUInt(pDX, m_schriftgroesse, 5, 128);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNeuesFormular, CDialog)
	//{{AFX_MSG_MAP(CNeuesFormular)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNeuesFormular message handlers

void CNeuesFormular::OnOK() 
{
	UpdateData();
	if (m_name.GetLength() == 0)
	{
		AfxMessageBox("Bitte einen aussagekräftigen Formularnamen angeben.");
		GetDlgItem(IDC_NAME)->SetFocus();
		return;
	}
	
	CString csTemp = m_dateiname;
	csTemp.MakeLower();
	if (csTemp.GetLength() <= 4)
	{
		AfxMessageBox("Bitte einen gültigen Dateinamen angeben.");
		GetDlgItem(IDC_DATEINAME)->SetFocus();
		return;
	}
	if (csTemp.Right(4) != ".ecf")
	{
		AfxMessageBox("Dateiname muss mit '.ecf' enden.");
		GetDlgItem(IDC_DATEINAME)->SetFocus();
		return;
	}

	if (m_seiten <= 0)
	{
		AfxMessageBox("Bitte die Anzahl an Formularseiten angeben.");
		GetDlgItem(IDC_SEITEN)->SetFocus();
		return;
	}
	
	CDialog::OnOK();
}
