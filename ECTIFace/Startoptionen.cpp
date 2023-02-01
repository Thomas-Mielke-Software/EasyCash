// Startoptionen.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "Startoptionen.h"


// CStartoptionen dialog : implementation
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

IMPLEMENT_DYNAMIC(CStartoptionen, CDialog)

CStartoptionen::CStartoptionen(CWnd* pParent /*=NULL*/)
	: CDialog(CStartoptionen::IDD, pParent)
{

}

CStartoptionen::~CStartoptionen()
{
}

void CStartoptionen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OEFFNEN, m_OeffnenBn);
	DDX_Control(pDX, IDC_OEFFNEN2, m_Oeffnen2Bn);
	DDX_Control(pDX, IDC_DATENVERZEICHNIS, m_DatenverzeichnisBn);
	DDX_Control(pDX, IDC_DATENVERZEICHNIS2, m_Datenverzeichnis2Bn);
	DDX_Control(pDX, IDC_LEER, m_LeerBn);
	DDX_Control(pDX, IDC_LEER2, m_Leer2Bn);
	DDX_Control(pDX, IDC_OHNE, m_OhneBn);
	DDX_Control(pDX, IDC_OHNE2, m_Ohne2Bn);
	DDX_Control(pDX, IDC_EXIT, m_ExitBn);
	DDX_Control(pDX, IDC_EXIT2, m_Exit2Bn);
}


BEGIN_MESSAGE_MAP(CStartoptionen, CDialog)
	ON_BN_CLICKED(IDC_OEFFNEN, &CStartoptionen::OnBnClickedOeffnen)
	ON_BN_CLICKED(IDC_DATENVERZEICHNIS, &CStartoptionen::OnBnClickedDatenverzeichnis)
	ON_BN_CLICKED(IDC_LEER, &CStartoptionen::OnBnClickedLeer)
	ON_BN_CLICKED(IDC_OHNE, &CStartoptionen::OnBnClickedOhne)
	ON_BN_CLICKED(IDC_EXIT, &CStartoptionen::OnBnClickedExit)
	ON_BN_CLICKED(IDC_OEFFNEN2, &CStartoptionen::OnBnClickedOeffnen)
	ON_BN_CLICKED(IDC_DATENVERZEICHNIS2, &CStartoptionen::OnBnClickedDatenverzeichnis)
	ON_BN_CLICKED(IDC_LEER2, &CStartoptionen::OnBnClickedLeer)
	ON_BN_CLICKED(IDC_OHNE2, &CStartoptionen::OnBnClickedOhne)
	ON_BN_CLICKED(IDC_EXIT2, &CStartoptionen::OnBnClickedExit)
END_MESSAGE_MAP()


// CStartoptionen message handlers

BOOL CStartoptionen::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// läuft unter Wine auf Linux?
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Wine", &hKey) == ERROR_SUCCESS)
	{
		m_OeffnenBn.ShowWindow(SW_HIDE);
		m_DatenverzeichnisBn.ShowWindow(SW_HIDE);
		m_LeerBn.ShowWindow(SW_HIDE);
		m_OhneBn.ShowWindow(SW_HIDE);
		m_ExitBn.ShowWindow(SW_HIDE);

		/*
		//Create the ToolTip control
		if( !m_ToolTip.Create(this))
		{
			 TRACE0("Unable to create the ToolTip!");
		}
		else
		{
			
	  		m_ToolTip.AddTool( &m_Oeffnen2Bn, (CString)_T(""));
			m_ToolTip.AddTool( &m_Datenverzeichnis2Bn, _T(""));

			m_ToolTip.Activate(TRUE);			
		} 
		*/
	}
	else
	{
		m_Oeffnen2Bn.ShowWindow(SW_HIDE);
		m_Datenverzeichnis2Bn.ShowWindow(SW_HIDE);
		m_Leer2Bn.ShowWindow(SW_HIDE);
		m_Ohne2Bn.ShowWindow(SW_HIDE);
		m_Exit2Bn.ShowWindow(SW_HIDE);

		/*
		//Create the ToolTip control
		if( !m_ToolTip.Create(this))
		{
			 TRACE0("Unable to create the ToolTip!");
		}
		else
		{
			
	  		m_ToolTip.AddTool( &m_OeffnenBn, (CString)_T(""));
			m_ToolTip.AddTool( &m_DatenverzeichnisBn, _T(""));

			m_ToolTip.Activate(TRUE);			
		} 
		*/     
	}

	return TRUE;
}

void CStartoptionen::OnBnClickedOeffnen()
{
	CDialog::EndDialog(0);
}

void CStartoptionen::OnBnClickedDatenverzeichnis()
{
	CDialog::EndDialog(1);
}

void CStartoptionen::OnBnClickedLeer()
{
	CDialog::EndDialog(2);
}

void CStartoptionen::OnBnClickedOhne()
{
	CDialog::EndDialog(3);
}

void CStartoptionen::OnBnClickedExit()
{
	CDialog::EndDialog(4);
}
