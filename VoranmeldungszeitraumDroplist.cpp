// VoranmeldungszeitraumDroplist.cpp : implementation file
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
#include "ECTIFace\EasyCashDoc.h"
#include "VoranmeldungszeitraumDroplist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVoranmeldungszeitraumDroplist dialog


CVoranmeldungszeitraumDroplist::CVoranmeldungszeitraumDroplist(CWnd* pParent /*=NULL*/)
	: CDialog(CVoranmeldungszeitraumDroplist::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVoranmeldungszeitraumDroplist)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVoranmeldungszeitraumDroplist::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVoranmeldungszeitraumDroplist)
	DDX_Control(pDX, IDC_COMBO, m_combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVoranmeldungszeitraumDroplist, CDialog)
	//{{AFX_MSG_MAP(CVoranmeldungszeitraumDroplist)
	ON_CBN_SELCHANGE(IDC_COMBO, OnSelchangeCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVoranmeldungszeitraumDroplist message handlers

BOOL CVoranmeldungszeitraumDroplist::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_combo.ResetContent();
	if (m_monatliche_voranmeldung == 1)		// Quartal
	{
		m_combo.AddString("1. Quartal");
		m_combo.AddString("2. Quartal");
		m_combo.AddString("3. Quartal");
		m_combo.AddString("4. Quartal");
	}
	else	// Monat
	{	
		m_combo.AddString("Januar");
		m_combo.AddString("Februar");
		m_combo.AddString("März");
		m_combo.AddString("April");
		m_combo.AddString("Mai");
		m_combo.AddString("Juni");
		m_combo.AddString("Juli");
		m_combo.AddString("August");
		m_combo.AddString("September");
		m_combo.AddString("Oktober");
		m_combo.AddString("November");
		m_combo.AddString("Dezember");
	}
	
	return TRUE;  
}

void CVoranmeldungszeitraumDroplist::OnSelchangeCombo() 
{
	int nZeitraum = m_combo.GetCurSel();
	if (nZeitraum != CB_ERR)
	{
		if (m_monatliche_voranmeldung == 1)		// Quartal
			nZeitraum += 12;	// Quartale haben die IDs 12-15;
		m_pDoc->SetFormularVoranmeldungszeitraum(nZeitraum);	
	}
}
