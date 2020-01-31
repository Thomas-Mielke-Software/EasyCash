// VoranmeldezeitraumDlg.cpp : implementation file
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
#include "ECTIFace\easycashdoc.h"
#include "easycashview.h"
#include "VoranmeldezeitraumDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VoranmeldezeitraumDlg dialog


VoranmeldezeitraumDlg::VoranmeldezeitraumDlg(CEasyCashView* pParent)
	: CDialog(VoranmeldezeitraumDlg::IDD, pParent)
{
	m_pParent = pParent;
	
	//{{AFX_DATA_INIT(VoranmeldezeitraumDlg)
	//}}AFX_DATA_INIT
}


void VoranmeldezeitraumDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VoranmeldezeitraumDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VoranmeldezeitraumDlg, CDialog)
	//{{AFX_MSG_MAP(VoranmeldezeitraumDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VoranmeldezeitraumDlg message handlers

BOOL VoranmeldezeitraumDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	char buf[100];
	int mm, jj;
	
	CTime now = CTime::GetCurrentTime();

	if (m_pParent->einstellungen1->m_monatliche_voranmeldung == 1)
	{ // Quartalsweise
		jj = m_pParent->GetDocument()->nJahr;
		if (now >= CTime(jj+1, 1, 1, 0, 0, 0)) // schon neues Jahr? dann letztes Quartal voreinstellen
			((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(1);
		else if (now >= CTime(jj, 10, 1, 0, 0, 0))
			((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(1);
		else if (now >= CTime(jj, 7, 1, 0, 0, 0))
			((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(1);
		else if (now >= CTime(jj, 4, 1, 0, 0, 0))
			((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(1);

		GetDlgItem(IDC_QUARTALMONAT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATUM_BIS_MONAT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATUM_BIS_JAHR)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PUNKT)->ShowWindow(SW_HIDE);
	}
	else
	{ // Monatlich
		now -= CTimeSpan(28, 0, 0, 0);	
		mm = now.GetMonth();
		jj = now.GetYear();
		GetDlgItem(IDC_RADIO1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO4)->ShowWindow(SW_HIDE);
	}
	
	((CButton *)GetDlgItem(IDC_MERKEN))->SetCheck(m_bVorauszahlungssollMerken);

	if (m_pParent->GetDocument()->nJahr !=  jj) { mm = 12; jj = m_pParent->GetDocument()->nJahr; }

	sprintf(buf, "%d", (int)mm);
	SetDlgItemText(IDC_DATUM_BIS_MONAT, buf);
	sprintf(buf, "%d", (int)jj);
	SetDlgItemText(IDC_DATUM_BIS_JAHR, buf);
	
	Button1Bmp.LoadBitmap(IDB_BUTTON1);
	((CButton *)GetDlgItem(IDOK))->SetBitmap(Button1Bmp);
	StopBmp.LoadBitmap(IDB_STOP);
	((CButton *)GetDlgItem(IDCANCEL))->SetBitmap(StopBmp);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void VoranmeldezeitraumDlg::OnOK() 
{
	int mv, jv, mb, jb;
	char buf[300];

	if (m_pParent->einstellungen1->m_monatliche_voranmeldung == 1)	// Quartal!
	{
		// ausführen für welches Quartal
		if (((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck()) mv = 1;
		else if (((CButton *)GetDlgItem(IDC_RADIO2))->GetCheck()) mv = 4;
		else if (((CButton *)GetDlgItem(IDC_RADIO3))->GetCheck()) mv = 7;
		else mv = 10;

		jv = jb = m_pParent->GetDocument()->nJahr;
		mb = mv + 2;	// insgesamt 3 Monate erfassen!
		if (mb > 12)
		{
			mb -= 12;
			jb++;
		}
	}
	else
	{
		// ausführen bis zu welchem datum ?
		GetDlgItemText(IDC_DATUM_BIS_MONAT, buf, sizeof(buf)); mb = atoi(buf);
		GetDlgItemText(IDC_DATUM_BIS_JAHR, buf, sizeof(buf)); jb = atoi(buf);
		if (jb < 100 && jb > 37)
		{
			jb += 1900;
		}
		else if (jb <= 37 && jb >= 0)
		{
			jb += 2000;
		}
		jv = jb;
		mv = mb;
	}

	CTime *temp_datum = new CTime(jb, mb, 31, 0, 0, 0);
	if (temp_datum->GetDay() != 31) 
	{ 
		delete temp_datum; 
		temp_datum = new CTime(jb, mb, 30, 0, 0, 0); 
	
		if (temp_datum->GetDay() != 30) 
		{ 
			delete temp_datum; 
			temp_datum = new CTime(jb, mb, 29, 0, 0, 0); 
			
			if (temp_datum->GetDay() != 29) 
			{ 
				delete temp_datum; 
				temp_datum = new CTime(jb, mb, 28, 0, 0, 0); 
			}
		}
	}
	CTime von_datum = CTime(jv, mv, 1, 0, 0, 0);
	CTime bis_datum = *temp_datum;
	delete temp_datum; 

	if (m_pParent->einstellungen1->m_monatliche_voranmeldung == 1)
	{
		if (mv != 1 && mv != 4  && mv != 7 && mv != 10)
		{
			AfxMessageBox("Bei quartalsmäßiger Vorauszahlung muß der Anfangsmonat des Quartals angegeben werden!", MB_ICONSTOP);
			return;
		}
	}
	
	m_von_datum = von_datum;
	m_bis_datum = bis_datum;
	m_bVorauszahlungssollMerken = ((CButton *)GetDlgItem(IDC_MERKEN))->GetCheck();

	CDialog::OnOK();
}
