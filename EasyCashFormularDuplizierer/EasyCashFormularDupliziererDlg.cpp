// EasyCashFormularDupliziererDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCashFormularDuplizierer.h"
#include "EasyCashFormularDupliziererDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEasyCashFormularDupliziererDlg dialog

CEasyCashFormularDupliziererDlg::CEasyCashFormularDupliziererDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEasyCashFormularDupliziererDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEasyCashFormularDupliziererDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEasyCashFormularDupliziererDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEasyCashFormularDupliziererDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEasyCashFormularDupliziererDlg, CDialog)
	//{{AFX_MSG_MAP(CEasyCashFormularDupliziererDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyCashFormularDupliziererDlg message handlers

BOOL GetIniFileName(char *buffer3, int size)
{
	char *cp;
	extern CEasyCashFormularDupliziererApp theApp;

	DWORD dw = GetModuleFileName(theApp.m_hInstance, buffer3, size);
	//for (cp = buffer3; *cp; cp++)
	//	*cp = tolower(*cp);
	if (cp = strrchr(buffer3, '\\'))
	{
		strcpy(cp, "\\easyct.ini");
		return TRUE;
	}
	else
		return FALSE;
}
static BOOL GetProgrammverzeichnis(char *buffer3, int size)
{
	char *cp;
	if (GetIniFileName(buffer3, size) && (cp = strrchr(buffer3, '\\')))
	{
		*cp = '\0';
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CEasyCashFormularDupliziererDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CTime now = CTime::GetCurrentTime();
	CString s;
	char buffer[10000];
	*buffer = '\0';
	GetProgrammverzeichnis(buffer, sizeof(buffer));
	s.Format("%s\\uva-d-%-0.0d-01.ecf", buffer, now.GetYear());
	SetDlgItemText(IDC_DATEI, s);
	SetDlgItemText(IDC_SPALTE2, "250");
	SetDlgItemText(IDC_SPALTE3, "393");
	SetDlgItemText(IDC_SPALTE4, "632");
	SetDlgItemText(IDC_VERTIKAL_OFFSET, "41");
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEasyCashFormularDupliziererDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEasyCashFormularDupliziererDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEasyCashFormularDupliziererDlg::OnOK() 
{
	CString s;
	GetDlgItemText(IDC_DATEI, s);

	// öffnen
	CFile f;
	if (!f.Open(s, CFile::modeRead)) 
	{
		AfxMessageBox("Konnte Datei nicht zum Lesen öffnen");
		return;
	}

	// einlesen original Januar-Datei und schließen
	CString original;
	f.Read(original.GetBuffer(f.GetLength()+1), f.GetLength());
	original.GetBuffer(f.GetLength()+1)[f.GetLength()] = '\0';
	original.ReleaseBuffer();
	f.Close();

	// Krezposition-Parameter aus Dialog holen
	CString temp;
	GetDlgItemText(IDC_SPALTE2, temp);
	int spalte2 = atoi(temp);
	GetDlgItemText(IDC_SPALTE3, temp);
	int spalte3 = atoi(temp);
	GetDlgItemText(IDC_SPALTE4, temp);
	int spalte4 = atoi(temp);
	GetDlgItemText(IDC_VERTIKAL_OFFSET, temp);
	int vertikal_offset = atoi(temp);
	
	// Februar- bis 4. Quartal Dateien generieren
	int nDauerfristErsetzt = 0;
	for (int i = 2; i <= 16; i++)
	{
		CString kopie = original;
		CString replace;

		// Voranmeldungszeitraum Nummer ersetzen
		replace.Format("voranmeldungszeitraum=\"%d\"", i);
		kopie.Replace("voranmeldungszeitraum=\"1\"", replace);

		// Zeitraum im Namen ersetzen
		switch (i)
		{
			case 1: replace = "Januar"; break;
			case 2: replace = "Februar"; break;
			case 3: replace = "März"; break;
			case 4: replace = "April"; break;
			case 5: replace = "Mai"; break;
			case 6: replace = "Juni"; break;
			case 7: replace = "Juli"; break;
			case 8: replace = "August"; break;
			case 9: replace = "September"; break;
			case 10: replace = "Oktober"; break;
			case 11: replace = "November"; break;
			case 12: replace = "Dezember"; break;
			case 13: replace = "1. Quartal"; break;
			case 14: replace = "2. Quartal"; break;
			case 15: replace = "3. Quartal"; break;
			case 16: replace = "4. Quartal"; break;
		}	
		CString voranmeldungszeitraum_string = replace;
		kopie.Replace(" Januar ", " " + replace + " ");

		// Kreuzchen verändern
		// Voranmeldungszeitraum Kreuzchen
		replace = "<feld id=\"9\" typ=\"Freitext\" seite=\"1\" horizontal=\"";
		int position = kopie.Find(replace, 0);
		if (position >= 0)
		{	
			position += replace.GetLength();
			if (i > 4) // andere spalte als erste?
			{
				if (i > 12)
					replace.Format("%-03.3d", spalte4);	// 4. Spalte (Quartale)
				else if (i > 8) 
					replace.Format("%-03.3d", spalte3);	// 3. Spalte
				else
					replace.Format("%-03.3d", spalte2);		// 2. Spalte
				kopie.GetBuffer(0)[position] = replace[0];
				kopie.GetBuffer(0)[position+1] = replace[1];
				kopie.GetBuffer(0)[position+2] = replace[2];
			}

			replace = "\" vertikal=\"";
			int position_vertikal = kopie.Find(replace, position);
			position_vertikal += replace.GetLength();
			if (position_vertikal > 0)
			{
				int position_feldende = kopie.Find("</feld>", position);

				if (position_feldende >= position && position_vertikal < position_feldende)	// noch im <feld> tag?
				{
					int vertikal = atoi(kopie.Mid(position_vertikal, 3));
					vertikal += vertikal_offset * ((i-1) % 4);
					replace.Format("%-03.3d", vertikal);
					kopie.GetBuffer(0)[position_vertikal] = replace[0];
					kopie.GetBuffer(0)[position_vertikal+1] = replace[1];
					kopie.GetBuffer(0)[position_vertikal+2] = replace[2];
				}
			}
		}
		else
		{
			if (i <= 12)
			{
				kopie.Replace("<name>Monat</name><erweiterung>Januar", "<name>Monat</name><erweiterung>" + voranmeldungszeitraum_string); // Januar ersetzen mit aktuellem Monat
			}
			else
			{
				kopie.Replace("<name>Quartal</name><erweiterung>", "<name>Quartal</name><erweiterung>" + voranmeldungszeitraum_string); // Quartal einsetzen
				kopie.Replace("<name>Monat</name><erweiterung>Januar", "<name>Monat</name><erweiterung>"); // "Januar" löschen
			}
		}

		// Dauerfristverlängerung-Feld einbauen im Dezember/4. Quartal
		if (i == 12 || i == 16)
		{
			char *suchdauerfrist = "<name>Anrechnung (Abzug) der festgesetzten Sondervorauszahlung für Dauerfristverlängerung -- für Dezember/4.Quartal bitte in Erweiterung eintragen: [Dauerfristverlängerung]Sondervorauszahlung$J</name><erweiterung/>";
			char *ersetzdauerfrist = "<name>Anrechnung (Abzug) der festgesetzten Sondervorauszahlung für Dauerfristverlängerung</name><erweiterung>[Dauerfristverlängerung]Sondervorauszahlung$J</erweiterung>";
			if (kopie.Find(suchdauerfrist) >= 0)
			{
				kopie.Replace(suchdauerfrist, ersetzdauerfrist);
				CString Zeitraum = (i == 12 ? "Dezember" : "4. Quartal");
				AfxMessageBox("Dauerfristfeld für " + Zeitraum + " ersetzt");
				nDauerfristErsetzt++;
			}
		}
		
		// Dauerfristverlängerung-Feld einbauen im Dezember/4. Quartal --- das gleiche für formatiertes XML
		if (i == 12 || i == 16)
		{
			char *suchdauerfrist = "<name>Anrechnung (Abzug) der festgesetzten Sondervorauszahlung für Dauerfristverlängerung -- für Dezember/4.Quartal bitte in Erweiterung eintragen: [Dauerfristverlängerung]Sondervorauszahlung$J</name>\r\n\t\t\t<erweiterung/>";
			char *ersetzdauerfrist = "<name>Anrechnung (Abzug) der festgesetzten Sondervorauszahlung für Dauerfristverlängerung</name>\r\n\t\t\t<erweiterung>[Dauerfristverlängerung]Sondervorauszahlung$J</erweiterung>";
			if (kopie.Find(suchdauerfrist) >= 0)
			{
				kopie.Replace(suchdauerfrist, ersetzdauerfrist);
				CString Zeitraum = (i == 12 ? "Dezember" : "4. Quartal");
				AfxMessageBox("Dauerfristfeld für " + Zeitraum + " ersetzt");
				nDauerfristErsetzt++;
			}
		}
		
		// Dauerfristverlängerung-Feld einbauen im Dezember/4. Quartal --- das gleiche für formatiertes XML und Leerzeichen statt Tabs und nur \n als Zeilentrenner
		if (i == 12 || i == 16)
		{
			char *suchdauerfrist = "<name>Anrechnung (Abzug) der festgesetzten Sondervorauszahlung für Dauerfristverlängerung -- für Dezember/4.Quartal bitte in Erweiterung eintragen: [Dauerfristverlängerung]Sondervorauszahlung$J</name>\n      <erweiterung/>";
			char *ersetzdauerfrist = "<name>Anrechnung (Abzug) der festgesetzten Sondervorauszahlung für Dauerfristverlängerung</name>\n      <erweiterung>[Dauerfristverlängerung]Sondervorauszahlung$J</erweiterung>";
			if (kopie.Find(suchdauerfrist) >= 0)
			{
				kopie.Replace(suchdauerfrist, ersetzdauerfrist);
				CString Zeitraum = (i == 12 ? "Dezember" : "4. Quartal");
				AfxMessageBox("Dauerfristfeld für " + Zeitraum + " ersetzt");
				nDauerfristErsetzt++;
			}
		}

		GetDlgItemText(IDC_DATEI, s);
		if (i <= 12)
			replace.Format("-%-02.2d.ecf", i);
		else
			replace.Format("-q%-1.1d.ecf", i-12);
		s.Replace("-01.ecf", replace);
		if (!f.Open(s, CFile::modeWrite|CFile::modeCreate)) 
		{
			AfxMessageBox("Konnte Datei nicht zum Schreiben öffnen");
			break;
		}
		f.Write(kopie.GetBuffer(kopie.GetLength()+1), kopie.GetLength());
		f.Close();
	}	

	if (nDauerfristErsetzt != 2)
	{
		AfxMessageBox("Dauerfristfeld-Ersetzen hat nicht funktioniert! Probier mal Notepad++ -> Plugins -> XML Tools -> Pretty Print.");
	}

	CDialog::OnOK();
}
