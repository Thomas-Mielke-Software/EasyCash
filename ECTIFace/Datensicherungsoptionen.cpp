// Datensicherungsoptionen.cpp : implementation file
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
#include "resource.h"
#include "Datensicherungsoptionen.h"


// CDatensicherungsoptionen dialog

IMPLEMENT_DYNAMIC(CDatensicherungsoptionen, CDialog)

CDatensicherungsoptionen::CDatensicherungsoptionen(int *npNachfrageIntervall, CWnd* pParent /*=NULL*/)
	: CDialog(CDatensicherungsoptionen::IDD, pParent)
{
	m_npNachfrageIntervall = npNachfrageIntervall;
	bNachfrageIntervallGeaendert = FALSE;
}

CDatensicherungsoptionen::~CDatensicherungsoptionen()
{
}

void CDatensicherungsoptionen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DISK, m_DiskBtn);
	DDX_Control(pDX, IDC_EXTERN, m_ExternBtn);
	DDX_Control(pDX, IDC_INTERN, m_InternBtn);
	DDX_Control(pDX, IDC_KEINE, m_KeineBtn);
	DDX_Control(pDX, IDC_ZEIT, m_ZeitBtn);
	DDX_Control(pDX, IDC_DISK2, m_DiskBtn2);
	DDX_Control(pDX, IDC_EXTERN2, m_ExternBtn2);
	DDX_Control(pDX, IDC_INTERN2, m_InternBtn2);
	DDX_Control(pDX, IDC_KEINE2, m_KeineBtn2);
	DDX_Control(pDX, IDC_ZEIT2, m_ZeitBtn2);
	DDX_Control(pDX, IDC_INTERVALL, m_IntervallStatic);
}


BEGIN_MESSAGE_MAP(CDatensicherungsoptionen, CDialog)
	ON_BN_CLICKED(IDC_KEINE, &CDatensicherungsoptionen::OnBnClickedKeine)
	ON_BN_CLICKED(IDC_INTERN, &CDatensicherungsoptionen::OnBnClickedIntern)
	ON_BN_CLICKED(IDC_EXTERN, &CDatensicherungsoptionen::OnBnClickedExtern)
	ON_BN_CLICKED(IDC_DISK, &CDatensicherungsoptionen::OnBnClickedDisk)
	ON_BN_CLICKED(IDC_ZEIT, &CDatensicherungsoptionen::OnBnClickedZeit)
	ON_BN_CLICKED(IDC_KEINE2, &CDatensicherungsoptionen::OnBnClickedKeine)
	ON_BN_CLICKED(IDC_INTERN2, &CDatensicherungsoptionen::OnBnClickedIntern)
	ON_BN_CLICKED(IDC_EXTERN2, &CDatensicherungsoptionen::OnBnClickedExtern)
	ON_BN_CLICKED(IDC_DISK2, &CDatensicherungsoptionen::OnBnClickedDisk)
	ON_BN_CLICKED(IDC_ZEIT2, &CDatensicherungsoptionen::OnBnClickedZeit)
	ON_BN_CLICKED(IDCANCEL, &CDatensicherungsoptionen::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDatensicherungsoptionen message handlers

BOOL CDatensicherungsoptionen::PreTranslateMessage(MSG* pMsg)
{
     m_ToolTip.RelayEvent(pMsg);

     return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDatensicherungsoptionen::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// läuft unter Wine auf Linux?
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Wine", &hKey) == ERROR_SUCCESS)
	{
		m_KeineBtn.ShowWindow(SW_HIDE);
		m_InternBtn.ShowWindow(SW_HIDE);
		m_ExternBtn.ShowWindow(SW_HIDE);
		m_DiskBtn.ShowWindow(SW_HIDE);
		m_ZeitBtn.ShowWindow(SW_HIDE);

		//Create the ToolTip control
		if( !m_ToolTip.Create(this))
		{
			 TRACE0("Unable to create the ToolTip!");
		}
		else
		{
	  		m_ToolTip.AddTool( &m_KeineBtn2, (CString)_T("jetzt keine Datensicherung machen und in ") + (*m_npNachfrageIntervall == 7 ? _T("einer Woche") : _T("einem Monat")) + _T(" noch einmal nachfragen"));
			m_ToolTip.AddTool( &m_InternBtn2, _T("einfachste Option mit nur einem Klick -- aber Festplatten gehen manchmal kaputt..."));
			m_ToolTip.AddTool( &m_ExternBtn2, _T("speichern auf externem Laufwerk, falls die interne Festplatte mal den Geist aufgibt"));
			m_ToolTip.AddTool( &m_DiskBtn2, _T("Das Archivieren auf einer nur einmal beschreibbaren Disk entspricht am ehesten den 'Grundsätzen ordnungsmäßiger DV-gestützter Buchführungssystemen' (GoBS)"));

			m_ToolTip.Activate(TRUE);
		}    
	}
	else
	{
		m_KeineBtn2.ShowWindow(SW_HIDE);
		m_InternBtn2.ShowWindow(SW_HIDE);
		m_ExternBtn2.ShowWindow(SW_HIDE);
		m_DiskBtn2.ShowWindow(SW_HIDE);		
		m_ZeitBtn2.ShowWindow(SW_HIDE);

		//Create the ToolTip control
		if( !m_ToolTip.Create(this))
		{
			 TRACE0("Unable to create the ToolTip!");
		}
		else
		{
	  		m_ToolTip.AddTool( &m_KeineBtn, (CString)_T("jetzt keine Datensicherung machen und in ") + (*m_npNachfrageIntervall == 7 ? _T("einer Woche") : _T("einem Monat")) + _T(" noch einmal nachfragen"));
			m_ToolTip.AddTool( &m_InternBtn, _T("einfachste Option mit nur einem Klick -- aber Festplatten gehen manchmal kaputt..."));
			m_ToolTip.AddTool( &m_ExternBtn, _T("speichern auf externem Laufwerk, falls die interne Festplatte mal den Geist aufgibt"));
			m_ToolTip.AddTool( &m_DiskBtn, _T("Das Archivieren auf einer nur einmal beschreibbaren Disk entspricht am ehesten den 'Grundsätzen ordnungsmäßiger DV-gestützter Buchführungssystemen' (GoBS)"));

			m_ToolTip.Activate(TRUE);
		}     
	}

	Nachfrageintervall();
	bNachfrageIntervallGeaendert = TRUE;

	return TRUE;
}

void CDatensicherungsoptionen::OnBnClickedKeine()
{
	CDialog::EndDialog(0);
}

void CDatensicherungsoptionen::OnBnClickedIntern()
{
	CDialog::EndDialog(1);
}

void CDatensicherungsoptionen::OnBnClickedExtern()
{
	CDialog::EndDialog(2);
}

void CDatensicherungsoptionen::OnBnClickedDisk()
{
	CDialog::EndDialog(3);
}

void CDatensicherungsoptionen::OnBnClickedZeit()
{
	if (*m_npNachfrageIntervall == 7) 
		*m_npNachfrageIntervall = 30;
	else
		*m_npNachfrageIntervall = 7;
	Nachfrageintervall();
}

void CDatensicherungsoptionen::OnBnClickedCancel()
{
	CDialog::EndDialog(5);
}

void CDatensicherungsoptionen::Nachfrageintervall()
{
	if (*m_npNachfrageIntervall == 7)
	{
		m_ZeitBtn.SetWindowText(_T("Bitte nur noch jeden Monat mit dieser Nachfrage nerven"));
		m_ZeitBtn2.SetWindowText(_T("Bitte nur noch jeden Monat mit dieser Nachfrage nerven"));
	}
	else
	{
		m_ZeitBtn.SetWindowText(_T("Bitte erinnere mich wöchentlich statt monatlich ans Backup"));
		m_ZeitBtn2.SetWindowText(_T("Bitte erinnere mich wöchentlich statt monatlich ans Backup"));
	}

	m_IntervallStatic.SetWindowText((CString)(*m_npNachfrageIntervall == 7 ? _T("wöchentlich") : _T("monatlich")) + (CString)(bNachfrageIntervallGeaendert ? _T(" (geändert, jetzt bitte eine der ersten vier Optionen wählen)") : ""));
}
