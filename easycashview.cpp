// EasyCashView.cpp : implementation of the CEasyCashView class
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
#include "EasyCash.h"
#include "ExtSplitter.h"
#include "ECTIFace\EasyCashDoc.h"
#include "BuchenDlg.h"
#include "DauerbuchungenDlg.h"
#include "DauBuchAusfuehren.h"
#include "EasyCashView.h"
#include "ChildFrm.h"
#include "AFXPRIV.H"
#include "DruckauswahlDlg.h"
#include "VoranmeldezeitraumDlg.h"
#include "MainFrm.h"
#include "ECTIFace\AfAGenauigkeit.h"
#include "oleidl.h"
#include "comdef.h"
#include "NeuesFormular.h"
#include "Formularabschnitt.h"
#include "XFolderDialog.h"
#include "IconAuswahlBestandskonto.h"
#include "IconAuswahlBetrieb.h"
#include "Shlobj.h"
#include "DatenverzeichnisDlg.h"
#include "UstVorauszahlungenDlg.h"

//#if defined(NDEBUG)
//#include "CrashRpt.h"
//#endif

#include <atlbase.h>
#include <string>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEasyCashView

IMPLEMENT_DYNCREATE(CEasyCashView, CScrollView)

BEGIN_MESSAGE_MAP(CEasyCashView, CScrollView)
	//{{AFX_MSG_MAP(CEasyCashView)
	ON_COMMAND(ID_EDIT_EINNAHME_BUCHEN, OnEditEinnahmeBuchen)
	ON_COMMAND(ID_EDIT_AUSGABE_BUCHEN, OnEditAusgabeBuchen)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_DAUERBUCHUNGEN_EINGEBEN, OnEditDauerbuchungenEingeben)
	ON_COMMAND(ID_EDIT_DAUERBUCHUNGEN_AUSFUEHREN, OnEditDauerbuchungenAusfuehren)
	ON_COMMAND(ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, OnEditUmsatzsteuervorauszahlungen)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_FILE_BACKUP_SUBFOLDER, OnFileBackupSubfolder)
	ON_COMMAND(ID_FILE_BACKUP_EXTERNAL, OnFileBackupExternal)
	ON_COMMAND(ID_FILE_BACKUP, OnFileBackup)
	ON_COMMAND(ID_FILE_JAHRESWECHSEL, OnFileJahreswechsel)
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_VIEW_JOURNAL_DATUM, OnViewJournalDatum)
	ON_COMMAND(ID_VIEW_JOURNAL_KONTEN, OnViewJournalKonten)
	ON_COMMAND(ID_VIEW_JOURNAL_SWITCH, OnViewJournalSwitch)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_FORMULAR_INFO, OnFormularInfo)
	ON_COMMAND(ID_FORMULAR_NEU, OnFormularNeu)
	ON_COMMAND(ID_FORMULAR_MENUUPDATE, OnFormularMenuUpdate)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_NEXT, OnFindNext)
	ON_COMMAND(ID_PREV, OnFindPrev)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_VIEW_FINDTOOLBAR, OnViewFindtoolbar)
	ON_COMMAND(ID_VIEW_JOURNAL_BESTANDSKONTO, OnViewJournalBestandskonto)
	ON_COMMAND(ID_VIEW_JOURNAL_BETRIEB, OnViewJournalBetrieb)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_VIEW_JOURNAL_BESTKONTEN, OnViewJournalBestkonten)
	ON_COMMAND(ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, OnViewJournalAnlagenverzeichnis)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT2, OnFilePrint2)
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	//ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_BUCHEN_DAUAUS_JANUAR, &CEasyCashView::OnBuchenDauausJanuar)
	ON_COMMAND(ID_BUCHEN_DAUAUS_FEBRUAR, &CEasyCashView::OnBuchenDauausFebruar)
	ON_COMMAND(ID_BUCHEN_DAUAUS_MAERZ, &CEasyCashView::OnBuchenDauausMaerz)
	ON_COMMAND(ID_BUCHEN_DAUAUS_APRIL, &CEasyCashView::OnBuchenDauausApril)
	ON_COMMAND(ID_BUCHEN_DAUAUS_MAI, &CEasyCashView::OnBuchenDauausMai)
	ON_COMMAND(ID_BUCHEN_DAUAUS_JUNI, &CEasyCashView::OnBuchenDauausJuni)
	ON_COMMAND(ID_BUCHEN_DAUAUS_JULI, &CEasyCashView::OnBuchenDauausJuli)
	ON_COMMAND(ID_BUCHEN_DAUAUS_AUGUST, &CEasyCashView::OnBuchenDauausAugust)
	ON_COMMAND(ID_BUCHEN_DAUAUS_SEPTEMBER, &CEasyCashView::OnBuchenDauausSeptember)
	ON_COMMAND(ID_BUCHEN_DAUAUS_OKTOBER, &CEasyCashView::OnBuchenDauausOktober)
	ON_COMMAND(ID_BUCHEN_DAUAUS_NOVEMBER, &CEasyCashView::OnBuchenDauausNovember)
	ON_COMMAND(ID_BUCHEN_DAUAUS_DEZEMBER, &CEasyCashView::OnBuchenDauausDezember)
	ON_COMMAND(ID_VIEW_JOURNAL_MONAT, &CEasyCashView::OnViewJournalMonat)
	ON_COMMAND(ID_VIEW_JOURNAL_KONTO, &CEasyCashView::OnViewJournalKonto)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_ZOOM_SWITCH, &CEasyCashView::OnViewZoomSwitch)
	ON_COMMAND(ID_ZOOMFAKTOR_VERGROESSERN, &CEasyCashView::OnZoomfaktorVergroessern)
	ON_COMMAND(ID_ZOOMFAKTOR_VERKLEINERN, &CEasyCashView::OnZoomfaktorVerkleinern)
	ON_COMMAND(ID_ZOOMFAKTOR_50, &CEasyCashView::OnZoomfaktor50)
	ON_COMMAND(ID_ZOOMFAKTOR_75, &CEasyCashView::OnZoomfaktor75)
	ON_COMMAND(ID_ZOOMFAKTOR_100, &CEasyCashView::OnZoomfaktor100)
	ON_COMMAND(ID_ZOOMFAKTOR_125, &CEasyCashView::OnZoomfaktor125)
	ON_COMMAND(ID_ZOOMFAKTOR_150, &CEasyCashView::OnZoomfaktor150)
	ON_COMMAND(ID_ZOOMFAKTOR_175, &CEasyCashView::OnZoomfaktor175)
	ON_COMMAND(ID_ZOOMFAKTOR_200, &CEasyCashView::OnZoomfaktor200)
	ON_COMMAND(ID_ZOOMFAKTOR_250, &CEasyCashView::OnZoomfaktor250)
	ON_COMMAND(ID_ZOOMFAKTOR_300, &CEasyCashView::OnZoomfaktor300)
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyCashView construction/destruction

static char *cpMonat[] = { "Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember" };

CEasyCashView::CEasyCashView()
{
	buchenDlg = NULL;
	dauerbuchungenDlg = NULL;
	pBuchungAendernDlg = NULL;
	pFormularfeldDlg = NULL;
	pPluginWnd = NULL;
	m_pNavigationWnd = NULL;
	nSelected = -1;
	ptFeldmarke.x = 0;
	ptFeldmarke.y = 0;
	m_nFeldMove = -1;
	max_seitenzahl = 1;
	m_zoomfaktor = 100;
	m_vt = 1;
	m_vm = 1;
	m_bt = 31;
	m_bm = 12;
	m_bAfaKorrketuren = 0;

	letzte_zeile = 0;

	int i;
	for (i = 0; i < MAX_BUCHUNGEN; i++)
		ppPosBuchungsliste[i] = NULL;

	propdlg = NULL;
	einstellungen1 = NULL;
	einstellungen2 = NULL;
	einstellungen3 = NULL;
	einstellungen4 = NULL;
	einstellungen5 = NULL;

	// ini file
	char buffer[1000];
	extern CEasyCashApp theApp;
	char EasyCashIniFilenameBuffer[1000];
	if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return; }
	GetPrivateProfileString("Allgemein", "Anzeige", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	m_nAnzeige = atoi(buffer);

	m_MonatsFilterDisplay = 0;
	m_KontenFilterDisplay = "<alle Konten>";
	m_KontenFilterPrinter = "<alle Konten>";

	ZeroMemory(&m_osvi, sizeof(OSVERSIONINFOEX));
	m_osvi.dwOSVersionInfoSize = sizeof(m_osvi);
	GetVersionEx((OSVERSIONINFO *)&m_osvi);

	m_GewaehltesFormular = -1;
	m_bFormularfelderAnzeigen = FALSE;
	for (i = 0; i < FORMULARSEITENCACHESIZE; i++)
		m_pFormularseitenImageCache[i] = NULL;	
}

CEasyCashView::~CEasyCashView()
{
	if (buchenDlg) delete buchenDlg;
	if (dauerbuchungenDlg) delete dauerbuchungenDlg;
	DestroyPlugin();
	if (pFormularfeldDlg) delete pFormularfeldDlg;
	
	if (propdlg) delete propdlg;
	if (einstellungen1) delete einstellungen1;
	if (einstellungen2) delete einstellungen2;
	if (einstellungen3) delete einstellungen3;
	if (einstellungen4) delete einstellungen4;
	if (einstellungen5) delete einstellungen5;

	// ini file
	char buffer[1000];
	extern CEasyCashApp theApp;
	char EasyCashIniFilenameBuffer[1000];
	if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return; }
	sprintf(buffer, "%d", (int)m_nAnzeige);
	WritePrivateProfileString("Allgemein", "Anzeige", (LPCSTR)buffer, EasyCashIniFilenameBuffer);	
	int i;
	for (i = 0; i < FORMULARSEITENCACHESIZE; i++)
		if (m_pFormularseitenImageCache[i])
			delete m_pFormularseitenImageCache[i];	
}

BOOL CEasyCashView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}


BOOL CEasyCashView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYUP && (UINT)pMsg->wParam == VK_RETURN)	
	{
		CString csSuchtext = ((CMainFrame*)AfxGetMainWnd())->m_pSucheCombobox->GetEditText();
		if (csSuchtext != "")
			OnFindNext();
	}

	return CScrollView::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CEasyCashView drawing


void CEasyCashView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	int nMandant;
	if ((nMandant = GetMandant()) >= 0)
	{
		CString csKey, csMandantName;
		csKey.Format("Mandant%-02.2dName", nMandant);
		csMandantName = theApp.GetProfileString("Mandanten", csKey, "");
		if (!csMandantName.IsEmpty()) GetParent()->SetWindowText(GetDocument()->GetTitle() + " / " + csMandantName);
	}
	else
		GetParent()->SetWindowText(GetDocument()->GetTitle());

	m_zoomfaktor = m_wunschzoomfaktor = theApp.GetProfileInt("Fenster", "JournalZoomFaktor", 100);
	m_timeLetzteZwangsverkleinerung = CTime::GetCurrentTime() - CTimeSpan(1);
	SetupScroll();

	// Popup-Menü aufbauen
	PopUp.CreatePopupMenu();
	PopUp.AppendMenu(MF_STRING, POPUP_AENDERN, "Buchung &ändern");
	PopUp.AppendMenu(MF_STRING, POPUP_LOESCHEN, "Buchung &löschen");
	PopUp.AppendMenu(MF_STRING, POPUP_KOPIEREN, "Buchung &kopieren");
	PopUp.AppendMenu(MF_STRING, POPUP_KOPIEREN_BELEGNUMMER, "Buchung kopieren mit neuer &Belegnummer");	
	PopUp.AppendMenu(MF_STRING, POPUP_AFA_ABGANG, "Anlagengut &ausscheiden lassen");	

	// Popup-Menü aufbauen
	PopUpFormular.CreatePopupMenu();
	PopUpFormular.AppendMenu(MF_STRING, POPUPFORMULAR_NEUES_FELD, "&Neues Feld hier erzeugen");
	PopUpFormular.AppendMenu(MF_STRING, POPUPFORMULAR_NEUER_ABSCHNITT, "Neuen Ab&schnitt hier erzeugen");
	PopUpFormular.AppendMenu(MF_STRING, POPUPFORMULAR_FELDER_BEARBEITEN, "&Feld bearbeiten");
	//PopUpFormular.AppendMenu(MF_STRING|MF_GRAYED, POPUPFORMULAR_KALKULATION_BEARBEITEN, "&Kalkulation bearbeiten");
	PopUpFormular.AppendMenu(MF_STRING, POPUPFORMULAR_FOLMULARDATEI_OEFFNEN, "Formulardatei im &Editor öffnen");
	PopUpFormular.AppendMenu(MF_STRING, POPUPFORMULAR_FELDER_ANZEIGEN, "Felder &anzeigen/ausblenden");
	PopUpFormular.AppendMenu(MF_STRING, POPUPFORMULAR_FELDWERT_KOPIEREN, "Feldwert in die &Zwischenablage kopieren");	

	propdlg = new CPropertySheet("EasyCash Einstellungen", this);
	// propdlg->EnableDynamicLayout(TRUE); Sheet-Fenster vergrößert sich bei jedem DoModal() :(
	einstellungen1 = new CEinstellungen1();
	einstellungen1->m_pDoc = GetDocument();
	einstellungen1->m_pView = this;
	einstellungen2 = new CEinstellungen2();
	einstellungen3 = new CEinstellungen3();
	einstellungen4 = new CEinstellungen4();
	einstellungen4->m_einstellungen1 = einstellungen1;
	einstellungen5 = new CEinstellungen5();
	einstellungen5->m_einstellungen1 = einstellungen1;
	propdlg->AddPage(einstellungen1);
	propdlg->AddPage(einstellungen2);
	propdlg->AddPage(einstellungen3);
	propdlg->AddPage(einstellungen4);
	propdlg->AddPage(einstellungen5);

	LoadProfile();
	// frisch installiert?
	if (einstellungen3->m_steuernummer == "")
	{
		CString Mandant0Existiert = theApp.GetProfileString("Mandanten", "Mandant00Datenverzeichnis", "");
		if (Mandant0Existiert.IsEmpty())
			AfxMessageBox("EasyCash wurde frisch installiert oder die Daten wurden aus dem Datenverzeichnis entfernt. Bitte geben Sie zunächst Ihre \
Daten für den Formulardruck ein (zumindest die Steuernummer, damit diese Meldung nicht mehr erscheint). \
Für den Fall, dass die Daten verschoben wurden, ändern Sie bitte das Datenverzeichnis entsprechend \
(im Applikationsmenü, der runde Knopf oben links, vorletzter Punkt im Menü: 'Wähle neues Datenverzeichnis aus')", MB_ICONEXCLAMATION);
		else
			AfxMessageBox("Das Datenverzeichnis für den Mandanten wurde neu angelegt oder die Daten wurden daraus entfernt. Bitte geben Sie zunächst seine \
Daten für den Formulardruck ein (zumindest die Steuernummer, damit diese Meldung nicht mehr erscheint). \
Für den Fall, dass die Daten verschoben wurden, ändern Sie bitte das Datenverzeichnis entsprechend \
(im Applikationsmenü, der runde Knopf oben links, vorletzter Punkt im Menü: 'Wähle neues Datenverzeichnis aus')", MB_ICONEXCLAMATION);
		OnViewOptions();
	}

	// Jahreswechsel fällig?
	CTime now = CTime::GetCurrentTime();
	if (now.GetYear() == GetDocument()->nJahr + 1)
		if (m_nJahreswechselAbfrage < now.GetYear())
		{
			if (AfxMessageBox("Ein neues Jahr ist angebrochen. Im Menü unter Datei->Jahreswechsel können Sie Abschreibungen und Dauerbuchungen in eine neue Jahres-Buchungsdatei übernehmen. Soll ich diesen Hinweis nächstes Jahr wieder anzeigen?", MB_YESNO) == IDYES)
				m_nJahreswechselAbfrage = now.GetYear();
			else
				m_nJahreswechselAbfrage = 3001;
			SaveProfile();
		}

	//RECT r;
	//GetParent()->GetParent()->GetClientRect(&r);
	//GetParent()->SetWindowPos(NULL, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOSIZE);
	GetParent()->ShowWindow(SW_SHOWMAXIMIZED);

//VS9
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(3))
		switch (m_nAnzeige)
		{
		case 0:	AfxGetMainWnd()->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_BESTANDSKONTEN, ID_VIEW_JOURNAL_DATUM, MF_BYCOMMAND); break;
		case 1:	AfxGetMainWnd()->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_BESTANDSKONTEN, ID_VIEW_JOURNAL_KONTEN, MF_BYCOMMAND); break;
		case 2:	AfxGetMainWnd()->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_BESTANDSKONTEN, ID_VIEW_JOURNAL_BESTANDSKONTEN, MF_BYCOMMAND); break;
		case 3:	AfxGetMainWnd()->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_BESTANDSKONTEN, ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, MF_BYCOMMAND); break;
		}

	// Formulare in Menü laden
	UpdateFormularMenu();

	m_cbmIcons.LoadBitmap(IDB_ICONS);
	m_tbiIcons.SetImageSize(CSize(32, 32));
	m_tbiIcons.Load(IDB_ICONS);
	m_cbmIconsBestandskonten.LoadBitmap(IDB_ICONS_BESTANDSKONTEN);
	m_tbiIconsBestandskonten.SetImageSize(CSize(32, 32));
	m_tbiIconsBestandskonten.Load(IDB_ICONS_BESTANDSKONTEN);	

	// neuer Ribbon-Kram für alten Plugin-Kram
	CMFCRibbonButton* pAnsichtPluginsButton;
	if (pAnsichtPluginsButton = ((CMainFrame*)AfxGetMainWnd())->m_pAnsichtPluginsButton)
	{
		ExtensionDLLsClass *pExtensionDLLs = ExtensionDLLs;

		while (pExtensionDLLs)
		{
			//pm->AppendMenu(MF_STRING, pExtensionDLLs->menu_item_id, pExtensionDLLs->menu_item_string);
			if (pAnsichtPluginsButton->FindSubItemIndexByID(pExtensionDLLs->menu_item_id) < 0)
				if (pAnsichtPluginsButton) pAnsichtPluginsButton->AddSubItem(new CMFCRibbonButton(pExtensionDLLs->menu_item_id, (LPCTSTR)pExtensionDLLs->menu_item_string, 25, 25));

			pExtensionDLLs = pExtensionDLLs->next;
		}
	}
	CIterateExtensionDLLs("ECTE_SetMainWindow", (void *)AfxGetMainWnd());

	UpdateBetriebeMenu();
	UpdateBestandskontenMenu();

	if (!theApp.m_csStartupPlugin.IsEmpty())
	{
		CPluginElement* pPluginDaten = ((CMainFrame*)AfxGetMainWnd())->m_pPlugins;
		while (pPluginDaten)
		{
			if (pPluginDaten->name.GetLength() >= theApp.m_csStartupPlugin.GetLength() && 
				pPluginDaten->name.Left(theApp.m_csStartupPlugin.GetLength()) == theApp.m_csStartupPlugin)
			{	// Finden wir ein Plugin bei dem die ersten Buchstaben des Startup-Plugin vom Cmd-Parameter passen?
				PostMessage(WM_COMMAND, pPluginDaten->id, 0L);
				break;
			}

			pPluginDaten = pPluginDaten->next;
		}

		theApp.m_csStartupPlugin = "";
	}

/*	// Dauertest Code
int iii;
for (iii = 0; iii < 50; iii++)
{
	SendMessage(WM_COMMAND, ID_CMD_PLUGIN_BASE+1, 0L);

    // Must call Create() before using the dialog
    ASSERT(m_hWnd!=NULL);

    MSG msg;
    // Handle dialog messages
	int jjj;
	for (jjj = 0; jjj < 100; jjj++)
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!::IsWindow(m_hWnd)) return;
		  if(!IsDialogMessage(&msg))
		  {
			  Sleep(1);
			TranslateMessage(&msg);
			DispatchMessage(&msg);  
		  }
		}

	SendMessage(WM_COMMAND, ID_FILE_SAVE, 0L);
	CString cs;
	cs.Format("Schleifendurchlauf %d", iii);
	((CMainFrame*)AfxGetMainWnd())->SetStatus(cs);

}
*/
/*
	// Must call Create() before using the dialog
    ASSERT(m_hWnd!=NULL);

    // Handle dialog messages
	for (jjj = 0; jjj < 100; jjj++)
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!::IsWindow(m_hWnd)) return;
		  if(!IsDialogMessage(&msg))
		  {
			  Sleep(1);
			TranslateMessage(&msg);
			DispatchMessage(&msg);  
		  }
		}
*/

/*
CEasyCashDoc* pDoc = GetDocument();

CBuchung *m_pBuchung = NULL;
int iii;
for (iii = 0; iii < 100; iii++)
{
	int jjj;
	for (jjj=0; jjj < pDoc->Ausgaben->Beschreibung.GetLength(); jjj++)
	{
		
//		if (m_pBuchung) 
//		{
//			delete m_pBuchung;
//		}
//		m_pBuchung = new CBuchung;
//
//		m_pBuchung->Datum = CTime::GetCurrentTime() + CTimeSpan((int)pDoc->Einnahmen->Beschreibung[jjj]);
//		m_pBuchung->Betrag = (int)pDoc->Einnahmen->Beschreibung[jjj]*100;
//		m_pBuchung->Beschreibung = (CString)pDoc->Einnahmen->Beschreibung[jjj];
//		m_pBuchung->Belegnummer = (CString)pDoc->Einnahmen->Beschreibung[jjj];
//		m_pBuchung->MWSt = 0;
//		m_pBuchung->AbschreibungRestwert = 0;
//		m_pBuchung->AbschreibungNr = 1;
//		m_pBuchung->AbschreibungJahre = 1;
//		m_pBuchung->AbschreibungDegressiv = FALSE;
//		m_pBuchung->AbschreibungSatz = 0;
//		
//		m_pBuchung->next = pDoc->Einnahmen;
//		pDoc->Einnahmen = m_pBuchung;
//		m_pBuchung = NULL;
		

		CBuchung **p;

		p = &(pDoc->Einnahmen);

		while (*p)
			p = &((*p)->next);

		*p = new CBuchung;
		(*p)->next = NULL;

		m_pBuchung = *p;
		m_pBuchung->Datum = CTime::GetCurrentTime() + CTimeSpan((int)pDoc->Ausgaben->Beschreibung[jjj]);
		m_pBuchung->Betrag = (int)pDoc->Ausgaben->Beschreibung[jjj]*100;
		m_pBuchung->Beschreibung = (CString)pDoc->Ausgaben->Beschreibung[jjj];
		m_pBuchung->Belegnummer = (CString)pDoc->Ausgaben->Beschreibung[jjj];
		m_pBuchung->MWSt = 0;
		m_pBuchung->AbschreibungRestwert = 0;
		m_pBuchung->AbschreibungNr = 1;
		m_pBuchung->AbschreibungJahre = 1;
		m_pBuchung->AbschreibungDegressiv = FALSE;
		m_pBuchung->AbschreibungSatz = 0;


		pDoc->SetModifiedFlag("Neue Einnahmenbuchung wurde eingefügt");
	}

	pDoc->Sort();

	SendMessage(WM_COMMAND, ID_FILE_SAVE, 0L);
}
SendMessage(WM_COMMAND, ID_FILE_SAVE, 0L);

*/

//SetTimer(0, 10000, NULL);
}

void CEasyCashView::UpdateFormularMenu()
{
	LadeECFormulare(m_csaFormulare);
	m_csaFormularnamen.RemoveAll();
	m_csaFormularfilter.RemoveAll();
	int nPos = 0;
	char inifile[1000], betriebe_existieren[1000]; 
	GetIniFileName(inifile, sizeof(inifile)); 
	GetPrivateProfileString("Betriebe", "Betrieb00Name", "", betriebe_existieren, sizeof(betriebe_existieren), inifile);
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(4))	//VS9
		while (AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->GetMenuItemCount())
			AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->DeleteMenu(0, MF_BYPOSITION);
	CArray<CMFCRibbonBaseElement* ,CMFCRibbonBaseElement*> arAnsichtFormulareButtons;
	CMFCRibbonBar *pRibbon = ((CMDIFrameWndEx*) AfxGetMainWnd())->GetRibbonBar();
	pRibbon->GetElementsByID(ID_ANSICHT_FORMULARE, arAnsichtFormulareButtons);
//	CMFCRibbonButton* pBtnAnsichtFormular;
	int i;
/*	for(i = 0; i < arAnsichtFormulareButtons.GetSize(); i++)
		if (pBtnAnsichtFormular = DYNAMIC_DOWNCAST(CMFCRibbonButton, arAnsichtFormulareButtons[i]))
			pBtnAnsichtFormular->RemoveAllSubItems();
*/	CMFCRibbonButton* pBtnAnsichtFormulare = ((CMainFrame*)AfxGetMainWnd())->m_pAnsichtFormulareButton;
	if (pBtnAnsichtFormulare)	// Dummy von Ribbon-Button Menü entfernen	// XXXXXXXXXXXXX
		pBtnAnsichtFormulare->RemoveAllSubItems();							// XXXXXXXXXXXXX
	CMFCRibbonButton* pArrBtnSubmenus[200];	// für das Gruppieren von Voranmeldungsformularen
	for (i = 0; i < 200; i++)
		pArrBtnSubmenus[i] = NULL;
	for (i = 0; i < m_csaFormulare.GetSize(); i++)
	{
		XDoc xmldoc;
		xmldoc.LoadFile(m_csaFormulare[i]);
		LPXNode xml = xmldoc.GetRoot();
		if (xml) 
		{
			LPCTSTR attr_name = xml->GetAttrValue("name");
			LPCTSTR attr_anzeigename = xml->GetAttrValue("anzeigename");
			LPCTSTR attr_filter = xml->GetAttrValue("filter");
//TRACE1("%s:\r\n", attr_anzeigename);
			if (attr_name) 
			{
				
				if (!attr_anzeigename) attr_anzeigename = attr_name;
				if (attr_filter && !stricmp(attr_filter, "betrieb") && *betriebe_existieren)
				{
					int iBetrieb = 0;
					char betrieb[1000];
					while (TRUE)
					{
						CString csKey;
						csKey.Format("Betrieb%02dName", iBetrieb);
						GetPrivateProfileString("Betriebe", csKey, "", betrieb, sizeof(betrieb), inifile);
						if (!*betrieb || iBetrieb > 100) { if (iBetrieb>0) i--; break; }
						CString csMenuText = (CString)attr_anzeigename + " für " + betrieb;
						if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(4)) //VS9
							AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->InsertMenu(nPos++, MF_BYPOSITION, ID_FORMULAR_BASE+i, csMenuText);
						if (pBtnAnsichtFormulare)	// Ribbon-Button Menü aufbauen
						{
							if (!strncmp(attr_anzeigename, "Umsatzsteuer-Voranmeldung ", 26) || !strncmp(attr_anzeigename, "U30 ", 4))	// Extrawurst für UST-VA: Submenüs für die entspr. Jahre
							{
								int nStringlength = !strncmp(attr_anzeigename, "U30 ", 4) ? 4 : 26;
								int nOesterreichOffset = nStringlength == 4 ? 100 : 0;
								int nJahr = atoi(attr_anzeigename + nStringlength) % 100;	// kleine Hashmap der Jahre, für die USt-VA-Formulare existieren
								if (!pArrBtnSubmenus[nJahr+nOesterreichOffset])
								{
									pArrBtnSubmenus[nJahr+nOesterreichOffset] = new CMFCRibbonButton(ID_ANSICHT_FORMULARE, (LPCTSTR)csMenuText.Left(nStringlength+4), 21);
									pBtnAnsichtFormulare->AddSubItem(pArrBtnSubmenus[nJahr+nOesterreichOffset]);
									pArrBtnSubmenus[nJahr+nOesterreichOffset]->SetMenu(IDR_ANSICHT_FORMULARE);
									pArrBtnSubmenus[nJahr+nOesterreichOffset]->RemoveAllSubItems();
								}
								CString csZeitraum = csMenuText.Mid(nStringlength+4);
								if (csZeitraum.Right(4) == " (D)") csZeitraum = csZeitraum.Mid(0, csZeitraum.GetLength()-4);
								if (csZeitraum.Right(5) == " (AT)") csZeitraum = csZeitraum.Mid(0, csZeitraum.GetLength()-5);
								pArrBtnSubmenus[nJahr+nOesterreichOffset]->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)csZeitraum));
							}
							else if (!strncmp(attr_anzeigename, "E/Ü-Rechnung ", 13))
								pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)csMenuText, 23));
							else if (!strncmp(attr_anzeigename, "Umsatzsteuererklärung ", 22))
								pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)csMenuText, 24));
							else 
								pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)csMenuText));
						}

						m_csaFormularnamen.Add(attr_name);
						m_csaFormularfilter.Add(betrieb);
						if (iBetrieb>0)
						{
							csKey = m_csaFormulare[i-1];
							m_csaFormulare.InsertAt(i, csKey);	// m_csaFormulare muss erweitert werden, wenn es mehr als einen Betrieb gibt!	
						}
						i++;
						iBetrieb++;
					} 
				}
				else
				{
					if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(4))	//VS9
						AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->InsertMenu(nPos++, MF_BYPOSITION, ID_FORMULAR_BASE+i, attr_anzeigename);
					if (pBtnAnsichtFormulare)	// Ribbon-Button Menü aufbauen
					{
						if (!strncmp(attr_anzeigename, "Umsatzsteuer-Voranmeldung ", 26) || !strncmp(attr_anzeigename, "U30 ", 4))
						{
							int nStringlength = !strncmp(attr_anzeigename, "U30 ", 4) ? 4 : 26;
							int nOesterreichOffset = nStringlength == 4 ? 100 : 0;
							int nJahr = atoi(attr_anzeigename + nStringlength) % 100;	// kleine Hashmap der Jahre, für die USt-VA-Formulare existieren
							if (!pArrBtnSubmenus[nJahr+nOesterreichOffset])
							{
								pArrBtnSubmenus[nJahr+nOesterreichOffset] = new CMFCRibbonButton(ID_ANSICHT_FORMULARE, (LPCTSTR)((CString)attr_anzeigename).Left(nStringlength+4), 21);
								pBtnAnsichtFormulare->AddSubItem(pArrBtnSubmenus[nJahr+nOesterreichOffset]);
								pArrBtnSubmenus[nJahr+nOesterreichOffset]->SetMenu(IDR_ANSICHT_FORMULARE);
								pArrBtnSubmenus[nJahr+nOesterreichOffset]->RemoveAllSubItems();
							}
							CString csZeitraum = attr_anzeigename + nStringlength+4;
							if (csZeitraum.Right(4) == " (D)") csZeitraum = csZeitraum.Mid(0, csZeitraum.GetLength()-4);
							if (csZeitraum.Right(5) == " (AT)") csZeitraum = csZeitraum.Mid(0, csZeitraum.GetLength()-5);
							pArrBtnSubmenus[nJahr+nOesterreichOffset]->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)csZeitraum));
						}
						else if (!strncmp(attr_anzeigename, "E/Ü-Rechnung ", 13))
							pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)attr_anzeigename, 23));
						else if (!strncmp(attr_anzeigename, "Umsatzsteuererklärung ", 22))
							pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)attr_anzeigename, 24));
						else 
							pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_BASE + i, (LPCTSTR)attr_anzeigename));
					}
					m_csaFormularnamen.Add(attr_anzeigename);
					m_csaFormularfilter.Add("");
				}
			}
			else
			{
				m_csaFormularnamen.Add("");
				m_csaFormularfilter.Add("");
			}
		}
		//int ii;
		//for (ii = 0; ii < m_csaFormularnamen.GetSize(); ii++)
		//	TRACE("%02d: %s // %s // %s\r\n", ii, (LPCTSTR)m_csaFormularnamen[ii], (LPCTSTR)m_csaFormularfilter[ii], (LPCTSTR)m_csaFormulare[ii]);
	}
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(4)) //VS9
		AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->InsertMenu(AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->GetMenuItemCount(), MF_BYPOSITION, ID_FORMULAR_NEU, "<neues Formular erzeugen>");
	else
	{
		if (pBtnAnsichtFormulare) pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_INFO, "<weitere Formulare ...>"));
		if (pBtnAnsichtFormulare) pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_NEU, "<eigenes Formular erzeugen>"));
		if (pBtnAnsichtFormulare) pBtnAnsichtFormulare->AddSubItem(new CMFCRibbonButton(ID_FORMULAR_MENUUPDATE, "<Formular-Menü aktualisieren>"));
	}

	ASSERT(m_csaFormulare.GetSize() == m_csaFormularnamen.GetSize());
	ASSERT(m_csaFormulare.GetSize() == m_csaFormularfilter.GetSize());
	TRACE3("m_csaFormulare: %d  m_csaFormularnamen: %d  m_csaFormularfilter: %d\r\n", m_csaFormulare.GetSize(), m_csaFormularnamen.GetSize(), m_csaFormularfilter.GetSize());
}

static int timerzaehler = 0;
void CEasyCashView::OnTimer(UINT_PTR nIDEvent)
{
	SendMessage(WM_COMMAND, ID_FILE_SAVE, 0L);
	SendMessage(WM_COMMAND, ID_CMD_PLUGIN_BASE+1, 0L);
	if (timerzaehler++ > 300) KillTimer(0);
	CString cs;
	cs.Format("Schleifendurchlauf %d", timerzaehler);
	((CMainFrame*)AfxGetMainWnd())->SetStatus(cs);

	CScrollView::OnTimer(nIDEvent);
}

void CEasyCashView::UpdateBetriebeMenu()
{
	// neuer Ribbon-Kram für Betriebe-Filter
	UpdateBetriebe();
	CMFCRibbonButton* m_pBtnFilterBetrieb;
	if (m_pBtnFilterBetrieb = ((CMainFrame*)AfxGetMainWnd())->m_pFilterBetriebButton)
	{
		m_pBtnFilterBetrieb->RemoveAllSubItems();

		std::auto_ptr<CMFCRibbonButtonEx> apBtn(new CMFCRibbonButtonEx(ID_BETRIEBEFILTER_BASE-1, "<alle Betriebe>", -1, -1, true));
		apBtn->SetToolTipText("Filter deaktivieren und wieder alle Betriebe im Journal anzeigen");
		m_pBtnFilterBetrieb->AddSubItem(apBtn.release());

		int i;
		for (i = 0; i < m_csaBetriebeNamen.GetSize(); i++)
		{
			std::auto_ptr<CMFCRibbonButtonEx> apBtn(new CMFCRibbonButtonEx(ID_BETRIEBEFILTER_BASE + i, (LPCTSTR)m_csaBetriebeNamen[i], 
														m_tbiIcons.ExtractIcon(atoi(m_csaBetriebeIcons[i])), true, NULL, true, true));
			apBtn->SetToolTipText(m_csaBetriebeUnternehmensarten[i]);
			m_pBtnFilterBetrieb->AddSubItem(apBtn.release());
		}
	}
}

void CEasyCashView::UpdateBestandskontenMenu()
{
	// neuer Ribbon-Kram für Bestandskonto-Filter		
	UpdateBestandskonten();
	CMFCRibbonButton* m_pBtnFilterBestandskonto;
	if (m_pBtnFilterBestandskonto = ((CMainFrame*)AfxGetMainWnd())->m_pFilterBestandskontoButton)
	{
		m_pBtnFilterBestandskonto->RemoveAllSubItems();
		
		std::auto_ptr<CMFCRibbonButton> apBtn(new CMFCRibbonButton(ID_BESTANDSKONTENFILTER_BASE-1, "<alle Bestandskonten>", -1, -1, true));
		apBtn->SetToolTipText("Filter deaktivieren und wieder alle Bestandskonten im Journal anzeigen");
		m_pBtnFilterBestandskonto->AddSubItem(apBtn.release());

		int i;
		for (i = 0; i < m_csaBestandskontenNamen.GetSize(); i++)
		{
			std::auto_ptr<CMFCRibbonButtonEx> apBtn(new CMFCRibbonButtonEx(ID_BESTANDSKONTENFILTER_BASE + i, (LPCTSTR)m_csaBestandskontenNamen[i], 
																		   m_tbiIconsBestandskonten.ExtractIcon(atoi(m_csaBestandskontenIcons[i])),
																		   true, NULL, true, true));
			apBtn->SetToolTipText("Vorjahressaldo: " + m_csaBestandskontenSalden[i]);
			m_pBtnFilterBestandskonto->AddSubItem(apBtn.release());
		}
	}
}


void CEasyCashView::UpdateBetriebe()
{
	m_csaBetriebeNamen.RemoveAll();
	m_csaBetriebeIcons.RemoveAll();
	m_csaBetriebeUnternehmensarten.RemoveAll();

	char inifile[1000], buffer[1000]; 
	GetIniFileName(inifile, sizeof(inifile)); 
	int i;
	for (i = 0; i < 100; i++)
	{
		CString csKey;
		csKey.Format("Betrieb%-02.2dName", i);
		GetPrivateProfileString("Betriebe", csKey.GetBuffer(0), "", buffer, sizeof(buffer), inifile);
		if (*buffer == '\0') break;
		m_csaBetriebeNamen.Add(buffer);
		csKey.Format("Betrieb%-02.2dUnternehmensart", i);
		GetPrivateProfileString("Betriebe", csKey.GetBuffer(0), "", buffer, sizeof(buffer), inifile);
		m_csaBetriebeUnternehmensarten.Add(buffer);
		csKey.Format("Betrieb%-02.2dIcon", i);
		GetPrivateProfileString("Betriebe", csKey.GetBuffer(0), "0", buffer, sizeof(buffer), inifile);
		m_csaBetriebeIcons.Add(buffer);
	}
}

void CEasyCashView::UpdateBestandskonten()
{
	m_csaBestandskontenNamen.RemoveAll();
	m_csaBestandskontenIcons.RemoveAll();
	m_csaBestandskontenSalden.RemoveAll();

	char inifile[1000], buffer[1000]; 
	GetIniFileName(inifile, sizeof(inifile)); 
	int i;
	CEasyCashDoc *pDoc = GetDocument();
	for (i = 0; i < 100; i++)
	{
		CString csKey;
		csKey.Format("Bestandskonto%-02.2dName", i);
		GetPrivateProfileString("Bestandskonten", csKey.GetBuffer(0), "", buffer, sizeof(buffer), inifile);
		if (*buffer == '\0') break;
		m_csaBestandskontenNamen.Add(buffer);
		csKey.Format("Bestandskonto%-02.2dSaldo%04d", i, pDoc->nJahr-1);
		GetPrivateProfileString("Bestandskonten", csKey.GetBuffer(0), "", buffer, sizeof(buffer), inifile);
		m_csaBestandskontenSalden.Add(buffer);
		csKey.Format("Bestandskonto%-02.2dIcon", i);
		GetPrivateProfileString("Bestandskonten", csKey.GetBuffer(0), "0", buffer, sizeof(buffer), inifile);
		m_csaBestandskontenIcons.Add(buffer);
	}
}

// callback für sortierte Gruppen (Bestandskonten) in der Navigations-Seitenleiste
int CALLBACK GroupCompare(int Arg1, int Arg2, void *Arg3)
{
	if (Arg1 == Arg2) return 0;
	if (Arg1 < Arg2) 
		return -1;
	else
		return 1;
}

void CEasyCashView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CEasyCashDoc* pDoc = GetDocument();

	SetupScroll();

	if (m_GewaehltesFormular >= 0)
		BerechneFormularfeldwerte();

	// Hook Erweiterungs-DLLs
	CIterateExtensionDLLs("ECTE_UpdateDocument", (void *)GetDocument());

	// Ansicht Menü anpassen	 // Ansicht	    // Journal für Konto
	CWnd *w = AfxGetMainWnd();

	int i, j;
	CBuchung *p;
	CString einnahmen_posten_name[100];
	CString ausgaben_posten_name[100];
	CStringArray csaBestandskontenMitBuchungenUnsortiert;	
	BOOL bUnzugewieseneEinnahmenbuchungenExistieren = FALSE, bUnzugewieseneAusgabenbuchungenExistieren = FALSE;
	int nKontenMitBuchungenErsteAusgabenbuchung = 0;

	for (i = 0; i < 100; i++)
	{
		einnahmen_posten_name[i]  = "";
		ausgaben_posten_name[i]   = "";
	}

	// benutzte E/Ü-Konten und Bestandskonten in Einnahmen finden
	if (m_nAnzeige != 3)  // Anlagenverzeichnis braucht nur Ausgaben
		for (p = pDoc->Einnahmen; p; p = p->next)
		{
			for (j = 0; j < 100; j++)
			{
				if (p->Konto.IsEmpty())
				{
					bUnzugewieseneEinnahmenbuchungenExistieren = TRUE;
					break;
				}

				// leer? Dann neu übernehmen
				if (einnahmen_posten_name[j].IsEmpty())
				{
					einnahmen_posten_name[j] = p->Konto;
					break;
				}

				if (p->Konto == einnahmen_posten_name[j])
				{
					break;
				}
			}

			for (j = 0; j < csaBestandskontenMitBuchungenUnsortiert.GetSize(); j++)
				if (csaBestandskontenMitBuchungenUnsortiert[j] == p->Bestandskonto)
					break;
			if (j >= csaBestandskontenMitBuchungenUnsortiert.GetSize())
				csaBestandskontenMitBuchungenUnsortiert.Add(p->Bestandskonto);			
		}

	// benutzte E/Ü-Konten und Bestandskonten in Ausgaben finden
	for (p = pDoc->Ausgaben; p; p = p->next)
	{
		if (m_nAnzeige == 3 && p->AbschreibungJahre <= 1 && p->Erweiterung.Find("|UrspruenglichesKonto=") == -1)
			continue;  // wenn Anlagenverzeichnis: nur AfA-Buchungen berücksichtigen

		for (j = 0; j < 100; j++)
		{
			CString csKonto = "";  // AfA-Abgang im Anlagenverzeichnis? Dann nicht das Restbuchwertkonto anzeigen, sondern das ursprüngliche Konto
			if (m_nAnzeige == 3 && p->AbschreibungJahre <= 1)
			{
				CString *pcsUrspruenglichesKonto = GetErweiterungKeyCS(p->Erweiterung, "EasyCash", "UrspruenglichesKonto");
				if (pcsUrspruenglichesKonto->IsEmpty())
				{
					delete pcsUrspruenglichesKonto;
					break;
				}		
				csKonto = (*pcsUrspruenglichesKonto).GetBuffer();
				delete pcsUrspruenglichesKonto;
			}
			else
				csKonto = p->Konto;

			if (csKonto.IsEmpty())
			{
				bUnzugewieseneAusgabenbuchungenExistieren = TRUE;
				break;
			}

			if (ausgaben_posten_name[j].IsEmpty())
			{

				ausgaben_posten_name[j] = csKonto;
				break;
			}

			if (csKonto == ausgaben_posten_name[j])
				break;
		}

		for (j = 0; j < csaBestandskontenMitBuchungenUnsortiert.GetSize(); j++)
			if (csaBestandskontenMitBuchungenUnsortiert[j] == p->Bestandskonto)
				break;
		if (j >= csaBestandskontenMitBuchungenUnsortiert.GetSize())
			csaBestandskontenMitBuchungenUnsortiert.Add(p->Bestandskonto);
	}

	// BestandskontenMitBuchungen sortieren	
	m_csaBestandskontenMitBuchungen.RemoveAll();
	int anzahlGroups = csaBestandskontenMitBuchungenUnsortiert.GetSize();
	int group;
	for (i = 0, group = 0; i < m_csaBestandskontenNamen.GetSize(); i++)
		for (j = 0; j < anzahlGroups; j++)
			if (m_csaBestandskontenNamen[i] == csaBestandskontenMitBuchungenUnsortiert[j])
			{
				TRACE2("\r\nBestandskonto %d: %s", i, m_csaBestandskontenNamen[i]);
				m_csaBestandskontenMitBuchungen.Add(csaBestandskontenMitBuchungenUnsortiert[j]);
				group++;
				break;
			}


	// für Menüaufbau benötigte Daten in m_KontenMitBuchungen speichern:
//		if (!pDrawInfo->pm) 	
	{
		m_KontenMitBuchungen.RemoveAll();
		m_KontenMitBuchungen.Add("<alle Konten>");
 
		for (i = 0; i < 100; i++)  // in der Reihenfolge der regulären Konten wie in den Einstellungen auflisten
		{			
			if (!einstellungen1 || einstellungen1->EinnahmenRechnungsposten[i].IsEmpty()) break;
			for (j = 0; j < 100; j++)
			{
				if (einnahmen_posten_name[j].IsEmpty()) break;
				if (einstellungen1 && einstellungen1->EinnahmenRechnungsposten[i] == einnahmen_posten_name[j])
				{
					m_KontenMitBuchungen.Add((CString)"Einnahmen: " + einnahmen_posten_name[j]);
					break;
				}
			}
		}
		for (j = 0; j < 100; j++)  // auch die Konten aus Buchungen auflisten, für die es in den Einstellungen keine Konten gibt
		{
			if (einnahmen_posten_name[j].IsEmpty()) break;
			for (i = 0; i < m_KontenMitBuchungen.GetSize(); i++)
				if ("Einnahmen: " + einnahmen_posten_name[j] == m_KontenMitBuchungen[i])
					break;  // schon drin
			if (i == m_KontenMitBuchungen.GetSize())  // Rouge-Konto in Buchung?
				m_KontenMitBuchungen.Add((CString)"Einnahmen: " + einnahmen_posten_name[j]);  // zusätzlich aufnehmen
		}
		nKontenMitBuchungenErsteAusgabenbuchung = m_KontenMitBuchungen.GetSize();
		if (bUnzugewieseneEinnahmenbuchungenExistieren)
			m_KontenMitBuchungen.Add((CString)"--- [noch zu keinem Konto zugewiesene Einnahmen] ---");
		for (i = 0; i < 100; i++)  // in der Reihenfolge der regulären Konten wie in den Einstellungen auflisten
		{			
			if (!einstellungen1 || einstellungen1->AusgabenRechnungsposten[i].IsEmpty()) break;
			for (j = 0; j < 100; j++)
			{
				if (ausgaben_posten_name[j].IsEmpty()) break;
				if (einstellungen1 && einstellungen1->AusgabenRechnungsposten[i] == ausgaben_posten_name[j])
				{
					m_KontenMitBuchungen.Add((CString)"Ausgaben: " + ausgaben_posten_name[j]);
					break;
				}
			}
		}
		for (j = 0; j < 100; j++)  // auch die Konten aus Buchungen auflisten, für die es in den Einstellungen keine Konten gibt
		{
			if (ausgaben_posten_name[j].IsEmpty()) break;
			for (i = 0; i < m_KontenMitBuchungen.GetSize(); i++)
				if ("Ausgaben: " + ausgaben_posten_name[j] == m_KontenMitBuchungen[i])
					break;  // schon drin
			if (i == m_KontenMitBuchungen.GetSize())  // Rouge-Konto in Buchung?
				m_KontenMitBuchungen.Add((CString)"Ausgaben: " + ausgaben_posten_name[j]);  // zusätzlich aufnehmen
		}
		if (bUnzugewieseneAusgabenbuchungenExistieren)
			m_KontenMitBuchungen.Add((CString)"--- [noch zu keinem Konto zugewiesene Ausgaben] ---");
	}

/*		// alter Menü-Kram für Konten-Filter
	CMenu *pJFK = NULL;		
	int n = 0;	//VS9
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu())
		n = AfxGetMainWnd()->GetMenu()->GetMenuItemCount();
	if (n > 3)	//VS9
	{
		pJFK = AfxGetMainWnd()->GetMenu()->GetSubMenu(3);
		CString csTemp;		
		AfxGetMainWnd()->GetMenu()->GetSubMenu(3)->GetMenuString(5, csTemp, MF_BYPOSITION);

		pJFK = AfxGetMainWnd()->GetMenu()->GetSubMenu(3)->GetSubMenu(5);
	}	//VS9
	if (pJFK)
	{
		while (pJFK->RemoveMenu(0, MF_BYPOSITION))
			;

		int i;
		for (i = 0; i < m_KontenMitBuchungen.GetSize(); i++)
			pJFK->AppendMenu(MF_STRING|MF_UNCHECKED, ID_VIEW_JOURNAL_FUER_KONTO_BASE + i, (LPCTSTR)m_KontenMitBuchungen[i]);
	}
*/
	// neuer Ribbon-Kram für Konten-Filter
	CMFCRibbonButton* pBtnFilterKonto;
	if (pBtnFilterKonto = ((CMainFrame*)AfxGetMainWnd())->m_pFilterKontoButton)
	{
		pBtnFilterKonto->RemoveAllSubItems();

		for (i = 0; i < m_KontenMitBuchungen.GetSize(); i++)
			pBtnFilterKonto->AddSubItem(new CMFCRibbonButton(ID_VIEW_JOURNAL_FUER_KONTO_BASE + i, (LPCTSTR)m_KontenMitBuchungen[i]));
	}
	
#define SPACES_ZU_ITEMS_HINZUFUEGEN 20

	// Navigationsleiste aktualisieren
	if (m_pNavigationWnd)
	{
		CListCtrl &nav = m_pNavigationWnd->GetListCtrl();
		nav.ModifyStyle(LVS_ICON, LVS_REPORT);
		if (nav.GetHeaderCtrl()->GetItemCount())	// Spalte hinzufügen, wenn noch nicht vorhanden
			nav.DeleteColumn(0);
		CRect clir;
		m_pNavigationWnd->GetClientRect(&clir);
		nav.InsertColumn(0, "Navigation", LVCFMT_CENTER, clir.Width());

		// Navigations-ListView: Gruppen initialisieren, wenn es sich nicht um ein Formular handelt
		nav.RemoveAllGroups();
		LVGROUP lg = {0};
		lg.cbSize = sizeof(lg);
		lg.state = LVGS_NORMAL;
		lg.uAlign = LVGA_HEADER_CENTER;
		lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
		if (m_GewaehltesFormular >= 0)
		{
			// wenn Abschnitte vorhanden, einzelne Seiten als Group
			if (m_csaFormulare.GetSize() > 0)
			{
				// Formulardefinitionsdatei in xmldoc laden
				XDoc xmldoc;
				xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
				LPXNode xml = xmldoc.GetRoot();
				if (xml)
				{
					CString csAnzahlSeiten = xml->GetAttrValue("seiten");
					m_anzahl_formularseiten = atoi(csAnzahlSeiten);
					LPXNode abschnitte = NULL;
					abschnitte = xml->Find("abschnitte");

					if (abschnitte && abschnitte->GetChildCount() > 0)  // Abschnitte-Sektion vorhanden?...
					{
						for (i = 0; i < m_anzahl_formularseiten; i++)
						{
							CString csGroupText;
							csGroupText.Format("Seite %d", i + 1);

							lg.iGroupId = i;
							WCHAR wcTemp[1000];
							if (!MultiByteToWideChar(CP_ACP, 0, csGroupText, (int)csGroupText.GetLength(), wcTemp, sizeof(wcTemp)))
								continue;
							wcTemp[csGroupText.GetLength()] = L'\0';
							lg.pszHeader = wcTemp;	
							lg.cchHeader = wcslen(lg.pszHeader);
							nav.InsertGroup(i, &lg);
						}
					}
					else  // ... ansonsten nur eine einzelne Group "Seiten" und deren Nummern als Items darunter
					{
						lg.iGroupId = 0;
						lg.pszHeader = L"Seiten";	
						lg.cchHeader = wcslen(lg.pszHeader);
						nav.InsertGroup(0, &lg);					
					}
				}
			}
		}
		else if (m_nAnzeige < 2)
		{
			lg.iGroupId = 0;
			lg.pszHeader = L"Einnahmen";	
			lg.cchHeader = wcslen(lg.pszHeader);
			nav.InsertGroup(0, &lg);
			lg.iGroupId = 1;
			lg.pszHeader = L"Ausgaben";
			lg.cchHeader = wcslen(lg.pszHeader);
			nav.InsertGroup(1, &lg);
			anzahlGroups = 2;
		}
		else if (m_nAnzeige == 2) // Journal nach Bestandskonten
		{
			anzahlGroups = m_csaBestandskontenMitBuchungen.GetSize();
			int i;
			for (i = 0; i < anzahlGroups; i++)
			{
				lg.iGroupId = i;
				WCHAR wcTemp[1000];
				if (!MultiByteToWideChar(CP_ACP, 0, m_csaBestandskontenMitBuchungen[i], (int)m_csaBestandskontenMitBuchungen[i].GetLength(), wcTemp, sizeof(wcTemp)))
					continue;
				wcTemp[m_csaBestandskontenMitBuchungen[i].GetLength()] = L'\0';
				lg.pszHeader = wcTemp;	
				lg.cchHeader = wcslen(lg.pszHeader);
				//LVINSERTGROUPSORTED lgs;
				//lgs.pfnGroupCompare = (PFNLVGROUPCOMPARE)GroupCompare;
				//lgs.pvData = NULL;  // wird nicht gebraucht, weil wir nach Group-ID sortieren
				//lgs.lvGroup = lg;
				//nav.InsertGroupSorted(&lgs);
				nav.InsertGroup(i, &lg);
			}
			anzahlGroups = m_csaBestandskontenMitBuchungen.GetSize();
		}
		else if (m_nAnzeige == 3)
		{
			lg.iGroupId = 1;
			lg.pszHeader = L"Anlagengruppen";
			lg.cchHeader = wcslen(lg.pszHeader);
			nav.InsertGroup(1, &lg);
			anzahlGroups = 1;
		}

		// Navigations-ListView, Items neu aufbauen
		nav.SetRedraw(FALSE);
		nav.DeleteAllItems();
		nav.SetRedraw(TRUE);	

		int group;
		switch (m_GewaehltesFormular >= 0 ? -1 : m_nAnzeige)
		{
		case 0:	// Journal nach Datum 		
		case 2: // Journal nach Bestandskonten
			for (group = 0; group < anzahlGroups; group++)
				for (i = 0; i < 12; i++)
			{
				CString csItemText;
				csItemText.Format("%s%s", cpMonat[i], CString(_T(' '), max(0, SPACES_ZU_ITEMS_HINZUFUEGEN-strlen(cpMonat[i]))).GetString());
				int iItem = nav.InsertItem(group * 12 + i, csItemText);

				LVITEM lvItem = {0};
				lvItem.mask = LVIF_GROUPID;
				lvItem.iItem = iItem;
				lvItem.iSubItem = 0;
				lvItem.iGroupId = group;
				nav.SetItem(&lvItem);
			}
					
			break;		
		case 1: // Journal nach Konten
		case 3: // Anlagenverzeichnis
			for (i = 0, group = 0; i < m_KontenMitBuchungen.GetSize(); i++)
			{
				if (m_KontenMitBuchungen[i] == "<alle Konten>") continue;

				CString csTemp;
				if (m_KontenMitBuchungen[i] == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---")
				{
					if (!bUnzugewieseneEinnahmenbuchungenExistieren) continue;
					csTemp = "[noch zu keinem Konto zugewiesene Einnahmen]";
				}
				else if (m_KontenMitBuchungen[i] == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---")
				{
					if (!bUnzugewieseneAusgabenbuchungenExistieren) continue;
					csTemp = "[noch zu keinem Konto zugewiesene Ausgaben]";
				}
				else
					csTemp = m_KontenMitBuchungen[i].Mid(10).TrimLeft();
				int iItem = nav.InsertItem(nav.GetItemCount(), csTemp);

				if (i >= nKontenMitBuchungenErsteAusgabenbuchung) group = 1; // fangen die Ausgaben an? dann Gruppe umschalten
				LVITEM lvItem = {0};
				lvItem.mask = LVIF_GROUPID;
				lvItem.iItem = iItem;
				lvItem.iSubItem = 0;
				lvItem.iGroupId = group;
				nav.SetItem(&lvItem);
			}
			break;
		// irgendein Formular (dann nur die Seiten anzeigen)
		default:
			if (m_csaFormulare.GetSize() > 0)
			{
				// noch mal Formulardefinitionsdatei in xmldoc laden
				XDoc xmldoc;
				xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
				LPXNode xml = xmldoc.GetRoot();
				if (!xml) break;
				LPXNode abschnitte = NULL;
				abschnitte = xml->Find("abschnitte");
				CString csAnzahlSeiten = xml->GetAttrValue("seiten");
				m_anzahl_formularseiten = atoi(csAnzahlSeiten);
				
				m_cuiaScrollPos.RemoveAll();
				if (abschnitte && abschnitte->GetChildCount() > 0)  // Abschnitte-Sektion vorhanden?
				{
					m_cuiaScrollPos.SetSize(abschnitte->GetChildCount());

					// Abschnitte in Seiten-Groups einsortieren
					for (i = 0; i < abschnitte->GetChildCount(); i++)
					{
						LPXNode child = abschnitte->GetChild(i);
						CString csName = child->GetAttrValue("name");
						int seite = atoi(child->GetAttrValue("seite"));					 
						int vertikal = atoi(child->GetAttrValue("vertikal"));

						CString csItemText;
						//csItemText.Format("%s%s", csName, CString(_T(' '), SPACES_ZU_ITEMS_HINZUFUEGEN).GetString());
						int iItem = nav.InsertItem(i, csName);
						m_cuiaScrollPos[iItem] = (seite - 1) * 1414 + vertikal;

						LVITEM lvItem = {0};
						lvItem.mask = LVIF_GROUPID;
						lvItem.iItem = iItem;
						lvItem.iSubItem = 0;
						lvItem.iGroupId = seite - 1;
						nav.SetItem(&lvItem);
					}
				}
				else  // nur Seitennummern auflisten
				{
					for (i = 0; i < m_anzahl_formularseiten; i++)
					{
						CString csItemText;
						csItemText.Format("%d%s", i + 1, CString(_T(' '), SPACES_ZU_ITEMS_HINZUFUEGEN).GetString());
						int iItem = nav.InsertItem(i, csItemText);

						LVITEM lvItem = {0};
						lvItem.mask = LVIF_GROUPID;
						lvItem.iItem = iItem;
						lvItem.iSubItem = 0;
						lvItem.iGroupId = 0;
						nav.SetItem(&lvItem);
					}	
				}
			}		
		}
	}

	// Status bar
	((CMainFrame*)AfxGetMainWnd())->UpdateSaldo(this);

	CaptionBoxCheckOnUpdate();

	CScrollView::OnUpdate(pSender, lHint, pHint);
}

void CEasyCashView::CaptionBoxCheckOnUpdate()
{
	CEasyCashDoc *pDoc = GetDocument();

	if (theApp.IsCaptionBoxShown()) return;

	// gab es Korrekturen bei der AfA-Dauer?
	if (m_bAfaKorrketuren)
	{
		CString csTemp;
		csTemp.Format("Es wurden %d Ausgabenbuchungen mit einer AfA-Dauer von 0 gefunden und automatisch auf den Minimalwert von 1 korrigiert. Gegebenenfalls kann sich dadurch die Summe der Vorsteuer geändert haben. Bitte evtl. bereits versendete USt.-Voranmeldungen auf Differenzen prüfen.", m_bAfaKorrketuren);
		m_bAfaKorrketuren = 0;
		pDoc->SetModifiedFlag("AfA-Buchungen korrigiert");
		theApp.CaptionBox(csTemp, ID_FILE_SAVE, "Speichern", "korrigierte Buchungsdaten speichern");
		return;
	}

	// stimmt das Buchungsjahr des Dokuments mit denen der Buchungen überein?
	BOOL bGefunden = FALSE;
	if (pDoc->Einnahmen || pDoc->Ausgaben)
	{
		CBuchung *pb = pDoc->Einnahmen;
		while (pb)
		{
			if (pb->Datum.GetYear() == pDoc->nJahr)
			{
				bGefunden = TRUE;
				break;
			}
				
			pb = pb->next;
		}

		if (!bGefunden)
		{
			pb = pDoc->Ausgaben;
			while (pb)
			{
				if (pb->Datum.GetYear() == pDoc->nJahr)
				{
					bGefunden = TRUE;
					break;
				}
					
				pb = pb->next;
			}
		}

		if (!bGefunden)
		{
			CString csMeldung;
			csMeldung.Format("Alle Buchungen liegen außerhalb des Buchungsjahres %04d, weshalb Auswertungen und Formulare leer erscheinen werden.", pDoc->nJahr);
			theApp.CaptionBox(csMeldung, ID_VIEW_OPTIONS, "Beheben", "aktuelles Buchungsjahr in den Einstellungen anpassen");
			return;
		}
	}

	// Quartals- vs. Monatsvorauszahlungen: Kollision checken
	CString m1, m2, m3, q;
	GetUmsatzsteuervorauszahlung(1, m1);
	GetUmsatzsteuervorauszahlung(2, m2);
	GetUmsatzsteuervorauszahlung(3, m3);
	GetUmsatzsteuervorauszahlung(41, q);
	if ((m1.Trim() != "" || m2.Trim() != "" || m3.Trim() != "") && q.Trim() != "")
	{
		theApp.CaptionBox("Es gibt eine Vorauszahlung für das 1. Quartal, die mit einer Monats-Vorauszahlung in dem Zeitraum kollidiert.",
			ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, "Beheben", "zum Umsatzsteuerzahlungen-Dialog gehen, um das zu korrigieren");
		return;
	}
	else
	{
		GetUmsatzsteuervorauszahlung(4, m1);
		GetUmsatzsteuervorauszahlung(5, m2);
		GetUmsatzsteuervorauszahlung(6, m3);
		GetUmsatzsteuervorauszahlung(42, q);
		if ((m1.Trim() != "" || m2.Trim() != "" || m3.Trim() != "") && q.Trim() != "")
		{
			theApp.CaptionBox("Es gibt eine Vorauszahlung für das 2. Quartal, die mit einer Monats-Vorauszahlung in dem Zeitraum kollidiert.",
				ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, "Beheben", "zum Umsatzsteuerzahlungen-Dialog gehen, um das zu korrigieren");
			return;
		}
		else
		{
			GetUmsatzsteuervorauszahlung(7, m1);
			GetUmsatzsteuervorauszahlung(8, m2);
			GetUmsatzsteuervorauszahlung(9, m3);
			GetUmsatzsteuervorauszahlung(43, q);
			if ((m1.Trim() != "" || m2.Trim() != "" || m3.Trim() != "") && q.Trim() != "")
			{
				theApp.CaptionBox("Es gibt eine Vorauszahlung für das 3. Quartal, die mit einer Monats-Vorauszahlung in dem Zeitraum kollidiert.",
					ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, "Beheben", "zum Umsatzsteuerzahlungen-Dialog gehen, um das zu korrigieren");
				return;
			}
			else
			{
				GetUmsatzsteuervorauszahlung(10, m1);
				GetUmsatzsteuervorauszahlung(11, m2);
				GetUmsatzsteuervorauszahlung(12, m3);
				GetUmsatzsteuervorauszahlung(44, q);
				if ((m1.Trim() != "" || m2.Trim() != "" || m3.Trim() != "") && q.Trim() != "")
				{
					theApp.CaptionBox("Es gibt eine Vorauszahlung für das 4. Quartal, die mit einer Monats-Vorauszahlung in dem Zeitraum kollidiert.",
						ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, "Beheben", "zum Umsatzsteuerzahlungen-Dialog gehen, um das zu korrigieren");
					return;
				}
			}
		}
	}

	if (m_csaBetriebeNamen.GetSize())
	{
		CEasyCashDoc* pDoc = GetDocument();
		int nEinnahmenOhneBetrieb = 0, nAusgabenOhneBetrieb = 0;
		CBuchung *p;
		for (p = pDoc->Einnahmen; p; p = p->next)
			if (p->Betrieb == "")
				nEinnahmenOhneBetrieb++;
		for (p = pDoc->Ausgaben; p; p = p->next)
			if (p->Betrieb == "")
				nAusgabenOhneBetrieb++;
		if (nEinnahmenOhneBetrieb || nAusgabenOhneBetrieb)
		{
			CString csMessage = "Es existieren ";
			CString csTemp;
			if (nEinnahmenOhneBetrieb)
			{
				csTemp.Format("%d Einnahmebuchungen ", nEinnahmenOhneBetrieb);
				csMessage += csTemp;
			}
			if (nEinnahmenOhneBetrieb && nAusgabenOhneBetrieb) 
				csMessage += "und ";
			if (nAusgabenOhneBetrieb)
			{
				csTemp.Format("%d Ausgabenbuchungen ", nAusgabenOhneBetrieb);
				csMessage += csTemp;
			}
			csMessage += "ohne Zuweisung zu einem Betrieb.";

			theApp.CaptionBox(csMessage);
			return;
		}
	}

	if (m_csaBestandskontenNamen.GetSize())
	{
		CEasyCashDoc* pDoc = GetDocument();
		int nEinnahmenOhneBestandskonto = 0, nAusgabenOhneBestandskonto = 0;
		CBuchung *p;
		for (p = pDoc->Einnahmen; p; p = p->next)
			if (p->Bestandskonto == "")
				nEinnahmenOhneBestandskonto++;
		for (p = pDoc->Ausgaben; p; p = p->next)
			if (p->Bestandskonto == "")
				nAusgabenOhneBestandskonto++;
		if (nEinnahmenOhneBestandskonto || nAusgabenOhneBestandskonto)
		{
			CString csMessage = "Es existieren ";
			CString csTemp;
			if (nEinnahmenOhneBestandskonto)
			{
				csTemp.Format("%d Einnahmebuchungen ", nEinnahmenOhneBestandskonto);
				csMessage += csTemp;
			}
			if (nEinnahmenOhneBestandskonto && nAusgabenOhneBestandskonto) 
				csMessage += "und ";
			if (nAusgabenOhneBestandskonto)
			{
				csTemp.Format("%d Ausgabenbuchungen ", nAusgabenOhneBestandskonto);
				csMessage += csTemp;
			}
			csMessage += "ohne Zuweisung zu einem Bestandskonto.";

			theApp.CaptionBox(csMessage,
				ID_VIEW_JOURNAL_BESTKONTEN, "Bestandskonten anzeigen", "wechselt zur Bestandskonten-Journalansicht");
			return;
		}
	}

	theApp.CaptionBoxHide();
}

void CEasyCashView::OnSize(UINT nType, int cx, int cy) 
{
	TRACE3("CEasyCashView::OnSize(nType=%d, cx=%d, cy=%d)", nType, cx, cy);
	CScrollView::OnSize(nType, cx, cy);
	static int cx_old = -1;
	
	if (pPluginWnd)
	{
//		CPluginElement *pPluginDaten = ((CMainFrame*)AfxGetMainWnd())->m_pPlugins;
//		if (pPluginDaten->typ == "scroll")
		CRect r;
		GetParent()->GetParent()->GetClientRect(&r);
		pPluginWnd->ResizeClient(r.right, r.bottom);
	}

	if (m_timeLetzteZwangsverkleinerung < CTime::GetCurrentTime() - CTimeSpan(0, 0, 0, 1)  // unterbinden, wenn gerade Zoomverkleinerung im Gange ist
		&& m_zoomfaktor < m_wunschzoomfaktor && cx > cx_old) // bei Vergrößerung der Fensterbreite versuchen, den Wunsch-Zoomlevel wieder herzustellen, wenn er zwangsweise verkleinert wurde
			m_zoomfaktor = m_wunschzoomfaktor; 
	cx_old = cx;

	SetupScroll();	
}

// zoomfaktor in prozent
void CEasyCashView::SetupScroll()
{
	SIZE linescroll, pagescroll;
	CDC *pDC = GetDC();
	CEasyCashDoc* pDoc = GetDocument();

	TEXTMETRIC Metrics;
	VERIFY(pDC->GetOutputTextMetrics(&Metrics));

	// A4 für vert. Größe setzen, ansonsten Fensterbreite nehmen...
	charwidth = Metrics.tmAveCharWidth * m_zoomfaktor / 100;
	charheight = Metrics.tmHeight * m_zoomfaktor / 100;
	CRect r;
	GetClientRect(&r);
	if (m_GewaehltesFormular != -1) 
	{
		if (letzte_spalte > 0)
			gesamtgroesse.cx = letzte_spalte * charwidth;
	}
	else
	{
		gesamtgroesse.cx = r.right;
		letzte_spalte = gesamtgroesse.cx / charwidth;
	}
	gesamtgroesse.cy = charheight * letzte_zeile;
	linescroll.cy = charheight;
	linescroll.cx = charwidth;
	pagescroll.cy = charheight * 36;
	pagescroll.cx = charwidth * 40;

//gesamtgroesse.cy *= 10;
	SetScrollSizes(MM_TEXT, gesamtgroesse, pagescroll, linescroll);
}


BOOL CEasyCashView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_TRACKPOS;

	if (LOBYTE(nScrollCode) == SB_THUMBTRACK)
	{
		GetScrollInfo(SB_HORZ, &info);
		nPos = info.nTrackPos;
	}

	if (HIBYTE(nScrollCode) == SB_THUMBTRACK)
	{
		GetScrollInfo(SB_VERT, &info);
		nPos = info.nTrackPos;
		nSelected = -1;
	}
	
	return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}

void CEasyCashView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEasyCashView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CEasyCashView::OnDraw(CDC* pDC_par)
{
	if (!pPluginWnd) 
	{
		DrawInfo di;
		di.pm = FALSE;
		di.pInfo = NULL;
		di.fontsize = -1;
		di.printer_fontsize = -1;
		di.m_pDC = pDC_par;
		COLORREF bkc = pDC_par->GetBkColor();
		pDC_par->SetBkColor(GetSysColor(COLOR_WINDOW));
		if (m_GewaehltesFormular >= 0)
			DrawFormularToDC(pDC_par, &di);
		else
		{
			DrawToDC_Marker(&di, pDC_par, nSelected, 0x0080ffff);

			switch (m_nAnzeige)
			{
			case 0: DrawToDC_Datum(pDC_par, &di); break;
			case 1: DrawToDC_Konten(pDC_par, &di); break;
			case 2: DrawToDC_Bestandskonten(pDC_par, &di); break;
			case 3: DrawToDC_Anlagenverzeichnis(pDC_par, &di); break;
			}

			DrawToDC_Selection(&di, pDC_par);
		}
		pDC_par->SetBkColor(bkc);
	}
}

// Hilfsfunktion zur Steuerung des Layouts
void CEasyCashView::CheckLayout(DrawInfo *pDrawInfo)
{
	CEasyCashDoc* pDoc = GetDocument();

	if (pDrawInfo->pm) pDrawInfo->printer_fontsize = 0;

	pDrawInfo->zeige_betriebicon = FALSE;
	pDrawInfo->zeige_bestandskontoicon = FALSE;
	pDrawInfo->zeige_belegnummernspalte = FALSE;
	pDrawInfo->zeige_steuerspalte = FALSE;

	if (m_csaBetriebeNamen.GetSize()) pDrawInfo->zeige_betriebicon = 3;
	if (m_csaBestandskontenNamen.GetSize()) pDrawInfo->zeige_bestandskontoicon = 3;

	int zusaetzliche_stellen_belegnummernspalte = 0;

	CBuchung *p;
	for (p = pDoc->Einnahmen; p; p = p->next)
	{
		if (!p->Belegnummer.IsEmpty())
		{
			pDrawInfo->zeige_belegnummernspalte = TRUE;
		}
		int n = p->Belegnummer.GetLength();
		if (n-5 > zusaetzliche_stellen_belegnummernspalte) zusaetzliche_stellen_belegnummernspalte = n-5;
	}
	for (p = pDoc->Ausgaben; p; p = p->next)
	{
		if (!p->Belegnummer.IsEmpty())
		{
			pDrawInfo->zeige_belegnummernspalte = TRUE;
		}
		int n = p->Belegnummer.GetLength();
		if (n-5 > zusaetzliche_stellen_belegnummernspalte) zusaetzliche_stellen_belegnummernspalte = n-5;
	}

	for (p = pDoc->Einnahmen; p; p = p->next)
	{
		if (p->MWSt)
		{
			pDrawInfo->zeige_steuerspalte = TRUE;
			break;
		}
	}

	if (!pDrawInfo->zeige_steuerspalte)
	{
		for (p = pDoc->Ausgaben; p; p = p->next)
		{
			if (p->MWSt)
			{
				pDrawInfo->zeige_steuerspalte = TRUE;
				break;
			}
		}
	}

	querformat_faktor = 100; // %
	if (pDrawInfo->printer_gesamtgroesse.cx > pDrawInfo->printer_gesamtgroesse.cy)
		querformat_faktor = 70; // %

	pDrawInfo->letzte_spalte_device = pDrawInfo->pm ? HCHARS : letzte_spalte;
	pDrawInfo->spalte_betriebicon = pDrawInfo->pm ? 4 : 1;
	pDrawInfo->spalte_bestandskontoicon = pDrawInfo->zeige_betriebicon + (pDrawInfo->pm ? 4 : 1);
	pDrawInfo->spalte_datum = pDrawInfo->zeige_betriebicon + pDrawInfo->zeige_bestandskontoicon + (pDrawInfo->pm ? 4 : 1);
	pDrawInfo->spalte_belegnummer =      pDrawInfo->zeige_betriebicon + pDrawInfo->zeige_bestandskontoicon + (pDrawInfo->pm ? 9 + 3 * querformat_faktor/100 : (m_zoomfaktor <= 50 ? 14 : 12));
	pDrawInfo->spalte_belegnummer_ende = pDrawInfo->zeige_betriebicon + pDrawInfo->zeige_bestandskontoicon + (pDrawInfo->pm ? 9 + 7 * querformat_faktor/100 + (zusaetzliche_stellen_belegnummernspalte * 2 / 3) : 20 + zusaetzliche_stellen_belegnummernspalte);
	//pDrawInfo->spalte_beschreibung =     pDrawInfo->zeige_betriebicon + pDrawInfo->zeige_bestandskontoicon + (pDrawInfo->pm ? 9 + 8 * querformat_faktor/100 + (zusaetzliche_stellen_belegnummernspalte * 2 / 3) : 19 + zusaetzliche_stellen_belegnummernspalte) - (pDrawInfo->zeige_belegnummernspalte ? 0 : 7);
	if  (pDrawInfo->zeige_belegnummernspalte)
		pDrawInfo->spalte_beschreibung = pDrawInfo->zeige_betriebicon + pDrawInfo->zeige_bestandskontoicon + (pDrawInfo->pm ? 9 + 8 * querformat_faktor/100 + (zusaetzliche_stellen_belegnummernspalte * 2 / 3) : 21 + zusaetzliche_stellen_belegnummernspalte);
	else
		pDrawInfo->spalte_beschreibung = pDrawInfo->spalte_belegnummer;
	pDrawInfo->spalte_beschreibung_ende = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 35 * querformat_faktor/100 : 40) + (pDrawInfo->zeige_steuerspalte?0:(pDrawInfo->pm ? 20 * querformat_faktor/100 : 24));
	pDrawInfo->spalte_netto = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 26 * querformat_faktor/100 : 33);
	pDrawInfo->spalte_ust = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 20 * querformat_faktor/100 : 28);
	pDrawInfo->spalte_ust_betrag = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 13 * querformat_faktor/100 : 19);
	pDrawInfo->spalte_brutto = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 6 * querformat_faktor/100 : 8);
	pDrawInfo->spalte_afanr = pDrawInfo->letzte_spalte_device-2;
}

// Hilfsfunktion zur Steuerung des Layouts des Anlagenverzeichnisses
void CEasyCashView::CheckAnlagenverzeichnisLayout(DrawInfo *pDrawInfo)
{
	CEasyCashDoc* pDoc = GetDocument();

	if (pDrawInfo->pm) pDrawInfo->printer_fontsize = 0;

	pDrawInfo->zeige_betriebicon = FALSE;
	pDrawInfo->zeige_bestandskontoicon = FALSE;
	pDrawInfo->zeige_belegnummernspalte = FALSE;
	pDrawInfo->zeige_steuerspalte = FALSE;

	if (m_csaBetriebeNamen.GetSize()) pDrawInfo->zeige_betriebicon = 3;

	CBuchung *p;
	if (!pDrawInfo->zeige_steuerspalte)
	{
		for (p = pDoc->Ausgaben; p; p = p->next)
		{
			if (p->MWSt)
			{
				pDrawInfo->zeige_steuerspalte = TRUE;
				break;
			}
		}
	}

	querformat_faktor = 100; // %
	if (pDrawInfo->printer_gesamtgroesse.cx > pDrawInfo->printer_gesamtgroesse.cy)
		querformat_faktor = 70; // %

	pDrawInfo->letzte_spalte_device = pDrawInfo->pm ? HCHARS : letzte_spalte;
	pDrawInfo->spalte_betriebicon = pDrawInfo->pm ? 3 : 1;
	pDrawInfo->spalte_beschreibung = pDrawInfo->zeige_betriebicon + (pDrawInfo->pm ? 3 * querformat_faktor/100 : (m_zoomfaktor <= 50 ? 2 : 1));
	pDrawInfo->spalte_beschreibung_ende = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 61 * querformat_faktor/100 : 94);
	pDrawInfo->spalte_datum = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 59 * querformat_faktor/100 : 93);
	pDrawInfo->spalte_anschaffungskosten = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 40 * querformat_faktor/100 : 67); 
	pDrawInfo->spalte_buchwert_beginn = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 30 * querformat_faktor/100 : 52);
	pDrawInfo->spalte_afa = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 22 * querformat_faktor/100 : 37);
	pDrawInfo->spalte_abgaenge = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 15 * querformat_faktor/100 : 22);
	pDrawInfo->spalte_buchwert_ende = pDrawInfo->letzte_spalte_device - (pDrawInfo->pm ? 6 * querformat_faktor/100 : 9);	
	pDrawInfo->spalte_afanr = pDrawInfo->letzte_spalte_device-2;
}

// nicht mehr genügend Platz für "Beschreibung" Spaltenüberschrift? Dann verkleinern!
bool CEasyCashView::CheckPlatzFuerBeschreibung(DrawInfo *pDrawInfo)
{
	if (!pDrawInfo->pm)
	{
		int beschreibung_rechter_rand = pDrawInfo->spalte_beschreibung + 20;	
		int netto_oder_brutto_linker_rand;
		if (pDrawInfo->zeige_steuerspalte)
			netto_oder_brutto_linker_rand = pDrawInfo->spalte_netto;
		else
			netto_oder_brutto_linker_rand = pDrawInfo->spalte_brutto;
		if (netto_oder_brutto_linker_rand < beschreibung_rechter_rand)	
		{
			m_zoomfaktor -= 25;
			m_timeLetzteZwangsverkleinerung = CTime::GetCurrentTime();
			if (m_zoomfaktor < 25) 
				m_zoomfaktor = 25;
			else
			{
				SetupScroll();
				RedrawWindow();
				return FALSE;
			}
		}
	}
	return TRUE;
}

// return: FALSE, wenn kein Platz mehr für Beschreibung ist und Zoomlevel verkleinert werden muss
bool CEasyCashView::DrawToDC_EinnahmenHeader(DrawInfo *pDrawInfo)
{
//	TRACE0("DrawToDC_EinnahmenHeader");
//	if (pDrawInfo->zeige_bestandskontoicon)
//		Icon(pDrawInfo, pDrawInfo->spalte_bestandskontoicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIconsBestandskonten, 1);

	// nicht mehr genügend Platz für "Beschreibung" Spaltenüberschrift? Dann verkleinern!
	if (!CheckPlatzFuerBeschreibung(pDrawInfo))
		return FALSE;

	// Spaltenüberschriften
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, "Datum");
	if (pDrawInfo->zeige_belegnummernspalte) Text(pDrawInfo, pDrawInfo->spalte_belegnummer, pDrawInfo->line, "Beleg");
	Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, "Beschreibung");
	int netto_oder_brutto_linker_rand = -1;
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	if (pDrawInfo->zeige_steuerspalte)
	{
		Text(pDrawInfo, pDrawInfo->spalte_netto, pDrawInfo->line, "Netto");
		Text(pDrawInfo, pDrawInfo->spalte_ust, pDrawInfo->line, "UST");
		Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, "UST-Betr");
	}
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, "Brutto");
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
	if (pDrawInfo->pm)
	{
		pDrawInfo->line+=2;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
	{
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->spalte_afanr);
		pDrawInfo->line++;
	}

	return TRUE;
}

void CEasyCashView::DrawToDC_AusgabenHeader(DrawInfo *pDrawInfo)
{
//	TRACE0("DrawToDC_AusgabenHeader");
//	if (pDrawInfo->zeige_bestandskontoicon)
//		Icon(pDrawInfo, pDrawInfo->spalte_bestandskontoicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIconsBestandskonten, 1);

	// Spaltenüberschriften
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, "Datum");
	if (pDrawInfo->zeige_belegnummernspalte) Text(pDrawInfo, pDrawInfo->spalte_belegnummer, pDrawInfo->line, "Beleg");
	Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, "Beschreibung");
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	if (pDrawInfo->zeige_steuerspalte)
	{
		Text(pDrawInfo, pDrawInfo->spalte_netto, pDrawInfo->line, "Netto");
		Text(pDrawInfo, pDrawInfo->spalte_ust, pDrawInfo->line, "VST");
		Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, "VST-Betr");
	}
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, "Brutto");
	Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, "AfANr");
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	if (pDrawInfo->pm)
	{
		pDrawInfo->line+=2;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
	{
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->spalte_afanr);
		pDrawInfo->line++;
	}
}

void CEasyCashView::DrawToDC_AnlagenverzeichnisHeader(DrawInfo *pDrawInfo)
{
	// Spaltenüberschriften
	Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, "Beschreibung");
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, "Ansch.Datum");
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	Text(pDrawInfo, pDrawInfo->spalte_anschaffungskosten, pDrawInfo->line, "Ansch.kosten");
	Text(pDrawInfo, pDrawInfo->spalte_buchwert_beginn, pDrawInfo->line, "Buchw.Beginn");
	Text(pDrawInfo, pDrawInfo->spalte_afa, pDrawInfo->line, "AfA");
	Text(pDrawInfo, pDrawInfo->spalte_abgaenge, pDrawInfo->line, "Abgänge");
	Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, "AfANr");
	Text(pDrawInfo, pDrawInfo->spalte_buchwert_ende, pDrawInfo->line, "Buchw.Ende");
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	if (pDrawInfo->pm)
	{
		pDrawInfo->line+=2;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
	{
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->spalte_afanr);
		pDrawInfo->line++;
	}
}

bool CEasyCashView::DrawToDC_BestandskontenHeader(DrawInfo *pDrawInfo, int nIcon, int nAnfangssaldo, int nBuchungenDavor, CTime von)
{
	char buf[300];	

	// nicht mehr genügend Platz für "Beschreibung" Spaltenüberschrift? Dann verkleinern!
	if (!CheckPlatzFuerBeschreibung(pDrawInfo))
		return FALSE;

	pDrawInfo->brutto_summe = nAnfangssaldo;

	// großes Bestandskonto-Icon anzeigen
	if (nIcon >= 0)
		Icon(pDrawInfo, 1, pDrawInfo->line-2, pDrawInfo->line, &m_cbmIconsBestandskonten, nIcon);

	// Spaltenüberschriften
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, "Datum");
	if (pDrawInfo->zeige_belegnummernspalte) Text(pDrawInfo, pDrawInfo->spalte_belegnummer, pDrawInfo->line, "Beleg");
	Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, "Beschreibung");
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, "Brutto");
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, "Saldo");
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
	if (pDrawInfo->pm)
	{
		pDrawInfo->line+=2;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
	{
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->spalte_afanr);
		pDrawInfo->line++;
	}	

	// Anfangssaldo-Zeile
	int nOldBkMode = pDrawInfo->m_pDC->GetBkMode();
	pDrawInfo->m_pDC->SetBkMode(TRANSPARENT);	// nötig für:
	// jede gerade Zeile grau unterlegen
	if (!(pDrawInfo->line % 2)) GraueBox(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, pDrawInfo->spalte_afanr, pDrawInfo->line+1);
	
	char* cp = "Anfangssaldo/Übertrag";
	if (nBuchungenDavor)
		sprintf(buf, "%s (inkl. %d Buchung(en) vor dem %d.%d.%d -- siehe Journal nach Datum)", cp, nBuchungenDavor, von.GetDay(), von.GetMonth(), von.GetYear());
	else
		strcpy(buf, cp);
	Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(nAnfangssaldo, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
	pDrawInfo->line++;

	pDrawInfo->m_pDC->SetBkMode(nOldBkMode);

	return TRUE;
}

void CEasyCashView::DrawToDC_EinnahmenFooter(DrawInfo *pDrawInfo)
{
	char buf[300];

	if (pDrawInfo->pm)
	{
		pDrawInfo->line++;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line-1, pDrawInfo->spalte_afanr);
	if (pDrawInfo->zeige_steuerspalte)
	{
		int_to_currency_tausenderpunkt(pDrawInfo->netto_summe, 10, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, pDrawInfo->spalte_netto, pDrawInfo->line, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

		int_to_currency_tausenderpunkt(pDrawInfo->ust, 10, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, buf);
	}
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(pDrawInfo->brutto_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line+(pDrawInfo->brutto_summe >= 10000000?1:0), buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, ((CEasyCashDoc *)GetDocument())->csWaehrung.GetBuffer(0));
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->line++;
}

void CEasyCashView::DrawToDC_AusgabenFooter(DrawInfo *pDrawInfo)
{
	char buf[300];

	if (pDrawInfo->pm)
	{
		pDrawInfo->line++;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line-1, pDrawInfo->spalte_afanr);
	if (pDrawInfo->zeige_steuerspalte)
	{
		int_to_currency_tausenderpunkt(pDrawInfo->netto_summe, 10, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, pDrawInfo->spalte_netto, pDrawInfo->line, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

		int_to_currency_tausenderpunkt(pDrawInfo->vst, 10, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, buf);
	}
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(pDrawInfo->brutto_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line+(pDrawInfo->brutto_summe >= 10000000?1:0), buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, ((CEasyCashDoc *)GetDocument())->csWaehrung.GetBuffer(0));
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->line++;
}

void CEasyCashView::DrawToDC_BestandskontenFooter(DrawInfo *pDrawInfo)
{
	char buf[300];

	if (pDrawInfo->pm)
	{
		pDrawInfo->line++;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line-1, pDrawInfo->spalte_afanr);
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(pDrawInfo->brutto_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line+(pDrawInfo->brutto_summe >= 10000000?1:0), buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, ((CEasyCashDoc *)GetDocument())->csWaehrung.GetBuffer(0));
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->line++;
}

void CEasyCashView::DrawToDC_AnlagenverzeichnisFooter(DrawInfo *pDrawInfo)
{
	char buf[300];

	if (pDrawInfo->pm)
	{
		pDrawInfo->line++;
		Underline10(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line*10-5, pDrawInfo->spalte_afanr);
	}
	else
		Underline(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line-1, pDrawInfo->spalte_afanr);

	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);

	// Buchwert zu Jahresbeginn
	int_to_currency_tausenderpunkt(pDrawInfo->buchwert_beginn_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_buchwert_beginn, pDrawInfo->line, buf);

	// Netto-AfA-Betrag 
	int_to_currency_tausenderpunkt(pDrawInfo->netto_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_afa, pDrawInfo->line, buf);

	// Abgänge
	int_to_currency_tausenderpunkt(pDrawInfo->abgaenge_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_abgaenge, pDrawInfo->line, buf);

	// Buchwert zu Jahresende
	int_to_currency_tausenderpunkt(pDrawInfo->buchwert_ende_summe, 10, buf);
	Text(pDrawInfo, pDrawInfo->spalte_buchwert_ende, pDrawInfo->line, buf);

	Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, ((CEasyCashDoc *)GetDocument())->csWaehrung.GetBuffer(0));

	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->line++;
}

void CEasyCashView::DrawToDC_EinnahmenLine(DrawInfo *pDrawInfo, CBuchung *p)
{
	char buf[3000];

	int nOldBkMode = pDrawInfo->m_pDC->GetBkMode();
	pDrawInfo->m_pDC->SetBkMode(TRANSPARENT);	// nötig für:
	// jede gerade Zeile grau unterlegen
	if (!(pDrawInfo->line % 2) && pDrawInfo->line != nSelected) GraueBox(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, pDrawInfo->spalte_afanr, pDrawInfo->line+1);
	
	if (pDrawInfo->zeige_betriebicon)
	{
		int i;
		int n = m_csaBetriebeNamen.GetSize();
		for (i = 0; i < n; i++)
		if (m_csaBetriebeNamen.GetAt(i) == p->Betrieb)
			{
				Icon(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIcons, atoi(m_csaBetriebeIcons.GetAt(i)));
				break;
			}
	}
	if (pDrawInfo->zeige_bestandskontoicon)
	{
		int i;
		int n = m_csaBestandskontenNamen.GetSize();
		for (i = 0; i < n; i++)
		if (m_csaBestandskontenNamen.GetAt(i) == p->Bestandskonto)
			{
				Icon(pDrawInfo, pDrawInfo->spalte_bestandskontoicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIconsBestandskonten, atoi(m_csaBestandskontenIcons.GetAt(i)));
				break;
			}
	}
	
	sprintf(buf, "%02d.%02d.%04d", 
		p->Datum.GetDay(), p->Datum.GetMonth(), p->Datum.GetYear());
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, buf);

	if (pDrawInfo->zeige_belegnummernspalte) 
	{
		TextEx(pDrawInfo, pDrawInfo->spalte_belegnummer, pDrawInfo->line, pDrawInfo->spalte_belegnummer_ende, pDrawInfo->line, p->Belegnummer.GetBuffer(0));
	}

	TextEx(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, pDrawInfo->spalte_beschreibung_ende, pDrawInfo->line, p->Beschreibung.GetBuffer(0));

	// Netto- und MwSt-Betrag berechnen
	long mwst_betrag, netto;
	netto = p->GetNetto();
	mwst_betrag = p->Betrag - netto;
	
	// Summen mitführen
	pDrawInfo->ust += mwst_betrag;
	pDrawInfo->brutto_summe += p->Betrag;
	pDrawInfo->netto_summe += netto;

	if (pDrawInfo->zeige_steuerspalte)
	{
		// Netto-Betrag
		int_to_currency_tausenderpunkt(netto, 8, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, pDrawInfo->spalte_netto, pDrawInfo->line, buf);

		// MwSt-Satz
		p->GetMWSt(buf);
		if (mwst_betrag < 1000000) 
			strcat(buf, "%");
		else
			strcat(buf, " ");
		Text(pDrawInfo, pDrawInfo->spalte_ust, pDrawInfo->line, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

		// MwSt-Betrag
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		int_to_currency_tausenderpunkt(mwst_betrag, 7, buf);
		Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, buf);
	}

	// Brutto-Betrag
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(p->Betrag, 8, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->m_pDC->SetBkMode(nOldBkMode);
}

void CEasyCashView::DrawToDC_AusgabenLine(DrawInfo *pDrawInfo, CBuchung *p)
{
	int nOldBkMode = pDrawInfo->m_pDC->GetBkMode();
	pDrawInfo->m_pDC->SetBkMode(TRANSPARENT);	// nötig für:
	// jede gerade Zeile grau unterlegen
	if (!(pDrawInfo->line % 2) && pDrawInfo->line != nSelected) GraueBox(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, pDrawInfo->spalte_afanr, pDrawInfo->line+1);
	
	if (pDrawInfo->zeige_betriebicon)
	{
		int i;
		int n = m_csaBetriebeNamen.GetSize();
		for (i = 0; i < n; i++)
		if (m_csaBetriebeNamen.GetAt(i) == p->Betrieb)
			{
				Icon(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIcons, atoi(m_csaBetriebeIcons.GetAt(i)));
				break;
			}
	}
	if (pDrawInfo->zeige_bestandskontoicon)
	{
		int i;
		int n = m_csaBestandskontenNamen.GetSize();
		for (i = 0; i < n; i++)
		if (m_csaBestandskontenNamen.GetAt(i) == p->Bestandskonto)
			{
				Icon(pDrawInfo, pDrawInfo->spalte_bestandskontoicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIconsBestandskonten, atoi(m_csaBestandskontenIcons.GetAt(i)));
				break;
			}
	}
	
	CEasyCashDoc* pDoc = GetDocument();
	char buf[3000];

	sprintf(buf, "%02d.%02d.%04d", p->Datum.GetDay(), p->Datum.GetMonth(), p->Datum.GetYear());
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, buf);

	if (pDrawInfo->zeige_belegnummernspalte) 
	{
		TextEx(pDrawInfo, pDrawInfo->spalte_belegnummer, pDrawInfo->line, pDrawInfo->spalte_belegnummer_ende, pDrawInfo->line, p->Belegnummer.GetBuffer(0));
	}

	TextEx(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, pDrawInfo->spalte_beschreibung_ende, pDrawInfo->line, p->Beschreibung.GetBuffer(0));

	long mwst_betrag = 0, netto = 0;
	if (p->Konto == "VST-Beträge separat")
	{
		mwst_betrag = p->Betrag;
		pDrawInfo->vst += mwst_betrag;
		pDrawInfo->brutto_summe += mwst_betrag;

		// MwSt-Betrag
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		int_to_currency_tausenderpunkt(p->Betrag, 7, buf);
		Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, buf);
	}
	else
	{
		// Netto-Betrag berechnen
		netto = p->GetBuchungsjahrNetto(pDoc);

		// Korrektur bei ungültigen AfA-Angaben
		if (p->AbschreibungNr < 1 || p->AbschreibungJahre < 1)
		{
			m_bAfaKorrketuren++;
			if (p->AbschreibungNr < 1) p->AbschreibungNr = 1;
			if (p->AbschreibungJahre < 1) p->AbschreibungJahre = 1;
		}

		// MwSt-Betrag berechnen
		if (p->AbschreibungNr == 1)	// nur im erstern Jahr Vorsteuer berechnen!
			mwst_betrag = p->Betrag - p->GetNetto();
		else
			mwst_betrag = 0;
		
		// Summen mitführen
		pDrawInfo->vst += mwst_betrag;
		if (p->AbschreibungNr == 1) 
			pDrawInfo->brutto_summe += netto + mwst_betrag;
		else
			pDrawInfo->brutto_summe += netto;
		pDrawInfo->netto_summe += netto;

		if (pDrawInfo->zeige_steuerspalte)
		{
			// Netto-Betrag
			int_to_currency_tausenderpunkt(netto, 8, buf);
			pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
			Text(pDrawInfo, pDrawInfo->spalte_netto, pDrawInfo->line, buf);

			// MwSt-Satz
			p->GetMWSt(buf);
			if (mwst_betrag < 1000000) 
				strcat(buf, "%");
			else
				strcat(buf, " ");
			Text(pDrawInfo, pDrawInfo->spalte_ust, pDrawInfo->line, buf);
			pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

			// MwSt-Betrag
			pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
			int_to_currency_tausenderpunkt(mwst_betrag, 7, buf);
			Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, buf);
		}
	}

	// Brutto-Betrag
	// if (p->AbschreibungNr == 1)	// Nur im ersten Jahr den Umsatz berechnen
	// {
	//	int_to_currency_tausenderpunkt(p->Betrag, 8, buf);
	//	Text(pDrawInfo, 72, pDrawInfo->line, buf);
	//	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
	//}
	// Immer netto + MWSt als Brutto-Summe nehmen (wg. Abschreibungen)
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(mwst_betrag + netto, 8, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	if (p->AbschreibungJahre > 1)
	{
		sprintf(buf, "%d/%d", p->AbschreibungNr, p->AbschreibungJahre);
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, buf);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
	}

	pDrawInfo->m_pDC->SetBkMode(nOldBkMode);
}

void CEasyCashView::DrawToDC_AnlagenverzeichnisLine(DrawInfo *pDrawInfo, CBuchung *p)
{
	int nOldBkMode = pDrawInfo->m_pDC->GetBkMode();
	pDrawInfo->m_pDC->SetBkMode(TRANSPARENT);	// nötig für:
	// jede gerade Zeile grau unterlegen
	if (!(pDrawInfo->line % 2) && pDrawInfo->line != nSelected) GraueBox(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, pDrawInfo->spalte_afanr, pDrawInfo->line+1);
	
	if (pDrawInfo->zeige_betriebicon)
	{
		int i;
		int n = m_csaBetriebeNamen.GetSize();
		for (i = 0; i < n; i++)
		if (m_csaBetriebeNamen.GetAt(i) == p->Betrieb)
			{
				Icon(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIcons, atoi(m_csaBetriebeIcons.GetAt(i)));
				break;
			}
	}
	
	CEasyCashDoc* pDoc = GetDocument();
	char buf[3000];
	BOOL bIstAbgang = FALSE;

	// Beschreibungstext
	TextEx(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, pDrawInfo->spalte_beschreibung_ende, pDrawInfo->line, p->Beschreibung.GetBuffer(0));

	// Korrektur bei ungültigen AfA-Angaben
	if (p->AbschreibungNr < 1 || p->AbschreibungJahre < 1)
	{
		m_bAfaKorrketuren++;
		if (p->AbschreibungNr < 1) p->AbschreibungNr = 1;
		if (p->AbschreibungJahre < 1) p->AbschreibungJahre = 1;
	}

	// Datum
	CString *pcsErweiterungswert = GetErweiterungKeyCS(p->Erweiterung, "EasyCash", "UrspruenglichesAnschaffungsdatum");	// Abgang?
	if (*pcsErweiterungswert != "")
	{
		strcpy(buf, (LPCSTR)(*pcsErweiterungswert));
		bIstAbgang = TRUE;
	}
	else
		sprintf(buf, "%02d.%02d.%04d", p->Datum.GetDay(), p->Datum.GetMonth(), p->Datum.GetYear() - p->AbschreibungNr + 1);
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, buf);
	delete pcsErweiterungswert;

	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);

	if (bIstAbgang)
	{
		// Anschaffungskosten
		pcsErweiterungswert = GetErweiterungKeyCS(p->Erweiterung, "EasyCash", "UrspruenglicherNettobetrag");
		Text(pDrawInfo, pDrawInfo->spalte_anschaffungskosten, pDrawInfo->line, (*pcsErweiterungswert).GetBuffer());
		delete pcsErweiterungswert;

		// Buchwert zu Jahresbeginn
		pcsErweiterungswert = GetErweiterungKeyCS(p->Erweiterung, "EasyCash", "UrspruenglicherRestwert");
		Text(pDrawInfo, pDrawInfo->spalte_buchwert_beginn, pDrawInfo->line, (*pcsErweiterungswert).GetBuffer());		
		pDrawInfo->buchwert_beginn_summe += currency_to_int((*pcsErweiterungswert).TrimLeft().GetBuffer());
		delete pcsErweiterungswert;

		// Abgänge
		int_to_currency_tausenderpunkt(p->GetNetto(), 8, buf);
		Text(pDrawInfo, pDrawInfo->spalte_abgaenge, pDrawInfo->line, buf);
		pDrawInfo->abgaenge_summe += p->GetNetto();

		// Buchwert zu Jahresende
		Text(pDrawInfo, pDrawInfo->spalte_buchwert_ende, pDrawInfo->line, "0,00");
	}
	else
	{
		// Anschaffungskosten
		int_to_currency_tausenderpunkt(p->GetNetto(), 8, buf);
		Text(pDrawInfo, pDrawInfo->spalte_anschaffungskosten, pDrawInfo->line, buf);

		// Buchwert zu Jahresbeginn
		int_to_currency_tausenderpunkt(p->AbschreibungRestwert, 8, buf);
		Text(pDrawInfo, pDrawInfo->spalte_buchwert_beginn, pDrawInfo->line, buf);
		pDrawInfo->buchwert_beginn_summe += p->AbschreibungRestwert;

		// Netto-AfA-Betrag 
		long netto = p->GetBuchungsjahrNetto(pDoc);	
		pDrawInfo->netto_summe += netto;	// Summe mitführen
		int_to_currency_tausenderpunkt(netto, 8, buf);
		Text(pDrawInfo, pDrawInfo->spalte_afa, pDrawInfo->line, buf);

		// Buchwert zu Jahresende
		int_to_currency_tausenderpunkt(p->AbschreibungRestwert - netto, 8, buf);
		Text(pDrawInfo, pDrawInfo->spalte_buchwert_ende, pDrawInfo->line, buf);
		pDrawInfo->buchwert_ende_summe += p->AbschreibungRestwert - netto;

		if (p->AbschreibungJahre > 1)
		{
			sprintf(buf, "%d/%d", p->AbschreibungNr, p->AbschreibungJahre);
			Text(pDrawInfo, pDrawInfo->spalte_afanr, pDrawInfo->line, buf);
		}
	}

	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->m_pDC->SetBkMode(nOldBkMode);
}

void CEasyCashView::DrawToDC_BestandskontenLine(DrawInfo *pDrawInfo, CBuchung *p, char cBuchungsart)
{
	char buf[3000];

	int nOldBkMode = pDrawInfo->m_pDC->GetBkMode();
	pDrawInfo->m_pDC->SetBkMode(TRANSPARENT);	// nötig für:
	// jede gerade Zeile grau unterlegen
	if (!(pDrawInfo->line % 2)) GraueBox(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, pDrawInfo->spalte_afanr, pDrawInfo->line+1);
	
	if (pDrawInfo->zeige_betriebicon)
	{
		int i;
		int n = m_csaBetriebeNamen.GetSize();
		for (i = 0; i < n; i++)
		if (m_csaBetriebeNamen.GetAt(i) == p->Betrieb)
			{
				Icon(pDrawInfo, pDrawInfo->spalte_betriebicon, pDrawInfo->line, pDrawInfo->line+1, &m_cbmIcons, atoi(m_csaBetriebeIcons.GetAt(i)));
				break;
			}
	}

	sprintf(buf, "%02d.%02d.%04d", 
		p->Datum.GetDay(), p->Datum.GetMonth(), p->Datum.GetYear());
	Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, buf);

	if (pDrawInfo->zeige_belegnummernspalte) 
	{
		TextEx(pDrawInfo, pDrawInfo->spalte_belegnummer, pDrawInfo->line, pDrawInfo->spalte_belegnummer_ende, pDrawInfo->line, p->Belegnummer.GetBuffer(0));
	}

	TextEx(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, pDrawInfo->spalte_ust, pDrawInfo->line, p->Beschreibung.GetBuffer(0));

	// Summen mitführen
	pDrawInfo->brutto_summe += cBuchungsart == 'e' ? p->Betrag : -(p->Betrag);

	// Netto-Betrag
	int_to_currency_tausenderpunkt(cBuchungsart == 'e' ? p->Betrag : -(p->Betrag), 8, buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	Text(pDrawInfo, pDrawInfo->spalte_ust_betrag, pDrawInfo->line, buf);

	// Brutto-Betrag
	pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
	int_to_currency_tausenderpunkt(pDrawInfo->brutto_summe, 8, buf);
	Text(pDrawInfo, pDrawInfo->spalte_brutto, pDrawInfo->line, buf);
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

	pDrawInfo->m_pDC->SetBkMode(nOldBkMode);
}

// n = folgende Zeilen, die zusammenhängend auf eine Seite sollen
void CEasyCashView::DrawToDC_LineBreak(DrawInfo *pDrawInfo, int n)
{
	if (pDrawInfo->pm)
	{
		if (((pDrawInfo->line + n + 4) * pDrawInfo->printer_charheight) > pDrawInfo->printer_gesamtgroesse.cy)
		{
			DrawToDC_PrintLineNumber(pDrawInfo);

			seitenzaehler++;

			// Ein bisschem mehr Platz oben beim Drucken
			if (pDrawInfo->pm) 
				pDrawInfo->line = 2;
			else
				pDrawInfo->line = 0;
		}
	}
}

void CEasyCashView::DrawToDC_PrintLineNumber(DrawInfo *pDrawInfo)
{
	char buf[100];
	pDrawInfo->m_pDC->SetTextAlign(TA_CENTER);
	sprintf(buf, "- Seite %d -", seitenzaehler);
	Text(pDrawInfo, HCHARS/2, (int)((float)pDrawInfo->printer_gesamtgroesse.cy / (float)pDrawInfo->printer_charheight) - 2, buf);
	if (pDrawInfo->printer_gesamtgroesse.cx < pDrawInfo->printer_gesamtgroesse.cy)
		Underline(pDrawInfo, 1, 33, 2); // Querformat: Mitte-Lochermarkierung oben
	else
		Text(pDrawInfo, HCHARS/2, 0, "|"); // Hochformat: Mitte-Lochermarkierung an der Seite
	pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
}

void CEasyCashView::DrawToDC_Datum(CDC* pDC_par, DrawInfo *pDrawInfo)
{
	char buffer[1000];
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CheckLayout(pDrawInfo);

	seitenzaehler = 1;
/*	if (!pDrawInfo->pInfo)						XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
		nCurPage = 0;
	else
		pDrawInfo->pInfo->m_nCurPage = pDrawInfo->pInfo->m_nCurPage;
*/	extern char last_path[500];

TRACE0("Enter DrawToDC_Datum\r\n");
if (pDrawInfo && pDrawInfo->pInfo)
TRACE1("pDrawInfo->pInfo->m_nCurPage: %d\r\n", (int)pDrawInfo->pInfo->m_nCurPage);

	CBuchung **pp;
	CBuchung *p;

	pDrawInfo->m_pDC = pDC_par;

	if (!pDrawInfo->pm)
		pDrawInfo->fontsize = -1 ;
	else
		pDrawInfo->printer_fontsize = -1;

	strcpy(last_path, pDoc->GetPathName());

	CTime von(pDoc->nJahr, m_vm, m_vt, 0, 0, 0);
	CTime bis(pDoc->nJahr, m_bm, m_bt, 23, 59, 59);

	if (von > bis) bis = CTime(pDoc->nJahr+1, m_bm, m_bt, 23, 59, 59);

	if (m_BetriebFilterPrinter == "") m_BetriebFilterPrinter = "<alle Betriebe>";

	if (!pDrawInfo->pm) 
	{
		int i;
		for (i = 0; i < MAX_BUCHUNGEN; i++)
			ppPosBuchungsliste[i] = NULL;
	}

	// jetzt Zeilenindex
	// Ein bisschem mehr Platz oben beim Drucken
	if (pDrawInfo->pm) 
		pDrawInfo->line = 2;
	else
		pDrawInfo->line = 0;

	pDrawInfo->line++;

	//--- Farbeinstellungen ---
	m_TextColor = GetSysColor(COLOR_WINDOWTEXT);
	if (m_TextColor == 0x00000000 && !pDrawInfo->pm)
	{
		m_TextColorEinnahmen = 0x00d03010;
		m_TextColorAusgaben  = 0x001030d0;
	}
	else
	{
		m_TextColorEinnahmen = m_TextColor;
		m_TextColorAusgaben  = m_TextColor;
	}
	pDrawInfo->m_pDC->SetTextColor(m_TextColor);

	//--- Überschrift beim Druck ---

	if (pDrawInfo->pm) 
	{
		// Titel
		sprintf(buffer, "Buchungsjournal nach Einnahmen/Ausgaben - Zeitraum: %02d.%02d.%04d - %02d.%02d.%04d",
				m_vt, m_vm, pDoc->nJahr, m_bt, m_bm, bis.GetYear());
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, buffer);
		
		// aktuelles Datum rechts an den Rand
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		CTime now = CTime::GetCurrentTime();
		sprintf(buffer, " Stand: %02d.%02d.%04d", now.GetDay(), now.GetMonth(), now.GetYear());
		Text(pDrawInfo, 78, pDrawInfo->line++, buffer);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

		// Steuerpflichtiger
		strcpy(buffer, einstellungen2->m_vorname);
		strcat(buffer, " ");
		strcat(buffer, einstellungen2->m_name);
		if (*einstellungen3->m_steuernummer)
		{
			strcat(buffer, ", Steuernummer ");
			strcat(buffer, einstellungen3->m_steuernummer);
		}
		strcat(buffer, " ");
		if (strcmp(m_BetriebFilterPrinter, "<alle Betriebe>"))
		{
			strcat(buffer, ", Betrieb: ");
			strcat(buffer, (LPCSTR)m_BetriebFilterPrinter);
		}
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);

		pDrawInfo->line++;
	}
	else if (m_KontenFilterDisplay != "<alle Konten>" || m_MonatsFilterDisplay)
	{
		if (m_KontenFilterDisplay != "<alle Konten>" && m_MonatsFilterDisplay)
		{
			if (m_MonatsFilterDisplay > 12)
				sprintf(buffer, "Buchungsjournal für %s -- %d. Quartal %04d",
					(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay - 12, pDoc->nJahr);
			else
				sprintf(buffer, "Buchungsjournal für %s -- Monat %02d/%04d",
					(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay, pDoc->nJahr);
		}
		else if (m_MonatsFilterDisplay)
		{
			if (m_MonatsFilterDisplay > 12)
				sprintf(buffer, "Buchungsjournal für %d. Quartal %04d",
					m_MonatsFilterDisplay - 12, pDoc->nJahr);
			else
				sprintf(buffer, "Buchungsjournal für Monat %02d/%04d",
					m_MonatsFilterDisplay, pDoc->nJahr);
		}
		else
			sprintf(buffer, "Buchungsjournal für %s",
				(LPCSTR)m_KontenFilterDisplay);

		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
		pDrawInfo->line++;
	}

	//--- Einnahmen -----

	if (pDrawInfo->pm || ((m_KontenFilterDisplay.Left(10) != "Ausgaben: " ) && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---"))
	{
		pDrawInfo->m_pDC->SetTextColor(m_TextColorEinnahmen);
		if (!pDrawInfo->pm)
		{
			if (m_MonatsFilterDisplay)
			{
				if (m_MonatsFilterDisplay > 12)
					sprintf(buffer, "EINNAHMEN für %d. Quartal %04d",
						m_MonatsFilterDisplay - 12, pDoc->nJahr);
				else
					sprintf(buffer, "EINNAHMEN für Monat %02d/%04d",
						m_MonatsFilterDisplay, pDoc->nJahr);
			}
			else			
				sprintf(buffer, "EINNAHMEN für %d", pDoc->nJahr);
		}
		else 
			sprintf(buffer, "EINNAHMEN");
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
		pDrawInfo->m_pDC->SetTextColor(m_TextColor);
		pDrawInfo->line++;
		if (!DrawToDC_EinnahmenHeader(pDrawInfo))
			if (!pDrawInfo->pm)
				return; // Bildaufbau abbrechen und auf redraw mit kleinerem Zoomlevel warten
		pDrawInfo->brutto_summe = 0, pDrawInfo->netto_summe = 0, pDrawInfo->ust = 0;
		for (pp = &(pDoc->Einnahmen); *pp && pDrawInfo->line < MAX_BUCHUNGEN-20; pp = &((*pp)->next))
		{
			// Darstellung beschleunigen:
			if (!pDrawInfo->pm && m_MonatsFilterDisplay)
			{
				if (m_MonatsFilterDisplay > 12)
					while (*pp && ((*pp)->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)
						pp = &((*pp)->next);
				else
					while (*pp && (*pp)->Datum.GetMonth() != m_MonatsFilterDisplay)
						pp = &((*pp)->next);
			}
			if (!pDrawInfo->pm) ppPosBuchungsliste[pDrawInfo->line] = pp;
			p = *pp;
			if (!p) break;

			// Filter für Druck
			if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
			if (pDrawInfo->pm && m_KontenFilterPrinter != "<alle Konten>" && m_KontenFilterPrinter != p->Konto) continue;
			if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;		
			// Filter für Bildschirmanzeige
			if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
			if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" && !p->Konto.GetLength())) continue;
			if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" && p->Konto.GetLength())) continue;
			if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
			if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
			if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
			if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;

			DrawToDC_LineBreak(pDrawInfo, 3);

			DrawToDC_EinnahmenLine(pDrawInfo, p);

			pDrawInfo->line++;
		}
		if (pDrawInfo->line > 0) DrawToDC_EinnahmenFooter(pDrawInfo);

		// auf jeden Fall einen Seitenwechsel zwischen Einnahmen und Ausgaben machen
		DrawToDC_LineBreak(pDrawInfo, 200);
	}

	//--- Ausgaben -----

	CTime ctStartProcessing = CTime::GetCurrentTime();	// Zeitmessung

	if (pDrawInfo->pm || ((m_KontenFilterDisplay.Left(11) != "Einnahmen: " ) && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Einnahmen] ---"))
	{
		pDrawInfo->brutto_summe = 0; pDrawInfo->netto_summe = 0;
		pDrawInfo->vst = 0;
		pDrawInfo->line++; pDrawInfo->line++;
		DrawToDC_LineBreak(pDrawInfo, 6);
		pDrawInfo->m_pDC->SetTextColor(m_TextColorAusgaben);
		if (!pDrawInfo->pm)
		{
			if (m_MonatsFilterDisplay)
			{
				if (m_MonatsFilterDisplay > 12)
					sprintf(buffer, "AUSGABEN für %d. Quartal %04d",
						m_MonatsFilterDisplay - 12, pDoc->nJahr);
				else
					sprintf(buffer, "AUSGABEN für Monat %02d/%04d",
						m_MonatsFilterDisplay, pDoc->nJahr);
			}
			else			
				sprintf(buffer, "AUSGABEN für %d", pDoc->nJahr);
		}
		else 
			sprintf(buffer, "AUSGABEN");
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
		pDrawInfo->m_pDC->SetTextColor(m_TextColor);
		pDrawInfo->line++;
		DrawToDC_AusgabenHeader(pDrawInfo);
		for (pp = &pDoc->Ausgaben; *pp && pDrawInfo->line < MAX_BUCHUNGEN-20; pp = &((*pp)->next))
		{
			// Darstellung beschleunigen:
			if (!pDrawInfo->pm && m_MonatsFilterDisplay)
			{
				if (m_MonatsFilterDisplay > 12)
					while (*pp && ((*pp)->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)
						pp = &((*pp)->next);
				else
					while (*pp && (*pp)->Datum.GetMonth() != m_MonatsFilterDisplay)
						pp = &((*pp)->next);
			}

			ppPosBuchungsliste[pDrawInfo->line] = pp;
			p = *pp;
			if (!p) break;

			// Filter für Druck
			if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
			if (pDrawInfo->pm && m_KontenFilterPrinter != "<alle Konten>" && m_KontenFilterPrinter != p->Konto) continue;
			if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;	
			// Filter für Bildschirmanzeige
			if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
			if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && !p->Konto.GetLength())) continue;
			if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && p->Konto.GetLength())) continue;
			if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
			if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
			if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
			if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;

			DrawToDC_LineBreak(pDrawInfo, 3);

			DrawToDC_AusgabenLine(pDrawInfo,  p);

			pDrawInfo->line++;
		}
		if (pDrawInfo->line > 0) DrawToDC_AusgabenFooter(pDrawInfo);
	}

	if (!pDrawInfo->pm)
	{
		int old_letzte_zeile = letzte_zeile;
		letzte_zeile = pDrawInfo->line+1;
		if (old_letzte_zeile != letzte_zeile) 
			GetDocument()->UpdateAllViews(NULL);
	}

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = seitenzaehler;
		DrawToDC_PrintLineNumber(pDrawInfo);
	}

	CTime ctStopProcessing = CTime::GetCurrentTime();
	CTimeSpan ctsDauer = ctStopProcessing - ctStartProcessing;
	int nDauer = (int)ctsDauer.GetSeconds();
TRACE2("Exit DrawToDC_Datum (brauchte %d Sekunden zur Darstellung von %d Zeilen)\r\n", nDauer, pDrawInfo->line);
}

void CEasyCashView::DrawToDC_Konten(CDC* pDC_par, DrawInfo *pDrawInfo)
{
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CheckLayout(pDrawInfo);

	seitenzaehler = 1;
/*	if (!pDrawInfo->pInfo)					XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		pDrawInfo->pInfo->m_nCurPage = 0;
	else
		pDrawInfo->pInfo->m_nCurPage = pDrawInfo->pInfo->m_nCurPage;
*/	extern char last_path[500];

	CBuchung **pp;
	CBuchung *p;
	int i, j;

	pDrawInfo->m_pDC = pDC_par;

	if (!pDrawInfo->pm)
		pDrawInfo->fontsize = -1; 
	else
		pDrawInfo->printer_fontsize = -1;

	strcpy(last_path, pDoc->GetPathName());

	if (!pDrawInfo->pm) 
	{
		int i;
		for (i = 0; i < MAX_BUCHUNGEN; i++)
			ppPosBuchungsliste[i] = NULL;
	}

	CTime von(pDoc->nJahr, m_vm, m_vt, 0, 0, 0);
	CTime bis(pDoc->nJahr, m_bm, m_bt, 23, 59, 59);

	if (von > bis) bis = CTime(pDoc->nJahr+1, m_bm, m_bt, 23, 59, 59);

	if (m_BetriebFilterPrinter == "") m_BetriebFilterPrinter = "<alle Betriebe>";

	// Zeilenindex
	// Ein bisschem mehr Platz oben beim Drucken
	if (pDrawInfo->pm) 
		pDrawInfo->line = 2;
	else
		pDrawInfo->line = 0;

	pDrawInfo->line++;

	CString einnahmen_posten_name[100];
	BOOL einnahmen_posten_buchungen[100];
	BOOL einnahmen_posten_buchungen_unzugewiesen = FALSE;
	CString ausgaben_posten_name[100];
	BOOL ausgaben_posten_buchungen[100];
	BOOL ausgaben_posten_buchungen_unzugewiesen = FALSE;

	// reguläre Konten 
	for (i = 0; i < 100; i++)
	{
		einnahmen_posten_name[i]  = einstellungen1->EinnahmenRechnungsposten[i];
		einnahmen_posten_buchungen[i] = FALSE;
		ausgaben_posten_name[i]   = einstellungen1->AusgabenRechnungsposten[i];
		ausgaben_posten_buchungen[i] = FALSE;
	}

	//--- Farbeinstellungen ---
	m_TextColor = GetSysColor(COLOR_WINDOWTEXT);
	if (m_TextColor == 0x00000000 && !pDrawInfo->pm)
	{
		m_TextColorEinnahmen = 0x00d03010;
		m_TextColorAusgaben  = 0x001030d0;
	}
	else
	{
		m_TextColorEinnahmen = m_TextColor;
		m_TextColorAusgaben  = m_TextColor;
	}
	pDrawInfo->m_pDC->SetTextColor(m_TextColor);

	//--- Überschrift beim Druck ---

	if (pDrawInfo->pm) 
	{
		char buffer[1000];

		// Titel
		sprintf(buffer, "Buchungsjournal nach Konten - Zeitraum: %02d.%02d.%04d - %02d.%02d.%04d",
				m_vt, m_vm, pDoc->nJahr, m_bt, m_bm, bis.GetYear());
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line, buffer);
		
		// aktuelles Datum rechts an den Rand
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		CTime now = CTime::GetCurrentTime();
		sprintf(buffer, " Stand: %02d.%02d.%04d", now.GetDay(), now.GetMonth(), now.GetYear());
		Text(pDrawInfo, 78, pDrawInfo->line++, buffer);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

		// Steuerpflichtiger
		strcpy(buffer, einstellungen2->m_vorname);
		strcat(buffer, " ");
		strcat(buffer, einstellungen2->m_name);
		if (*einstellungen3->m_steuernummer)
		{
			strcat(buffer, ", Steuernummer ");
			strcat(buffer, einstellungen3->m_steuernummer);
		}
		strcat(buffer, " ");
		if (strcmp(m_BetriebFilterPrinter, "<alle Betriebe>"))
		{
			strcat(buffer, ", Betrieb: ");
			strcat(buffer, (LPCSTR)m_BetriebFilterPrinter);
		}
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);

		pDrawInfo->line++;

	}
	else if (m_KontenFilterDisplay != "<alle Konten>" || m_MonatsFilterDisplay)
	{
		char buffer[1000];
		if (m_KontenFilterDisplay != "<alle Konten>" && m_MonatsFilterDisplay)
		{
			if (m_MonatsFilterDisplay > 12)
				sprintf(buffer, "Buchungsjournal für %s -- %d. Quartal %04d",
					(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay - 12, pDoc->nJahr);
			else
				sprintf(buffer, "Buchungsjournal für %s -- Monat %02d/%04d",
					(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay, pDoc->nJahr);
		}
		else if (m_MonatsFilterDisplay)
		{
			if (m_MonatsFilterDisplay > 12)
				sprintf(buffer, "Buchungsjournal für %d. Quartal %04d",
					m_MonatsFilterDisplay - 12, pDoc->nJahr);
			else
				sprintf(buffer, "Buchungsjournal für Monat %02d/%04d",
					m_MonatsFilterDisplay, pDoc->nJahr);
		}
		else
			sprintf(buffer, "Buchungsjournal für %s",
				(LPCSTR)m_KontenFilterDisplay);

		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
		pDrawInfo->line++;
	}

	//--- Konten für Einnahmen vorbereiten ---

	BOOL bDruckeUeberschrift = FALSE;
	for (p = pDoc->Einnahmen; p; p = p->next)
	{
		// Filter für Druck
		if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
		if (pDrawInfo->pm && m_KontenFilterPrinter != "<alle Konten>" && m_KontenFilterPrinter != p->Konto) continue;
		if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;		
		// Filter für Bildschirmanzeige
		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" && !p->Konto.GetLength())) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" && p->Konto.GetLength())) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---")) continue;
		if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
		if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;

		for (j = 0; j < 100; j++)
		{
			if (p->Konto.IsEmpty())
			{
				einnahmen_posten_buchungen_unzugewiesen = TRUE;
				break;
			}

			// leer? Dann neu übernehmen
			if (einnahmen_posten_name[j].IsEmpty())
			{
				einnahmen_posten_name[j] = p->Konto;
				einnahmen_posten_buchungen[j] = TRUE;
				bDruckeUeberschrift = TRUE;
				break;
			}

			if (p->Konto == einnahmen_posten_name[j])
			{
				einnahmen_posten_buchungen[j] = TRUE;
				bDruckeUeberschrift = TRUE;
				break;
			}

			if (j >= 100) 
			{
				AfxMessageBox("Zu viele Einnahmen-Konten. Bitte überflüssige Konten aus Buchungen entfernen!");			
				m_nAnzeige = 0;
				return;
			}
		}
	}

	//--- Einnahmen -----

	if (bDruckeUeberschrift)
	{
		pDrawInfo->m_pDC->SetTextColor(m_TextColorEinnahmen);
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, "EINNAHMEN");
		pDrawInfo->m_pDC->SetTextColor(m_TextColor);
	}

	for (i = 0; i < 100; i++)
	{
		if (einnahmen_posten_buchungen[i] 
			|| (einnahmen_posten_name[i].IsEmpty() 
				&& einnahmen_posten_buchungen_unzugewiesen))
		{
			pDrawInfo->line++;
			DrawToDC_LineBreak(pDrawInfo, 6);
			pDrawInfo->m_pDC->SetTextColor(m_TextColorEinnahmen);
			if (!einnahmen_posten_name[i].IsEmpty())
			{
				char buffer[1000];
				sprintf(buffer, "--- %s ---", einnahmen_posten_name[i].GetBuffer(0));
				Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
			}
			else
				Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, "--- [noch zu keinem Konto zugewiesene Einnahmen] ---");
			pDrawInfo->m_pDC->SetTextColor(m_TextColor);

			if (!DrawToDC_EinnahmenHeader(pDrawInfo))
				if (!pDrawInfo->pm)
					return; // Bildaufbau abbrechen und auf redraw mit kleinerem Zoomlevel warten
			pDrawInfo->brutto_summe = 0, pDrawInfo->netto_summe = 0, pDrawInfo->ust = 0;
			for (pp = &(pDoc->Einnahmen); *pp && pDrawInfo->line < MAX_BUCHUNGEN-20; pp = &((*pp)->next))
			{
				// Darstellung beschleunigen:
				if (!pDrawInfo->pm && m_MonatsFilterDisplay)
				{
					if (m_MonatsFilterDisplay > 12)
						while (*pp && ((*pp)->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)
							pp = &((*pp)->next);
					else
						while (*pp && (*pp)->Datum.GetMonth() != m_MonatsFilterDisplay)
							pp = &((*pp)->next);
				}
				if (!*pp) break;

				if ((*pp)->Konto == einnahmen_posten_name[i])
				{
					if (!pDrawInfo->pm) ppPosBuchungsliste[pDrawInfo->line] = pp;
					p = *pp;
					if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
					if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;	
					if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" && !p->Konto.GetLength())) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---" && p->Konto.GetLength())) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---")) continue;
					if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
					if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;
					DrawToDC_LineBreak(pDrawInfo, 3);
					DrawToDC_EinnahmenLine(pDrawInfo, p);			
					pDrawInfo->line++;
				}
			}
			if (pDrawInfo->line > 0) DrawToDC_EinnahmenFooter(pDrawInfo);

			// ggf. Seitenwechsel forcieren
			if (pDrawInfo->pm && einstellungen4->m_nach_konten_seitenumbruch) DrawToDC_LineBreak(pDrawInfo, 200);

			if (einnahmen_posten_name[i].IsEmpty()) break;
		}
	}

	//--- Konten für Ausgaben vorbereiten ---

	bDruckeUeberschrift = FALSE;
	for (p = pDoc->Ausgaben; p; p = p->next)
	{
		// Filter für Druck
		if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
		if (pDrawInfo->pm && m_KontenFilterPrinter != "<alle Konten>" && m_KontenFilterPrinter != p->Konto) continue;
		if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;	
		// Filter für Bildschirmanzeige
		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && !p->Konto.GetLength())) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && p->Konto.GetLength())) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---")) continue;
		if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
		if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;

		for (j = 0; j < 100; j++)
		{
			if (p->Konto.IsEmpty())
			{
				ausgaben_posten_buchungen_unzugewiesen = TRUE;
				break;
			}

			if (ausgaben_posten_name[j].IsEmpty())
			{
				ausgaben_posten_name[j] = p->Konto;
				ausgaben_posten_buchungen[j] = TRUE;
				bDruckeUeberschrift = TRUE;
				break;
			}

			if (p->Konto == ausgaben_posten_name[j])
			{
				ausgaben_posten_buchungen[j] = TRUE;
				bDruckeUeberschrift = TRUE;
				break;
			}

			if (j >= 100) 
			{
				AfxMessageBox("Zu viele Ausgaben-Konten. Bitte überflüssige Konten aus Buchungen entfernen!");			
				m_nAnzeige = 0;
				return;
			}
		}
	}

	//--- Ausgaben -----

	pDrawInfo->line++; pDrawInfo->line++;
	DrawToDC_LineBreak(pDrawInfo, 8);

	if (bDruckeUeberschrift)
	{
		pDrawInfo->m_pDC->SetTextColor(m_TextColorAusgaben);
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, "AUSGABEN");
		pDrawInfo->m_pDC->SetTextColor(m_TextColor);
	}

	for (i = 0; i < 100; i++)
	{
		if (ausgaben_posten_buchungen[i] 
			|| (ausgaben_posten_name[i].IsEmpty() 
				&& ausgaben_posten_buchungen_unzugewiesen))
		{
			pDrawInfo->m_pDC->SetTextColor(m_TextColorAusgaben);
			pDrawInfo->line++;
			DrawToDC_LineBreak(pDrawInfo, 6);
			if (!ausgaben_posten_name[i].IsEmpty())
			{
				char buffer[1000];
				sprintf(buffer, "--- %s ---", ausgaben_posten_name[i].GetBuffer(0));
				Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
			}
			else
				Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, "--- [noch zu keinem Konto zugewiesene Ausgaben] ---");
			pDrawInfo->m_pDC->SetTextColor(m_TextColor);

			DrawToDC_AusgabenHeader(pDrawInfo);
			pDrawInfo->brutto_summe = 0, pDrawInfo->netto_summe = 0, pDrawInfo->vst = 0;
			for (pp = &(pDoc->Ausgaben); *pp && pDrawInfo->line < MAX_BUCHUNGEN-20; pp = &((*pp)->next))
			{
				// Darstellung beschleunigen:
				if (!pDrawInfo->pm && m_MonatsFilterDisplay)
				{
					if (m_MonatsFilterDisplay > 12)
						while (*pp && ((*pp)->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)
							pp = &((*pp)->next);
					else
						while (*pp && (*pp)->Datum.GetMonth() != m_MonatsFilterDisplay)
							pp = &((*pp)->next);
				}
				if (!*pp) break;

				if ((*pp)->Konto == ausgaben_posten_name[i])
				{
					if (!pDrawInfo->pm) ppPosBuchungsliste[pDrawInfo->line] = pp;
					p = *pp;
					if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
					if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;	
					if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && !p->Konto.GetLength())) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && p->Konto.GetLength())) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---")) continue;
					if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
					if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;
					DrawToDC_LineBreak(pDrawInfo, 3);
					DrawToDC_AusgabenLine(pDrawInfo,  p);			
					pDrawInfo->line++;
				}
			}
			if (pDrawInfo->line > 0) DrawToDC_AusgabenFooter(pDrawInfo);

			// ggf. Seitenwechsel forcieren
			if (pDrawInfo->pm && einstellungen4->m_nach_konten_seitenumbruch) DrawToDC_LineBreak(pDrawInfo, 200);

			if (ausgaben_posten_name[i].IsEmpty()) break;
		}
	}

	if (!pDrawInfo->pm)
	{
		int old_letzte_zeile = letzte_zeile;
		letzte_zeile = pDrawInfo->line+1;
		if (old_letzte_zeile != letzte_zeile) 
			GetDocument()->UpdateAllViews(NULL);
	}

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = seitenzaehler;
		DrawToDC_PrintLineNumber(pDrawInfo);
	}
}

// Hilfsfunktion für DrawToDC_Bestandskonten
BOOL CEasyCashView::BestandskontoExistiertInBuchungen(CString &bestandskontoname)
{
	CEasyCashDoc* pDoc = GetDocument();

	CBuchung *liste, *p;
	for (liste = pDoc->Einnahmen; ; liste = pDoc->Ausgaben)
	{
		for (p = liste; p; p = p->next)
		{
			if (p->Bestandskonto == bestandskontoname)
				return TRUE;
		}
		if (liste == pDoc->Ausgaben) break;  // nach 2. Durchlauf raus
	}
	return FALSE;
}

void CEasyCashView::DrawToDC_Bestandskonten(CDC* pDC_par, DrawInfo *pDrawInfo)
{
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CheckLayout(pDrawInfo);

	seitenzaehler = 1;
/*	if (!pDrawInfo->pInfo)					XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		pDrawInfo->pInfo->m_nCurPage = 0;
	else
		pDrawInfo->pInfo->m_nCurPage = pDrawInfo->pInfo->m_nCurPage;
*/	extern char last_path[500];

	CBuchung *p;
	int i, j;

	pDrawInfo->m_pDC = pDC_par;

	if (!pDrawInfo->pm)
		pDrawInfo->fontsize = -1 ;
	else
		pDrawInfo->printer_fontsize = -1;

	strcpy(last_path, pDoc->GetPathName());

	if (!pDrawInfo->pm) 
	{
		int i;
		for (i = 0; i < MAX_BUCHUNGEN; i++)
			ppPosBuchungsliste[i] = NULL;
	}

	CTime von(pDoc->nJahr, m_vm, m_vt, 0, 0, 0);
	CTime bis(pDoc->nJahr, m_bm, m_bt, 23, 59, 59);

	if (von > bis) bis = CTime(pDoc->nJahr+1, m_bm, m_bt, 23, 59, 59);

	if (!pDrawInfo->pm && m_MonatsFilterDisplay)
	{
		int startmonat, endmonat;
		if (m_MonatsFilterDisplay > 12)
		{
			startmonat = (m_MonatsFilterDisplay - 12) * 3 - 2;
			endmonat = startmonat + 2;
		}
		else
		{
			startmonat = m_MonatsFilterDisplay;
			endmonat = startmonat;
		}
		von = CTime(pDoc->nJahr, startmonat, 1, 0, 0, 0);
		if (m_MonatsFilterDisplay == 12 || m_MonatsFilterDisplay == 16)
			bis = CTime(pDoc->nJahr, endmonat, 31, 23, 59, 59);
		else
			bis = CTime(pDoc->nJahr, endmonat+1, 1, 23, 59, 59) - CTimeSpan(1, 0, 0, 0); // dumme Schaltjahre... lass es CTime ausrechnen
	}

	if (!pDrawInfo->pm) 
	{
		int i;
		for (i = 0; i < MAX_BUCHUNGEN; i++)
			ppPosBuchungsliste[i] = NULL;
	}

	// Zeilenindex
	// Ein bisschem mehr Platz oben beim Drucken
	if (pDrawInfo->pm) 
		pDrawInfo->line = 2;
	else
		pDrawInfo->line = 0;

	// temporär Bestandskonten hier speichern, für die tatsächlich auch Buchungen existieren
	CString bestandskonto_name[100];
	int     bestandskonto_icon[100];
	int     bestandskonto_anfangssaldo[100];

	int bestandskonten_anzahl = 0;

	// Nur ein einzelnes Bestandskonto anzeigen?
	if ((!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "")) || (pDrawInfo->pm && (m_KontenFilterPrinter != "<alle Konten>")))
	{
		for (j = 0; j < m_csaBestandskontenNamen.GetSize(); j++)
		{
			if ((!pDrawInfo->pm && m_csaBestandskontenNamen[j] == m_BestandskontoFilterDisplay) || (pDrawInfo->pm && m_csaBestandskontenNamen[j] == m_KontenFilterPrinter))
			{
				bestandskonto_name[0] = m_csaBestandskontenNamen[j];
				bestandskonto_icon[0] = atoi(m_csaBestandskontenIcons[j]);
				bestandskonto_anfangssaldo[0] = currency_to_int(m_csaBestandskontenSalden[j].GetBuffer(0));	
				bestandskonten_anzahl = 1;
			}
		}
	}
	else
	{
		// erst einmal reguläre Bestandskonten mit der gewohnten Sortierung aus den Einstellungen inkl. Icons/Salden auflisten
		for (i = 0, bestandskonten_anzahl = 0; i < m_csaBestandskontenNamen.GetSize(); i++)
		{
			int anfangssaldo = currency_to_int(m_csaBestandskontenSalden[i].GetBuffer(0));
			if (BestandskontoExistiertInBuchungen(m_csaBestandskontenNamen[i]) || anfangssaldo != 0)
			{
				bestandskonto_name[bestandskonten_anzahl] = m_csaBestandskontenNamen[i];
				bestandskonto_icon[bestandskonten_anzahl] = atoi(m_csaBestandskontenIcons[i]);
				bestandskonto_anfangssaldo[bestandskonten_anzahl] = anfangssaldo;	
				TRACE2("\r\nreguläres Bestandskonto %d: %s", bestandskonten_anzahl, m_csaBestandskontenNamen[bestandskonten_anzahl]);
				bestandskonten_anzahl++;
			}
		}

		// danach versprengte Bestandskonten aus Buchungen zusammenkratzen 
		CBuchung *liste;
		for (liste = pDoc->Einnahmen; ; liste = pDoc->Ausgaben)
		{
			for (p = liste; p; p = p->next)
			{
				for (i = 0; i < bestandskonten_anzahl; i++)
				{
					if (p->Bestandskonto == bestandskonto_name[i])
						break;
				}

				if (i >= sizeof(bestandskonto_name)/sizeof(bestandskonto_name[0]))
				{
					Text(pDrawInfo, 3, 3, "Zu viele Bestandskonten in den Buchungen gefunden -- bitte auf < 100 reduzieren!");
					return;
				}

				if (i >= bestandskonten_anzahl)
				{
					bestandskonto_name[i] = p->Bestandskonto;

					for (j = 0; j < m_csaBestandskontenNamen.GetSize(); j++)
					{
						if (m_csaBestandskontenNamen[j] == p->Bestandskonto)
						{
							 bestandskonto_icon[i] = atoi(m_csaBestandskontenIcons[j]);
							 bestandskonto_anfangssaldo[i] = currency_to_int(m_csaBestandskontenSalden[j].GetBuffer(0));					 
							 break;
						}
					}
					if (j >= m_csaBestandskontenNamen.GetSize()) 
					{
						bestandskonto_icon[i] = -1;
						bestandskonto_anfangssaldo[i] = 0;
						TRACE2("\r\nirreguläres Bestandskonto %d: %s", bestandskonten_anzahl, m_csaBestandskontenNamen[bestandskonten_anzahl]);
					}

					bestandskonten_anzahl++;
				}
			}	
			if (liste == pDoc->Ausgaben) break;
		}
	}

	//--- Farbeinstellungen ---
	m_TextColor = GetSysColor(COLOR_WINDOWTEXT);
	if (m_TextColor == 0x00000000 && !pDrawInfo->pm)
	{
		m_TextColorEinnahmen = 0x00d03010;
		m_TextColorAusgaben  = 0x001030d0;
	}
	else
	{
		m_TextColorEinnahmen = m_TextColor;
		m_TextColorAusgaben  = m_TextColor;
	}
	pDrawInfo->m_pDC->SetTextColor(m_TextColor);

	//--- Überschrift beim Druck ---

	if (pDrawInfo->pm) 
	{
		char buffer[1000];
		sprintf(buffer, "Buchungsjournal nach Bestandskonten"/*,		  - Zeitraum: %02d.%02d.%04d - %02d.%02d.%04d
				m_vt, m_vm, pDoc->nJahr, m_bt, m_bm, bis.GetYear()*/);
		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
		pDrawInfo->line++;
	}
/* --> evtl Filter nach Bestandskonto!
	else if (m_KontenFilterDisplay != "<alle Konten>" || m_MonatsFilterDisplay)
	{
		char buffer[1000];
		if (m_KontenFilterDisplay != "<alle Konten>" && m_MonatsFilterDisplay)
			sprintf(buffer, "Buchungsjournal für %s -- Monat %02d/%04d",
				(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay, pDoc->nJahr);
		else if (m_MonatsFilterDisplay)
			sprintf(buffer, "Buchungsjournal für Monat %02d/%04d",
				m_MonatsFilterDisplay, pDoc->nJahr);
		else
			sprintf(buffer, "Buchungsjournal für %s",
				(LPCSTR)m_KontenFilterDisplay);

		Text(pDrawInfo, pDrawInfo->spalte_datum, pDrawInfo->line++, buffer);
		pDrawInfo->line++;
	}
*/
	//--- Bestandskonten anzeigen ---

	for (i = 0; i < bestandskonten_anzahl; i++)
	{
		pDrawInfo->line++; pDrawInfo->line++;
		DrawToDC_LineBreak(pDrawInfo, 8);
		pDrawInfo->m_pDC->SetTextColor(m_TextColor);
		int nSpalteUeberschrift = (!m_csaBetriebeNamen.GetSize() && m_csaBestandskontenNamen.GetSize()) ? pDrawInfo->spalte_datum + 2 : pDrawInfo->spalte_datum;	// wenn nur Bestandskonten und keine Betriebe, dann 2 Zeichen weiter nach rechts schieben, sonst überlappt das Icon des Bestandskontos
		if (bestandskonto_name[i] != "")
			Text(pDrawInfo, nSpalteUeberschrift, pDrawInfo->line++, bestandskonto_name[i].GetBuffer(0));
		else
			Text(pDrawInfo, nSpalteUeberschrift, pDrawInfo->line++, "<Buchungen mit nicht zugewiesenem Bestandskonto>");
		pDrawInfo->line++;

		CBuchung **ppBEinnahmen = &pDoc->Einnahmen;
		CBuchung **ppBAusgaben = &pDoc->Ausgaben;

		int nAnfangssaldo = bestandskonto_anfangssaldo[i];
		int nBuchungenDavor = 0;

		// Monats- bzw. Datumsfilter
		while (*ppBEinnahmen && (*ppBEinnahmen)->Datum < von)
		{
			if ((*ppBEinnahmen)->Bestandskonto == bestandskonto_name[i])
			{
				nAnfangssaldo += (*ppBEinnahmen)->Betrag;
				nBuchungenDavor++;
			}
			ppBEinnahmen = &((*ppBEinnahmen)->next);
		}

		while (*ppBAusgaben && (*ppBAusgaben)->Datum < von)
		{
			if ((*ppBAusgaben)->Bestandskonto == bestandskonto_name[i] && (*ppBAusgaben)->AbschreibungNr == 1)
			{
				nAnfangssaldo -= (*ppBAusgaben)->Betrag;
				nBuchungenDavor++;
			}
			ppBAusgaben = &((*ppBAusgaben)->next);
		}
		
		if (!DrawToDC_BestandskontenHeader(pDrawInfo, bestandskonto_icon[i], nAnfangssaldo, nBuchungenDavor, von))
			if (!pDrawInfo->pm)
				return; // Bildaufbau abbrechen und auf redraw mit kleinerem Zoomlevel warten

		while (*ppBEinnahmen || *ppBAusgaben)
		{
			// Filter für Druck
//			if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
	//		if (pDrawInfo->pm && m_KontenFilterPrinter != "<alle Konten>" && m_KontenFilterPrinter != p->Konto) continue;
			// Filter für Bildschirmanzeige
	//		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
//			if (!pDrawInfo->pm && (m_MonatsFilterDisplay && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
//			if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
//			if (!pDrawInfo->pm && (m_BestandskontoFilterDisplay != "" && p->Bestandskonto != m_BestandskontoFilterDisplay)) continue;

			if (*ppBEinnahmen && (!*ppBAusgaben || (*ppBEinnahmen)->Datum <= (*ppBAusgaben)->Datum)  && pDrawInfo->line < MAX_BUCHUNGEN-20)
			{
				if ((*ppBEinnahmen)->Datum > bis) break;

				if ((*ppBEinnahmen)->Bestandskonto == bestandskonto_name[i])
				{
					ppPosBuchungsliste[pDrawInfo->line] = ppBEinnahmen;
					pDrawInfo->m_pDC->SetTextColor(m_TextColorEinnahmen);
					DrawToDC_LineBreak(pDrawInfo, 3);
					DrawToDC_BestandskontenLine(pDrawInfo, *ppBEinnahmen, 'e');			
					pDrawInfo->line++;
				}
				
				pDrawInfo->m_pDC->SetTextColor(m_TextColor);

				ppBEinnahmen = &((*ppBEinnahmen)->next);
			}				
			else if (*ppBAusgaben)
			{
				if ((*ppBAusgaben)->Datum > bis) break;

				if ((*ppBAusgaben)->Bestandskonto == bestandskonto_name[i] && (*ppBAusgaben)->AbschreibungNr <= 1)
				{
					ppPosBuchungsliste[pDrawInfo->line] = ppBAusgaben;
					pDrawInfo->m_pDC->SetTextColor(m_TextColorAusgaben);
					DrawToDC_LineBreak(pDrawInfo, 3);
					DrawToDC_BestandskontenLine(pDrawInfo, *ppBAusgaben, 'a');			
					pDrawInfo->line++;
				}	

				ppBAusgaben = &((*ppBAusgaben)->next);
			}
		}

		pDrawInfo->m_pDC->SetTextColor(m_TextColor);

		if (pDrawInfo->line > 0) DrawToDC_BestandskontenFooter(pDrawInfo);

		// ggf. Seitenwechsel forcieren
		if (i < bestandskonten_anzahl-1)
			if (pDrawInfo->pm && einstellungen4->m_nach_konten_seitenumbruch) DrawToDC_LineBreak(pDrawInfo, 200);

	}

	if (!pDrawInfo->pm)
	{
		int old_letzte_zeile = letzte_zeile;
		letzte_zeile = pDrawInfo->line+1;
		if (old_letzte_zeile != letzte_zeile) 
			GetDocument()->UpdateAllViews(NULL);
	}

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = seitenzaehler;
		DrawToDC_PrintLineNumber(pDrawInfo);
	}
}

void CEasyCashView::DrawToDC_Anlagenverzeichnis(CDC* pDC_par, DrawInfo *pDrawInfo)
{
	char buffer[1000];
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	CheckAnlagenverzeichnisLayout(pDrawInfo);

	seitenzaehler = 1;
	extern char last_path[500];

	CBuchung **pp;
	CBuchung *p;
	int i, j;

	pDrawInfo->m_pDC = pDC_par;

	if (!pDrawInfo->pm)
		pDrawInfo->fontsize = -1 ;
	else
		pDrawInfo->printer_fontsize = -1;

	strcpy(last_path, pDoc->GetPathName());

	CTime von(pDoc->nJahr, m_vm, m_vt, 0, 0, 0);
	CTime bis(pDoc->nJahr, m_bm, m_bt, 23, 59, 59);

	if (von > bis) bis = CTime(pDoc->nJahr+1, m_bm, m_bt, 23, 59, 59);

	if (m_BetriebFilterPrinter == "") m_BetriebFilterPrinter = "<alle Betriebe>";

	if (!pDrawInfo->pm) 
	{
		int i;
		for (i = 0; i < MAX_BUCHUNGEN; i++)
			ppPosBuchungsliste[i] = NULL;
	}

	// jetzt Zeilenindex
	// Ein bisschem mehr Platz oben beim Drucken
	if (pDrawInfo->pm) 
		pDrawInfo->line = 2;
	else
		pDrawInfo->line = 0;

	pDrawInfo->line++;

	CString ausgaben_posten_name[100];
	BOOL ausgaben_posten_buchungen[100];
	BOOL ausgaben_posten_buchungen_unzugewiesen = FALSE;

	// reguläre Konten 
	for (i = 0; i < 100; i++)
	{
		ausgaben_posten_name[i]   = einstellungen1->AusgabenRechnungsposten[i];
		ausgaben_posten_buchungen[i] = FALSE;
	}

	//--- Farbeinstellungen ---
	m_TextColor = GetSysColor(COLOR_WINDOWTEXT);
	if (m_TextColor == 0x00000000 && !pDrawInfo->pm)
	{
		m_TextColorEinnahmen = 0x00d03010;
		m_TextColorAusgaben  = 0x001030d0;
	}
	else
	{
		m_TextColorEinnahmen = m_TextColor;
		m_TextColorAusgaben  = m_TextColor;
	}
	pDrawInfo->m_pDC->SetTextColor(m_TextColor);

	//--- Überschrift beim Druck ---

	if (pDrawInfo->pm) 
	{
		// Titel
		sprintf(buffer, "Anlagenverzeichnis - Zeitraum: %02d.%02d.%04d - %02d.%02d.%04d",
				m_vt, m_vm, pDoc->nJahr, m_bt, m_bm, bis.GetYear());
		Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line, buffer);
		
		// aktuelles Datum rechts an den Rand
		pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
		CTime now = CTime::GetCurrentTime();
		sprintf(buffer, " Stand: %02d.%02d.%04d", now.GetDay(), now.GetMonth(), now.GetYear());
		Text(pDrawInfo, 78, pDrawInfo->line++, buffer);
		pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);

		// Steuerpflichtiger
		strcpy(buffer, einstellungen2->m_vorname);
		strcat(buffer, " ");
		strcat(buffer, einstellungen2->m_name);
		if (*einstellungen3->m_steuernummer)
		{
			strcat(buffer, ", Steuernummer ");
			strcat(buffer, einstellungen3->m_steuernummer);
		}
		strcat(buffer, " ");
		if (strcmp(m_BetriebFilterPrinter, "<alle Betriebe>"))
		{
			strcat(buffer, ", Betrieb: ");
			strcat(buffer, (LPCSTR)m_BetriebFilterPrinter);
		}
		Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line++, buffer);

		pDrawInfo->line++;
	}
	else if (m_KontenFilterDisplay != "<alle Konten>" || m_MonatsFilterDisplay)
	{
		if (m_KontenFilterDisplay != "<alle Konten>" && m_MonatsFilterDisplay)
		{
			if (m_MonatsFilterDisplay > 12)
				sprintf(buffer, "Anlagenverzeichnis für %s -- %d. Quartal %04d",
					(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay - 12, pDoc->nJahr);
			else
				sprintf(buffer, "Anlagenverzeichnis für %s -- Monat %02d/%04d",
					(LPCSTR)m_KontenFilterDisplay, m_MonatsFilterDisplay, pDoc->nJahr);
		}
		else if (m_MonatsFilterDisplay)
		{
			if (m_MonatsFilterDisplay > 12)
				sprintf(buffer, "Anlagenverzeichnis für %d. Quartal %04d",
					m_MonatsFilterDisplay - 12, pDoc->nJahr);
			else
				sprintf(buffer, "Anlagenverzeichnis für Monat %02d/%04d",
					m_MonatsFilterDisplay, pDoc->nJahr);
		}
		else
			sprintf(buffer, "Anlagenverzeichnis für %s",
				(LPCSTR)m_KontenFilterDisplay);

		Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line++, buffer);
		pDrawInfo->line++;
	}

	//--- Konten für Ausgaben vorbereiten ---

	BOOL bDruckeUeberschrift = FALSE;
	for (p = pDoc->Ausgaben; p; p = p->next)
	{
		// Filter für Druck
		if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
		if (pDrawInfo->pm && m_KontenFilterPrinter != "<alle Konten>" && m_KontenFilterPrinter != p->Konto) continue;
		if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;	
		// Filter für Bildschirmanzeige
		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && p->Konto.GetLength() && "Einnahmen: " + p->Konto != m_KontenFilterDisplay && "Ausgaben: " + p->Konto != m_KontenFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && !p->Konto.GetLength())) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && p->Konto.GetLength())) continue;
		if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---")) continue;
		if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
		if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
		if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;

		for (j = 0; j < 100; j++)
		{
			char *cp = GetErweiterungKey(p->Erweiterung, "EasyCash", "UrspruenglichesAnschaffungsdatum");
			char *cp2 = strchr(cp, '|');
			if (p->AbschreibungJahre <= 1 && cp2 - cp != 10) continue;  // keine AfA und auch kein Abgang? dann weitersuchen

			CString csKonto;
			if (p->AbschreibungJahre == 1)	// muss dann wohl Abgang sein...
			{
				CString *pcs = GetErweiterungKeyCS(p->Erweiterung, "EasyCash", "UrspruenglichesKonto");
				csKonto = *pcs;
				delete pcs;
			}
			else
				csKonto = p->Konto;

			if (p->Konto.IsEmpty())
			{
				ausgaben_posten_buchungen_unzugewiesen = TRUE;
				break;
			}

			if (ausgaben_posten_name[j].IsEmpty())
			{
				ausgaben_posten_name[j] = csKonto;
				ausgaben_posten_buchungen[j] = TRUE;
				bDruckeUeberschrift = TRUE;
				break;
			}

			if (csKonto == ausgaben_posten_name[j])
			{
				ausgaben_posten_buchungen[j] = TRUE;
				bDruckeUeberschrift = TRUE;
				break;
			}

			if (j >= 100) 
			{
				AfxMessageBox("Zu viele Ausgaben-Konten. Bitte überflüssige Konten aus Buchungen entfernen!");			
				m_nAnzeige = 0;
				return;
			}
		}
	}

	//--- Ausgaben -----

	pDrawInfo->line++; pDrawInfo->line++;
	DrawToDC_LineBreak(pDrawInfo, 8);

	if (bDruckeUeberschrift)
	{
		pDrawInfo->m_pDC->SetTextColor(m_TextColorAusgaben);
		Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line++, "ANLAGENVERZEICHNIS");
		pDrawInfo->m_pDC->SetTextColor(m_TextColor);
	}

	for (i = 0; i < 100; i++)
	{
		if (ausgaben_posten_buchungen[i] 
			|| (ausgaben_posten_name[i].IsEmpty() 
				&& ausgaben_posten_buchungen_unzugewiesen))
		{
			pDrawInfo->m_pDC->SetTextColor(m_TextColorAusgaben);
			pDrawInfo->line++;
			DrawToDC_LineBreak(pDrawInfo, 6);
			if (!ausgaben_posten_name[i].IsEmpty())
			{
				char buffer[1000];
				sprintf(buffer, "--- %s ---", ausgaben_posten_name[i].GetBuffer(0));
				Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line++, buffer);
			}
			else
				Text(pDrawInfo, pDrawInfo->spalte_beschreibung, pDrawInfo->line++, "--- [noch zu keinem Konto zugewiesene Ausgaben] ---");
			pDrawInfo->m_pDC->SetTextColor(m_TextColor);

			DrawToDC_AnlagenverzeichnisHeader(pDrawInfo);
			pDrawInfo->buchwert_beginn_summe = 0, pDrawInfo->netto_summe = 0, pDrawInfo->buchwert_ende_summe = 0, pDrawInfo->abgaenge_summe = 0;
			for (pp = &(pDoc->Ausgaben); *pp && pDrawInfo->line < MAX_BUCHUNGEN-20; pp = &((*pp)->next))
			{
				// Darstellung beschleunigen:
				if (!pDrawInfo->pm && m_MonatsFilterDisplay)
				{
					if (m_MonatsFilterDisplay > 12)
						while (*pp && ((*pp)->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)
							pp = &((*pp)->next);
					else
						while (*pp && (*pp)->Datum.GetMonth() != m_MonatsFilterDisplay)
							pp = &((*pp)->next);
				}
				if (!*pp) break;

				CString csKonto;
				if ((*pp)->AbschreibungJahre == 1)	// muss dann wohl Abgang sein...
				{
					CString *pcs = GetErweiterungKeyCS((*pp)->Erweiterung, "EasyCash", "UrspruenglichesKonto");
					csKonto = *pcs;
					delete pcs;
				}
				else
					csKonto = (*pp)->Konto;

				if (csKonto == ausgaben_posten_name[i])
				{
					if (!pDrawInfo->pm) ppPosBuchungsliste[pDrawInfo->line] = pp;
					p = *pp;
					char *cp = GetErweiterungKey(p->Erweiterung, "EasyCash", "UrspruenglichesAnschaffungsdatum");
					char *cp2 = strchr(cp, '|');
					if (p->AbschreibungJahre <= 1 && cp2 - cp != 10) continue;  // keine AfA und auch kein Abgang? dann weitersuchen
					if (pDrawInfo->pm && (p->Datum < von || p->Datum > bis)) continue;
					if (pDrawInfo->pm && m_BetriebFilterPrinter != "<alle Betriebe>" && m_BetriebFilterPrinter != p->Betrieb) continue;	
					if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && csKonto.GetLength() && "Einnahmen: " + csKonto != m_KontenFilterDisplay && "Ausgaben: " + csKonto != m_KontenFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay != "<alle Konten>" && m_KontenFilterDisplay != "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && !csKonto.GetLength())) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Ausgaben] ---" && csKonto.GetLength())) continue;
					if (!pDrawInfo->pm && (m_KontenFilterDisplay == "--- [noch zu keinem Konto zugewiesene Einnahmen] ---")) continue;
					if (!pDrawInfo->pm && (m_MonatsFilterDisplay >= 1 && m_MonatsFilterDisplay <= 12 && p->Datum.GetMonth() != m_MonatsFilterDisplay)) continue;
					if (!pDrawInfo->pm && (m_MonatsFilterDisplay > 12 && (p->Datum.GetMonth() - 1) / 3 + 1 != m_MonatsFilterDisplay - 12)) continue;
					if (!pDrawInfo->pm && (m_BetriebFilterDisplay != "" && p->Betrieb != m_BetriebFilterDisplay)) continue;
					DrawToDC_LineBreak(pDrawInfo, 3);
					DrawToDC_AnlagenverzeichnisLine(pDrawInfo,  p);			
					pDrawInfo->line++;
				}
			}
			if (pDrawInfo->line > 0) DrawToDC_AnlagenverzeichnisFooter(pDrawInfo);

			// ggf. Seitenwechsel forcieren
			if (pDrawInfo->pm && einstellungen4->m_nach_konten_seitenumbruch) DrawToDC_LineBreak(pDrawInfo, 200);

			if (ausgaben_posten_name[i].IsEmpty()) break;
		}
	}

	if (!pDrawInfo->pm)
	{
		int old_letzte_zeile = letzte_zeile;
		letzte_zeile = pDrawInfo->line+1;
		if (old_letzte_zeile != letzte_zeile) 
			GetDocument()->UpdateAllViews(NULL);
	}

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = seitenzaehler;
		DrawToDC_PrintLineNumber(pDrawInfo);
	}

	if (!m_BestandskontoFilterDisplay.IsEmpty())
		((CMainFrame*)AfxGetMainWnd())->SetStatus("Hinweis: Der Bestandskontenfilter wird in der Anlagenverzeichnis-Ansicht ignoriert.");
}

void CEasyCashView::DrawToDC_Selection(DrawInfo *pDrawInfo, CDC *pDC)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm) return; // Selektionsbox nicht auf Drucker, nur auf Bildschirm

	if (nSelected < 0) return; // nix selektiert?

	CRect r;
	GetClientRect(r);
	r.top = nSelected * charheight;
	r.bottom = (nSelected+1) * charheight - 1;

	pDC->DrawFocusRect(&r);
}

void CEasyCashView::DrawToDC_Marker(DrawInfo *pDrawInfo, CDC *pDC, int nLine, COLORREF color)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm) return; // Selektionsbox nicht auf Drucker, nur auf Bildschirm

	if (nSelected < 0) return; // nix selektiert?

	CRect r;
	GetClientRect(r);
	r.top = nSelected * charheight;
	r.bottom = (nSelected+1) * charheight - 1;

	CBrush selection_brush(color);
	pDC->FillRect(&r, &selection_brush);
}

void CEasyCashView::RedrawSelection()
{
//	if (pDrawInfo->pm) return; // Selektionsbox nicht auf Drucker, nur auf Bildschirm
	
	int n;
	if (nSelected < 0) 
		n = -nSelected;
	else
		n = nSelected;

	CRect r;
	r.left = 0;
	r.right = 2048;
	r.top = n * charheight-1;
	r.bottom = (n+1) * charheight + 2;

	InvalidateRect(NULL);
}

//---

void CEasyCashView::DrawUmStErklaerungToDC(CDC* pDC, DrawInfo *pDrawInfo)
{
	char buffer[1000];
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;
	int jahr;

	pDC->SetTextAlign(TA_LEFT);

	seitenzaehler = 1;

	CTime von(pDoc->nJahr, m_vm, m_vt, 0, 0, 0);
	CTime bis(pDoc->nJahr, m_bm, m_bt, 23, 59, 59);

	if (von > bis) bis = CTime(pDoc->nJahr+1, m_bm, m_bt, 23, 59, 59);

	// Überschrift
	{
		jahr = pDoc->nJahr;

		if (m_vt == 1 && m_vm == 1 && m_bt == 31 && m_bm == 12)
		{
			sprintf(buffer, "USt-Erklärung für %d", jahr);
			Text(pDrawInfo, 7, 4, "=========================");
		}
		else
		{
			sprintf(buffer, "Vorläufige USt-Erklärung für den Zeitraum %02d.%02d.%04d - %02d.%02d.%04d", m_vt, m_vm, jahr, m_bt, m_bm, bis.GetYear());
			Text(pDrawInfo, 7, 4, "=================================================================");
		}
		Text(pDrawInfo, 7, 3,  buffer);
	}

	// Steuerzahler incl. Adresse
	{
		CString temp = einstellungen2->m_vorname + " " + einstellungen2->m_name;

		Text(pDrawInfo, 7, 6, temp.Left(30).GetBuffer(0));
		Text(pDrawInfo, 7, 7, einstellungen2->m_unternehmensart1.GetBuffer(0));
		Text(pDrawInfo, 7, 8, einstellungen2->m_unternehmensart2.GetBuffer(0));
		Text(pDrawInfo, 7, 9, einstellungen2->m_strasse.GetBuffer(0));
		Text(pDrawInfo, 7, 11, einstellungen2->m_plz.GetBuffer(0));
		Text(pDrawInfo, 13, 11,einstellungen2->m_ort.GetBuffer(0));
	}

	int line;
	long ust_summe_gesamt = 0, brutto_summe_gesamt = 0, netto_summe_gesamt = 0;
	long ust_summe[100000]; // pro Satz ein Zähler
	long netto_summe[100000]; // pro Satz ein Zähler
	for (i = 0; i < 100000; i++)
	{
		ust_summe[i] = netto_summe[i] = 0;
	}
	long SteuerfreieUmsaetze = 0;
	long WeitereSteuerfreieUmsaetze = 0;
	long UStID = 0;
	{
		CBuchung *p;
		for (p = pDoc->Einnahmen; p; p = p->next)
			if (p->Datum >= von && p->Datum <= bis && p->Konto[0] != '/')
			{
				// Netto- und MwSt-Betrag berechnen
				long mwst_betrag, netto;
				netto = p->GetNetto();
				mwst_betrag = p->Betrag - netto;
				
				// Summen mitführen
				if (p->MWSt >= 0 && p->MWSt < 100000)
				{
					ust_summe[p->MWSt] += mwst_betrag;
					netto_summe[p->MWSt] += netto;
					ust_summe_gesamt += mwst_betrag;
					brutto_summe_gesamt += p->Betrag;
					netto_summe_gesamt += netto;
				}

				// Feld 48 berechnen
				if (p->Konto.Right(19) == "Steuerfreie Umsätze")
					SteuerfreieUmsaetze += p->Betrag;

				// Feld 43 berechnen
				if (p->Konto.Right(46) == "Weitere Steuerfreie Umsätze mit Vorsteuerabzug")
					WeitereSteuerfreieUmsaetze += p->Betrag;

				// UST-ID-Umsätze berechnen
				if (p->Konto.Find("UST-ID") != -1)
					UStID += p->Betrag;
			}

		sprintf(buffer, "Währung: %s", (LPCSTR)pDoc->csWaehrung);
		Text(pDrawInfo, 7, 14, buffer);

		Text(pDrawInfo, 7, 16, "C. Lieferungen und sonstige Leistungen:");

		Text(pDrawInfo, 7, 18, "USt-Satz");
		Text(pDrawInfo, 7+10, 18, "Netto-Betrag");
		Text(pDrawInfo, 7+10+14, 18, "USt-Betrag");
		Text(pDrawInfo, 7+10+14+14, 18, "Brutto-Betrag");
		Box(pDrawInfo, 7, 19, 70, 19);

		for (i = 99999, line = 20; i >= 0; i--)
		{
			if (netto_summe[i])
			{
				//ust_summe[i] = netto_summe[i] = 0;
				if (i%1000)
					sprintf(buffer, "%5.3f%%", (float)i/1000);
				else
					sprintf(buffer, "%d%%", i/1000);
				Text(pDrawInfo, 7, line, buffer);
				int_to_currency_tausenderpunkt(netto_summe[i], 10, buffer);
				Text(pDrawInfo, 7+10, line, buffer);
				int_to_currency_tausenderpunkt(ust_summe[i], 10, buffer);
				Text(pDrawInfo, 7+10+14, line, buffer);
				int_to_currency_tausenderpunkt(netto_summe[i] + ust_summe[i], 10, buffer);
				Text(pDrawInfo, 7+10+14+14, line, buffer);
				line++;
			}
		}
		Box(pDrawInfo, 7, line, 70, line); line++;
		Text(pDrawInfo, 7, line, "Gesamt:");
		int_to_currency_tausenderpunkt(netto_summe_gesamt, 10, buffer);
		Text(pDrawInfo, 7+10, line, buffer);
		int_to_currency_tausenderpunkt(ust_summe_gesamt, 10, buffer);
		Text(pDrawInfo, 7+10+14, line, buffer);
		int_to_currency_tausenderpunkt(brutto_summe_gesamt, 10, buffer);
		Text(pDrawInfo, 7+10+14+14, line, buffer);
		line++; line++;
	}

	long Einfuhrumsatzsteuer = 0;
	{
		CBuchung *p;
		long brutto_summe = 0, netto_summe = 0, vst = 0;
		for (p = pDoc->Ausgaben; p; p = p->next)
			if (p->Datum >= von && p->Datum <= bis && p->Konto[0] != '/')
			{
				if (p->Konto == "VST-Beträge separat")
				{
					vst += p->Betrag;
				}
				else
				{
					// Netto-Betrag berechnen
					long mwst_betrag, netto;
					netto = p->GetBuchungsjahrNetto(pDoc);

					// MwSt-Betrag berechnen
					if (p->AbschreibungNr == 1)	// nur im erstern Jahr Vorsteuer berechnen!
						mwst_betrag = p->Betrag - p->GetNetto();
					else
						mwst_betrag = 0;

					// Summen mitführen
					vst += mwst_betrag;
					if (p->AbschreibungNr == 1) brutto_summe += p->Betrag;
					netto_summe += netto;

					// Feld 762 berechnen
					if (p->Konto.Right(19) == "Einfuhrumsatzsteuer")
						Einfuhrumsatzsteuer += p->Betrag;
				}
			}

		Text(pDrawInfo, 7, line, "D. Abziehbare Vorsteuerbeträge:");
		line++;
		Text(pDrawInfo, 7, line, "aus Rechnungen von anderen Unternehmen:");
		int_to_currency_tausenderpunkt(vst, 10, buffer);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		line++;
		Text(pDrawInfo, 7, line, "entrichtete Einfuhrumsatzsteuer:");
		int_to_currency_tausenderpunkt(Einfuhrumsatzsteuer, 10, buffer);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		line++; line++;

		Text(pDrawInfo, 7, line, "F. Berechnung der zu entrichtenden Umsatzsteuer:");
		line++; line++;
		Text(pDrawInfo, 7, line, "Verbleibender Betrag:");
		int_to_currency_tausenderpunkt(ust_summe_gesamt-vst-Einfuhrumsatzsteuer, 10, buffer);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		line++; line++;
		Text(pDrawInfo, 7, line, "Vorauszahlungssoll für das Jahr:");
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, "- _______");
		pDC->SetTextAlign(TA_LEFT);
		line++; 
		Text(pDrawInfo, 10, line, "(alle Vorauszahlungen für dieses Jahr)");		
		line++;
		
		// gemerkte Vorauszahlungen berücksichtigen
		char inibuf[1000], inijahr[20], buffer2[1000];
		CString csMonatsVorauszahlungsbetrag, csQuartalsVorauszahlungsbetrag;
		int vorauszahlung, gesamt_voranmeldungssoll = 0;
		BOOL bUeberschrift = FALSE;
		GetIniFileName(inibuf, sizeof(inibuf));
		int j;
		for (j = von.GetYear(); j <= bis.GetYear(); j++)
		{
			sprintf(inijahr, "Jahr%04d", jahr);
			for (i = von.GetMonth(); i <= bis.GetMonth(); i++)
			{
				GetUmsatzsteuervorauszahlung(i, csMonatsVorauszahlungsbetrag);
				if ((i-1) % 3 == 0)
					GetUmsatzsteuervorauszahlung((i-1) / 3 + 41, csQuartalsVorauszahlungsbetrag);
				else
					csQuartalsVorauszahlungsbetrag = "";
				if (csMonatsVorauszahlungsbetrag != "" || csQuartalsVorauszahlungsbetrag)
				{
					if (!bUeberschrift)
					{
						bUeberschrift = TRUE;
						Text(pDrawInfo, 7, line, "Gemerkte Vorauszahlungen (bzw. Erstattungen bei Minusbeträgen):");
						line++;
					}

					if (csMonatsVorauszahlungsbetrag != "")
					{
						vorauszahlung = currency_to_int(csMonatsVorauszahlungsbetrag.GetBuffer(20));
						int_to_currency_tausenderpunkt(vorauszahlung, 10, buffer);
						gesamt_voranmeldungssoll += vorauszahlung;
						if (vorauszahlung >= 0)
							sprintf(buffer2, "Vorauszahlung für Monat %d:", i);
						else
							sprintf(buffer2, "Erstattung    für Monat %d:", i);
						Text(pDrawInfo, 10, line, buffer2);
						pDC->SetTextAlign(TA_RIGHT);
						Text(pDrawInfo, 55, line, buffer);
						pDC->SetTextAlign(TA_LEFT);
						line++;
					}

					if (csQuartalsVorauszahlungsbetrag != "")
					{
						vorauszahlung = currency_to_int(csQuartalsVorauszahlungsbetrag.GetBuffer(20));
						int_to_currency_tausenderpunkt(vorauszahlung, 10, buffer);
						gesamt_voranmeldungssoll += vorauszahlung;
						if (vorauszahlung >= 0)
							sprintf(buffer2, "Vorauszahlung für Quartal %d:", (i-1) / 3 + 1);
						else
							sprintf(buffer2, "Erstattung    für Quartal %d:", (i-1) / 3 + 1);
						Text(pDrawInfo, 10, line, buffer2);
						pDC->SetTextAlign(TA_RIGHT);
						Text(pDrawInfo, 55, line, buffer);
						pDC->SetTextAlign(TA_LEFT);
						line++;
					}
				}
			}
		}
		if (bUeberschrift) // Endsumme
		{
				pDC->SetTextAlign(TA_RIGHT);
				Text(pDrawInfo, 55, line, "===========");
				pDC->SetTextAlign(TA_LEFT);
				line++;
			
				int_to_currency_tausenderpunkt(gesamt_voranmeldungssoll, 11, buffer);
				Text(pDrawInfo, 10, line, "Summe aller gemerkten Vorauszahlungen:");
				pDC->SetTextAlign(TA_RIGHT);
				Text(pDrawInfo, 55, line, buffer);
				pDC->SetTextAlign(TA_LEFT);
				line++;
				line++;
		}

		Text(pDrawInfo, 7, line, "Noch zu entrichten/Erstattungsanspruch:");
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, "= _______");
		pDC->SetTextAlign(TA_LEFT);
		line++;
	}

	// Anlage UR
	if (UStID / 100 || SteuerfreieUmsaetze / 100 || WeitereSteuerfreieUmsaetze / 100)
	{
		line++; line++;	
		Text(pDrawInfo, 7, line, "Anlage UR:");
		line++;
		Text(pDrawInfo, 7, line, "Innergemeinschaftliche Lieferungen an Abnehmer mit USt-ID:");
		sprintf(buffer, "%d", UStID / 100);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		line++;

		Text(pDrawInfo, 7, line, "Weitere Steuerfreie Umsätze mit Vorsteuerabzug:");
		sprintf(buffer, "%d", WeitereSteuerfreieUmsaetze / 100);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		line++;

		Text(pDrawInfo, 7, line, "Steuerfreie Umsätze ohne Vorsteuerabzug:");
		sprintf(buffer, "%d", SteuerfreieUmsaetze / 100);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		line++;
	}

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = 1;
	}
}

void CEasyCashView::DrawEURechungToDC(CDC* pDC, DrawInfo *pDrawInfo)
{
	char buffer[1000];
	char buffer_unterstrich[1000];
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i, j;
	seitenzaehler = 1;
	int jahr;

	/* rätselhafter Code:
	if (!pDrawInfo->pInfo)
		pDrawInfo->pInfo->m_nCurPage = 0;
	else
		pDrawInfo->pInfo->m_nCurPage = pDrawInfo->pInfo->m_nCurPage;
	*/

	int nSpaltenzahl = 72;
	int nZeilenzahl = 63;
	if (pDrawInfo->printer_gesamtgroesse.cx > pDrawInfo->printer_gesamtgroesse.cy)  // Querformat?
	{
		nSpaltenzahl = nSpaltenzahl * 150 / 100; 
		nZeilenzahl = nZeilenzahl * 100 / 150;
	}

	pDC->SetTextAlign(TA_LEFT);

	CTime von(pDoc->nJahr, m_vm, m_vt, 0, 0, 0);
	CTime bis(pDoc->nJahr, m_bm, m_bt, 23, 59, 59);

	if (von > bis) bis = CTime(pDoc->nJahr+1, m_bm, m_bt, 23, 59, 59);

	if (m_BetriebFilterPrinter == "<alle Betriebe>") m_BetriebFilterPrinter = "";

	// Überschrift
	{
		char rechnungsname[1000];
		jahr = pDoc->nJahr;
		if (!strcmp(GetLandeskuerzel(), "AT"))
			strcpy(rechnungsname, "Einnahmen-Ausgabenrechnung");
		else
			strcpy(rechnungsname, "Einnahmen-Überschussrechnung");

		if (m_BetriebFilterPrinter != "")
		{
			strcat(rechnungsname, " '");
			strcat(rechnungsname, m_BetriebFilterPrinter.GetBuffer(0));
			strcat(rechnungsname, "' ");
			m_BetriebFilterPrinter.ReleaseBuffer();
		}

		if (m_vt == 1 && m_vm == 1 && m_bt == 31 && m_bm == 12)
		{
			sprintf(buffer, "%s für %d", rechnungsname, jahr);
			strcpy(buffer_unterstrich, "=====================================");
		}
		else
		{
			sprintf(buffer, "%s für den Zeitraum %02d.%02d.%04d - %02d.%02d.%04d", 
				rechnungsname, m_vt, m_vm, jahr, m_bt, m_bm, bis.GetYear());
			strcpy(buffer_unterstrich, "=====================================================================");
		}
		Text(pDrawInfo, 7, 3,  buffer);
		for (i = 0; i < 4 + m_BetriebFilterPrinter.GetLength(); i++)
			strcat(buffer_unterstrich, "=");
		Text(pDrawInfo, 7, 4, buffer_unterstrich);
	}

	// Steuerzahler incl. Adresse
	{
		CString temp = einstellungen2->m_vorname + " " + einstellungen2->m_name;

		Text(pDrawInfo, 7, 6, temp.Left(30).GetBuffer(0));
		Text(pDrawInfo, 7, 7, einstellungen2->m_unternehmensart1.GetBuffer(0));
		Text(pDrawInfo, 7, 8, einstellungen2->m_unternehmensart2.GetBuffer(0));
		Text(pDrawInfo, 7, 9, einstellungen2->m_strasse.GetBuffer(0));
		Text(pDrawInfo, 7, 11, einstellungen2->m_plz.GetBuffer(0));
		Text(pDrawInfo, 13, 11,einstellungen2->m_ort.GetBuffer(0));
		
		Text(pDrawInfo, 60, 6, "Steuernummer");
		CString csSteuernummer;
		if (m_BetriebFilterPrinter != "")
		{
			char inifile[1000], betriebe[1000]; 
			GetIniFileName(inifile, sizeof(inifile)); 
			CString csKey;
			int iBetriebe;
			for (iBetriebe = 0; iBetriebe < 100; iBetriebe++)
			{
				csKey.Format("Betrieb%02dName", iBetriebe);
				GetPrivateProfileString("Betriebe", csKey, "", betriebe, sizeof(betriebe), inifile);
				if (!*betriebe) 
				{ 
					csSteuernummer = ""; 
					break; 
				}
				else if (!strcmp(betriebe, m_BetriebFilterPrinter)) 
				{
					csKey.Format("Betrieb%02dUnternehmensart", iBetriebe);
					GetPrivateProfileString("Betriebe", csKey, "", betriebe, sizeof(betriebe), inifile);
					char *cp = strchr(betriebe, '\t');	// Unternehmensart1, Unternehmensart2 (Rechtsform) und Steuernummer sind durch Tabs getrennt
					if (cp) cp = strchr(cp+1, '\t');
					if (!cp || cp[1] == '\0' || cp[1] == '\t')
					{
						csSteuernummer = ""; 
					}
					else
					{
						csSteuernummer = ++cp;
					}
					break;
				}
			}
			if (csSteuernummer == "")  // keine Betriebssteuernummer gefunden? dann Hauptsteuernummer benutzen
				csSteuernummer = einstellungen3->m_steuernummer.GetBuffer(0);
		}
		else
			einstellungen3->m_steuernummer.GetBuffer(0);
		Text(pDrawInfo, 60, 7, csSteuernummer.GetBuffer(0));
	}

	int line = 0;
	int indent;
	int unterkategorie_nummerierung;

	long einnahmen_gesamtsumme = 0;
	long einnahmen_mwst_summe = 0;
	long einnahmen_posten_summe[101];	// pro E/Ü-Rechnungsposten ein Zähler (Einnahmen)
	CString einnahmen_posten_name[101];
	long ausgaben_gesamtsumme = 0;
	long ausgaben_mwst_summe = 0;
	long ausgaben_posten_summe[101];	// pro E/Ü-Rechnungsposten ein Zähler (Ausgaben)
	CString ausgaben_posten_name[101];

	for (i = 0; i < 100; i++)
	{
		einnahmen_posten_name[i]  = einstellungen1->EinnahmenRechnungsposten[i];
		einnahmen_posten_summe[i] = 0;
		ausgaben_posten_name[i]   = einstellungen1->AusgabenRechnungsposten[i];
		ausgaben_posten_summe[i]  = 0;
	}

	// Einnahmen-Rechnungsposten aufsummieren
	{
		CBuchung *p;
		for (p = pDoc->Einnahmen; p; p = p->next)
		{
			ASSERT(!p->Konto.IsEmpty());

			if (p->Datum < von || p->Datum > bis) continue;
			if (!m_BetriebFilterPrinter.IsEmpty() && p->Betrieb != m_BetriebFilterPrinter) continue;	// nur Buchungen für gewählten Betrieb aufsummieren

			for (j = 0; j < 101; j++)
			{
				if (einnahmen_posten_name[j].IsEmpty())
					einnahmen_posten_name[j] = p->Konto;

				if (p->Konto == einnahmen_posten_name[j])
				{
					// Netto- und MwSt-Betrag berechnen
					long mwst_betrag, netto;
					netto = p->GetNetto();
					mwst_betrag = p->Betrag - netto;

					if (einstellungen4->m_ustvst_gesondert)
					{
						if (p->Konto[0] != '/')  // neutrale Konten ignorieren
						{
							einnahmen_posten_summe[j] += netto;
							einnahmen_gesamtsumme += p->Betrag;
						}
						einnahmen_mwst_summe += mwst_betrag;		// wird zum Schluß in ein Konto UST geschrieben
					}
					else
					{
						if (p->Konto[0] != '/')  // neutrale Konten ignorieren
						{
							einnahmen_posten_summe[j] += p->Betrag;
							einnahmen_gesamtsumme += p->Betrag;
						}
						else
						{
							einnahmen_posten_summe[j] += mwst_betrag;
							einnahmen_gesamtsumme += mwst_betrag;
						}
					}

					break;
				}

				if (j >= 100) 
				{
					Text(pDrawInfo, 6, 15, "--> FEHLER: Zu viele Konten in der E/Ü-Rechnung! Bitte reduzieren Sie die Anzahl Ihrer Konten auf unter 100. <--");
					return;
				}
			}
		}

		if (einstellungen4->m_ustvst_gesondert || !einnahmen_mwst_summe)
			Text(pDrawInfo, 7, 14, "1. Betriebseinnahmen");
		else
			Text(pDrawInfo, 7, 14, "1. Betriebseinnahmen (einschl. Umsatzsteuer)");

		if (einstellungen4->m_ustvst_gesondert)
		{
			for (j = 0; j < 101; j++)
				if (einnahmen_posten_name[j].IsEmpty()) break;

			if (j >= 100) 
			{
				Text(pDrawInfo, 6, 15, "--> FEHLER: Zu viele Konten in der E/Ü-Rechnung! Bitte reduzieren Sie die Anzahl Ihrer Konten auf unter 100. <--");
				return;
			}

			einnahmen_posten_summe[j] = einnahmen_mwst_summe;
			einnahmen_posten_name[j] = "UST";
		}

		for (i = 0, line = 15, indent = 10, unterkategorie_nummerierung = 1; i < 100; i++)
		{		
			if (!einstellungen1->EinnahmenUnterkategorien[i].IsEmpty())	// Unterkategorien vorhanden?
			{
				indent = 13;
				CString csUnterkategorietext;
				csUnterkategorietext.Format("1.%d %s", unterkategorie_nummerierung++, einstellungen1->EinnahmenUnterkategorien[i].GetBuffer(0));
				Text(pDrawInfo, 10, ++line, csUnterkategorietext.GetBuffer(0));
				line++;
			}

			if (!einnahmen_posten_name[i].IsEmpty() && einnahmen_posten_summe[i] != 0)
			{
				if (ausgaben_posten_name[i][0] == '/') ausgaben_posten_name[i] += " (nur UST)";
				if (einnahmen_posten_name[i].GetLength() > nSpaltenzahl) 
					einnahmen_posten_name[i] = einnahmen_posten_name[i].Left(nSpaltenzahl) + "...";
				Text(pDrawInfo, indent, line, einnahmen_posten_name[i].GetBuffer(0));

				int_to_currency_tausenderpunkt(einnahmen_posten_summe[i], 10, buffer);
				pDC->SetTextAlign(TA_RIGHT);
				Text(pDrawInfo, 70, line, buffer);
				pDC->SetTextAlign(TA_LEFT);
				sprintf(buffer, " %s", (LPCSTR)pDoc->csWaehrung);
				Text(pDrawInfo, 70, line, buffer);

				line++;

				if (line > 65)
				{
					pDC->EndPage();
					pDC->StartPage();
					line = 3;
					seitenzaehler++;
				}
			}
		}

		Box(pDrawInfo, 6, line, 71+strlen((LPCSTR)pDoc->csWaehrung), line); line++;
		Text(pDrawInfo, 7, line, "Summe");
		int_to_currency_tausenderpunkt(einnahmen_gesamtsumme, 11, buffer);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		sprintf(buffer, " %s", (LPCSTR)pDoc->csWaehrung);
		Text(pDrawInfo, 70, line, buffer);

		if (line > nZeilenzahl - 3)
		{
			pDC->EndPage();
			pDC->StartPage();
			seitenzaehler++;
			line = 3;
		}
		else
			line+=2;

	}

	// Ausgaben-Rechnungsposten aufsummieren
	{
		CBuchung *p;
		for (p = pDoc->Ausgaben; p; p = p->next)
		{
			ASSERT(!p->Konto.IsEmpty());

			if (p->Datum < von || p->Datum > bis) continue;
			if (!m_BetriebFilterPrinter.IsEmpty() && p->Betrieb != m_BetriebFilterPrinter) continue;	// nur Buchungen für gewählten Betrieb aufsummieren

			if (p->Konto == "VST-Beträge separat" && einstellungen4->m_ustvst_gesondert)	// separate VST einfach nur mitzählen, weil wir sowieso ein VST-Konto haben zum Schluss
			{
				ausgaben_mwst_summe += p->Betrag;
				ausgaben_gesamtsumme += p->Betrag;
			}
			else 
			{
				for (j = 0; j < 101; j++)
				{
					if (ausgaben_posten_name[j].IsEmpty())
						ausgaben_posten_name[j] = p->Konto;

					if (p->Konto == ausgaben_posten_name[j])
					{
						// Netto-Betrag berechnen
						long mwst_betrag, netto;
						netto = p->GetBuchungsjahrNetto(pDoc);

						// MwSt-Betrag berechnen
						if (p->AbschreibungNr == 1)	// nur im erstern Jahr Vorsteuer berechnen!
							mwst_betrag = p->Betrag - p->GetNetto();
						else
							mwst_betrag = 0;

						if (einstellungen4->m_ustvst_gesondert)
						{
							if (p->Konto[0] != '/')  // neutrale Konten ignorieren
							{
								ausgaben_posten_summe[j] += netto;
								ausgaben_gesamtsumme += netto + mwst_betrag;
							}
							ausgaben_mwst_summe += mwst_betrag;		// wird zum Schluß in ein Konto VST geschrieben
						}
						else
						{
							if (p->Konto[0] != '/')  // neutrale Konten ignorieren
							{
								ausgaben_posten_summe[j] += netto + mwst_betrag;
								ausgaben_gesamtsumme += netto + mwst_betrag;
							}
							else
							{
								ausgaben_posten_summe[j] += mwst_betrag;
								ausgaben_gesamtsumme += mwst_betrag;
							}
						}

						break;
					}

					if (j >= 100) 
					{
						Text(pDrawInfo, 6, 15, "--> FEHLER: Zu viele Konten in der E/Ü-Rechnung! Bitte reduzieren Sie die Anzahl Ihrer Konten auf unter 100. <--");
						return;
					}
				}
			}
		}

		Text(pDrawInfo, 7, line, "2. Betriebsausgaben");
		line++;

		if (einstellungen4->m_ustvst_gesondert)
		{
			for (j = 0; j < 101; j++)
				if (ausgaben_posten_name[j].IsEmpty()) break;

			if (j >= 100) 
			{
				Text(pDrawInfo, 6, 15, "--> FEHLER: Zu viele Konten in der E/Ü-Rechnung! Bitte reduzieren Sie die Anzahl Ihrer Konten auf unter 100. <--");
				return;
			}

			ausgaben_posten_summe[j] = ausgaben_mwst_summe;
			ausgaben_posten_name[j] = "VST";
		}

		for (i = 0, indent = 10, unterkategorie_nummerierung = 1; i < 100; i++)
		{
			if (!einstellungen1->AusgabenUnterkategorien[i].IsEmpty())	// Unterkategorien vorhanden?
			{
				indent = 13;
				CString csUnterkategorietext;
				csUnterkategorietext.Format("2.%d %s", unterkategorie_nummerierung++, einstellungen1->AusgabenUnterkategorien[i].GetBuffer(0));
				Text(pDrawInfo, 10, ++line, csUnterkategorietext.GetBuffer(0));
				line++;
			}

			if (!ausgaben_posten_name[i].IsEmpty() && ausgaben_posten_summe[i] != 0)
			{
				if (ausgaben_posten_name[i][0] == '/') ausgaben_posten_name[i] += " (nur VST)";
				if (ausgaben_posten_name[i].GetLength() > nSpaltenzahl) 
					ausgaben_posten_name[i] = ausgaben_posten_name[i].Left(nSpaltenzahl) + "...";
				Text(pDrawInfo, indent, line, ausgaben_posten_name[i].GetBuffer(0));

				int_to_currency_tausenderpunkt(ausgaben_posten_summe[i], 10, buffer);
				pDC->SetTextAlign(TA_RIGHT);
				Text(pDrawInfo, 70, line, buffer);
				pDC->SetTextAlign(TA_LEFT);
				sprintf(buffer, " %s", (LPCSTR)pDoc->csWaehrung);
				Text(pDrawInfo, 70, line, buffer);

				line++;

				if (line > nZeilenzahl)
				{
					pDC->EndPage();
					pDC->StartPage();
					line = 3;
					seitenzaehler++;
				}
			}
		}

		// Ausgaben Gesamtsumme
		Box(pDrawInfo, 6, line, 71+strlen((LPCSTR)pDoc->csWaehrung), line); line++;
		Text(pDrawInfo, 7, line, "Summe");
		int_to_currency_tausenderpunkt(ausgaben_gesamtsumme, 11, buffer);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		sprintf(buffer, " %s", (LPCSTR)pDoc->csWaehrung);
		Text(pDrawInfo, 70, line, buffer);
		line++;

		// Gewinn/Verlust
		Box(pDrawInfo, 6, line, 71+strlen((LPCSTR)pDoc->csWaehrung), line+2); line++;
		Text(pDrawInfo, 7, line, "Gewinn/Verlust");
		int_to_currency_tausenderpunkt(einnahmen_gesamtsumme - ausgaben_gesamtsumme, 11, buffer);
		pDC->SetTextAlign(TA_RIGHT);
		Text(pDrawInfo, 70, line, buffer);
		pDC->SetTextAlign(TA_LEFT);
		sprintf(buffer, " %s", (LPCSTR)pDoc->csWaehrung);
		Text(pDrawInfo, 70, line, buffer);
		line++;
	}

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = seitenzaehler;
	}
}

void CEasyCashView::BerechneFormularfeldwerte()
{
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (m_csaFormulare.GetSize())
		pDoc->BerechneFormularfeldwerte(m_csaFormulare[m_GewaehltesFormular], m_csaFormularfeldwerte, NULL, NULL, m_csaFormularfilter[m_GewaehltesFormular]);
}

void CEasyCashView::DrawFormularToDC(CDC* pDC, DrawInfo *pDrawInfo)
{
	CEasyCashDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	int i;

	extern char last_path[500];

//	for (i = 0; i < m_csaFormularnamen.GetCount(); i++)
//	{
//		TRACE3("%02d: %s // %s\r\n", i, m_csaFormularnamen[i], m_csaFormulare[i]);
//	}

	if (!m_csaFormulare.GetSize()) return;
	// Formulardefinitionsdatei in xmldoc laden
	XDoc xmldoc;
	xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
	LPXNode xml = xmldoc.GetRoot();
	if (!xml)
	{
		char buffer[1000];
		strcpy(buffer, "Formular '");
		strcat(buffer, m_csaFormulare[m_GewaehltesFormular]);
		strcat(buffer, "' konnte nicht gefunden werden.");
		Text(pDrawInfo, 7, 3, buffer);
		return;
	}
	LPXNode felder = NULL;
	LPXNode seiten = NULL;
	felder = xml->Find("felder");
	seiten = xml->Find("seiten");
	CString csAnzahlSeiten = xml->GetAttrValue("seiten");
	int anzahl_seiten = atoi(csAnzahlSeiten);
	CString schriftart = xml->GetAttrValue("schriftart");
	CString csSchriftgroesse = xml->GetAttrValue("schriftgroesse");
	int schriftgroesse = atoi(csSchriftgroesse) * m_zoomfaktor / 100;
	CString csQuerformat = xml->GetAttrValue("querformat");
	pDrawInfo->querformat = atoi(csQuerformat);
	querformat_faktor = pDrawInfo->querformat && !pDrawInfo->pm ? 2 : 1;	// horizontale hochskalieren bei Querformat
	letzte_spalte = 0;

	// Wichtig für das Printer-Mainframe um die Anzahl Seiten zu ermitteln
	if (pDrawInfo->pInfo)
	{
		max_seitenzahl = anzahl_seiten;
	}

	letzte_zeile = (VCHARS + PAGE_GAP) * anzahl_seiten;
	SetupScroll();

	if (m_nFeldMove < 0) 
		for (page = 0; page < anzahl_seiten; page++)
		{
			if (seiten)
			{
				LPXNode child;
				for(int i = 0; i < seiten->GetChildCount(); i++)
				{
					child = seiten->GetChild(i);
					if (child && page+1 == atoi(child->GetAttrValue("nr")) && (!pDrawInfo->pInfo || pDrawInfo->pInfo->m_nCurPage == page+1))	// Nur bei richtiger Seitennummer
					{
						if (!child->value.IsEmpty())
						{
							char buffer[1000];
							GetProgrammverzeichnis(buffer, sizeof(buffer));
							strcat(buffer, "\\");
							strcat(buffer, child->value);
							if (pDrawInfo->pInfo)
								seitenzaehler = pDrawInfo->pInfo->m_nCurPage;
							else 
								seitenzaehler = 0;
							Image2(pDrawInfo, buffer);
						}
					}
				}
			}
		}
	
	pDC->SetBkMode(TRANSPARENT);

	// Wenn gerade ein Feld verändert wird: Positionsmarke zeigen
	if (m_nFeldMove >= 0 || (pFormularfeldDlg && pFormularfeldDlg->IsWindowVisible()))
	{
		CBrush brush;
		brush.CreateSolidBrush(RGB(0xc0,0x10,0x00));
		CRect r;
		int feldmarkenhoehe;
		if (schriftgroesse) 
			feldmarkenhoehe = schriftgroesse;
		else
			feldmarkenhoehe = charheight;
		if (!pDrawInfo->pm)
		{
			if (m_bFeldmarkeRechtsbuendig)	// rechtsbündig
			{
				r.left = (int)((double)ptFeldmarke.x * (charheight * (VCHARS + PAGE_GAP) * querformat_faktor * 1000 / 1414) / 1000 - charheight * 5);
				r.top = (int)((double)ptFeldmarke.y * ((VCHARS + PAGE_GAP) * charheight) / 1414 - feldmarkenhoehe / 2);
				r.right = (int)((double)ptFeldmarke.x * (charheight * (VCHARS + PAGE_GAP) * querformat_faktor * 1000 / 1414) / 1000);
				r.bottom = (int)((double)ptFeldmarke.y * ((VCHARS + PAGE_GAP) * charheight) / 1414 + feldmarkenhoehe / 2);
			}
			else									// linksbündig
			{
				r.left = (int)((double)ptFeldmarke.x * (charheight * (VCHARS + PAGE_GAP) * querformat_faktor * 1000 / 1414) / 1000);
				r.top = (int)((double)ptFeldmarke.y * ((VCHARS + PAGE_GAP) * charheight) / 1414 - feldmarkenhoehe / 2);
				r.right = (int)((double)ptFeldmarke.x * (charheight * (VCHARS + PAGE_GAP) * querformat_faktor * 1000 / 1414) / 1000 + charheight * 5);
				r.bottom = (int)((double)ptFeldmarke.y * ((VCHARS + PAGE_GAP) * charheight) / 1414 + feldmarkenhoehe / 2);
			}
			pDC->FrameRect(&r, &brush);
		}
	}

	// während Feldmarken-Drag keine Inhalte anzeigen
	if (m_nFeldMove >= 0) return;

	// Feldwerte einblenden
	if (felder)
	{
		// Font
		CFont *standardFont = pDC->GetCurrentFont();
		LOGFONT FontStats;
		CFont neuerFont;	// Font kreieren
		CFont *oldFont;

		if (!schriftart.IsEmpty())
		{
			memset(&FontStats, 0x00, sizeof(FontStats));
			standardFont->GetLogFont(&FontStats);
			strcpy(FontStats.lfFaceName, (LPCSTR)schriftart); 
			if (pDrawInfo->pm) schriftgroesse = (int)((float)schriftgroesse * pDrawInfo->printer_charheight / charheight * 1000 / (pDrawInfo->querformat ? 1414 : 1000));
			FontStats.lfHeight  = schriftgroesse > 0 ? schriftgroesse : charheight;
			FontStats.lfWidth   = 0;	// Breite relativ zur Höhe verkleinern
			FontStats.lfQuality = pDrawInfo->pm ? PROOF_QUALITY : DEFAULT_QUALITY;
			neuerFont.CreateFontIndirect(&FontStats);
			oldFont = pDC->SelectObject(&neuerFont);	// in den device context damit
		}

		LPXNode child;
		CString csVeraltetMessage;
		for(i = 0; i < felder->GetChildCount(); i++)
		{
			int nID;
			int nSeite;			////// Position
			int nHorizontal;	////
			int nVertikal;		//
			int nVeraltet;
			LPCTSTR cpName;

			child = felder->GetChild(i);
			
			if (child && !child->value.IsEmpty())
			{
				// Hier kommt der Feldinhalt hinein:
				CString csFeldinhalt = m_csaFormularfeldwerte[i];


				nID = atoi(child->GetAttrValue("id"));
				cpName = child->GetChildValue("name");
				nSeite = atoi(child->GetAttrValue("seite"));			////// Position
				nHorizontal = atoi(child->GetAttrValue("horizontal")) * querformat_faktor;	////
				nVertikal = atoi(child->GetAttrValue("vertikal"));		//
				if ((CString)child->GetAttrValue("veraltet") == "ja")  // wenn Formularfeld als veraltet markiert ist: nicht anzeigen und Warnung geben, wenn E/A-Typ und != 0
					nVeraltet = 1;
				else
					nVeraltet = 0;

				// ggf. Message über veraltete Felder aufbereiten
				if (nVeraltet && csFeldinhalt.GetLength()) 
				{
					if (csVeraltetMessage.GetLength())
						csVeraltetMessage += ", ";
					csVeraltetMessage += "Feld " + (CString)child->GetAttrValue("id") + " (" + cpName + ") mit " + csFeldinhalt;
				}

				// gerade bearbeitetes Feld?
				if (pFormularfeldDlg && pFormularfeldDlg->IsWindowVisible() && pFormularfeldDlg->m_id == nID) 				
				{
					// dann Daten aus dem 'Feld bearbeiten' Dialog entnehmen
					nSeite =      pFormularfeldDlg->m_seite;
					nHorizontal = pFormularfeldDlg->m_horizontal * querformat_faktor;
					nVertikal =   pFormularfeldDlg->m_vertikal; 
				}

				if (!pDrawInfo->pInfo || !pDrawInfo->pInfo->m_nCurPage || pDrawInfo->pInfo->m_nCurPage == nSeite)
				{
					LPCTSTR cp = child->GetAttrValue("ausrichtung");
					if (cp && !stricmp(cp,"linksbuendig"))
						pDC->SetTextAlign(TA_LEFT);
					else // if (child->GetAttrValue("ausrichtung").CompareNoCase("rechtsbuendig"))
						pDC->SetTextAlign(TA_RIGHT);

					// von Tausendstel-Seitenbreite bzw. -länge auf logische units umrechnen:
					if (pDrawInfo->pm)
					{
						nHorizontal = nHorizontal * pDrawInfo->printer_gesamtgroesse.cx / 1000;
						nVertikal = nVertikal * pDrawInfo->printer_gesamtgroesse.cy / 1414;
						nVertikal -= (int)pDrawInfo->printer_charheight / 2;
					}
					else
					{
						nHorizontal = (int)((double)nHorizontal * (charheight * (VCHARS + PAGE_GAP) * 1000 / 1414) / 1000);
						nVertikal = (int)((double)nVertikal * ((VCHARS + PAGE_GAP) * charheight) / 1414);
						nVertikal += ((nSeite-1) * ((VCHARS + PAGE_GAP) * charheight)) - charheight / 2;
					}

					if (m_bFormularfelderAnzeigen && csFeldinhalt == "") csFeldinhalt = "0,00"; // dummy wert eintragen, um besser positionieren zu können
					pDC->TextOut(nHorizontal, nVertikal, csFeldinhalt);
					if (letzte_spalte < nHorizontal / charwidth) letzte_spalte = nHorizontal / charwidth;

					// Wenn über das Popup-Menü die Felder kategorisch alle angezeigt werden sollen): Positionsmarke rosa zeigen
					if (!pDrawInfo->pm && ((nVeraltet && csFeldinhalt.GetLength()) || m_bFormularfelderAnzeigen))
					{
						// nur anzeigen, wenn nicht schon das rote Kästchen des aktuell bearbeiteten Feldes angezeigt wird:
						if (!(pFormularfeldDlg && pFormularfeldDlg->IsWindowVisible()))
						{
							CBrush brush;	
							COLORREF colourFelddarstellung;

							// Wenn Felder per mittlerer Maustaste für Multiselekt ausgewählt werden, selektierte Felder blau markieren, ansonsten rot
							colourFelddarstellung = RGB(0xe0,0x80,0x80);
							if (m_ptFeldMoveMultiselect.GetCount() > 0)
							{
								int id = atoi(child->GetAttrValue("id"));
								for (int i = 0; i < m_ptFeldMoveMultiselect.GetCount(); i++)
									if (m_ptFeldMoveMultiselect[i] == id)
										colourFelddarstellung = RGB(0x80,0x80,0xe0);
							}
							brush.CreateSolidBrush(colourFelddarstellung);
							CRect r;
							LPCTSTR cp;
							int feldmarkenhoehe;

							if (schriftgroesse) 
								feldmarkenhoehe = schriftgroesse;
							else
								feldmarkenhoehe = charheight;
						
							cp = child->GetAttrValue("ausrichtung");
							if (cp && !stricmp(cp,"rechtsbuendig"))	// rechtsbündig
							{
								r.left = (int)((double)nHorizontal - charheight * 8);
								r.top = (int)((double)nVertikal);
								r.right = (int)((double)nHorizontal);
								r.bottom = (int)((double)nVertikal + feldmarkenhoehe);
							}
							else									// linksbündig
							{
								r.left = (int)((double)nHorizontal);
								r.top = (int)((double)nVertikal);
								r.right = (int)((double)nHorizontal + charheight * 8);
								r.bottom = (int)((double)nVertikal + feldmarkenhoehe);
							}								
						
							pDC->FrameRect(&r, &brush);

							// in rot als veraltet markieren
							pDC->SetTextColor(colourFelddarstellung);
							if (nVeraltet && csFeldinhalt.GetLength())
							{
								if (cp && !stricmp(cp,"rechtsbuendig"))
								{
									pDrawInfo->m_pDC->SetTextAlign(TA_LEFT);
									pDC->TextOut(r.right, r.top, " <-- VERALTET!");	
								}
								else
								{
									pDrawInfo->m_pDC->SetTextAlign(TA_RIGHT);
									pDC->TextOut(r.right, r.top, "VERALTET! --> ");	
								}
							}

							// kleinere Schriftgröße
							CFont *standardFont = pDC->GetCurrentFont();
							LOGFONT FontStats;
							CFont neuerFont;	// Font kreieren
							CFont *oldFont;
							memset(&FontStats, 0x00, sizeof(FontStats));
							standardFont->GetLogFont(&FontStats);
							strcpy(FontStats.lfFaceName, (LPCSTR)schriftart); 
							FontStats.lfHeight /= 2;
							FontStats.lfWidth   = 0;	// Breite relativ zur Höhe verkleinern
							FontStats.lfQuality = DEFAULT_QUALITY;
							neuerFont.CreateFontIndirect(&FontStats);
							oldFont = pDC->SelectObject(&neuerFont);	// in den device context damit
							CString csID;
							csID.Format("%d", nID);
							pDC->SetTextAlign(TA_LEFT);
							pDC->TextOut(r.left + 2, r.top, csID);
							pDC->SetTextColor(RGB(0x00,0x00,0x00));
							pDC->SelectObject(oldFont);	
						}
					}
				}
			}
		}
		
		// ggf. alten Font restaurieren
		if (!schriftart.IsEmpty())
			pDC->SelectObject(oldFont);	

		if (csVeraltetMessage.GetLength())
		{
			csVeraltetMessage = "Achtung: veraltetes " + csVeraltetMessage + ". Bitte unter Einstellungen -> E/Ü-Konten das Konto mit einem anderen Feld verknüpfen!";
			theApp.CaptionBox(csVeraltetMessage);
		}
	}

	SetupScroll();
}


//-- Helfer für Draw-Members

// einache Textausgabe in einer Zeile, angabe der Position in Zehntel-Zeilen/Spalten-Schritten
void CEasyCashView::Text(DrawInfo *pDrawInfo, int left, int top, char *s)
{
	Text10(pDrawInfo, left * 10, top * 10, s);
}

// einache Textausgabe in einer Zeile
void CEasyCashView::Text10(DrawInfo *pDrawInfo, int left, int top, char *s)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = (int)((float)left * pDrawInfo->printer_charwidth);
		top  = (int)((float)top * pDrawInfo->printer_charheight);
	}
	else
	{
		left *= charwidth;
		top  *= charheight;
		//top  += page * charheight * (PAGE_GAP + VCHARS);

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return;
	}
	left /= 10;
	top  /= 10;

	CRect r(0,  // left
			top-(pDrawInfo->pm?(int)pDrawInfo->printer_charheight:charheight), 
			HCHARS * (pDrawInfo->pm?(int)pDrawInfo->printer_charwidth:charwidth),  // right
			top + 2*(pDrawInfo->pm?(int)pDrawInfo->printer_charheight:charheight)  // bottom
		   );
	if (!pDrawInfo->pm && !pDrawInfo->m_pDC->RectVisible(&r)) return;  // Optimierung: rect außerhalb von sichtbarem Bereich? Dann muss nicht dargestellt werden
	if (pDrawInfo->pm && pDrawInfo->pInfo && seitenzaehler != pDrawInfo->pInfo->m_nCurPage) return;  // bei Druck: nur diese Seite drucken

	// max. erlaubte Fontgröße undefiniert?
	if ((pDrawInfo->pm ? pDrawInfo->printer_fontsize :  pDrawInfo->fontsize) == -1)	
		TextEx(pDrawInfo, left, top, left+100, top, "%%%calibration\test%%%");	// dann mit TextEx messen

	// Font wählen:
	CFont *standardFont = pDrawInfo->m_pDC->GetCurrentFont();
	LOGFONT FontStats;
	memset(&FontStats, 0x00, sizeof(FontStats));
	standardFont->GetLogFont(&FontStats);
	strcpy(FontStats.lfFaceName, pDrawInfo->pm ? einstellungen1->m_Druckerschrift : einstellungen1->m_Bildschirmschrift); 
	FontStats.lfHeight  = pDrawInfo->pm ? (einstellungen1->m_Druckerschriftgroesse ? MulDiv(einstellungen1->m_Druckerschriftgroesse, pDrawInfo->m_pDC->GetDeviceCaps(LOGPIXELSY), 72) : (long)pDrawInfo->printer_charheight) : (einstellungen1->m_Bildschirmschriftgroesse ? MulDiv(einstellungen1->m_Bildschirmschriftgroesse, pDrawInfo->m_pDC->GetDeviceCaps(LOGPIXELSY), 72) * m_zoomfaktor / 100 : charheight);
	if (pDrawInfo->pm)
	{
		if (FontStats.lfHeight > pDrawInfo->printer_fontsize) FontStats.lfHeight = pDrawInfo->printer_fontsize;
	}
	else	
		if (FontStats.lfHeight > pDrawInfo->fontsize) FontStats.lfHeight = pDrawInfo->fontsize;	

	FontStats.lfWidth   = 0;	// Breite relativ zur Höhe verkleinern
	FontStats.lfQuality = pDrawInfo->pm ? PROOF_QUALITY : DEFAULT_QUALITY;
	FontStats.lfWeight  = pDrawInfo->pm ? einstellungen1->m_DruckerschriftFett : einstellungen1->m_BildschirmschriftFett;
	FontStats.lfItalic  = pDrawInfo->pm ? einstellungen1->m_DruckerschriftKursiv : einstellungen1->m_BildschirmschriftKursiv;
	CFont neuerFont;	// Font kreieren
	neuerFont.CreateFontIndirect(&FontStats);
	CFont *oldFont = pDrawInfo->m_pDC->SelectObject(&neuerFont);	// in den device context damit

	pDrawInfo->m_pDC->TextOut(left, top, s, strlen(s));

	// alten Font restaurieren
	pDrawInfo->m_pDC->SelectObject(oldFont);
}

RECT CEasyCashView::TextEx(DrawInfo *pDrawInfo, int left, int top, int right, int bottom, char *s_param)
{//if (*s_param != ' ' || s_param[1] != 0) { pDrawInfo->printer_fontsize = 193; return; }
	if (pDrawInfo->pInfo && seitenzaehler != pDrawInfo->pInfo->m_nCurPage) return CRect(0, 0, 0, 0);

	RECT r;
	if (pDrawInfo->pm)
	{
		left = (int)((float)pDrawInfo->printer_charwidth * (float)left);
		right = (int)((float)pDrawInfo->printer_charwidth * (float)right);
		top = (int)((float)pDrawInfo->printer_charheight * (float)top);
		bottom = (int)((float)pDrawInfo->printer_charheight * (float)bottom + (pDrawInfo->printer_charheight - 1));
	}
	else
	{
		left = charwidth * left;
		right = charwidth * right;
		top = charheight * top;
		bottom = charheight * bottom + charheight;

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return CRect(0, 0, 0, 0);
	}

	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;

	if (!pDrawInfo->pm)
	{	// Bildschirm DC
		if (!pDrawInfo->m_pDC->RectVisible(&r) && pDrawInfo->fontsize != -1)  // -1: Kalibrierungstest, nicht aussteigen, sondern maximale Texthöhe messen!
			return CRect(0, 0, 0, 0);   //  RectVisible() funktioniert manchmal irgendwie nicht richtig...
		// else 
		// 	TRACE1("RectVisible: %s\r\n", s_param);

		/*if (pDrawInfo->fontsize != -1)
		{
			HRGN hRgn;
			hRgn = CreateRectRgnIndirect(&r); 
			int n;
			BOOL bRectVisible = TRUE;
			if ((n =GetRandomRgn(pDrawInfo->m_pDC->m_hDC, hRgn, SYSRGN)) == 1)
			{
				CRect rScreened = r;
				ClientToScreen(&rScreened);
				bRectVisible = ::RectInRegion(hRgn, rScreened);
				if (bRectVisible || pDrawInfo->m_pDC->RectVisible(&r))
					TRACE3("%s: %d/%d\r\n", s, (int) bRectVisible, (int)pDrawInfo->m_pDC->RectVisible(&r));
			}
			::DeleteObject(hRgn);
			if (!bRectVisible) { delete s; return; }
		}*/
		;
	}
		

	// Konvertierung von '&' vermeiden
	char *s;
	s = new char[strlen(s_param)*2+1];
	char *cp1 = s_param;
	char *cp2 = s;
	while (*cp2++ = *cp1++)		
		if (*(cp1-1) == '&')
			*cp2++ = '&';

//CStdioFile debug;
//if (*s_param == ' ' && s_param[1] == 0) {
//debug.Open("ECTDebug.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite);
//debug.SeekToEnd();
//debug.WriteString("\n\r");
//debug.WriteString(s); }

	// Eigenschaften des momentan verwendeten Fonts herausfinden
	CFont *standardFont = pDrawInfo->m_pDC->GetCurrentFont();
	LOGFONT oldFontStats;
	memset(&oldFontStats, 0x00, sizeof(oldFontStats));
	standardFont->GetLogFont(&oldFontStats);

	// Struktur enthält Werte für den modifizierten Font
	LOGFONT newFontStats;
	memcpy(&newFontStats, &oldFontStats, sizeof(newFontStats));

	// Font sukszessive verkleinern, bis Text in Rect passt...
	int h;	// Höhe des neuen Fonts
	int increment; // um welchen Wert wird h erhöht oder verringert -- benötigt für binäres Suchen
	BOOL bPasst;	// Flag
//CString csTemp;
//if (*s_param == ' ' && s_param[1] == 0) {
//csTemp.Format("\n\rfor h=%d", (int)pDrawInfo->pm ? (einstellungen1->m_Druckerschriftgroesse ? -MulDiv(einstellungen1->m_Druckerschriftgroesse, pDrawInfo->m_pDC->GetDeviceCaps(LOGPIXELSY), 72) : (int)pDrawInfo->printer_charheight) : (einstellungen1->m_Bildschirmschriftgroesse ? -MulDiv(einstellungen1->m_Bildschirmschriftgroesse, pDrawInfo->m_pDC->GetDeviceCaps(LOGPIXELSY), 72) : charheight));
//debug.WriteString(csTemp); }
	BOOL bErsterDurchlauf = TRUE;
	for (h = pDrawInfo->pm ? (einstellungen1->m_Druckerschriftgroesse ?  // Druckerschriftgröße in den Einstellungen gewählt?
				 MulDiv(einstellungen1->m_Druckerschriftgroesse, pDrawInfo->m_pDC->GetDeviceCaps(LOGPIXELSY), 72)
			 : // else
				 (int)pDrawInfo->printer_charheight) : (einstellungen1->m_Bildschirmschriftgroesse ?
					  MulDiv(einstellungen1->m_Bildschirmschriftgroesse, pDrawInfo->m_pDC->GetDeviceCaps(LOGPIXELSY), 72)  //  * m_zoomfaktor / 100
				 : // else
					  charheight) / 2, increment = -h, bPasst = FALSE;  // for-Initialisierung
		 (abs(increment) > 0 && !bPasst && h > 0);  // for-Bedingung
		 h += increment)  // for-Inkrementor
	{
//if (*s_param == ' ' && s_param[1] == 0) {
//csTemp.Format("\n\rHöhe: %d", (int)h);
//debug.WriteString(csTemp); }
		// Font verkleinern
		newFontStats.lfHeight  = h;
		newFontStats.lfWidth   = 0;				// Breite relativ zur Höhe verkleinern
		strcpy(newFontStats.lfFaceName, pDrawInfo->pm ? einstellungen1->m_Druckerschrift : einstellungen1->m_Bildschirmschrift); 
		newFontStats.lfWeight  = pDrawInfo->pm ? einstellungen1->m_DruckerschriftFett : einstellungen1->m_BildschirmschriftFett;
		newFontStats.lfItalic  = pDrawInfo->pm ? einstellungen1->m_DruckerschriftKursiv : einstellungen1->m_BildschirmschriftKursiv;
		newFontStats.lfOrientation = 0;
//if (*s_param == ' ' && s_param[1] == 0) {
//debug.WriteString(newFontStats.lfFaceName);
//debug.WriteString("\n\r"); }
	
		// mit neuer Größe ausprobieren...
		{
			// Font kreieren
			CFont kleinererFont;
			kleinererFont.CreateFontIndirect(&newFontStats);

			// in den device context damit
			CFont *oldFont = pDrawInfo->m_pDC->SelectObject(&kleinererFont);

			// neue Höhe test-schreiben
			RECT r2 = r;
			int testhoehe = pDrawInfo->m_pDC->DrawText(s, strlen(s), &r2, DT_LEFT|DT_WORDBREAK|DT_CALCRECT);

			if ((r2.bottom-r2.top <= r.bottom-r.top && r2.right-r2.left <= r.right-r.left))
			{
				if (r2.bottom-r2.top == r.bottom-r.top)	// ausreichend angenähert?
				{
					bPasst = TRUE;
				}
				else	
				{
					increment = abs(increment)/2;	// noch zu klein
					if (bErsterDurchlauf)
					{
					//	h = h * (r.bottom-r.top) / (r2.bottom-r2.top) + 2;	// beim ersten Durchlauf Fonthöhe in der Relation der Rects reduzieren
						bErsterDurchlauf = FALSE;							// schade: funtionierte nicht bei sehr langen Texten, die zunächst zweizeilige Ergebnisse zeitigten
					}
				}
			}
			else
				increment = -abs(increment)/2;	// noch zu groß

			if (!bPasst && increment == 0)	// ab hier Schritt für Schritt die Höhe um 1 verkleinern
			{
				if (testhoehe > r.bottom-r.top)
					increment--;
				else
					bPasst = TRUE;
			}

			if (bPasst)
			{
				// passt? dann wirklich schreiben:
				if (strcmp(s, "%%%calibration\test%%%"))
					pDrawInfo->m_pDC->DrawText(s, strlen(s), &r, DT_LEFT|DT_WORDBREAK);
				else
				{
					if (pDrawInfo->pm) pDrawInfo->printer_fontsize = h; else pDrawInfo->fontsize = h;
				}
			}

			// alten Font restaurieren
			pDrawInfo->m_pDC->SelectObject(oldFont);
		}
	}

	delete s;
//if (*s_param == ' ' && s_param[1] == 0) {
//debug.Close(); }
	//if (!pDrawInfo->pm || pDrawInfo->m_pDC->RectVisible(&r))
	//	pDrawInfo->m_pDC->DrawText(s, strlen(s), &r, DT_LEFT|DT_WORDBREAK);

	return r;
}

void CEasyCashView::Box(DrawInfo *pDrawInfo, int left, int top, int right, int bottom)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = (int)((float)pDrawInfo->printer_charwidth / 2.0 + pDrawInfo->printer_charwidth * (float)left);
		right = (int)((float)pDrawInfo->printer_charwidth / 2.0 + pDrawInfo->printer_charwidth * (float)right);
		top = (int)((float)pDrawInfo->printer_charheight / 2.0 + pDrawInfo->printer_charheight * (float)top);
		bottom = (int)((float)pDrawInfo->printer_charheight / 2.0 + pDrawInfo->printer_charheight * (float)bottom);
	}
	else
	{
		left = charwidth / 2 + charwidth * left;
		right = charwidth / 2 + charwidth * right;
		top = charheight / 2 + charheight * top;
		bottom = charheight / 2 + charheight * bottom;
		top  += page * charheight * (PAGE_GAP + VCHARS);
		bottom  += page * charheight * (PAGE_GAP + VCHARS);

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return;
	}

	//CRect r(left, top, right, bottom);
	//if (!pDrawInfo->pm || pDrawInfo->m_pDC->RectVisible(&r))
	{	
		pDrawInfo->m_pDC->MoveTo(left, top);
		pDrawInfo->m_pDC->LineTo(right, top);
		pDrawInfo->m_pDC->LineTo(right, bottom);
		pDrawInfo->m_pDC->LineTo(left, bottom);
		pDrawInfo->m_pDC->LineTo(left, top);
	}
}

void CEasyCashView::GraueBox(DrawInfo *pDrawInfo, int left, int top, int right, int bottom)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = (int)((float)pDrawInfo->printer_charwidth * (float)left);
		right = (int)((float)pDrawInfo->printer_charwidth * (float)right);
		top = (int)((float)pDrawInfo->printer_charheight * (float)top);
		bottom = (int)((float)pDrawInfo->printer_charheight * (float)bottom);
	}
	else
	{
		left = charwidth * left;
		right = charwidth * right;
		top = charheight * top;
		bottom = charheight * bottom;
		//top  += page * charheight * (PAGE_GAP + VCHARS);
		//bottom  += page * charheight * (PAGE_GAP + VCHARS);

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return;
	}

	CRect r(left, top, right, bottom);
	if (!pDrawInfo->m_pDC->RectVisible(&r)) return;

	CBrush cbrGrau(0x00e8e8e8);
	CRgn crgnBox;
	crgnBox.CreateRectRgn(left, top, right, bottom);
	CBrush *cbrOldBrush = pDrawInfo->m_pDC->SelectObject(&cbrGrau);
	pDrawInfo->m_pDC->PaintRgn(&crgnBox);
	pDrawInfo->m_pDC->SelectObject(cbrOldBrush);
}

void CEasyCashView::Underline(DrawInfo *pDrawInfo, int left, int top, int right)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = (int)(pDrawInfo->printer_charwidth * (float)left);
		right = (int)(pDrawInfo->printer_charwidth * (float)right);
		top = int(pDrawInfo->printer_charheight * (float)(top + 1) - 1.0);
	}
	else
	{
		left = charwidth * left;
		right = charwidth * right;
		top = charheight * (top + 1) - 1;

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return;
	}

	//CRect r(left, top, right, top);
	//if (!pDrawInfo->pm || pDrawInfo->m_pDC->RectVisible(&r))
	{ 
		pDrawInfo->m_pDC->MoveTo(left, top);
		pDrawInfo->m_pDC->LineTo(right, top);
	}
}

void CEasyCashView::Underline10(DrawInfo *pDrawInfo, int left, int top, int right)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = (int)(pDrawInfo->printer_charwidth * (float)left);
		right = (int)(pDrawInfo->printer_charwidth * (float)right);
		top = (int)(pDrawInfo->printer_charheight * (float)(top + 1) - 1);
	}
	else
	{
		left = charwidth * left;
		right = charwidth * right;
		top = charheight * (top + 1) - 1;

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return;
	}

	top  /= 10;

	//CRect r(left, top, right, top);
	//if (!pDrawInfo->pm || pDrawInfo->m_pDC->RectVisible(&r))
	{ 
		pDrawInfo->m_pDC->MoveTo(left, top);
		pDrawInfo->m_pDC->LineTo(right, top);
	}
}

BOOL CEasyCashView::ScrollbugCheck(CDC *pDC, int top)
{
	if (top >= 28680 && m_osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		char text0[] = "  Wegen einer Betriebssystembeschränkung in Windows 95/98/ME können keine";
		pDC->TextOut(0, 28694, text0, strlen(text0));
		char text1[] = "  weiteren Buchungen dargestellt werden.";
		pDC->TextOut(0, 28712, text1, strlen(text1));
		char text2[] = "  Filtern Sie das Journal nach Monat oder Konto (zu finden im Ansicht-Menü).";
		pDC->TextOut(0, 28730, text2, strlen(text2));
		char text3[] = "  Der Druck und die Seitenansicht funktionieren auch weiterhin.";
		pDC->TextOut(0, 28748, text3, strlen(text3));
		return TRUE;	// nichts anzeigen außer dieser Meldung
	}
	return FALSE;	// normal anzeigen
}

void CEasyCashView::Image(DrawInfo *pDrawInfo, char *filename)
{
	int left, top, right, bottom;

	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = 0;
		right = pDrawInfo->printer_gesamtgroesse.cx;
		top = 0;
		bottom = pDrawInfo->printer_gesamtgroesse.cy;
	}
	else
	{
		left = 0;
		right = gesamtgroesse.cx;
		top = 0;
		bottom = gesamtgroesse.cy;
		top  += page * charheight * (PAGE_GAP + VCHARS);
		bottom  += page * charheight * (PAGE_GAP + VCHARS);
	}

	char path[2000];
	if (GetProgrammverzeichnis(path, sizeof(path))) 
	{ 
//		char *cp;

		strcat(path, "\\");
		strcat(path, filename);

		if (strlen(filename) < 3)
			return;
		extern int search_formats(char *);
		int format = search_formats(filename+strlen(filename)-3);

		::CImage *image = new ::CImage(path, format);
		int n = image->GetColorType();
		if (image->GetWidth() <= 0 || image->GetHeight() <= 0)
		{
			delete image;
			image = NULL;
			return;
		}

		CPalette *hOldPal = 0;
		if (image->GetPalette())
		{
			hOldPal = pDrawInfo->m_pDC->SelectPalette(image->GetPalette(), TRUE);
			pDrawInfo->m_pDC->RealizePalette();
		}

		image->Stretch(pDrawInfo->m_pDC, left, top, right-left, bottom-top, 0, 0, image->GetWidth(), image->GetHeight());
		delete image;
		image = NULL;

		pDrawInfo->m_pDC->SelectPalette(hOldPal, TRUE);

	}
}

// das Image für die .ecf-Formulardateien
void CEasyCashView::Image2(DrawInfo *pDrawInfo, char *filename)
{
	int left, top, right, bottom;

	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (strlen(filename) < 3)
		return;
	extern int search_formats(char *);
	int format = search_formats(filename+strlen(filename)-3);

	BOOL bDeleteImage = FALSE;	// normalerweise können wir das image cachen

	::CImage *image = NULL;
	int i;
	for (i = 0; i < m_csaFormularseitenPfade.GetSize() && i < FORMULARSEITENCACHESIZE; i++)
	{
		if (!m_pFormularseitenImageCache[i])
			break;
		if (m_csaFormularseitenPfade[i] == filename)
		{
			image = m_pFormularseitenImageCache[i];
			break;
		}
	}

	if (!image) // image noch nicht im Cache? 
	{
		//Laden
		image = new ::CImage(filename, format);
		int n = image->GetColorType();
		if (image->GetWidth() <= 0 || image->GetHeight() <= 0)
		{
			delete image;
			image = NULL;
			return;
		}

		// freien Platz im Cache suchen
		for (i = 0; i < FORMULARSEITENCACHESIZE; i++)
			if (!m_pFormularseitenImageCache[i])
			{
				m_pFormularseitenImageCache[i] = image;
				m_csaFormularseitenPfade.Add(filename);
				break;
			}
		if (i >= FORMULARSEITENCACHESIZE)	// kein Platz mehr im Cache? Dann am Ende wieder löschen
			bDeleteImage = TRUE;
	}

	if (pDrawInfo->pm)
	{
		left = 0;
		right = pDrawInfo->printer_gesamtgroesse.cx;
		top = 0;
		bottom = pDrawInfo->printer_gesamtgroesse.cy;
	}
	else
	{
		left = 0;
		if (pDrawInfo->querformat)
			right = charheight * (VCHARS + PAGE_GAP) * 1414 / 1000;
		else
			right = charheight * (VCHARS + PAGE_GAP) * 1000 / 1414; // * image->GetWidth() / image->GetHeight();// charwidth * HCHARS;
		top = 0;
		bottom = charheight * (PAGE_GAP + VCHARS);
		top  += page * charheight * (PAGE_GAP + VCHARS);
		bottom  += page * charheight * (PAGE_GAP + VCHARS);
		if (letzte_spalte < right / charwidth) letzte_spalte = right / charwidth;
	}

	CPoint scrollpos  = GetScrollPosition();
	CRect r;
	GetClientRect(&r);

	if (pDrawInfo->pm || (bottom >= scrollpos.y && top <= scrollpos.y+r.bottom-r.top))
	{
		CPalette *hOldPal = 0;
		if (image->GetPalette())
		{
			hOldPal = pDrawInfo->m_pDC->SelectPalette(image->GetPalette(), TRUE);
			pDrawInfo->m_pDC->RealizePalette();
		}

		image->Stretch(pDrawInfo->m_pDC, left, top, right-left, bottom-top, 0, 0, image->GetWidth(), image->GetHeight());

		pDrawInfo->m_pDC->SelectPalette(hOldPal, TRUE);
	}

	if (bDeleteImage) // war kein Platz mehr im Cache?
	{	// dann löschen
		delete image;
		image = NULL;
	}
}

void CEasyCashView::Icon(DrawInfo *pDrawInfo, int left, int top, /*int right,*/ int bottom, CBitmap *cbm, int n)
{
	if (pDrawInfo->pInfo && pDrawInfo->pInfo->m_nCurPage && pDrawInfo->pInfo->m_nCurPage != seitenzaehler) return;

	if (pDrawInfo->pm)
	{
		left = (int)((float)pDrawInfo->printer_charwidth * (float)left);
		//right = (int)((float)pDrawInfo->printer_charwidth * (float)right);
		top = (int)((float)pDrawInfo->printer_charheight * (float)top);
		bottom = (int)((float)pDrawInfo->printer_charheight * (float)bottom);
	}
	else
	{
		left = charwidth * left;
		//right = charwidth * right;
		top = charheight * top;
		bottom = charheight * bottom;
		//top  += page * charheight * (PAGE_GAP + VCHARS);
		//bottom  += page * charheight * (PAGE_GAP + VCHARS);

		if (ScrollbugCheck(pDrawInfo->m_pDC, top)) return;
	}

	CPoint scrollpos  = GetScrollPosition();
	CRect r;
	GetClientRect(&r);

	if (pDrawInfo->pm || (bottom >= scrollpos.y && top <= scrollpos.y+r.bottom-r.top))
	{
		HDC  hdc;
		CDC dcMem;
		BITMAP bm;

		VERIFY(hdc = pDrawInfo->m_pDC->m_hDC);

		//----- bitmap anzeigen

		// Create a memory DC for the bitmap.
		dcMem.CreateCompatibleDC(pDrawInfo->m_pDC);

		// Get the size of the bitmap.
		cbm->GetObject(sizeof(bm), &bm);

		// Select the bitmap into the memory DC.
		CBitmap* pbmOld;
		pbmOld = dcMem.SelectObject(cbm);

		pDrawInfo->m_pDC->StretchBlt(left, top,	// Destination
					bottom-top, bottom-top,	// Destination
					&dcMem,     // Source DC
					0+n*32, 0,  // Source position
					32, // Width
					bm.bmHeight,// Height
					SRCCOPY);   // Operation

		// Done with memory DC now, so clean up.
		dcMem.SelectObject(pbmOld);
		dcMem.DeleteDC();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CEasyCashView printing

void CEasyCashView::OnFilePrint2() 
{
	WasWirdGedruckt = 0;
	if (m_GewaehltesFormular > 0 && einstellungen2->m_land == 0)
		DSAMessageBox(IDS_FORMULARDRUCK_DE, MB_OK);

	CScrollView::OnFilePrint();	
}

void CEasyCashView::OnFilePrintPreview() 
{
	WasWirdGedruckt = 0;
	if (m_GewaehltesFormular > 0 && einstellungen2->m_land == 0)
		DSAMessageBox(IDS_FORMULARDRUCK_DE, MB_OK);

	//CScrollView::OnFilePrintPreview();
	AFXPrintPreview(this);	// mit Ribbon
}

void CEasyCashView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	BOOL bMakeItSo;

	if (pInfo)
	{
	if (pInfo->m_nCurPage > (UINT)max_seitenzahl)
			bMakeItSo = FALSE;
		else
			bMakeItSo = TRUE;
	}

	CScrollView::OnPrepareDC(pDC, pInfo);

	if (pInfo)
	{
		pInfo->m_bContinuePrinting = bMakeItSo;
	}
}

BOOL CEasyCashView::OnPreparePrinting(CPrintInfo* pInfo)
{
	CEasyCashDoc* pDoc = GetDocument();

	// neue Formulare:
	if (m_GewaehltesFormular == -1)
	{
		if (!WasWirdGedruckt)	// bei Druckvorschau nicht nochmal Dialog zeigen
		{
			DruckauswahlDlg dlg(this);
			dlg.m_vt = m_vt;
			dlg.m_vm = m_vm;
			dlg.m_bt = m_bt;
			dlg.m_bm = m_bm;
			dlg.m_nJahr = pDoc->nJahr;
			WasWirdGedruckt = dlg.DoModal();
			if (!WasWirdGedruckt) return 0;
			m_vt = dlg.m_vt;
			m_vm = dlg.m_vm;
			m_bt = dlg.m_bt;
			m_bm = dlg.m_bm;
		}

		switch (WasWirdGedruckt)
		{
		case IDC_BUCHUNGSLISTE:
		case IDC_BUCHUNGSLISTE_KONTEN:
		case IDC_BUCHUNGSLISTE_BESTANDSKONTEN:
		case IDC_ANLAGENVERZEICHNIS:
			break;	// alles ok

		case IDC_UMST_ERKLAERUNG:
			break;	// alles ok

		case IDC_EURECHNUNG:
		case IDC_EURECHNUNG_FORMULAR:
			BOOL bAlleBuchungenHabenRechnungsposten;
			CBuchung* pB;
			while (TRUE)
			{
				bAlleBuchungenHabenRechnungsposten = TRUE;

				for (pB = pDoc->Einnahmen; pB; pB = pB->next)
					if (pB->Konto.IsEmpty())
					{	
						bAlleBuchungenHabenRechnungsposten = FALSE;
						break;
					}
						
				for (pB = pDoc->Ausgaben; pB; pB = pB->next)
					if (pB->Konto.IsEmpty())
					{	
						bAlleBuchungenHabenRechnungsposten = FALSE;
						break;
					}
					
				if (bAlleBuchungenHabenRechnungsposten) break;
				
				if (AfxMessageBox("Einige Buchungen haben kein Konto zugewiesen bekommen. Ohne dass jede Buchung solch eine Zuweisung hat, kann keine Einnahmen/Überschuss-Rechnung gedruckt werden. Sollen die entsprechenden Buchungen jetzt bearbeitet werden?", MB_ICONQUESTION|MB_YESNO) == IDNO)
					return 0;
				
				// unvollständige Buchungen jetzt bearbeiten
				{
					for (pB = pDoc->Einnahmen; pB; pB = pB->next)
						if (pB->Konto.IsEmpty())
						{	
							BuchenDlg dlg(GetDocument(), FALSE, this, &pB);
							dlg.DoModal();
						}
							
					for (pB = pDoc->Ausgaben; pB; pB = pB->next)
						if (pB->Konto.IsEmpty())
						{	
							BuchenDlg dlg(GetDocument(), TRUE, this, &pB);
							dlg.DoModal();
						}					
				}
			}
			break;	// alles ok

		case 0:
		default:
			return 0;
		}
	}
	else	// .ecf Formular
	{
		XDoc xmldoc;
		xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
		LPXNode xml = xmldoc.GetRoot();
		if (xml) 
		{
			LPCTSTR attr_name = xml->GetAttrValue("seiten");
			if (attr_name)
			{
				pInfo->SetMinPage(1);
				pInfo->SetMaxPage(atoi(attr_name));
			}
		}
		
		CString csQuerformat = xml->GetAttrValue("querformat");
		int querformat = atoi(csQuerformat);
		{
			pInfo->m_pPD->m_pd.lStructSize = (DWORD)sizeof(PRINTDLG);
			BOOL bRet = theApp.GetPrinterDeviceDefaults(&(pInfo->m_pPD->m_pd));
			if(bRet)
			{
				DEVMODE FAR *pDevMode = (DEVMODE FAR *)::GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
				if (pDevMode)
				{
					pDevMode->dmPaperSize = DMPAPER_A4;
					pDevMode->dmOrientation = querformat ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
				}
				::GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);
			}
		}
	}

	// default preparation
	return DoPreparePrinting(pInfo);
}

void CEasyCashView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CEasyCashView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CEasyCashView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	DrawInfo di;
	di.m_pDC = pDC;

	// die Metrix herausfinden....
	di.printer_gesamtgroesse.cx = pInfo->m_rectDraw.Width(); 
	di.printer_gesamtgroesse.cy = pInfo->m_rectDraw.Height();
		di.printer_charwidth = (float)di.printer_gesamtgroesse.cx / (float)HCHARS;
	if (di.printer_gesamtgroesse.cx > di.printer_gesamtgroesse.cy) // Querformat?
	{
		di.printer_charheight = (float)di.printer_gesamtgroesse.cy / (float)VCHARS * (float)1.4142;
	}
	else
	{
		di.printer_charheight = (float)di.printer_gesamtgroesse.cy / (float)VCHARS;
	}
	di.printer_fontsize = -1;

	pDC->SetMapMode(MM_TEXT);

	if (m_GewaehltesFormular != -1)
	{
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawFormularToDC(pDC, &di);
	}

	switch (WasWirdGedruckt)
	{
	case IDC_BUCHUNGSLISTE:
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawToDC_Datum(pDC, &di);
		break;

	case IDC_BUCHUNGSLISTE_KONTEN:
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawToDC_Konten(pDC, &di);
		break;

	case IDC_BUCHUNGSLISTE_BESTANDSKONTEN:
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawToDC_Bestandskonten(pDC, &di);
		break;

	case IDC_ANLAGENVERZEICHNIS:
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawToDC_Anlagenverzeichnis(pDC, &di);
		break;

	case IDC_UMST_ERKLAERUNG:
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawUmStErklaerungToDC(pDC, &di);
		break;

	case IDC_EURECHNUNG:
		di.pm = TRUE;
		di.pInfo = pInfo;
		DrawEURechungToDC(pDC, &di);
		break;

	case 0:
	default:
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CEasyCashView diagnostics

#ifdef _DEBUG
void CEasyCashView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CEasyCashView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CEasyCashDoc* CEasyCashView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)));
	return (CEasyCashDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEasyCashView message handlers

void CEasyCashView::OnEditEinnahmeBuchen() 
{
	if (buchenDlg) 
	{
		buchenDlg->DestroyWindow();
		delete buchenDlg;
		buchenDlg = NULL;
	}
	buchenDlg = new BuchenDlg(GetDocument(), FALSE, this);
	buchenDlg->Create(IDD_BUCHUNG, this);
	buchenDlg->ShowWindow(SW_SHOW);
}

void CEasyCashView::OnEditAusgabeBuchen() 
{
	if (buchenDlg) 
	{
		buchenDlg->DestroyWindow();
		delete buchenDlg;
		buchenDlg = NULL;
	}
	buchenDlg = new BuchenDlg(GetDocument(), TRUE, this);
	buchenDlg->Create(IDD_BUCHUNG, this);
	buchenDlg->ShowWindow(SW_SHOW);
}

void CEasyCashView::OnEditDauerbuchungenEingeben() 
{
	if (dauerbuchungenDlg) 
	{
		dauerbuchungenDlg->DestroyWindow();
		delete dauerbuchungenDlg;
		dauerbuchungenDlg = NULL;
	}
	dauerbuchungenDlg = new DauerbuchungenDlg(GetDocument(), FALSE, this);
	dauerbuchungenDlg->Create(IDD_DAUERBUCHUNGEN, this);
	dauerbuchungenDlg->ShowWindow(SW_SHOW);
}

void CEasyCashView::OnEditDauerbuchungenAusfuehren() 
{
	DauBuchAusfuehren dlg(GetDocument(), this);
	if (dlg.DoModal() == IDOK)
		DauerbuchungenAusfuehren(dlg.m_jb, dlg.m_mb);
}

void CEasyCashView::OnBuchenDauausJanuar()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 1);
}
void CEasyCashView::OnBuchenDauausFebruar()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 2);
}
void CEasyCashView::OnBuchenDauausMaerz()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 3);
}
void CEasyCashView::OnBuchenDauausApril()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 4);
}
void CEasyCashView::OnBuchenDauausMai()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 5);
}
void CEasyCashView::OnBuchenDauausJuni()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 6);
}
void CEasyCashView::OnBuchenDauausJuli()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 7);
}
void CEasyCashView::OnBuchenDauausAugust()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 8);
}
void CEasyCashView::OnBuchenDauausSeptember()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 9);
}
void CEasyCashView::OnBuchenDauausOktober()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 10);
}
void CEasyCashView::OnBuchenDauausNovember()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 11);
}
void CEasyCashView::OnBuchenDauausDezember()
{
	DauerbuchungenAusfuehren(GetDocument()->nJahr, 12);
}

void CEasyCashView::DauerbuchungenAusfuehren(int jb, int mb) 
{
	CDauerbuchung *dbp;
	CEasyCashDoc* pDoc = GetDocument();

	if (jb < 100 && jb > 37)
	{
		jb += 1900;
	}
	else if (jb <= 37 && jb >= 0)
	{
		jb += 2000;
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
	CTime ausfuehren_bis_datum = *temp_datum;
	delete temp_datum; 

	int nAusserhalb_des_Buchungsjahrs = -1;	// -1 -> undefiniert, was passieren soll, wenn Vorjahresbuchungen auftreten, dann MessageBox

	// alle dauerbuchungen durchlaufen....
	for (dbp = pDoc->Dauerbuchungen; dbp; dbp=dbp->next)
	{
		int monate;	// Inkrement

		switch (dbp->Intervall)
		{
			case INTERVALL_MONAT:		monate = 1;		break;
			case INTERVALL_QUARTAL:		monate = 3;		break;
			case INTERVALL_HALBJAHR:	monate = 6;		break;
			case INTERVALL_JAHR:		monate = 12;	break;
			case INTERVALL_2MONATE:		monate = 2;		break;
		}

		// Ist Dauerbuchung schon dran?
		if (dbp->VonDatum > ausfuehren_bis_datum) continue;

		if (dbp->AktualisiertBisDatum <= ausfuehren_bis_datum)
		{
			// AktualisiertBisDatum initialisieren wenn noch nie hiervon gebucht
			if (dbp->AktualisiertBisDatum < dbp->VonDatum) 
				dbp->AktualisiertBisDatum = dbp->VonDatum;
		
			while (dbp->AktualisiertBisDatum <= ausfuehren_bis_datum
				&& dbp->AktualisiertBisDatum <= dbp->BisDatum)
			// neue Buchung
			{
				CBuchung **p;
				int m, y;

				if (dbp->AktualisiertBisDatum.GetYear() != pDoc->nJahr)
				{
					if (nAusserhalb_des_Buchungsjahrs == -1)
						nAusserhalb_des_Buchungsjahrs = AfxMessageBox("Mindestens eine Dauerbuchung liegt außerhalb des aktuellen Buchungsjahrs. Eventuell wurden die Dauerbuchungen vor dem 'Jahreswechsel' (Datei-Menü) noch nicht komplett bis Dezember ausgeführt (ggf. im Vorjahr checken). Sollen diese Dauerbuchungen trotzdem in diesem Buchungsjahr ausgeführt werden?", MB_YESNO);
				}

				if (dbp->AktualisiertBisDatum.GetYear() == pDoc->nJahr || nAusserhalb_des_Buchungsjahrs == IDYES)
				{				
					if (dbp->Buchungstyp == BUCHUNGSTYP_EINNAHMEN)
						p = &(pDoc->Einnahmen);
					else
						p = &(pDoc->Ausgaben);

					while (*p)
						p = &((*p)->next);

					*p = new CBuchung;
					(*p)->next = NULL;
					
					(*p)->Datum = dbp->AktualisiertBisDatum;					
					CTimeSpan ts(dbp->Buchungstag-1, 0, 0, 0);
					(*p)->Datum += ts;
					(*p)->Betrag = dbp->Betrag;
					(*p)->Beschreibung = dbp->Beschreibung;
					(*p)->Belegnummer = dbp->Belegnummer;
					(*p)->Konto = dbp->Konto;
					(*p)->MWSt = dbp->MWSt;
					(*p)->AbschreibungNr = 1;	// Erstes Jahr der Abschreibung
					(*p)->AbschreibungJahre = 1;
					(*p)->Betrieb = dbp->Betrieb;
					(*p)->Bestandskonto = dbp->Bestandskonto;

					// Platzhalter in die Beschreibung einfügen
					CString temp;
					int monat = (*p)->Datum.GetMonth();
					int jahr = (*p)->Datum.GetYear();
					BOOL bLaufendeBuchungsnummerFuerEinnahmenVergeben = FALSE, bLaufendeBuchungsnummerFuerAusgabenVergeben = FALSE;
					temp.Format("%-4.4d", jahr);	// Jahr Langform JJJJ
					(*p)->Beschreibung.Replace("$J", temp);
					temp.Format("%-2.2d", jahr % 100);	// Jahr Kurzform jj
					(*p)->Beschreibung.Replace("$j", temp);
					
					// Monatsmodifikator -1
					if (monat-1 < 1)						// Monat-1 Langform MM/JJJJ
						temp.Format("12/%-2.2d", jahr-1);
					else
						temp.Format("%-2.2d/%-2.2d", monat-1, jahr);
					(*p)->Beschreibung.Replace("$-M", temp);	
					if (monat-1 < 1)						// Monat-1 Kurzform m-mm/JJ
						temp.Format("12/%-4.4d", jahr-1);
					else
						temp.Format("%2d/%-2.2d", monat-1, jahr % 100);
					(*p)->Beschreibung.Replace("$-m", temp);
					
					// Monatsmodifikator -2
					if (monat-2 < 1)						// Monat-2 Langform MM/JJJJ
						temp.Format("%-2.2d/%-2.2d", monat-2+12, jahr-1);
					else
						temp.Format("%-2.2d/%-2.2d", monat-2, jahr);
					(*p)->Beschreibung.Replace("$--M", temp);	
					if (monat-2 < 1)						// Monat-2 Kurzform m-mm/JJ
						temp.Format("%2d/%-4.4d", monat-2+12, jahr-1);	
					else
						temp.Format("%2d/%-2.2d", monat-2, jahr % 100);
					(*p)->Beschreibung.Replace("$--m", temp);
					
					// Monatsmodifikator +1
					if (monat+1 > 12)						// Monat-1 Langform MM/JJJJ
						temp.Format("1/%-2.2d", jahr+1);
					else
						temp.Format("%-2.2d/%-2.2d", monat+1, jahr);
					(*p)->Beschreibung.Replace("$+M", temp);	
					if (monat+1 > 12)						// Monat-1 Kurzform m-mm/JJ
						temp.Format("1/%-4.4d", (jahr+1) % 100);
					else
						temp.Format("%2d/%-2.2d", monat+1, jahr % 100);
					(*p)->Beschreibung.Replace("$+m", temp);
					
					// Monatsmodifikator +2
					if (monat+2 > 12)						// Monat-2 Langform MM/JJJJ
						temp.Format("%-2.2d/%-2.2d", monat+2-12, jahr+1);
					else
						temp.Format("%-2.2d/%-2.2d", monat+2, jahr);
					(*p)->Beschreibung.Replace("$++M", temp);	
					if (monat+2 > 12)						// Monat-2 Kurzform m-mm/JJ
						temp.Format("%2d/%-4.4d", monat+2-12, jahr+1);	
					else
						temp.Format("%2d/%-2.2d", monat+2, jahr % 100);
					(*p)->Beschreibung.Replace("$++m", temp);

					temp.Format("%-2.2d", monat);	// Monat Langform MM
					(*p)->Beschreibung.Replace("$M", temp);				
					temp.Format("%d", monat);	// Monat Kurzform m-mm
					(*p)->Beschreibung.Replace("$m", temp);
					temp.Format("%d", (monat-1) / 3 + 1);	// Quartal q
					(*p)->Beschreibung.Replace("$q", temp);
					temp.Format("%d", (monat-1) / 6 + 1);	// Halbjahr h
					(*p)->Beschreibung.Replace("$h", temp);
					temp.Format("%d", (monat-1) / 2 + 1);	// 2-Monatszeitraum 2
					(*p)->Beschreibung.Replace("$2", temp);
					if (dbp->Buchungstyp == BUCHUNGSTYP_EINNAHMEN)
						temp.Format("E%04d", pDoc->nLaufendeBuchungsnummerFuerEinnahmen);	// laufende Buchungsnummer
					else
						temp.Format("A%04d", pDoc->nLaufendeBuchungsnummerFuerAusgaben);	
					if ((*p)->Beschreibung.Replace("$#", temp))
					{
						if (dbp->Buchungstyp == BUCHUNGSTYP_EINNAHMEN)
							bLaufendeBuchungsnummerFuerEinnahmenVergeben = TRUE;
						else
							bLaufendeBuchungsnummerFuerAusgabenVergeben = TRUE;
					}
					
					// Platzhalter in die Belegnummer einfügen
					temp.Format("%-4.4d", jahr);	// Jahr Langform JJJJ
					(*p)->Belegnummer.Replace("$J", temp);
					temp.Format("%-2.2d", jahr % 100);	// Jahr Kurzform jj
					(*p)->Belegnummer.Replace("$j", temp);
					temp.Format("%-2.2d", monat);	// Monat Langform MM
					(*p)->Belegnummer.Replace("$M", temp);				
					temp.Format("%-2.2d", monat);	// Monat Langform MM
					(*p)->Belegnummer.Replace("$-M", temp);				
					temp.Format("%d", monat);	// Monat Kurzform m-mm
					(*p)->Belegnummer.Replace("$m", temp);
					
					// Monatsmodifikator -1
					if (monat-1 < 1)						// Monat-1 Langform MM/JJJJ
						temp.Format("12/%-2.2d", jahr-1);
					else
						temp.Format("%-2.2d/%-2.2d", monat-1, jahr);
					(*p)->Belegnummer.Replace("$-M", temp);	
					if (monat-1 < 1)						// Monat-1 Kurzform m-mm/JJ
						temp.Format("12/%-4.4d", jahr-1);
					else
						temp.Format("%2d/%-2.2d", monat-1, jahr % 100);
					(*p)->Belegnummer.Replace("$-m", temp);
					
					// Monatsmodifikator -2
					if (monat-2 < 1)						// Monat-2 Langform MM/JJJJ
						temp.Format("%-2.2d/%-2.2d", monat-2+12, jahr-1);
					else
						temp.Format("%-2.2d/%-2.2d", monat-2, jahr);
					(*p)->Belegnummer.Replace("$--M", temp);	
					if (monat-2 < 1)						// Monat-2 Kurzform m-mm/JJ
						temp.Format("%2d/%-4.4d", monat-2+12, jahr-1);	
					else
						temp.Format("%2d/%-2.2d", monat-2, jahr % 100);
					(*p)->Belegnummer.Replace("$--m", temp);
					
					// Monatsmodifikator +1
					if (monat+1 > 12)						// Monat-1 Langform MM/JJJJ
						temp.Format("1/%-2.2d", jahr+1);
					else
						temp.Format("%-2.2d/%-2.2d", monat+1, jahr);
					(*p)->Belegnummer.Replace("$+M", temp);	
					if (monat+1 > 12)						// Monat-1 Kurzform m-mm/JJ
						temp.Format("1/%-4.4d", (jahr+1) % 100);
					else
						temp.Format("%2d/%-2.2d", monat+1, jahr % 100);
					(*p)->Belegnummer.Replace("$+m", temp);
					
					// Monatsmodifikator +2
					if (monat+2 > 12)						// Monat-2 Langform MM/JJJJ
						temp.Format("%-2.2d/%-2.2d", monat+2-12, jahr+1);
					else
						temp.Format("%-2.2d/%-2.2d", monat+2, jahr);
					(*p)->Belegnummer.Replace("$++M", temp);	
					if (monat+2 > 12)						// Monat-2 Kurzform m-mm/JJ
						temp.Format("%2d/%-4.4d", monat+2-12, jahr+1);	
					else
						temp.Format("%2d/%-2.2d", monat+2, jahr % 100);
					(*p)->Belegnummer.Replace("$++m", temp);

					temp.Format("%d", (monat-1) / 3 + 1);	// Quartal q
					(*p)->Belegnummer.Replace("$q", temp);
					temp.Format("%d", (monat-1) / 6 + 1);	// Halbjahr h
					(*p)->Belegnummer.Replace("$h", temp);
					temp.Format("%d", (monat-1) / 2 + 1);	// 2-Monatszeitraum 2
					(*p)->Belegnummer.Replace("$2", temp);
					if (dbp->Buchungstyp == BUCHUNGSTYP_EINNAHMEN)
						temp.Format("E%04d", pDoc->nLaufendeBuchungsnummerFuerEinnahmen);	// laufende Buchungsnummer
					else
						temp.Format("A%04d", pDoc->nLaufendeBuchungsnummerFuerAusgaben);	
					if ((*p)->Belegnummer.Replace("$#", temp))
					{
						if (dbp->Buchungstyp == BUCHUNGSTYP_EINNAHMEN)
							bLaufendeBuchungsnummerFuerEinnahmenVergeben = TRUE;
						else
							bLaufendeBuchungsnummerFuerAusgabenVergeben = TRUE;
					}

					if (bLaufendeBuchungsnummerFuerEinnahmenVergeben) pDoc->nLaufendeBuchungsnummerFuerEinnahmen++;
					if (bLaufendeBuchungsnummerFuerAusgabenVergeben) pDoc->nLaufendeBuchungsnummerFuerAusgaben++;
					
					CString csMessage;
					csMessage.Format("Dauerbuchungen wurden bis %d/%4d ausgeführt", mb, jb);
					pDoc->SetModifiedFlag(csMessage);
					pDoc->InkrementBuchungszaehler();
					pDoc->Sort();
					RedrawWindow();
					pDoc->UpdateAllViews(NULL);
				}

				// um 'monate' inkrementen...
				m = dbp->AktualisiertBisDatum.GetMonth();
				y = dbp->AktualisiertBisDatum.GetYear();
				m += monate;
				if (m > 12) 
				{
					y++;
					m -= 12;
				}
				CTime *tp;
				tp = new CTime(y, m, 1, 0, 0, 0);
				dbp->AktualisiertBisDatum = *tp;
				delete tp;
			}
		}
	}
}

void CEasyCashView::GetUmsatzsteuervorauszahlung(int nZeitraum, CString& csValue)
{
	CEasyCashDoc* pDoc = GetDocument();
	CString csKey;

	csKey.Format("UST-Zahlbetrag-%04d-%02d", pDoc->nJahr, nZeitraum);
	char *cp = GetErweiterungKey(pDoc->Erweiterung, "Elster", csKey);
	char *cp2;
	if (cp2 = strchr(cp, '|'))
	{
		strncpy(csValue.GetBuffer(cp2-cp+1), cp, cp2-cp);
		csValue.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
		csValue.ReleaseBuffer();
	}
	else
		csValue = "";
}

void CEasyCashView::SetUmsatzsteuervorauszahlung(int nZeitraum, CString& csValue)
{
	CEasyCashDoc* pDoc = GetDocument();
	CString csKey;

	int nValue = currency_to_int(csValue.GetBuffer(0));
	if (nValue)
	{
		int_to_currency(nValue, 4, csValue.GetBuffer(100));
		csValue.ReleaseBuffer();
	}
	else
		csValue = "";

	CString csVergleich;
	GetUmsatzsteuervorauszahlung(nZeitraum, csVergleich);
	if (csValue != csVergleich)
	{
		CString csZeitraum;
		if (nZeitraum <= 12)
			csZeitraum.Format("Monat %d", nZeitraum);
		else if (nZeitraum >= 41 && nZeitraum <= 44)
			csZeitraum.Format("%d. Quartal", nZeitraum - 40);
		else
			csZeitraum = "undefinierten";
		pDoc->SetModifiedFlag("Umsatzsteuervorauszahlungsbetrag für " + csZeitraum + " wurde " + (csVergleich == "" ? "eingetragen" : "geändert"));
	}

	csKey.Format("UST-Zahlbetrag-%04d-%02d", pDoc->nJahr, nZeitraum);
	SetErweiterungKey(pDoc->Erweiterung, "Elster", csKey, csValue);
}

void CEasyCashView::OnEditUmsatzsteuervorauszahlungen()
{
	CUstVorauszahlungenDlg dlg(this);

	GetUmsatzsteuervorauszahlung(1, dlg.m_m1);
	GetUmsatzsteuervorauszahlung(2, dlg.m_m2);
	GetUmsatzsteuervorauszahlung(3, dlg.m_m3);
	GetUmsatzsteuervorauszahlung(4, dlg.m_m4);
	GetUmsatzsteuervorauszahlung(5, dlg.m_m5);
	GetUmsatzsteuervorauszahlung(6, dlg.m_m6);
	GetUmsatzsteuervorauszahlung(7, dlg.m_m7);
	GetUmsatzsteuervorauszahlung(8, dlg.m_m8);
	GetUmsatzsteuervorauszahlung(9, dlg.m_m9);
	GetUmsatzsteuervorauszahlung(10, dlg.m_m10);
	GetUmsatzsteuervorauszahlung(11, dlg.m_m11);
	GetUmsatzsteuervorauszahlung(12, dlg.m_m12);
	GetUmsatzsteuervorauszahlung(41, dlg.m_q1);
	GetUmsatzsteuervorauszahlung(42, dlg.m_q2);
	GetUmsatzsteuervorauszahlung(43, dlg.m_q3);
	GetUmsatzsteuervorauszahlung(44, dlg.m_q4);
	CString Key;
	Key.Format("Sondervorauszahlung%-04.4d", GetDocument()->nJahr);
	char *cp = GetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key);
	char *cp2;
	if (cp2 = strchr(cp, '|'))
	{ 
		strncpy(dlg.m_vorauszahlung.GetBuffer(cp2-cp+1), cp, cp2-cp);
		dlg.m_vorauszahlung.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
		dlg.m_vorauszahlung.ReleaseBuffer();
	}
	else 
		dlg.m_vorauszahlung = "";
	CString vorauszahlung_merken = dlg.m_vorauszahlung;
	
	if (dlg.DoModal() == IDOK)
	{
		SetUmsatzsteuervorauszahlung(1, dlg.m_m1);
		SetUmsatzsteuervorauszahlung(2, dlg.m_m2);
		SetUmsatzsteuervorauszahlung(3, dlg.m_m3);
		SetUmsatzsteuervorauszahlung(4, dlg.m_m4);
		SetUmsatzsteuervorauszahlung(5, dlg.m_m5);
		SetUmsatzsteuervorauszahlung(6, dlg.m_m6);
		SetUmsatzsteuervorauszahlung(7, dlg.m_m7);
		SetUmsatzsteuervorauszahlung(8, dlg.m_m8);
		SetUmsatzsteuervorauszahlung(9, dlg.m_m9);
		SetUmsatzsteuervorauszahlung(10, dlg.m_m10);
		SetUmsatzsteuervorauszahlung(11, dlg.m_m11);
		SetUmsatzsteuervorauszahlung(12, dlg.m_m12);
		SetUmsatzsteuervorauszahlung(41, dlg.m_q1);
		SetUmsatzsteuervorauszahlung(42, dlg.m_q2);
		SetUmsatzsteuervorauszahlung(43, dlg.m_q3);
		SetUmsatzsteuervorauszahlung(44, dlg.m_q4);
		if (dlg.m_vorauszahlung != vorauszahlung_merken)
		{
			GetDocument()->SetModifiedFlag((CString)"Sondervorauszahlungsbetrag (Dauerfristverlängerung) für das Buchungsjahr wurde " + (vorauszahlung_merken == "" ? "eingetragen" : "geändert"));
			SetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key, dlg.m_vorauszahlung);
		}
	}
}

void CEasyCashView::OnFindNext() 
{
	OnFind(1);
}

void CEasyCashView::OnFindPrev() 
{
	OnFind(-1);	
}

void CEasyCashView::OnFind(int nIncrement) 
{
	if (pPluginWnd || m_GewaehltesFormular != -1) OnViewJournalSwitch();	// sicherstellen, dass Journal sichtbar ist

	CString csText;

	if (((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_hWnd)
		((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_comboBox1.GetWindowText(csText);	// VS9
	else
	{
		// GetEditText() würde nicht den tatsächlichen Feldinahlt wiedergeben, würden wir nicht diesen Aufwand zuvor treiben
		//CMainFrame *pFrame = static_cast<CMainFrame*>(GetTopLevelFrame());
		//CMFCRibbonBar* pRibbon = pFrame->GetRibbonBar();
		//ASSERT_VALID(pRibbon);
		//CMFCRibbonButton *pButtonForFocus = DYNAMIC_DOWNCAST(CMFCRibbonButton, pRibbon->FindByID(nIncrement > 0 ? ID_NEXT : ID_PREV));
		////pButtonForFocus->GetParentPanel()->SetFocused(pButtonForFocus); // Arx! Geht erst ab VS2010!!!
		//pRibbon->SetActiveCategory(pButtonForFocus->GetParentCategory());

		SetFocus();

		// jetzt endlich...
		csText = ((CMainFrame*)AfxGetMainWnd())->m_pSucheCombobox->GetEditText();
	}

	if(!csText.IsEmpty())
	{
		if (((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_hWnd)
			((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_comboBox1.AddString(csText); // VS9
		else
			((CMainFrame*)AfxGetMainWnd())->m_pSucheCombobox->AddItem(csText);

		csText.MakeLower();

		POINT scrollpos = GetScrollPosition();
		int line = scrollpos.y / charheight;
		if (nSelected >= 0) line = nSelected + nIncrement;
		CString csAktuelleBelegnummer;
		CString csAktuelleBeschreibung;
		CString csAktuellerBetrag;
		int i;
		for (i = line; i >= 0 && i < MAX_BUCHUNGEN; i += nIncrement)
			if (ppPosBuchungsliste[i] && *(ppPosBuchungsliste[i]))
			{
				CBuchung *pb = *(ppPosBuchungsliste[i]);
				csAktuelleBelegnummer = pb->Belegnummer;
				csAktuelleBelegnummer.MakeLower();
				csAktuelleBeschreibung = pb->Beschreibung;
				csAktuelleBeschreibung.MakeLower();
				int_to_currency_tausenderpunkt(pb->Betrag, 8, csAktuellerBetrag.GetBuffer(30));
				csAktuellerBetrag.ReleaseBuffer();
				csAktuellerBetrag.MakeLower();
				csAktuellerBetrag.TrimLeft();
				if (csAktuelleBelegnummer.Find(csText) != -1 || csAktuelleBeschreibung.Find(csText) != -1 || csText == csAktuellerBetrag)
				{
					ScrolleZuBuchung(i);
					((CMainFrame*)AfxGetMainWnd())->SetStatus("Suchbegriff in Buchung " + (pb->Belegnummer != "" ? pb->Belegnummer : pb->Beschreibung) + " gefunden.");
					break;
				}
			}

		if (i < 0 || i >= MAX_BUCHUNGEN) 
		{
			((CMainFrame*)AfxGetMainWnd())->SetStatus("Keine weiteren Vorkommen des Suchbegriffs gefunden.");
			MessageBeep(MB_ICONASTERISK);
		}		
	}
}

void CEasyCashView::ScrolleZuBuchung(int b)
{
	RECT r;
	GetWindowRect(&r);
	nSelected = b;
	int vpos = b * charheight + charheight/2 - (r.bottom - r.top) / 2;
	if (vpos < 0) vpos = 0;
	SetScrollPos(SB_VERT, vpos);
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::ScrolleZuSeite(int s, int vertikal)
{	
	if (m_GewaehltesFormular >= 0 && s >= 1 && s <= m_anzahl_formularseiten)
	{
		CPoint scrollpos;
		scrollpos.SetPoint(0, (s - 1) * charheight * (PAGE_GAP + VCHARS) + vertikal);
		ScrollToPosition(scrollpos);
	}
}

void CEasyCashView::ShowFindToolbar(int nShowstate)
{
	if (nShowstate == SW_SHOW && pPluginWnd)
	{
		// plugin fenster schließen
		DestroyPlugin();
		ShowWindow(SW_SHOW);
	}

	// Formularansicht zurücksetzen
	m_GewaehltesFormular = -1;
	GetParent()->ShowWindow(SW_SHOW);

	if (((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_hWnd)
	{
		((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.ShowWindow(nShowstate);
		((CMainFrame*)AfxGetMainWnd())->RecalcLayout(TRUE);
		((CMainFrame*)AfxGetMainWnd())->DockControlBar(&(((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar), AFX_IDW_DOCKBAR_TOP);
		((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.SetFocus();
		if (nShowstate == SW_SHOW)
			((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_comboBox1.SetFocus();
		else
			SetFocus();	// Fokus an das CView
	}
}


void CEasyCashView::OnViewFindtoolbar() 
{
	if (((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_hWnd)	// VS9
		ShowFindToolbar(((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

void CEasyCashView::OnDestroy() 
{
	CScrollView::OnDestroy();
	
	if (buchenDlg)
	{
		if (buchenDlg->m_hWnd) buchenDlg->DestroyWindow();
		delete buchenDlg;
		buchenDlg = NULL;
	}

	if (dauerbuchungenDlg) 
	{
		if (dauerbuchungenDlg->m_hWnd) dauerbuchungenDlg->DestroyWindow();
		delete dauerbuchungenDlg;
		dauerbuchungenDlg = NULL;
	}

	// plugin fenster schließen
	DestroyPlugin();
}


//-- Menü Behandlung --------------

void CEasyCashView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CRect r;

	GetWindowRect(&r);
	if (m_GewaehltesFormular >= 0)
		PopUpFormular.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x + r.left, point.y + r.top, this);
	else
	{
		RECT r;
		int index = (GetScrollPosition() + point).y / charheight;
		CBuchung **ppb = ppPosBuchungsliste[index];
		if (ppb)
		{
			nSelected = index;
			GetWindowRect(&r);
			int vpos = index * charheight + charheight/2 - (r.bottom - r.top) / 2;
			//RedrawWindow();
			GetDocument()->UpdateAllViews(NULL);
			PopUp.EnableMenuItem(POPUP_AFA_ABGANG, MF_BYCOMMAND | MF_GRAYED);
			if ((**ppb).AbschreibungJahre > 1)
			{
				char *cp = GetErweiterungKey((**ppb).Erweiterung, "EasyCash", "UrspruenglichesAnschaffungsdatum");
				char *cp2 = strchr(cp, '|');
				if (cp2 - cp != 10)  // noch nicht ausgeschieden?
					PopUp.EnableMenuItem(POPUP_AFA_ABGANG, MF_BYCOMMAND | MF_ENABLED);
			}
			PopUp.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x + r.left, point.y + r.top, this);
		}
	}
	PopUpPosition = GetScrollPosition() + point;

	CScrollView::OnRButtonDown(nFlags, point);
}

BOOL CEasyCashView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	CBuchung **ppb, *pb;
	int Buchungstyp, index;
	CEasyCashDoc *pDoc = GetDocument();

	// Journalansicht-Menü
	if (wParam == POPUP_AENDERN || wParam == POPUP_LOESCHEN || wParam == POPUP_KOPIEREN || wParam == POPUP_KOPIEREN_BELEGNUMMER || wParam == POPUP_AFA_ABGANG)
	{
		// aus Bildschirm/Scropplosition die Zeilennummer berechnen und 
		// somit den Pointer in ppPosBuchungsliste
		
		index = PopUpPosition.y / charheight;
		ppb = ppPosBuchungsliste[index];
		pb = pDoc->Einnahmen;
		Buchungstyp = BUCHUNGSTYP_AUSGABEN;
		while (pb && ppb)
		{
			if (pb == *ppb)
			{
				Buchungstyp = BUCHUNGSTYP_EINNAHMEN;
				break;
			}
			pb = pb->next;
		}

		if (ppb)
		{
			switch (wParam)
			{
			case POPUP_AENDERN:
				if (*ppb)
				{
					// Hinweis bei nachträglicher Änderung einer AfA
					if ((*ppb)->AbschreibungNr > 1)
						AfxMessageBox("Hinweis: Wenn Abschreibungen nachträglich geändert werden, kann dies zur Inkonsistenz mit den entspr. Buchungen aus den Vorjahren führen, insbesondere wenn Betrag, Abschreibungsdauer oder Restwert geändert werden. Ändern Sie deshalb auch die AfA-Buchungen in den Vorjahren entsprechend (was in der Regel nur bei nachträglicher Erfassung der Buchungen sinnvoll ist) oder erzeugen Sie zusätzlich eine Korrekturbuchung (z.B. Sonderabschreibung wegen ungewöhnlicher Abnutzung). Auch ist die VST im Brutto-Betrag zu berücksichtigen und ggf. noch eine weitere Buchung dafür vorzusehen. In jedem Fall liegt die korrekte Abstimmung von Betrag, Dauer und Restwert in Ihrer Verantwortung.");
					
					// Tu es!
					{
						BuchenDlg dlg(GetDocument(), Buchungstyp, this, ppb);
						nSelected = index;
						RedrawSelection(); // selektieren
						// GetDocument()->UpdateAllViews(NULL);	
						pBuchungAendernDlg = &dlg;
						dlg.DoModal();
						pBuchungAendernDlg = NULL;
						RedrawSelection();
					}
				}
				break;

			case POPUP_LOESCHEN:
				if (*ppb)
				{
					int nDoIt;
					nSelected = index;
					RedrawSelection();	// selektieren
					CString csText;
					csText.Format("Buchung '%s' wirklich löschen?", (LPCTSTR)(*ppb)->Beschreibung);
					nDoIt = AfxMessageBox(csText, MB_YESNO|MB_DEFBUTTON2);
					nSelected = -nSelected;	// deselektieren
					RedrawSelection();

					if (nDoIt == IDYES)
					{
						if (*GetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "SplitBasisbuchung")
							|| *GetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "SplitGegenbuchungMitVorsteuerabzug")
							|| *GetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "SplitGegenbuchungOhneVorsteuerabzug"))
								AfxMessageBox("Hinweis: Diese Buchung ist Teil einer Split-Buchung. Bitte löschen Sie gegebenenfalls auch den korrespondierenden Anteil.", MB_ICONEXCLAMATION);

						pb = (*ppb)->next;
						(*ppb)->next = NULL;	// ganz wichtig wegen Kettenlöschung
						delete *ppb;			// access violation v2.51.0.1-85be8316-6104-4e0f-8de8-bac7f47bc1ef v2.51.0.1-9511f9c5-8810-4591-a642-9c002744d709
						*ppb = pb;

						pDoc->SetModifiedFlag("Buchung wurde gelöscht");
						pDoc->Sort();
						//RedrawWindow();
						pDoc->UpdateAllViews(NULL);
					}
				}
				break;

			case POPUP_KOPIEREN:
				if (*ppb)
				{
					{
						BuchenDlg dlg(GetDocument(), Buchungstyp, this, ppb, TRUE);
						nSelected = index;
						RedrawSelection(); // selektieren
						// GetDocument()->UpdateAllViews(NULL);	
						pBuchungAendernDlg = &dlg;
						dlg.DoModal();
						pBuchungAendernDlg = NULL;
						nSelected = -nSelected;	// deselektieren
						RedrawSelection();
					}
				}
				break;

			case POPUP_KOPIEREN_BELEGNUMMER:
				if (*ppb)
				{
					{
						BuchenDlg dlg(GetDocument(), Buchungstyp, this, ppb, TRUE, TRUE);
						nSelected = index;
						RedrawSelection(); // selektieren
						// GetDocument()->UpdateAllViews(NULL);	
						pBuchungAendernDlg = &dlg;
						dlg.DoModal();
						pBuchungAendernDlg = NULL;
						nSelected = -nSelected;	// deselektieren
						RedrawSelection();
					}
				}
				break;

			case POPUP_AFA_ABGANG:
				if (*ppb)
				{
					AfAAbgang(ppb);
				}
				break;
			}
		}
	}
	// Popup-Menü für Formularansicht
	else if (wParam == POPUPFORMULAR_NEUES_FELD || wParam == POPUPFORMULAR_NEUER_ABSCHNITT || wParam == POPUPFORMULAR_FELDER_BEARBEITEN 
		|| wParam == POPUPFORMULAR_KALKULATION_BEARBEITEN || wParam == POPUPFORMULAR_FOLMULARDATEI_OEFFNEN
		|| wParam == POPUPFORMULAR_FELDER_ANZEIGEN || POPUPFORMULAR_FELDWERT_KOPIEREN)
	{
		switch (wParam)
		{
		case POPUPFORMULAR_NEUES_FELD:
			{
				// ggf. vorher Dialog löschen
				if (pFormularfeldDlg) 
				{
					pFormularfeldDlg->DestroyWindow();
					delete pFormularfeldDlg;
					pFormularfeldDlg = NULL;
				}
				
				// Feldmarkierung einblenden
				ptFeldmarke = PopUpPosition;
				ptFeldmarke.x = ptFeldmarke.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414)) / querformat_faktor;
				ptFeldmarke.y = ptFeldmarke.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);
			
				// Formulareigenschaften anzeigen
				pFormularfeldDlg = new CFormularfeld(this);
				pFormularfeldDlg->m_csFormulardatei = m_csaFormulare[m_GewaehltesFormular];
				pFormularfeldDlg->m_ausrichtung = 1;	// rechtsbündig
				pFormularfeldDlg->m_anteil = 1;
				pFormularfeldDlg->m_nachkommaanteil = 1;
				pFormularfeldDlg->m_bKeineFeldUeberschreibenMeldung = FALSE;
				pFormularfeldDlg->m_nNullwertdarstellung = 1;
				pFormularfeldDlg->m_veraltet = 0;
				pFormularfeldDlg->Create(IDD_FORMULARFELD, this);
				pFormularfeldDlg->m_seite = ptFeldmarke.y / 1414 + 1;
				pFormularfeldDlg->m_vertikal = ptFeldmarke.y - ((pFormularfeldDlg->m_seite-1) * 1414);
				pFormularfeldDlg->m_horizontal = ptFeldmarke.x;
				pFormularfeldDlg->UpdateData(FALSE);
				//pFormularfeldDlg->ShowWindow(SW_SHOW);
			}
			break;
		case POPUPFORMULAR_NEUER_ABSCHNITT:
			{				
				// Feldmarkierung einblenden
				ptFeldmarke = PopUpPosition;
				ptFeldmarke.x = ptFeldmarke.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414)) / querformat_faktor;
				ptFeldmarke.y = ptFeldmarke.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);
			
				// Formulareigenschaften anzeigen
				CFormularabschnitt dlg;
				dlg.m_seite = ptFeldmarke.y / 1414 + 1;
				dlg.m_vertikal = ptFeldmarke.y - ((dlg.m_seite-1) * 1414);

				if (dlg.DoModal() == IDOK)
				{
					// Formulardefinitionsdatei in xmldoc laden
					XDoc xmldoc;
					xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
					LPXNode xml = xmldoc.GetRoot();
					LPXNode abschnitte = NULL;
					if (xml)
					{
						 abschnitte = xml->Find("abschnitte");
						 if (!abschnitte)  // ggf. Abschnitte-Node erzeugen
							abschnitte = xml->AppendChild("abschnitte");
						 if (abschnitte)
						 {
							LPXNode child = abschnitte->AppendChild("abschnitt");	// Abschnitt-Node in 'Abschnitte' erzeugen
							if (child)
							{
								CString csFromInt;
								child->AppendAttr("name", dlg.m_name);
								_ultoa((DWORD)dlg.m_seite, csFromInt.GetBuffer(30), 10);
								child->AppendAttr("seite", csFromInt);
								_ultoa((DWORD)dlg.m_vertikal, csFromInt.GetBuffer(30), 10);
								child->AppendAttr("vertikal", csFromInt);

								DISP_OPT opt;
								opt.newline = false; // no new line
								if (!xmldoc.SaveFile(m_csaFormulare[m_GewaehltesFormular], &opt))
								{
									AfxMessageBox("Konnte den Abschnitt nicht in der Formulardatei speichern.");
								}
							}
						}
					}
				}
			}
			break;
		case POPUPFORMULAR_FELDER_BEARBEITEN:
			{
				// Formulardefinitionsdatei in xmldoc laden
				XDoc xmldoc;
				xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
				LPXNode xml = xmldoc.GetRoot();
				LPXNode felder = NULL;
				felder = xml->Find("felder");
				
				ptFeldmarke = PopUpPosition;
				int nNaechsteFeldIndex;

				if (felder && ((nNaechsteFeldIndex = GetFeldindexFromMausposition(felder)) >= 0))
				{
					// ggf. vorher Dialog löschen
					if (pFormularfeldDlg) 
					{
						pFormularfeldDlg->DestroyWindow();
						delete pFormularfeldDlg;
						pFormularfeldDlg = NULL;
					}
					
					// Formulareigenschaften anzeigen
					pFormularfeldDlg = new CFormularfeld(this);

					LPXNode child = felder->GetChild(nNaechsteFeldIndex);
					pFormularfeldDlg->m_id = atoi(child->GetAttrValue("id"));
					
					pFormularfeldDlg->m_seite = atoi(child->GetAttrValue("seite"));						
					pFormularfeldDlg->m_horizontal = atoi(child->GetAttrValue("horizontal"));						
					pFormularfeldDlg->m_vertikal = atoi(child->GetAttrValue("vertikal"));
					ptFeldmarke.x = pFormularfeldDlg->m_horizontal;	// Feldmarke korrigieren auf tatsächliche Feldposition
					ptFeldmarke.y = pFormularfeldDlg->m_vertikal;

					if ((CString)child->GetAttrValue("ausrichtung") == "linksbuendig")
					{
						m_bFeldmarkeRechtsbuendig = pFormularfeldDlg->m_ausrichtung = 0;
					}
					else	// ausrichtung == "rechtsbuendig"
					{
						m_bFeldmarkeRechtsbuendig = pFormularfeldDlg->m_ausrichtung = 1;
					}

					CString csAnteil = child->GetAttrValue("anteil");
					if (csAnteil == "brutto")
						pFormularfeldDlg->m_anteil = 0;
					else if (csAnteil == "netto")
						pFormularfeldDlg->m_anteil = 1;
					else // csAnteil ==  "mwst"
						pFormularfeldDlg->m_anteil = 2;

					if ((CString)child->GetAttrValue("nachkommaanteil") == "mit")
						pFormularfeldDlg->m_nachkommaanteil = 0;
					else // nachkommaanteil == "ohne"
						pFormularfeldDlg->m_nachkommaanteil = 1;

					if ((CString)child->GetAttrValue("nullwertdarstellung") == "ja")
						pFormularfeldDlg->m_nNullwertdarstellung = 0;
					else
						pFormularfeldDlg->m_nNullwertdarstellung = 1;

					if ((CString)child->GetAttrValue("veraltet") == "ja")
						pFormularfeldDlg->m_veraltet = 1;
					else
						pFormularfeldDlg->m_veraltet = 0;

					pFormularfeldDlg->m_name = child->GetChildValue("name");
					pFormularfeldDlg->m_langform = child->GetChildValue("erweiterung");

					pFormularfeldDlg->m_csFormulardatei = m_csaFormulare[m_GewaehltesFormular];
					pFormularfeldDlg->m_bKeineFeldUeberschreibenMeldung = TRUE;
					pFormularfeldDlg->Create(IDD_FORMULARFELD, this);
					//pFormularfeldDlg->ShowWindow(SW_SHOW);

					pFormularfeldDlg->m_typ.SelectString(-1, child->GetAttrValue("typ"));
					pFormularfeldDlg->UpdateData(FALSE);
				}
				else
					AfxMessageBox("Es sind noch keine Felder in diesem Formular definiert. Bitte erst ein neues Feld erzeugen.");
			}
			break;
		case POPUPFORMULAR_KALKULATION_BEARBEITEN:
			break;
		case POPUPFORMULAR_FOLMULARDATEI_OEFFNEN:
			ShellExecute(m_hWnd, "open", "notepad.exe", m_csaFormulare[m_GewaehltesFormular], NULL, SW_SHOWNORMAL);
			break;
		case POPUPFORMULAR_FELDER_ANZEIGEN:
			m_bFormularfelderAnzeigen = !m_bFormularfelderAnzeigen;
			if (m_bFormularfelderAnzeigen) ((CMainFrame*)AfxGetMainWnd())->SetStatus("In diesem Modus können Formularfelder bearbeitet werden: drag&drop mit linker Maustaste, Gruppenselektion mit mittlerer Maustaste");
			GetDocument()->UpdateAllViews(NULL);
			InvalidateRect(NULL, FALSE);
			break;
		case POPUPFORMULAR_FELDWERT_KOPIEREN:
			// Formulardefinitionsdatei in xmldoc laden
			XDoc xmldoc;
			xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
			LPXNode xml = xmldoc.GetRoot();
			LPXNode felder = NULL;
			if (xml && (felder = xml->Find("felder")))
			{
				ptFeldmarke = PopUpPosition;
				int nNaechsteFeldIndex = GetFeldindexFromMausposition(felder);

				if (nNaechsteFeldIndex >= 0)
				{
					HGLOBAL hResult = GlobalAlloc(GMEM_MOVEABLE, m_csaFormularfeldwerte[nNaechsteFeldIndex].GetLength()+1);	// In die Zwischenablage 
					LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hResult); 
					memcpy(lptstrCopy, (LPCSTR)m_csaFormularfeldwerte[nNaechsteFeldIndex], m_csaFormularfeldwerte[nNaechsteFeldIndex].GetLength()+1); 
					GlobalUnlock(hResult);
					OpenClipboard(); 
					EmptyClipboard();
					HGLOBAL hmem = SetClipboardData(CF_TEXT, hResult);
					CloseClipboard();	
				}
			}
			break;
		}
	}

	int i;
	for (i = 0; i < einstellungen5->m_privat_split_size+1; i++)
	if (wParam == (UINT)POPUP_SPLIT+i)
	{
		if (pBuchungAendernDlg) 
			pBuchungAendernDlg->PrivatSplit(i);
		else
			buchenDlg->PrivatSplit(i);
	}

	return CScrollView::OnCommand(wParam, lParam);
}

void CEasyCashView::AfAAbgang(CBuchung **ppb)
{
	CEasyCashDoc *pDoc = GetDocument();

	if (ppb && (*ppb)->AbschreibungNr > 1 && 
		(AfxMessageBox((CString)"Anlagengegenstand aus dem Betriebsvermögen ausscheiden lassen? (Die AfA-Buchung wird dabei in eine einfache Ausgaben-Buchung über den Restwert umgewandelt.)", MB_YESNO) == IDYES))
	{
		CString csRestwertKonto;
		CString csRestwertFeldnummer = einstellungen2->m_land == 1 ? "9210" : "1135";
		char *pRestwertKonto = HoleKontoFuerFeld('A', csRestwertFeldnummer);
		CString csEURoderE1a = einstellungen2->m_land == 1 ? "E1a" : "EÜR";
		if (!pRestwertKonto)
		{
			csRestwertKonto = "Restbuchwert abgegangener Anlagengüter";
			if (einstellungen2->m_land == 0 || einstellungen2->m_land == 1)
				AfxMessageBox("Es wurde kein Konto gefunden, das mit dem Formularfeld " + csRestwertFeldnummer + " verknüpft ist. Deshalb wurde in der Buchung provisorisch das Konto '" 
							  + csRestwertKonto + "' eingetragen. Wenn Sie Formulare benutzen, sollten Sie dieses Ausgabenkonto in den Einstellungen -> E/Ü-Konten anlegen und dem " + csEURoderE1a + "-Feld "
							  + csRestwertFeldnummer + " zuweisen.");
		}
		else
			csRestwertKonto = pRestwertKonto;

		CTime ctUrspruenglichesAnschaffungsdatum = CTime((*ppb)->Datum.GetYear() - (*ppb)->AbschreibungNr + 1, (*ppb)->Datum.GetMonth(), (*ppb)->Datum.GetDay(), 0, 0, 0);
		CString csUrspruenglichesAnschaffungsdatum = ctUrspruenglichesAnschaffungsdatum.Format("%d.%m.%Y");
		char urspruenglicherBetrag[30];
		int_to_currency((*ppb)->Betrag, 20, urspruenglicherBetrag);
		char urspruenglicherNettobetrag[30];
		int_to_currency((*ppb)->GetNetto(), 20, urspruenglicherNettobetrag);
		CString csUrspruenglicheAbschreibungNr;
		csUrspruenglicheAbschreibungNr.Format("%d", (*ppb)->AbschreibungNr);
		CString csUrspruenglicheAbschreibungJahre;
		csUrspruenglicheAbschreibungNr.Format("%d", (*ppb)->AbschreibungJahre);
		char urspruenglicherRestwert[30];
		int_to_currency((*ppb)->AbschreibungRestwert, 20, urspruenglicherRestwert);
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglichesAnschaffungsdatum", csUrspruenglichesAnschaffungsdatum);	/// benötigt im Anlagenverzeichnis
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglichesKonto", (*ppb)->Konto);									//
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglicherBetrag", urspruenglicherBetrag);						//
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglicherNettobetrag", urspruenglicherNettobetrag);				//
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglicheAbschreibungNr", csUrspruenglicheAbschreibungNr);		//
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglicheAbschreibungJahre", csUrspruenglicheAbschreibungJahre);	// TODO: "Abgang rückgängigmachen"-Funktion!
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglicherRestwert", urspruenglicherRestwert);					//
		SetErweiterungKey((*ppb)->Erweiterung, "EasyCash", "UrspruenglichesBestandskonto", (*ppb)->Bestandskonto);				//
		(*ppb)->Datum = CTime(pDoc->nJahr, 1, 1, 0, 0, 0);
		(*ppb)->Betrag = (*ppb)->AbschreibungRestwert;
		(*ppb)->MWSt = 0;
		(*ppb)->AbschreibungRestwert = 0;
		(*ppb)->AbschreibungNr = 1;
		(*ppb)->AbschreibungJahre = 1;
		(*ppb)->Konto = csRestwertKonto;
		(*ppb)->Bestandskonto = "kalkulatorische Restbuchwerte (bitte ignorieren)";
		pDoc->SetModifiedFlag("Anlagengut wurde aus dem Betriebsvermögen entnommen. Ggf. müssen Veräußerungserlöse oder Entsorgungskosten noch separat gebucht werden.");
		pDoc->InkrementBuchungszaehler();
		pDoc->UpdateAllViews(NULL);
	}
}

int CEasyCashView::GetFeldindexFromMausposition(LPXNode felder)
{
	// Feld über Clickposition bestimmen...
	ptFeldmarke.x = ptFeldmarke.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414) * querformat_faktor);
	ptFeldmarke.y = ptFeldmarke.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);

	int nNaechsteFeldIndex = -1, nNaechsteFeldIDNaehe = INT_MAX;

	if (felder && felder->GetChildCount() > 0)
	{
		LPXNode child;

		// besteht ID bereits?
		for(int i = 0; i < felder->GetChildCount(); i++)
		{
			child = felder->GetChild(i);
			if (child)
			{
				CString csTemp = child->GetChildValue("name");
				int dx = ptFeldmarke.x - atoi(child->GetAttrValue("horizontal"));
				int dy = ptFeldmarke.y - atoi(child->GetAttrValue("vertikal")) - ((atoi(child->GetAttrValue("seite"))-1) * 1414);
				if (dx*dx + dy*dy <= nNaechsteFeldIDNaehe)
				{
					nNaechsteFeldIDNaehe = dx*dx + dy*dy;	// Pythagoras...
					nNaechsteFeldIndex = i;								
				}
			}
		}
	}
	return nNaechsteFeldIndex;
}

void CEasyCashView::OnMouseMove(UINT nFlags, CPoint point) 
{
/*	CEasyCashDoc *pDoc = GetDocument();
	int index;
	index = point.y / charheight;
	if (ppPosBuchungsliste[index])
		nSelected = index;
	else
		nSelected = -index;	// deselektieren
	
	RedrawWindow();
	pDoc->UpdateAllViews(NULL);

Code zu hektisch */	

	if (pFormularfeldDlg) 
	{
		CPoint ptDoc;
		ptDoc = GetScrollPosition() + point;
		CString csMouseposText;
		csMouseposText.Format("Mausposition: Seite %d, %d, %d", 
			(ptDoc.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight)) / 1414 + 1, 
			(ptDoc.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight)) % 1414,
			ptDoc.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414)));
		pFormularfeldDlg->SetDlgItemText(IDC_MOUSEPOS, csMouseposText);
	}

	if (m_nFeldMove >= 0)
	{
		CPoint ptFeldmarkeAlt = ptFeldmarke;
		ptFeldmarke.x = ptFeldmarkeAlt.x + ((GetScrollPosition().x + point.x - ptLetzteMousePosition.x) * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight));
		ptFeldmarke.y = ptFeldmarkeAlt.y + ((GetScrollPosition().y + point.y - ptLetzteMousePosition.y) * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414))); 
		if (ptFeldmarke != ptFeldmarkeAlt) 
		{
			//RedrawWindow();
			GetDocument()->UpdateAllViews(NULL);
		}
		ptLetzteMousePosition = GetScrollPosition() + point;
		
	}

	if (m_GewaehltesFormular >= 0 && m_csaFormulare.GetSize() > 0)
	{
		// Formulardefinitionsdatei in xmldoc laden
		XDoc xmldoc;
		xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
		LPXNode xml = xmldoc.GetRoot();
		LPXNode felder = NULL;
		if (xml && (felder = xml->Find("felder")))
		{
			ptFeldmarke = GetScrollPosition() + point;				
			int nNaechsteFeldIndex = GetFeldindexFromMausposition(felder);

			if (nNaechsteFeldIndex >= 0)
			{
				// Feld ID über Index holen
				LPXNode child;
				int nNaechsteFeldId = -1;
				child = felder->GetChild(nNaechsteFeldIndex);			
				if (child && !child->value.IsEmpty())
					nNaechsteFeldId = atoi(child->GetAttrValue("id"));

				if (nNaechsteFeldId >= 0)
				{
					CEasyCashDoc *pDoc = GetDocument();
					if (pDoc->m_csaFeldStatustext[nNaechsteFeldId] != "")
						((CMainFrame*)AfxGetMainWnd())->SetStatus(pDoc->m_csaFeldStatustext[nNaechsteFeldId]);
					else
					{
						CString csTemp;
						csTemp.Format("Feldnummer %d", nNaechsteFeldId);
						((CMainFrame*)AfxGetMainWnd())->SetStatus(csTemp);
					}
				}
			}
		}
	}

	CScrollView::OnMouseMove(nFlags, point);
}

void CEasyCashView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (m_GewaehltesFormular < 0)
	{
		PopUpPosition = GetScrollPosition() + point;
		PostMessage(WM_COMMAND, POPUP_AENDERN, 0);
	}

	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CEasyCashView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_GewaehltesFormular < 0)
	{
		int index = (point.y + GetScrollPosition().y) / charheight;  // Buchungsindex aus Mausposition

		if (ppPosBuchungsliste[index])	// wenn Buchung dort vorhanden
		{
			if (index == nSelected)		// Zeile bereits selektiert?
				nSelected = -nSelected;	// dann deselektieren
			else 
				nSelected = index;		// sonst selektieren

			RedrawSelection();
		}		
	}

	if (m_bFormularfelderAnzeigen && m_GewaehltesFormular >= 0)
	{
		// Feld über Clickposition bestimmen...
		ptFeldmarke = GetScrollPosition() + point;
		ptFeldmarke.x = ptFeldmarke.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414));
		ptFeldmarke.y = ptFeldmarke.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);
	
		m_ptLinkerMausButtonDown = ptFeldmarke;

		// Formulardefinitionsdatei in xmldoc laden
		XDoc xmldoc;
		xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
		LPXNode xml = xmldoc.GetRoot();
		LPXNode felder = NULL;
		if (xml && (felder = xml->Find("felder")) && felder->GetChildCount() > 0)
		{
			LPXNode child;

			// besteht ID bereits?
			int nNaechsteFeldIndex = -1, nNaechsteFeldIDNaehe = INT_MAX;
			for(int i = 0; i < felder->GetChildCount(); i++)
			{
				child = felder->GetChild(i);
				if (child)
				{
					CString csTemp = child->GetChildValue("name");
					int dx = ptFeldmarke.x - atoi(child->GetAttrValue("horizontal"));
					int dy = ptFeldmarke.y - atoi(child->GetAttrValue("vertikal")) - ((atoi(child->GetAttrValue("seite"))-1) * 1414);
					if (dx*dx + dy*dy <= nNaechsteFeldIDNaehe)
					{
						nNaechsteFeldIDNaehe = dx*dx + dy*dy;	// Pythagoras...
						nNaechsteFeldIndex = i;								
					}
				}
			}

			if (nNaechsteFeldIndex >= 0)
			{				
				child = felder->GetChild(nNaechsteFeldIndex);
				//id = atoi(child->GetAttrValue("id"));
				
				//pFormularfeldDlg->m_seite = atoi(child->GetAttrValue("seite"));						
				ptFeldmarke.x = atoi(child->GetAttrValue("horizontal"));						
				ptFeldmarke.y = atoi(child->GetAttrValue("vertikal")) +  ((atoi(child->GetAttrValue("seite"))-1) * 1414);
				LPCTSTR cp = child->GetAttrValue("ausrichtung");
				m_bFeldmarkeRechtsbuendig = !stricmp(cp,"rechtsbuendig");	// rechtsbündig?
				ptLetzteMousePosition = GetScrollPosition() + point;
			}

			m_nFeldMove = nNaechsteFeldIndex;
		}

	}
	
	CScrollView::OnLButtonDown(nFlags, point);
}

void CEasyCashView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bFormularfelderAnzeigen && m_GewaehltesFormular >= 0)
	{
		//RedrawWindow();
		GetDocument()->UpdateAllViews(NULL);

		CPoint ptLinkerMausButtonUp = GetScrollPosition() + point;
		ptLinkerMausButtonUp.x = ptLinkerMausButtonUp.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414));
		ptLinkerMausButtonUp.y = ptLinkerMausButtonUp.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);
		CSize downUpDifferenz = ptLinkerMausButtonUp - m_ptLinkerMausButtonDown;

		if (downUpDifferenz == CSize(0, 0))
		{
			m_ptFeldMoveMultiselect.RemoveAll();  // bei einem einfachem Mausklick: Selektion löschen
			GetDocument()->UpdateAllViews(NULL);
		}

		// Formulardefinitionsdatei in xmldoc laden
		XDoc xmldoc;
		xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
		LPXNode xml = xmldoc.GetRoot();
		LPXNode felder = NULL;
		if (xml && (felder = xml->Find("felder")) && felder->GetChildCount() > 0)  // Multiselect-Gruppe bewegen
		{
			LPXNode child;

			if (m_ptFeldMoveMultiselect.GetCount() > 0)
			{
				for(int i = 0; i < felder->GetChildCount(); i++)
				{			
					child = felder->GetChild(i);
					if (child && !child->value.IsEmpty())
					{
						int id = atoi(child->GetAttrValue("id"));
						for (int j = 0; j < m_ptFeldMoveMultiselect.GetCount(); j++)
							if (m_ptFeldMoveMultiselect[j] == id)
								MoveFormularfeld(child, downUpDifferenz);
					}
				}
			}
			else if (m_nFeldMove >= 0)  // einzelnes Feld bewegen
			{		
				child = felder->GetChild(m_nFeldMove);
				if (child && !child->value.IsEmpty())
					MoveFormularfeld(child, downUpDifferenz);
			}
		}
	
		DISP_OPT opt;
		opt.newline = true; // no new line
		if (!xmldoc.SaveFile(m_csaFormulare[m_GewaehltesFormular], &opt))
		{
			AfxMessageBox("Konnte die Änderung der Feldposition nicht in der Formulardatei speichern.");
			return;
		}

		m_nFeldMove = -1;
	}
	
	CScrollView::OnLButtonUp(nFlags, point);
}

void CEasyCashView::MoveFormularfeld(LPXNode child, CSize delta)
{
	int nAlteSeite = atoi(child->GetAttrValue("seite"));
	CPoint ptAltePosition, ptNeuePosition;
	ptAltePosition.x = atoi(child->GetAttrValue("horizontal"));
	ptAltePosition.y = atoi(child->GetAttrValue("vertikal")) + (nAlteSeite - 1) * 1414;
	ptNeuePosition = ptAltePosition + delta;

	CString csSeite, csHorizontal, csVertikal;
	_ultoa((DWORD)ptNeuePosition.y / 1414 + 1, csSeite.GetBuffer(30), 10);
	_ultoa((DWORD)ptNeuePosition.x, csHorizontal.GetBuffer(30), 10);
	_ultoa((DWORD)ptNeuePosition.y % 1414, csVertikal.GetBuffer(30), 10);

	 child->GetAttr("seite")->value = csSeite;
	child->GetAttr("horizontal")->value = csHorizontal;
	child->GetAttr("vertikal")->value = csVertikal;
}

void CEasyCashView::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (m_bFormularfelderAnzeigen && m_GewaehltesFormular >= 0)
	{
		// Position von Mittlerer Mausknopf Down bestimmen...
		m_ptMittlererMausButtonDown = GetScrollPosition() + point;
		m_ptMittlererMausButtonDown.x = m_ptMittlererMausButtonDown.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414));
		m_ptMittlererMausButtonDown.y = m_ptMittlererMausButtonDown.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);
		
		m_ptFeldMoveMultiselect.RemoveAll();  // ggf. alter Multi-Selektion löschen
	}

	CScrollView::OnMButtonDown(nFlags, point);
}

void CEasyCashView::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (m_bFormularfelderAnzeigen && m_GewaehltesFormular >= 0)
	{
		m_ptFeldMoveMultiselect.RemoveAll();  // Array mit den IDs der selektierten Felder

		// Position von Mittlerer Mausknopf Up bestimmen...
		CPoint ptMittlererMausButtonUp = GetScrollPosition() + point;
		ptMittlererMausButtonUp.x = ptMittlererMausButtonUp.x * 1000 / (int)((double)(charheight * (VCHARS + PAGE_GAP) * 1000 / 1414));
		ptMittlererMausButtonUp.y = ptMittlererMausButtonUp.y * 1414 / (int)((double)(VCHARS + PAGE_GAP) * charheight);

		int multi1x, multi2x, multi1y, multi2y;
		if (m_ptMittlererMausButtonDown.x < ptMittlererMausButtonUp.x)
		{
			multi1x = m_ptMittlererMausButtonDown.x;
			multi2x = ptMittlererMausButtonUp.x;
		}
		else
		{
			multi2x = m_ptMittlererMausButtonDown.x;
			multi1x = ptMittlererMausButtonUp.x;
		}
		if (m_ptMittlererMausButtonDown.y < ptMittlererMausButtonUp.y)
		{
			multi1y = m_ptMittlererMausButtonDown.y;
			multi2y = ptMittlererMausButtonUp.y;
		}
		else
		{
			multi2y = m_ptMittlererMausButtonDown.y;
			multi1y = ptMittlererMausButtonUp.y;
		}

		// mehr als nur minimale Distanz zwischen MouseUp und MouseDown? Dann Selektion machen.
		CSize sizeDrag = m_ptMittlererMausButtonDown - ptMittlererMausButtonUp;
		if (abs(sizeDrag.cx) + abs(sizeDrag.cy) > 10)
		{
			// Formulardefinitionsdatei in xmldoc laden
			XDoc xmldoc;
			xmldoc.LoadFile(m_csaFormulare[m_GewaehltesFormular]);
			LPXNode xml = xmldoc.GetRoot();
			LPXNode felder = NULL;
			if (xml && (felder = xml->Find("felder")) && felder->GetChildCount() > 0)
			{
				LPXNode child;

				// alle Felder im aufgezogenen rect ins array m_ptFeldMoveMultiselect schreiben
				for(int i = 0; i < felder->GetChildCount(); i++)
				{
					child = felder->GetChild(i);
					if (child)
					{
						CString csTemp = child->GetChildValue("name");
						int x = atoi(child->GetAttrValue("horizontal"));
						int y = atoi(child->GetAttrValue("vertikal")) + ((atoi(child->GetAttrValue("seite"))-1) * 1414);
						if (x >= multi1x && x <= multi2x && y >= multi1y && y <= multi2y)
							m_ptFeldMoveMultiselect.Add(atoi(child->GetAttrValue("id")));
					}
				}
			}
		}
		else
			m_ptFeldMoveMultiselect.RemoveAll();  // ansonsten bei einem einfachen Mittlere-Maustaste-Klick: Selektion löschen

		GetDocument()->UpdateAllViews(NULL);
	}

	CScrollView::OnMButtonUp(nFlags, point);
}

void CEasyCashView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
/*
	if (bActivate)
	{
		// Menü verschönern
		CMenu *pMenu = pActivateView->GetMenu();
		if (pMenu)
		{
			bmpEinBuch.LoadBitmap(IDB_SPIKE_PLUS);
			pMenu->AppendMenu(MF_BYCOMMAND, 
				ID_EDIT_EINNAHME_BUCHEN, &bmpEinBuch);

		}
	}	
*/
}

BOOL CEasyCashView::Backup(char *backupdir)
{
	CEasyCashDoc *pDoc = GetDocument();
	DWORD dw = GetFileAttributes(backupdir);
	if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
	{
		AfxMessageBox("Backup-Verzeichnis kann nicht gefunden werden. Bitte Berechtigungen prüfen und nochmals versuchen");
		return FALSE;
	}
	else
	{
		char path[1000], inipath[1000], epath[1000], apath[1000];
		CString csOldPathName;

		strcpy(path, backupdir);
		sprintf(path + strlen(path), "\\Jahr%04d.eca", pDoc->nJahr);
		csOldPathName = pDoc->GetPathName();
		pDoc->SetPathName(path);			
		BOOL bModified = pDoc->IsModified();
		pDoc->OnSaveDocument(path);	
		pDoc->SetPathName(csOldPathName);
		pDoc->SetModifiedFlag("Datensicherung wurde in " + (CString)backupdir + " angelegt", bModified);

		strcpy(path, backupdir);
		strcat(path, "\\easyct.ini");
		if (GetIniFileName(inipath, sizeof(inipath)))
			if (!CopyFile(inipath, path, FALSE))
				AfxMessageBox("easyct.ini konnte nicht kopiert werden");

		strcpy(epath, backupdir);
		strcat(epath, "\\easyct_E.csv");
		strcpy(apath, backupdir);
		strcat(apath, "\\easyct_A.csv");
		Export(epath, apath, FALSE);	


		strcpy(path, backupdir);
		strcat(path, "\\EC&T-Registrierungsinformationen.ectreg");
		theApp.RegistrierungsinformationenSichern(path);

		return TRUE;
	}
}

#ifndef CSIDL_CDBURN_AREA
   #define CSIDL_CDBURN_AREA          0x003B
#endif

void CEasyCashView::OnFileBackupSubfolder()
{
	char backupdir[500], *cp;
	if (!GetIniFileName(backupdir, sizeof(backupdir)-30) || !(cp = strrchr(backupdir, '\\')))
	{
		AfxMessageBox("Die Datensicherung konnte nicht erfolgreich durchgeführt werden, weil das Datenverzeichnis nicht ermittelt werden konnte.");
		return;
	}
	CTime now = CTime::GetCurrentTime();
	CString csTimestamp = now.Format("\\Backup%Y-%m-%d-%H%M%S");
	strcpy(cp, csTimestamp);

	if (CreateDirectory(backupdir, NULL) && Backup(backupdir))
	{
		CString csMessageText;
		csMessageText.Format("Die Datensicherung wurde erfolgreich nach '%s' durchgeführt.", backupdir);
		AfxMessageBox(csMessageText);
	}
	else
		AfxMessageBox("Die Datensicherung konnte nicht erfolgreich durchgeführt werden. Bitte kopieren Sie die relevanten Dateien manuell auf ein externes Sicherungsmedium.");
}

void CEasyCashView::OnFileBackupExternal()
{
	CString csBackupDir = theApp.GetProfileString("Backup", "ExternalBackupDir", "");

	if (csBackupDir == "")
		if (!SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(), csBackupDir.GetBuffer(MAX_PATH), CSIDL_DRIVES, FALSE))
			csBackupDir = "C:\\";
	csBackupDir.ReleaseBuffer();

	// Backupverzeichnis wählen
	if (SelectFolder(csBackupDir.GetBuffer(MAX_PATH), "Backup-Medium auswählen (ein Unterverzeichnis mit Zeitstempel wird darin erstellt)")) 
	{
		csBackupDir.ReleaseBuffer();
		theApp.WriteProfileString("Backup", "ExternalBackupDir", csBackupDir);

		if (csBackupDir.GetLength() && csBackupDir[csBackupDir.GetLength()-1] == '\\')
			csBackupDir = csBackupDir.Left(csBackupDir.GetLength()-1);

		CTime now = CTime::GetCurrentTime();
		CString csTimestamp = now.Format("\\ECTBackup%Y-%m-%d-%H%M%S");
		csBackupDir += csTimestamp;

		if (CreateDirectory(csBackupDir, NULL) && Backup(csBackupDir.GetBuffer(0)))
		{
			CString csMessageText;
			csMessageText.Format("Die Datensicherung wurde erfolgreich nach '%s' durchgeführt.", csBackupDir.GetBuffer(0));
			AfxMessageBox(csMessageText);
		}
		else
			AfxMessageBox("Die Datensicherung konnte nicht erfolgreich durchgeführt werden. Bitte kopieren Sie die relevanten Dateien manuell auf ein externes Sicherungsmedium.");
	}
}

void CEasyCashView::OnFileBackup() // auf CD/DVD
{
	char backupdir[MAX_PATH];
	if (!SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(), backupdir, CSIDL_CDBURN_AREA, FALSE))
		strcpy(backupdir, "C:\\");

	{
		CString csBackupDir;
    	csBackupDir = backupdir;

		if (csBackupDir.GetLength() && csBackupDir[csBackupDir.GetLength()-1] == '\\')
			csBackupDir = csBackupDir.Left(csBackupDir.GetLength()-1);

		CTime now = CTime::GetCurrentTime();
		CString csTimestamp = now.Format("\\ECTBackup%Y-%m-%d-%H%M%S");
		csBackupDir += csTimestamp;

		if (CreateDirectory(csBackupDir, NULL) && Backup(csBackupDir.GetBuffer(0)))
		{
			CString csMessageText;
			csMessageText.Format("Die Datensicherung wurde erfolgreich nach '%s' durchgeführt. Die Disk kann jetzt gebrannt werden.", csBackupDir.GetBuffer(0));
			AfxMessageBox(csMessageText);
		}
		else
			AfxMessageBox("Die Datensicherung konnte nicht erfolgreich durchgeführt werden. Bitte brennen Sie die relevanten Dateien manuell auf eine disk.");
	}
}

void CEasyCashView::OnFileJahreswechsel() 
{
	CEasyCashDoc *pDoc = GetDocument();

	// sind da noch Dauerbuchungen auszuführen?
	CTime aktualisiert_bis_jahresende(pDoc->nJahr, 12, 31, 0, 0, 0);
	CString csMeldung = "";
	CDauerbuchung *dbp;
	for (dbp = pDoc->Dauerbuchungen; dbp; dbp = dbp->next)
	{
		if (dbp->AktualisiertBisDatum < dbp->BisDatum && dbp->AktualisiertBisDatum < aktualisiert_bis_jahresende)
		{
			if (csMeldung != "") csMeldung += ", ";
			csMeldung += dbp->Beschreibung;
		}
	}
	if (csMeldung != "")
		if (AfxMessageBox((CString)"Achtung: Es wurde(n) folgende Dauerbuchung(en) noch nicht bis zum Jahresende bzw. Bis-Datum ausgeführt: " + csMeldung + "\r\n\r\nSoll jetzt wirklich der Jahresabschluss gemacht werden?", MB_YESNO) == IDNO)
			return;

	CString csMessageText;
	csMessageText.Format("Es wird dringend empfohlen am Ende des Jahres eine Sicherungskopie der Daten anzulegen. Es wird jetzt im Datenverzeichnis ein Unterverzeichnis 'Backup%04d-Jahreswechsel' angelegt, wo die Daten des beendeten Buchungsjahrs gespeichert werden. Kopieren Sie das Verzeichnis möglichst gleich auf ein externes Medium, z.B. einen USB-Speicherstick. Um die gesetzlichen Vorgaben vollständig zu erfüllen, muss die Datensicherung aber auf einer CD-ROM geschehen, die nicht mehr geändert werden kann. Gespeichert wird die Buchungsdatei (Jahr%04d.eca), die Einstellungsdaten inkl. Kontenrahmen (easyct.ini) und jeweils ein CSV-Export der Einnahmen und Ausgaben (easyct_E.csv und easyct_A.csv). Die CD-ROM muss dann für min. 10 Jahre an einem sicheren Ort aufbewahrt werden.", pDoc->nJahr, pDoc->nJahr);
	if (AfxMessageBox(csMessageText, MB_OKCANCEL) == IDOK)
	{
		char backupdir[500];
		strcpy(backupdir, pDoc->GetPathName());
		char *cp = strrchr(backupdir, '\\');
		if (cp)
			*++cp = '\0';
		else
			cp = backupdir;

		sprintf(cp, "Backup%04d-Jahreswechsel", pDoc->nJahr);

		AfxMessageBox("Als erstes wird eine Kopie der Buchungsdatei des _alten_ Jahres im _Datensicherungsverzeichnis_ angelegt.");

		if (CreateDirectory(backupdir, NULL) && Backup(backupdir))
		{
			csMessageText.Format("Das Datensicherungsverzeichnis '%s' wurde erfolgreich angelegt und kann jetzt auf ein externes Medium kopiert werden. Speichern Sie als Nächstes die _neu_erzeugte_ Jahres-Buchungsdatei mit einem anderen Namen als die Datei des alten Jahres. Im Zweifelsfall beim 'Speichern unter' Dialog einfach nur den Speichern-Knopf drücken, ohne zuvor eine existierende Datei auszuwählen und damit evtl. versehentlich zu überschreiben!", backupdir);
			AfxMessageBox(csMessageText);
		}
		else
			AfxMessageBox("Die Datensicherung konnte nicht erfolgreich durchgeführt werden. Bitte kopieren Sie die relevanten Dateien manuel auf ein externes Sicherungsmedium.");
	}

	CEasyCashDoc *pNewDoc;	
	pNewDoc = (CEasyCashDoc*)pDoc->GetDocTemplate()->CreateNewDocument();

	CBuchung **ppB, *pB;

	pNewDoc->Buchungszaehler = pDoc->Buchungszaehler;
	pNewDoc->Einnahmen = NULL;
	pNewDoc->nJahr = pDoc->nJahr + 1;
	// AfA ab 2004 in BRD ändern
	if (pNewDoc->nJahr == 2004 && einstellungen2->m_land == 0/*BRD*/) 
	{
		CAfAGenauigkeit dlg;
		dlg.m_afa_genauigkeit = MONATSGENAUE_AFA;
		if (dlg.DoModal() == IDOK)
			pNewDoc->AbschreibungGenauigkeit = dlg.m_afa_genauigkeit;
		else
		{
			pNewDoc->AbschreibungGenauigkeit = MONATSGENAUE_AFA;
			AfxMessageBox("Monatsgenaue AfA wird angenommen.");
		}
	}
	pNewDoc->csWaehrung = pDoc->csWaehrung;
	pNewDoc->csUrspruenglicheWaehrung = pDoc->csUrspruenglicheWaehrung;
	pNewDoc->Erweiterung = pDoc->Erweiterung;
	
	// akt. Dokument selektiv duplizieren - Ausgaben
	ppB = &(pNewDoc->Ausgaben);
	pB  = pDoc->Ausgaben;
	CString csSlashAltesJahr;
	csSlashAltesJahr.Format("/%04d", pDoc->nJahr);
	while (pB)
	{
		if (pB->AbschreibungJahre > 1 && pB->AbschreibungRestwert > pB->GetBuchungsjahrNetto(pDoc))
		{
			*ppB = new CBuchung;
			**ppB = *pB;
			if ((*ppB)->Belegnummer.GetLength() && (*ppB)->Belegnummer.Mid((*ppB)->Belegnummer.GetLength()-5, 3) != "/20") (*ppB)->Belegnummer = (*ppB)->Belegnummer + csSlashAltesJahr;
			(*ppB)->AbschreibungRestwert -= (*ppB)->GetBuchungsjahrNetto(pDoc);
			(*ppB)->AbschreibungNr++;
			(*ppB)->Datum = CTime((*ppB)->Datum.GetYear() + 1, (*ppB)->Datum.GetMonth(), (*ppB)->Datum.GetDay(), 0, 0, 0);
			(*ppB)->next = NULL;
			ppB   = &((*ppB)->next);
		}
		pB    = pB->next;
	}

	CDauerbuchung **ppD, *pD;

	// akt. Dokument komplett duplizieren - Dauerbuchungen
	ppD = &(pNewDoc->Dauerbuchungen);
	pD  = pDoc->Dauerbuchungen;
	while (pD)
	{
		*ppD = new CDauerbuchung;
		**ppD = *pD;
		(*ppD)->next = NULL;
		ppD   = &((*ppD)->next);

		pD    = pD->next;
	}

	if (pNewDoc->nJahr >= 2002)
	{
		int i;
		for (i = 1; i < 11; i++)
			if (pNewDoc->csWaehrung == CEasyCashDoc::GetWaehrungskuerzel(i))
			{
				if (pNewDoc->ConvertToEuro())
					AfxMessageBox("Die Buchungsdaten wurden in Euro umgerechnet");
				else
					AfxMessageBox("Bei der Umrechnung in Euro ist ein Fehler aufgetreten. Daten sind evtl. inkonsistent.");
				break;
			}
	}

	///////pDoc->GetDocTemplate()->AddDocument(pNewDoc);
	/////CFrameWnd *pfw = pDoc->GetDocTemplate()->CreateNewFrame(pNewDoc, NULL);

	pNewDoc->nLaufendeBuchungsnummerFuerEinnahmen = 1;
	pNewDoc->nLaufendeBuchungsnummerFuerAusgaben = 1;


	char buf[500];
	strcpy(buf, pDoc->GetPathName());
	char *cp = strrchr(buf, '\\');
	if (cp)
		*++cp = '\0';
	else
		cp = buf;

	sprintf(cp, "Jahr%04d.eca", pNewDoc->nJahr);
	pNewDoc->SetPathName(buf);
	
	pNewDoc->SetModifiedFlag("Neue Buchungsdatei wurde über Jahreswechsel generiert");
	pNewDoc->SavePublic();
	strcpy(buf, pNewDoc->GetPathName());
	delete pNewDoc;

	// Bestandskontensalden speichern als Übertrag ins nächste Jahr
	int j;
	for (j = 0; j < m_csaBestandskontenNamen.GetSize(); j++)
	{
		int nSaldo = currency_to_int(m_csaBestandskontenSalden[j].GetBuffer(0));	

		CBuchung *p = pDoc->Einnahmen;
		while (p)
		{
			if (p->Bestandskonto == m_csaBestandskontenNamen[j])
				nSaldo += p->Betrag;
			p = p->next;
		}
		p = pDoc->Ausgaben;
		while (p)
		{
			if (p->Bestandskonto == m_csaBestandskontenNamen[j] && p->AbschreibungNr <= 1)
				nSaldo -= p->Betrag;
			p = p->next;
		}

		CString csKey;
		char saldobuffer[100];
		int_to_currency(nSaldo, 10, saldobuffer);
		csKey.Format("Bestandskonto%-02.2dSaldo%04d", j, pDoc->nJahr);
		char inifile[1000]; 
		GetIniFileName(inifile, sizeof(inifile)); 
		WritePrivateProfileString("Bestandskonten", csKey.GetBuffer(0), saldobuffer, inifile);
	}

	if (GetFileAttributes(buf) != 0xFFFFFFFF && AfxMessageBox("Soll das neue Buchungsjahr jetzt gleich geöffnet werden?", MB_YESNO) == IDYES)
	{
		//pDoc->OnOpenDocument(buf); <-- katastrophaler Fehler !!!

		// besser so:
		CCommandLineInfo cmdInfo;
		cmdInfo.m_strFileName = buf;
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileOpen;
		if (!theApp.ProcessShellCommand(cmdInfo))
		{
			AfxMessageBox("Problem beim Öffnen des neuen Buchungsjahrs.");
		}
	}

	//pfw->ShowWindow(SW_SHOW);
	//pfw->SendMessage(WM_INITIALUPDATE);
	//pfw->SetFocus();
}


void CEasyCashView::OnFileExport() 
{
	Export(NULL, NULL, TRUE);
}

BOOL CEasyCashView::Export(char *_Efilename, char *_Afilename, BOOL bExplorerOeffnen)
{ 
	extern CEasyCashApp theApp;
	char EinnahmenExportDateiname[1000];
	char AusgabenExportDateiname[1000];
	*EinnahmenExportDateiname = '\0';
	*AusgabenExportDateiname = '\0';

	CEasyCashDoc *pDoc = GetDocument();

	CString cse;
	if (!_Efilename)
	{
		if (!GetIniFileName(EinnahmenExportDateiname, sizeof(EinnahmenExportDateiname))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return FALSE; }
		strcpy(EinnahmenExportDateiname+strlen(EinnahmenExportDateiname)-4, "_E.csv");
		strcpy(AusgabenExportDateiname, EinnahmenExportDateiname);
		CFileDialog cf(FALSE, "*.CSV", EinnahmenExportDateiname, 0, "Character Separated Value Datei (*.csv)|*.CSV||",this);
		if (cf.DoModal() != IDOK)
			return FALSE;
		cse = cf.GetPathName();
		strcpy(EinnahmenExportDateiname, cse.GetBuffer(0));
	}
	else
	{
		strcpy(EinnahmenExportDateiname, _Efilename);
		cse = EinnahmenExportDateiname;
	}

	CFile File;

	if (File.Open(cse, CFile::modeWrite|CFile::modeCreate))
	{
		CBuchung *pB = pDoc->Einnahmen;

		char *header_einnahmen = "Datum;Belegnummer;Betrag;MWSt-Betrag;Beschreibung;Konto;Betrieb;Bestandskonto\r\n";
		TRY
		{
			File.Write(header_einnahmen, strlen(header_einnahmen));
		}
		CATCH(CFileException, e)
		{
			AfxMessageBox("Fehler beim Schreiben der Export-Datei", MB_ICONSTOP);
			File.Close();
			return FALSE;
		}
		END_CATCH

		while (pB)
		{
			char buffer[1000];
			char belegnummer_buffer[1000];
			char buffer2[1000];
			char buffer3[20];
			char buffer4[1000];
			char mwstbuf[20];
			char betrieb_buffer[1000];
			char bestandskonto_buffer[1000];

			strcpy (belegnummer_buffer, pB->Belegnummer);
			int i;
			for (i = 0; i < (int)strlen(belegnummer_buffer); i++)
				if (belegnummer_buffer[i] == '"') belegnummer_buffer[i] = '\'';

			strcpy (buffer2, pB->Beschreibung);
			for (i = 0; i < (int)strlen(buffer2); i++)
				if (buffer2[i] == '"') buffer2[i] = '\'';

			strcpy (buffer4, pB->Konto);
			for (i = 0; i < (int)strlen(buffer4); i++)
				if (buffer4[i] == '"') buffer4[i] = '\'';

			int_to_currency(pB->Betrag, 8, buffer3);

			strcpy (betrieb_buffer, pB->Betrieb);
			for (i = 0; i < (int)strlen(betrieb_buffer); i++)
				if (betrieb_buffer[i] == '"') betrieb_buffer[i] = '\'';

			strcpy (bestandskonto_buffer, pB->Bestandskonto);
			for (i = 0; i < (int)strlen(bestandskonto_buffer); i++)
				if (bestandskonto_buffer[i] == '"') bestandskonto_buffer[i] = '\'';

			pB->GetMWSt(mwstbuf);
			sprintf(buffer, "%02d.%02d.%04d;%s;\"%s\";%s;\"%s\";\"%s\";\"%s\";\"%s\"\r\n",
				pB->Datum.GetDay(), pB->Datum.GetMonth(), pB->Datum.GetYear(), 
				belegnummer_buffer,
				buffer3,
				mwstbuf,
				buffer2,
				buffer4,
				betrieb_buffer,
				bestandskonto_buffer);

			TRY
			{
				File.Write(buffer, strlen(buffer));
			}
			CATCH(CFileException, e)
			{
				AfxMessageBox("Fehler beim Schreiben der Export-Datei", MB_ICONSTOP);
				File.Close();
				return FALSE;
			}
			END_CATCH

			pB = pB->next;
		}

		File.Close();

		//-- Ausgaben --
		CString csa;
		if (!_Afilename)
		{
			if (strlen(EinnahmenExportDateiname) > 6 && !stricmp(EinnahmenExportDateiname + strlen(EinnahmenExportDateiname) - 6, "_E.csv"))
			{
				strcpy(AusgabenExportDateiname, EinnahmenExportDateiname);
				strcpy(AusgabenExportDateiname + strlen(AusgabenExportDateiname) - 6, "_A.csv");
				csa = AusgabenExportDateiname;
				strcpy(AusgabenExportDateiname, csa.GetBuffer(0));
			}
			else
			{
				strcpy(AusgabenExportDateiname+strlen(AusgabenExportDateiname)-6, "_A.csv");
				CFileDialog cf(FALSE, "*.CSV", AusgabenExportDateiname, 0, "Character Separated Value Datei (*.csv)|*.CSV||",this);
				if (cf.DoModal() != IDOK)
					return FALSE;
				csa = cf.GetPathName();
				strcpy(AusgabenExportDateiname, csa.GetBuffer(0));
			}
		}
		else
		{
			strcpy(AusgabenExportDateiname, _Afilename);
			csa = AusgabenExportDateiname;
		}

		if (File.Open(csa, CFile::modeWrite|CFile::modeCreate))
		{
			CBuchung *pB = pDoc->Ausgaben;

			char *header_ausgaben = "Datum;Belegnummer;Betrag;MWSt-Satz;Beschreibung;Konto;Betrieb;Bestandskonto;Abschreibung-Nr;Abschreibung-Jahre;Restwert;Buchungsjahr-Netto\r\n";
			TRY
			{
				File.Write(header_ausgaben, strlen(header_ausgaben));
			}
			CATCH(CFileException, e)
			{
				AfxMessageBox("Fehler beim Schreiben der Export-Datei", MB_ICONSTOP);
				File.Close();
				return FALSE;
			}
			END_CATCH

			while (pB)
			{
				char buffer[1000];
				char belegnummer_buffer[1000];
				char buffer2[1000];
				char buffer3[20];
				char restwert_buffer[1000];
				char buffer4[1000];
				char mwstbuf[20];
				char betrieb_buffer[1000];
				char bestandskonto_buffer[1000];
				char buchungsjahr_netto_buffer[20];

				strcpy (belegnummer_buffer, pB->Belegnummer);
				int i;
				for (i = 0; i < (int)strlen(belegnummer_buffer); i++)
					if (belegnummer_buffer[i] == '"') belegnummer_buffer[i] = '\'';

				strcpy (buffer2, pB->Beschreibung);
				for (i = 0; i < (int)strlen(buffer2); i++)
					if (buffer2[i] == '"') buffer2[i] = '\'';

				strcpy (buffer4, pB->Konto);
				for (i = 0; i < (int)strlen(buffer4); i++)
					if (buffer4[i] == '"') buffer4[i] = '\'';				

				int_to_currency(pB->Betrag, 8, buffer3);

				strcpy (betrieb_buffer, pB->Betrieb);
				for (i = 0; i < (int)strlen(betrieb_buffer); i++)
					if (betrieb_buffer[i] == '"') betrieb_buffer[i] = '\'';

				strcpy (bestandskonto_buffer, pB->Bestandskonto);
				for (i = 0; i < (int)strlen(bestandskonto_buffer); i++)
					if (bestandskonto_buffer[i] == '"') bestandskonto_buffer[i] = '\'';

				int_to_currency(pB->AbschreibungRestwert, 8, restwert_buffer);

				int_to_currency(pB->GetBuchungsjahrNetto(pDoc), 8, buchungsjahr_netto_buffer);

				pB->GetMWSt(mwstbuf);
				sprintf(buffer, "%02d.%02d.%04d;\"%s\";%s;%s;\"%s\";\"%s\";\"%s\";\"%s\";%d;%d;%s;%s\r\n",
					pB->Datum.GetDay(), pB->Datum.GetMonth(), pB->Datum.GetYear(), 
					belegnummer_buffer,
					buffer3,
					mwstbuf,
					buffer2,
					buffer4,
					betrieb_buffer,
					bestandskonto_buffer,
					pB->AbschreibungNr,
					pB->AbschreibungJahre,
					restwert_buffer,
					buchungsjahr_netto_buffer);

				TRY
				{
					File.Write(buffer, strlen(buffer));
				}
				CATCH(CFileException, e)
				{
					AfxMessageBox("Fehler beim Schreiben der Export-Datei", MB_ICONSTOP);
					File.Close();
					return FALSE;
				}
				END_CATCH

				pB = pB->next;
			}

			File.Close();

			char buffer[2000];
			sprintf(buffer, "Die Einnahmen wurden in der Datei '%s' abgelegt, die Ausgaben in der Datei '%s'. Das CSV-Format (Comma Separated Values) ist ein universelles Dateiformat für Daten in Tabellenform und wird u.A. von MS-Excel und MS-Access verstanden. Jetzt im Explorer öffnen?",
				cse.GetBuffer(0), csa.GetBuffer(0));
			if (bExplorerOeffnen && AfxMessageBox(buffer, MB_YESNO) == IDYES)
			{
				char* letzter_querstrich = strrchr(EinnahmenExportDateiname, '\\');
				if (letzter_querstrich && strncmp(EinnahmenExportDateiname, AusgabenExportDateiname, letzter_querstrich - EinnahmenExportDateiname))
				{ // in zwei unterschiedlichen Verzeichnissen abgelegt?
					ShellExecute(m_hWnd, "open", "explorer.exe", (CString)"/select," + cse, NULL, SW_SHOWNORMAL);
					ShellExecute(m_hWnd, "open", "explorer.exe", (CString)"/select," + csa, NULL, SW_SHOWNORMAL);
				}
				else
				{ 

#if _MFC_VER > 0x0600
#pragma message("CEasyCashView::OnFileExport() >>>>>>>>>>>>>>>>>>>> SHOpenFolderAndSelectItems vorbereitet. Einfach auskommentieren! <<<<<<<<<<<<<<<<<<<<<<<")
#endif
					// nur eine Datei selektieren -- nicht schön, aber geht nicht anders:
					ShellExecute(m_hWnd, "open", "explorer.exe", (CString)"/select," + cse, NULL, SW_SHOWNORMAL);

				/*				
					// beide im selben Verzeichnis selektieren:
					char Verzeichnis[sizeof(EinnahmenExportDateiname)];
					strcpy(Verzeichnis, EinnahmenExportDateiname);
					if (letzter_querstrich) *letzter_querstrich = '\0';

					//Directory to open
					ITEMIDLIST *dir = ILCreateFromPath(Verzeichnis);

					//Items in directory to select
					ITEMIDLIST *item1 = ILCreateFromPath(EinnahmenExportDateiname);
					ITEMIDLIST *item2 = ILCreateFromPath(AusgabenExportDateiname);
					const ITEMIDLIST* selection[] = {item1,item2};
					UINT count = sizeof(selection) / sizeof(ITEMIDLIST);

					//Perform selection
					SHOpenFolderAndSelectItems(dir, count, selection, 0);

					//Free resources
					ILFree(dir);
					ILFree(item1);
					ILFree(item2);
				*/
					return TRUE;
				}
			}
		}
		else
			AfxMessageBox("Konnte Export-Datei für Ausgaben nicht öffnen", MB_ICONSTOP);
	}
	else
		AfxMessageBox("Konnte Export-Datei für Einnahmen nicht öffnen", MB_ICONSTOP);

	return FALSE;
}

void CEasyCashView::OnViewOptions() 
{
	einstellungen1->m_lfd_Buchungsnummer_Einnahmen = GetDocument()->nLaufendeBuchungsnummerFuerEinnahmen;
	einstellungen1->m_lfd_Buchungsnummer_Ausgaben  = GetDocument()->nLaufendeBuchungsnummerFuerAusgaben;
	einstellungen1->m_lfd_Buchungsnummer_Bank   = GetDocument()->nLaufendeBuchungsnummerFuerBank;
	einstellungen1->m_lfd_Buchungsnummer_Kasse  = GetDocument()->nLaufendeBuchungsnummerFuerKasse;
	einstellungen1->m_buchungsjahr = GetDocument()->nJahr;
	CString Key;
	Key.Format("Sondervorauszahlung%-04.4d", GetDocument()->nJahr);
	char *cp = GetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key);
	char *cp2;
	if (cp2 = strchr(cp, '|'))
	{ 
		strncpy(einstellungen1->m_sondervorauszahlung.GetBuffer(cp2-cp+1), cp, cp2-cp);
		einstellungen1->m_sondervorauszahlung.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
		einstellungen1->m_sondervorauszahlung.ReleaseBuffer();
	}
	else 
		einstellungen1->m_sondervorauszahlung = "";
	int landmerker = einstellungen2->m_land;
	int afa_genauigkeit_merker = einstellungen1->m_AbschreibungGenauigkeit;
	if (propdlg->DoModal() == IDOK)
	{
		if (landmerker != einstellungen2->m_land || einstellungen2->m_land)
		{
			int nNationaleAfAGenauigkeit;
			switch (einstellungen2->m_land)
			{
			case 0: // BRD
				if (GetDocument()->nJahr < 2004)
					nNationaleAfAGenauigkeit = HALBJAHRES_AFA;
				else
					nNationaleAfAGenauigkeit = MONATSGENAUE_AFA;
				break;
			case 1: // Österreich
				nNationaleAfAGenauigkeit = HALBJAHRES_AFA;
				break;
			case 2: // Schweiz
				nNationaleAfAGenauigkeit = GANZJAHRES_AFA;
				break;
			}

			// Abfrage Voreinstellungen
			if (nNationaleAfAGenauigkeit != einstellungen1->m_AbschreibungGenauigkeit)
				if (AfxMessageBox("Das Land wurde geändert und damit auch Regeln zur Abschreibung von Anlagevermögen. Soll die AfA-Genauigkeit gemäß der nationalen Gesetzgebung (Stand 2004) in den Voreinstellungen angepasst werden? (Siehe auch Einstellungen->Allgemein->'Voreinstell. AfA-Genauigkeit')", MB_YESNO) == IDYES)
					einstellungen1->m_AbschreibungGenauigkeit = nNationaleAfAGenauigkeit;
			
			// Abfrage aktuelles Dokument
			if (nNationaleAfAGenauigkeit != GetDocument()->AbschreibungGenauigkeit)
				if (AfxMessageBox("Soll die AfA-Genauigkeit für das aktuelle Dokument geändert werden? (Hinweis: Wenn schon ein Jahreswechsel für dieses Jahr gemacht wurde, führt dies evtl. zu einer Inkonsistenz, da der Restwert im Folgejahr nicht mehr verändert werden kann.)", MB_YESNO) == IDYES)
					GetDocument()->AbschreibungGenauigkeit = nNationaleAfAGenauigkeit;
		}
		else if (afa_genauigkeit_merker != einstellungen1->m_AbschreibungGenauigkeit)
			if (AfxMessageBox("Soll auch die AfA-Genauigkeit für das aktuelle Dokument geändert werden? (Hinweis: Wenn schon ein Jahreswechsel für dieses Jahr gemacht wurde, führt dies evtl. zu einer Inkonsistenz, da der Restwert im Folgejahr nicht mehr verändert werden kann.)", MB_YESNO) == IDYES)
				GetDocument()->AbschreibungGenauigkeit = einstellungen1->m_AbschreibungGenauigkeit;

		SaveProfile();
		if (GetDocument()->nLaufendeBuchungsnummerFuerEinnahmen != einstellungen1->m_lfd_Buchungsnummer_Einnahmen)
		{
			GetDocument()->nLaufendeBuchungsnummerFuerEinnahmen = einstellungen1->m_lfd_Buchungsnummer_Einnahmen;
			GetDocument()->SetModifiedFlag();
		}
		if (GetDocument()->nLaufendeBuchungsnummerFuerAusgaben != einstellungen1->m_lfd_Buchungsnummer_Ausgaben)
		{
			GetDocument()->nLaufendeBuchungsnummerFuerAusgaben  = einstellungen1->m_lfd_Buchungsnummer_Ausgaben;
			GetDocument()->SetModifiedFlag();
		}
		if (GetDocument()->nLaufendeBuchungsnummerFuerBank != einstellungen1->m_lfd_Buchungsnummer_Bank)
		{
			GetDocument()->nLaufendeBuchungsnummerFuerBank  = einstellungen1->m_lfd_Buchungsnummer_Bank;
			GetDocument()->SetModifiedFlag();
		}
		if (GetDocument()->nLaufendeBuchungsnummerFuerKasse != einstellungen1->m_lfd_Buchungsnummer_Kasse)
		{
			GetDocument()->nLaufendeBuchungsnummerFuerKasse = einstellungen1->m_lfd_Buchungsnummer_Kasse;
			GetDocument()->SetModifiedFlag();
		}
		if (GetDocument()->nJahr != einstellungen1->m_buchungsjahr)
		{
			GetDocument()->nJahr = einstellungen1->m_buchungsjahr;
			GetDocument()->SetModifiedFlag();
		}

		// sondervorauszahlung gedöns
		CString Key;
		CString vorauszahlung_zuvor;
		Key.Format("Sondervorauszahlung%-04.4d", GetDocument()->nJahr);
		char *cp = GetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key);
		char *cp2;
		if (cp2 = strchr(cp, '|'))
		{ 
			strncpy(vorauszahlung_zuvor.GetBuffer(cp2-cp+1), cp, cp2-cp);
			vorauszahlung_zuvor.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
			vorauszahlung_zuvor.ReleaseBuffer();
		}
		else 
			vorauszahlung_zuvor = "";
		if (einstellungen1->m_sondervorauszahlung != vorauszahlung_zuvor)
		{
			SetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key, einstellungen1->m_sondervorauszahlung);		
			GetDocument()->SetModifiedFlag();
		}

		RedrawWindow();
	}
	else
	{
		LoadProfile();
		einstellungen1->m_lfd_Buchungsnummer_Einnahmen = GetDocument()->nLaufendeBuchungsnummerFuerEinnahmen;
		einstellungen1->m_lfd_Buchungsnummer_Ausgaben  = GetDocument()->nLaufendeBuchungsnummerFuerAusgaben;
		einstellungen1->m_lfd_Buchungsnummer_Bank  = GetDocument()->nLaufendeBuchungsnummerFuerBank;
		einstellungen1->m_lfd_Buchungsnummer_Kasse = GetDocument()->nLaufendeBuchungsnummerFuerKasse;
		einstellungen1->m_buchungsjahr = GetDocument()->nJahr;
		CString Key;
		Key.Format("Sondervorauszahlung%-04.4d", GetDocument()->nJahr);
		char *cp = GetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key);
		if (cp2 = strchr(cp, '|'))
		{ 
			strncpy(einstellungen1->m_sondervorauszahlung.GetBuffer(cp2-cp+1), cp, cp2-cp);
			einstellungen1->m_sondervorauszahlung.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
			einstellungen1->m_sondervorauszahlung.ReleaseBuffer();
		}
		else 
			einstellungen1->m_sondervorauszahlung = "";
	}
}

void CEasyCashView::OnViewJournalDatum() 
{
	// Wenn Plugin aktiv im Fenster, erstmal Plugin deaktivieren
	DestroyPlugin();
			
	// Formularansicht zurücksetzen
	m_GewaehltesFormular = nSelected = -1;

	GetParent()->ShowWindow(SW_SHOW);
//VS9
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(3))
		(AfxGetMainWnd())->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, ID_VIEW_JOURNAL_DATUM, MF_BYCOMMAND);
	m_nAnzeige = 0;	
	//RedrawWindow();
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::OnViewJournalKonten() 
{
	// Wenn Plugin aktiv im Fenster, erstmal Plugin deaktivieren
	DestroyPlugin();
			
	// Formularansicht zurücksetzen
	m_GewaehltesFormular = nSelected = -1;

	GetParent()->ShowWindow(SW_SHOW);
//VS9
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(3))
		(AfxGetMainWnd())->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, ID_VIEW_JOURNAL_KONTEN, MF_BYCOMMAND);
	m_nAnzeige = 1;	
	//RedrawWindow();
	GetDocument()->UpdateAllViews(NULL);
}

// Filter auf nächstes Konto
void CEasyCashView::OnViewJournalKonto()
{
	int i;
	for (i = 0; i < m_KontenMitBuchungen.GetSize(); i++)
	{
		if (m_KontenFilterDisplay == m_KontenMitBuchungen[i])
		{
			if (++i >= m_KontenMitBuchungen.GetSize()) i = 0;
			m_KontenFilterDisplay = m_KontenMitBuchungen[i];
			break;
		}
	}	
	GetDocument()->UpdateAllViews(NULL);
}

// umschalten zwischen aktuellem Monat und allen Monaten
void CEasyCashView::OnViewJournalMonat()
{
	if (!m_MonatsFilterDisplay) 
	{
		//if (einstellungen1->m_monatliche_voranmeldung)
			m_MonatsFilterDisplay = CTime::GetCurrentTime().GetMonth();
		//else
		//	m_MonatsFilterDisplay = (CTime::GetCurrentTime().GetMonth()-1) / 3 + 13;
	}
	else
		m_MonatsFilterDisplay = 0;
	
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::OnViewJournalBestandskonto() 
{
	CIconAuswahlBestandskonto dlg(this);
	dlg.m_nModus = 1;
	int nReturn = dlg.DoModal();	
	UpdateBestandskontenMenu();
	if (nReturn == IDOK)
	{
		if (dlg.m_nSelected >= 0)
			m_BestandskontoFilterDisplay = m_csaBestandskontenNamen[dlg.m_nSelected];
		else
			m_BestandskontoFilterDisplay = "";
	}
	else
		m_BestandskontoFilterDisplay = "";
	//RedrawWindow();
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::OnViewJournalBestkonten() 
{
	// Wenn Plugin aktiv im Fenster, erstmal Plugin deaktivieren
	DestroyPlugin();
			
	// Formularansicht zurücksetzen
	m_GewaehltesFormular = nSelected = -1;

	GetParent()->ShowWindow(SW_SHOW);
//VS9
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(3))
		(AfxGetMainWnd())->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, ID_VIEW_JOURNAL_BESTANDSKONTEN, MF_BYCOMMAND);
	m_nAnzeige = 2;	
	//RedrawWindow();
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::OnViewJournalAnlagenverzeichnis() 
{
	// Wenn Plugin aktiv im Fenster, erstmal Plugin deaktivieren
	DestroyPlugin();
			
	// Formularansicht zurücksetzen
	m_GewaehltesFormular = nSelected = -1;

	GetParent()->ShowWindow(SW_SHOW);
//VS9
	if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(3))
		(AfxGetMainWnd())->GetMenu()->GetSubMenu(3)->CheckMenuRadioItem(ID_VIEW_JOURNAL_DATUM, ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS, MF_BYCOMMAND);
	m_nAnzeige = 3;	
	//RedrawWindow();
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::OnViewJournalBestandskonten() 
{
	// Zombiecode
}

void CEasyCashView::OnViewJournalBetrieb() 
{
	CIconAuswahlBetrieb dlg(this);
	dlg.m_nModus = 1;
	int nReturn = dlg.DoModal();
	UpdateBetriebeMenu();
	if (nReturn == IDOK)
	{
		if (dlg.m_nSelected >= 0)
			m_BetriebFilterDisplay = m_csaBetriebeNamen[dlg.m_nSelected];
		else
			m_BetriebFilterDisplay = "";
	}
	else
		m_BetriebFilterDisplay = "";
	//RedrawWindow();
	GetDocument()->UpdateAllViews(NULL);
}

void CEasyCashView::OnViewJournalSwitch() 
{
	if (pPluginWnd || m_GewaehltesFormular != -1)
	{
		// plugin fenster schließen
		DestroyPlugin();

		// wenn plugin aktiv war, nicht vertauschen sondern den vorigen view beibehalten
		/*GetParent()->*/ShowWindow(SW_SHOW);
		switch (m_nAnzeige)
		{
		case 0: OnViewJournalDatum(); break;
		case 1: OnViewJournalKonten(); break;
		case 2: OnViewJournalBestkonten(); break;
		case 3: OnViewJournalAnlagenverzeichnis(); break;
		}
	}
	else
	{
		/*GetParent()->*/ShowWindow(SW_SHOW);
		if (!((CExtSplitter*)GetParent())->IsPaneVisible(0, 1))
			((CExtSplitter*)GetParent())->ShowColumn(1);
		switch (m_nAnzeige)
		{
		case 0: OnViewJournalKonten(); break;
		case 1: OnViewJournalBestkonten(); break;
		case 2: OnViewJournalAnlagenverzeichnis(); break;
		case 3: OnViewJournalDatum(); break;
		}
	}
}

void CEasyCashView::OnViewZoomSwitch()
{
	if (m_zoomfaktor == 100)
		m_zoomfaktor = 200;
	else
		m_zoomfaktor = 100;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktorVergroessern()
{
	m_zoomfaktor += 25;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktorVerkleinern()
{
	m_zoomfaktor -= 25;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor50()
{
	m_zoomfaktor = 50;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor75()
{
	m_zoomfaktor = 75;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor100()
{
	m_zoomfaktor = 100;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor125()
{
	m_zoomfaktor = 125;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor150()
{
	m_zoomfaktor = 150;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor175()
{
	m_zoomfaktor = 175;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor200()
{
	m_zoomfaktor = 200;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor250()
{
	m_zoomfaktor = 250;
	SetzeZoomfaktor();
}

void CEasyCashView::OnZoomfaktor300()
{
	m_zoomfaktor = 300;
	SetzeZoomfaktor();
}

extern CString MakeUuidString(UUID* pUUID/*=NULL*/);	// borgen wir aus BuchenDlg aus...

void CEasyCashView::LoadProfile()
{
	char buffer[10000];
	extern CEasyCashApp theApp;

	char EasyCashIniFilenameBuffer[1000];
	char NetzwerkEasyCashIniFilenameBuffer[1000];
	*NetzwerkEasyCashIniFilenameBuffer = '\0';

	CString Datenverzeichnis = theApp.GetProfileString("Allgemein", "Datenverzeichnis", "");
	if (Datenverzeichnis.IsEmpty())
	{
		AfxMessageBox("EasyCash&Tax benötigt ein Datenverzeichnis, in dem zukünftig alle Buchungsdateien (JahrXXXX.eca) sowie die Einstellungen inkl. Konten (easyct.ini) gespeichert werden. Mit dem folgenden Dialog kann solch ein Verzeichnis ausgewählt bzw. erzeugt werden. (Erzeugen geht mit dem mittleren der drei Knöpfe oben rechts im Datenverzeichnis auswählen Dialog). Vorzugsweise legt man das Datenverzeichnis in 'Eigene Dateien' oder -- wenn alle Benutzer dieses Computers darauf Zugriff haben sollen -- in 'Gemeinsame Dateien' an.");
	
		CDatenverzeichnisDlg dvdlg;
		dvdlg.DoModal();

		if (!SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(), Datenverzeichnis.GetBuffer(MAX_PATH), CSIDL_PERSONAL, FALSE))
			Datenverzeichnis = "C:\\";

		// Datenverzeichnis wählen
		if (SelectFolder(Datenverzeichnis.GetBuffer(MAX_PATH), "Datenverzeichnis auswählen")) 
		{
			Datenverzeichnis.ReleaseBuffer();

			if (Datenverzeichnis.GetLength() && Datenverzeichnis[Datenverzeichnis.GetLength()-1] == '\\')
				Datenverzeichnis = Datenverzeichnis.Left(Datenverzeichnis.GetLength()-1);

			// checken auf existierende easyct.ini im Programmverzeichnis
			char EasyCashIniImProgrammverzeichnisFilenameBuffer[1000];
			DWORD dw = GetModuleFileName(theApp.m_hInstance, EasyCashIniImProgrammverzeichnisFilenameBuffer, 1000);
			char *cp;
			if (cp = strrchr(EasyCashIniImProgrammverzeichnisFilenameBuffer, '\\'))
			{
				strcpy(cp, "\\easyct.ini");

				if (CopyFile(EasyCashIniImProgrammverzeichnisFilenameBuffer, Datenverzeichnis + "\\easyct.ini", TRUE))
				{
					DeleteFile(EasyCashIniImProgrammverzeichnisFilenameBuffer);
					CString csRecentFilesList;
					CString csRecentFile = theApp.GetProfileString("Recent File List", "File1", "");
					if (!csRecentFile.IsEmpty()) csRecentFilesList += (CString) "\r\n" + csRecentFile;
					csRecentFile = theApp.GetProfileString("Recent File List", "File2", "");
					if (!csRecentFile.IsEmpty()) csRecentFilesList += (CString) "\r\n" + csRecentFile;
					csRecentFile = theApp.GetProfileString("Recent File List", "File3", "");
					if (!csRecentFile.IsEmpty()) csRecentFilesList += (CString) "\r\n" + csRecentFile;
					csRecentFile = theApp.GetProfileString("Recent File List", "File4", "");
					if (!csRecentFile.IsEmpty()) csRecentFilesList += (CString) "\r\n" + csRecentFile;
					if (!csRecentFilesList.IsEmpty()) csRecentFilesList = " Hier sind die Pfade zu den zuletzt geöffneten Dateien: \r\n" + csRecentFilesList + "\r\n\r\nBitte den Explorer benutzen, um die Dateien (sofern relevant) ins Datenverzeichnis '" + Datenverzeichnis + "' zu verschieben.";
					AfxMessageBox("Die existierende Einstellungsdatei (easyct.ini) wurde aus dem Programmverzeichnis in das neue Datenverzeichnis verschoben. Es wird dringend empfohlen die Buchungsdateien ebenfalls dort zu speichern." + csRecentFilesList);
				}
			}
		}
		else
		{
			AfxMessageBox("Na gut, dann eben nicht... EasyCash&Tax wird die Einstellungen in der Datei easyct.ini im Programmverzeichnis speichern, was ein wenig suboptimal ist.");

			DWORD dw = GetModuleFileName(theApp.m_hInstance, Datenverzeichnis.GetBuffer(1000), 1000);
		}
		theApp.WriteProfileString("Allgemein", "Datenverzeichnis", Datenverzeichnis);			
	}
	DWORD dw = GetFileAttributes(Datenverzeichnis);
	if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
	{
		AfxMessageBox("Datenverzeichnis kann nicht gefunden werden. Bitte erneut auswählen!");
		if (SHGetFolderPath(AfxGetMainWnd()->GetSafeHwnd(), CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, Datenverzeichnis.GetBuffer(MAX_PATH)) != S_OK)
			Datenverzeichnis = "C:\\";
		Datenverzeichnis.ReleaseBuffer();
		CString FailsafeDatenverzeichnis;
		FailsafeDatenverzeichnis = Datenverzeichnis;

		do 
		{
			if (SelectFolder(Datenverzeichnis.GetBuffer(MAX_PATH), "Datenverzeichnis auswählen")) 
			{
				Datenverzeichnis.ReleaseBuffer();
				Datenverzeichnis = FailsafeDatenverzeichnis;
				AfxMessageBox("Datenverzeichnis wurde auf '" + Datenverzeichnis + "' gelegt.");
				break;
			}
			dw = GetFileAttributes(Datenverzeichnis);			
		}
		while (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY));
		theApp.WriteProfileString("Allgemein", "Datenverzeichnis", Datenverzeichnis);
	}
	strcpy(EasyCashIniFilenameBuffer, (LPCSTR)((CString)(Datenverzeichnis + "\\easyct.ini")));
	SetIniFileName(EasyCashIniFilenameBuffer);

	if (EasyCashIniFilenameBuffer[0] == '\\' && EasyCashIniFilenameBuffer[1] == '\\')	// UNC-Pfad ?
	{
		strcpy(NetzwerkEasyCashIniFilenameBuffer, EasyCashIniFilenameBuffer);			// dann schnellere lokale Kopie der ini Datei erstellen
		if (GetTempPath(sizeof(EasyCashIniFilenameBuffer), EasyCashIniFilenameBuffer))
		{
			CString uuid = MakeUuidString(NULL);
			if (strlen(EasyCashIniFilenameBuffer) + uuid.GetLength() > sizeof(EasyCashIniFilenameBuffer))	// Pfad wird zu lang?
			{
				strcpy(EasyCashIniFilenameBuffer, NetzwerkEasyCashIniFilenameBuffer);			// dann langsame Methode verwenden
				*NetzwerkEasyCashIniFilenameBuffer = '\0';
			}
			else
			{
				strcat(EasyCashIniFilenameBuffer, uuid.GetBuffer(0));	// Dateinamen an Pfad anfügen

				if (!CopyFile(NetzwerkEasyCashIniFilenameBuffer, EasyCashIniFilenameBuffer, FALSE))	// kann nicht kopieren?
				{
					strcpy(EasyCashIniFilenameBuffer, NetzwerkEasyCashIniFilenameBuffer);			// dann langsame Methode verwenden
					*NetzwerkEasyCashIniFilenameBuffer = '\0';
				}
			}
		}
	}

	
	//if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return; }

	if (!propdlg) return;

	// Allgemein
	GetPrivateProfileString("Allgemein", "monatliche_voranmeldung", "1", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_monatliche_voranmeldung = atoi(buffer);
	GetPrivateProfileString("Allgemein", "taeglich_buchen", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_taeglich_buchen = atoi(buffer);
	GetPrivateProfileString("Allgemein", "BuchungsdatumBelassen", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_BuchungsdatumBelassen = atoi(buffer);
	GetPrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerEinnahmen", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen = atoi(buffer);
	GetPrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerAusgaben", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerAusgaben = atoi(buffer);
	GetPrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerBank", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerBank = atoi(buffer);
	GetPrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerKasse", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerKasse = atoi(buffer);
	GetPrivateProfileString("Allgemein", "JahresfeldAktiviert", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_bJahresfeldAktiviert = atoi(buffer);
	GetPrivateProfileString("Allgemein", "MwstFeldAktiviert", "1", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_bMwstFeldAktiviert = atoi(buffer);
	int tausender_mwst_saetze = (int)GetPrivateProfileInt("Allgemein", "tausender_mwst_saetze", 0, EasyCashIniFilenameBuffer);
	GetPrivateProfileString("Allgemein", "AbschreibungGenauigkeit", "2", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_AbschreibungGenauigkeit = atoi(buffer);
	GetPrivateProfileString("Allgemein", "JahreswechselAbfrage", "1980", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	m_nJahreswechselAbfrage = atoi(buffer);

	// Druck
	GetPrivateProfileString("Druck", "umstvoranmeldung_h", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	m_umstvoranmeldung_h = atoi(buffer);
	GetPrivateProfileString("Druck", "umstvoranmeldung_h2", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	m_umstvoranmeldung_h2 = atoi(buffer);
	GetPrivateProfileString("Druck", "umstvoranmeldung_v", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	m_umstvoranmeldung_v = atoi(buffer);
	GetPrivateProfileString("Druck", "formular_nicht_mitdrucken", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	m_formular_nicht_mitdrucken = atoi(buffer);
	GetPrivateProfileString("Druck", "Bildschirmschrift", "Arial", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_Bildschirmschrift = buffer;
	GetPrivateProfileString("Druck", "Bildschirmschriftgroesse", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_Bildschirmschriftgroesse = abs(atoi(buffer));
	GetPrivateProfileString("Druck", "BildschirmschriftFett", "400", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_BildschirmschriftFett = atoi(buffer);
	GetPrivateProfileString("Druck", "BildschirmschriftKursiv", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_BildschirmschriftKursiv = atoi(buffer);
	GetPrivateProfileString("Druck", "Druckerschrift", "Arial", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_Druckerschrift = buffer;
	GetPrivateProfileString("Druck", "Druckerschriftgroesse", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_Druckerschriftgroesse = abs(atoi(buffer));
	GetPrivateProfileString("Druck", "DruckerschriftFett", "400", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_DruckerschriftFett = atoi(buffer);
	GetPrivateProfileString("Druck", "DruckerschriftKursiv", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen1->m_DruckerschriftKursiv = atoi(buffer);

	// Buchungspresets
	int i;
	for (i = 0; i < 100; i++)
	{
		char key_buffer[100];
		
		sprintf(key_buffer, "%2.2dText", i);
		GetPrivateProfileString("Buchungsposten", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->Buchungsposten[i].Beschreibung = buffer;

		sprintf(key_buffer, "%2.2dAusg", i);
		GetPrivateProfileString("Buchungsposten", key_buffer, "1", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->Buchungsposten[i].Ausgaben = atoi(buffer);

		sprintf(key_buffer, "%2.2dMWSt", i);
		GetPrivateProfileString("Buchungsposten", key_buffer, GetDefaultVAT(), buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->Buchungsposten[i].MWSt = atoi(buffer);
		if (!tausender_mwst_saetze) einstellungen1->Buchungsposten[i].MWSt *= 1000;

		sprintf(key_buffer, "%2.2dAfAJ", i);
		GetPrivateProfileString("Buchungsposten", key_buffer, "1", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->Buchungsposten[i].Abschreibungsjahre = atoi(buffer);

		sprintf(key_buffer, "%2.2dRech", i);
		GetPrivateProfileString("Buchungsposten", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->Buchungsposten[i].Rechnungsposten = buffer;
	}

	GetPrivateProfileString("Persoenliche_Daten", "name", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_name = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "vorname", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_vorname = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "unternehmensart1", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_unternehmensart1 = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "unternehmensart2", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_unternehmensart2 = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "strasse", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_strasse = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "plz", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_plz = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "ort", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_ort = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "land", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_land = atoi(buffer);
	GetPrivateProfileString("Persoenliche_Daten", "vat1", (einstellungen2->m_land == 0) ? "19" : ((einstellungen2->m_land == 1) ? "20" : "7,5"), buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_vat1 = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "vat2", (einstellungen2->m_land == 0) ? "7" : ((einstellungen2->m_land == 1) ? "10" : "0"), buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_vat2 = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "vat3", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_vat3 = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "vat4", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen2->m_vat4 = buffer;
	GetPrivateProfileString("Persoenliche_Daten", "vat_d_aenderung_2007", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	if (einstellungen2->m_land == 0 && atoi(einstellungen2->m_vat1) == 16 && !atoi(buffer))
	{	// MWSt-Erhöhung 2007 von 16% auf 19%:
		int n = AfxMessageBox("Soll EasyCash&Tax den MWSt.-Regelsatz in den persönlichen Einstellungen und den Buchungs-Presets von 16% auf 19% erhöhen? (Abbrechen = später nochmal fragen)", MB_YESNOCANCEL);
		switch (n)
		{
			int i;
		case IDYES:
			einstellungen2->m_vat1 = "19";

			for (i = 0; i < 100; i++)
			{
				if (einstellungen1->Buchungsposten[i].MWSt == 16000)
					einstellungen1->Buchungsposten[i].MWSt = 19000;
			}		

		case IDNO:

			// nicht mehr fragen
			WritePrivateProfileString("Persoenliche_Daten", "vat_d_aenderung_2007", "1", EasyCashIniFilenameBuffer);
		case IDCANCEL:
			;
		}
	}

	GetPrivateProfileString("Finanzamt", "name", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen3->m_name = buffer;
	GetPrivateProfileString("Finanzamt", "name2", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen3->m_name2 = buffer;
	GetPrivateProfileString("Finanzamt", "strasse", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen3->m_strasse = buffer;
	GetPrivateProfileString("Finanzamt", "plz", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen3->m_plz = buffer;
	GetPrivateProfileString("Finanzamt", "ort", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen3->m_ort = buffer;
	GetPrivateProfileString("Finanzamt", "steuernummer", "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen3->m_steuernummer = buffer;

	BOOL bInitRechnungsposten = TRUE;
	for (i = 0; i < 100; i++)
	{
		char key_buffer[100];
		
		sprintf(key_buffer, "%2.2d", i);

		GetPrivateProfileString("EinnahmenRechnungsposten", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->EinnahmenRechnungsposten[i] = buffer;

		GetPrivateProfileString("EinnahmenFeldzuweisungen", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->EinnahmenFeldzuweisungen[i] = buffer;

		GetPrivateProfileString("EinnahmenUnterkategorien", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->EinnahmenUnterkategorien[i] = buffer;

		GetPrivateProfileString("AusgabenRechnungsposten", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->AusgabenRechnungsposten[i] = buffer;

		GetPrivateProfileString("AusgabenFeldzuweisungen", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->AusgabenFeldzuweisungen[i] = buffer;

		GetPrivateProfileString("AusgabenUnterkategorien", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		einstellungen1->AusgabenUnterkategorien[i] = buffer;

		if (!einstellungen1->EinnahmenRechnungsposten[i].IsEmpty()
			|| !einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
				bInitRechnungsposten = FALSE;				
	}

	BOOL bUnterkategorienVorhanden = FALSE;

	// wenn frische Ini-Datei: Rechnungsposten initialisieren mit EÜR-Konten aus XML-Formulardatei
	if (bInitRechnungsposten)
	{
		char EUeRFormularDateinamenBuffer[1000];
		DWORD dw = GetModuleFileName(theApp.m_hInstance, EUeRFormularDateinamenBuffer, 1000);
		char *cp = strrchr(EUeRFormularDateinamenBuffer, '\\');
		if (cp)
		{
			int nJahr = 2038;
			do
			{
				if (--nJahr < 2006) break;

				sprintf(cp, "\\EUeR%-0.4d.ecf", nJahr);
				if (GetFileAttributes(EUeRFormularDateinamenBuffer) != 0xFFFFFFFF) 
				{
					einstellungen2->m_land = 0; // BRD
					break;
				}
				else 
				{
					sprintf(cp, "\\EUeR-d-%-0.4d.ecf", nJahr);
					if (GetFileAttributes(EUeRFormularDateinamenBuffer) != 0xFFFFFFFF) 
					{
						einstellungen2->m_land = 0; // BRD
						break;
					}
					else
					{
						sprintf(cp, "\\E1a-AT-%-0.4d.ecf", nJahr);
						if (GetFileAttributes(EUeRFormularDateinamenBuffer) != 0xFFFFFFFF) 
						{
							einstellungen2->m_land = 1; // Österreich
							break;
						}
					}
				}
			}
			while (TRUE);

			CFile xmlf;
			if (nJahr >= 2006 && xmlf.Open(EUeRFormularDateinamenBuffer, CFile::modeRead))
			{
				DWORD l = (DWORD)xmlf.GetLength();
				char *xmlbuffer = new char[l+1];
				TRY
				{
					xmlf.Read(xmlbuffer, l);
					xmlbuffer[l] = '\0';

					XDoc xmldoc;
					if (xmldoc.Load(xmlbuffer))
					{
						LPXNode xml = xmldoc.GetRoot();
						LPCTSTR formular_name = xml->GetAttrValue("name");
						int i, i_e, i_a;
						LPXNode child;
						LPXNode felder = NULL;
						if (xml && (felder = xml->Find("felder")))
						{
							for(i = 0, i_e = 0, i_a = 0; i < felder->GetChildCount() && i_e < 100 && i_a < 100; i++)
							{
								child = felder->GetChild(i);
								if (!child->GetAttrValue("veraltet") || stricmp(child->GetAttrValue("veraltet"), "ja"))
								{
									CString unterkategorie = child->GetAttrValue("unterkategorie");
									if (!unterkategorie.IsEmpty()) bUnterkategorienVorhanden = TRUE;
									if (!stricmp(child->GetAttrValue("typ"), "Einnahmen"))
									{
										einstellungen1->EinnahmenRechnungsposten[i_e] = child->GetChildValue("name");
										SetErweiterungKey(einstellungen1->EinnahmenFeldzuweisungen[i_e], "ECT", formular_name, child->GetAttrValue("id"));
										einstellungen1->EinnahmenUnterkategorien[i_e] = unterkategorie;
										i_e++;
									}
									else if (!stricmp(child->GetAttrValue("typ"), "Ausgaben"))
									{
										einstellungen1->AusgabenRechnungsposten[i_a] = child->GetChildValue("name");
										SetErweiterungKey(einstellungen1->AusgabenFeldzuweisungen[i_a], "ECT", formular_name, child->GetAttrValue("id"));
										einstellungen1->AusgabenUnterkategorien[i_a] = unterkategorie;
										i_a++;
									}
								}
							}
						}
						SaveProfile();
					}				
				}
				CATCH(CFileException, e)
				{
					AfxMessageBox("Fehler beim Lesen der EÜR-Formulardefinitionsdatei EUeR.ecf", MB_ICONSTOP);
				}
				END_CATCH
				xmlf.Close();
				delete xmlbuffer;
			}
		}
	}

	// ggf. Konto für die separate Buchung von Vorsteuerbeträgen anlegen
	BOOL bInitVSTSeparat = FALSE;
	if (einstellungen2->m_land == 0)	// nur Deutschland vorerst
	{
		BOOL bVSTBetraegeSeparatAngelegt;
		GetPrivateProfileString("Allgemein", "VST-Betraege-separat-angelegt", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		bVSTBetraegeSeparatAngelegt = atoi(buffer);
		bInitVSTSeparat = !bVSTBetraegeSeparatAngelegt;
		if (bInitRechnungsposten || bInitVSTSeparat)
		{
			for(int i = 0; i < 100 && einstellungen1->AusgabenRechnungsposten[i] != "VST-Beträge separat"; i++)
				if (einstellungen1->AusgabenRechnungsposten[i].IsEmpty())
				{
					einstellungen1->AusgabenRechnungsposten[i] = "VST-Beträge separat";
					SetErweiterungKey(einstellungen1->AusgabenFeldzuweisungen[i], "ECT", "E/Ü-Rechnung", "1185");
					SetErweiterungKey(einstellungen1->AusgabenFeldzuweisungen[i], "ECT", "Umsatzsteuer-Voranmeldung", "66");
					if (bUnterkategorienVorhanden) einstellungen1->AusgabenUnterkategorien[i] = "Unabhängig gebuchte Vorsteuer";
					WritePrivateProfileString("Allgemein", "VST-Betraege-separat-angelegt", "1", EasyCashIniFilenameBuffer);
					if (!bVSTBetraegeSeparatAngelegt && !bInitRechnungsposten) AfxMessageBox("Ein Konto 'VST-Beträge separat' wurde angelegt für VST-Beträge, die vom Netto-Betrag entkoppelt sind. Bitte kontrollieren Sie die Formularfeld-Zuweisungen unter Einstellungen->E/Ü-Konten!");
					bInitVSTSeparat = TRUE;
					break;
				}
		}
	}

	BOOL bInitPrivatSplit = TRUE;
	for (i = 0; i < einstellungen5->m_privat_split_size; i++)
	{
		char key_buffer[100];
		
		sprintf(key_buffer, "Art%2.2d", i);
		GetPrivateProfileString("Privat-Split", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		*(einstellungen5->m_part[i]) = buffer;

		sprintf(key_buffer, "Satz%2.2d", i);
		GetPrivateProfileString("Privat-Split", key_buffer, "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		*(einstellungen5->m_psatz[i]) = buffer;

		sprintf(key_buffer, "UstSatz%2.2d", i);
		GetPrivateProfileString("Privat-Split", key_buffer, "100", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		*(einstellungen5->m_pustsatz[i]) = buffer;

		if (!(einstellungen5->m_part[i])->IsEmpty()
			|| *(einstellungen5->m_psatz[i]) != "0"
			|| *(einstellungen5->m_pustsatz[i]) != "100")
				bInitPrivatSplit = FALSE;
	}

	if (bInitPrivatSplit)
	{
		*(einstellungen5->m_part[0]) = "Private Kraftfahrzeugnutzung";
		*(einstellungen5->m_psatz[0]) = "50,00";
		*(einstellungen5->m_pustsatz[0]) = "00,00";
		*(einstellungen5->m_part[1]) = "Private Telefonnutzung";
		*(einstellungen5->m_psatz[1]) = "47,11 (Beispielschätzwert)";
		*(einstellungen5->m_pustsatz[1]) = "100,00";
	}

	GetPrivateProfileString("Allgemein", "ustvst_gesondert", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen4->m_ustvst_gesondert = atoi(buffer);

	GetPrivateProfileString("Allgemein", "nach_konten_seitenumbruch", "0", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen4->m_nach_konten_seitenumbruch = atoi(buffer);

	BOOL bInitFinanzamtserstattungen = FALSE;
	GetPrivateProfileString("Allgemein", "Finanzamtserstattungen", "<noch nicht angelegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen4->m_finanzamtserstattungen = buffer;
	if (einstellungen4->m_finanzamtserstattungen == "<noch nicht angelegt>")	// noch kein Wert in der ini?
	{
		einstellungen4->m_finanzamtserstattungen = "";							// markieren als schon einmal geprüft
		for (i = 0; i < 100; i++)
		{
			CString csTemp = einstellungen1->EinnahmenRechnungsposten[i];
			csTemp.MakeLower();
			if (csTemp.Find("finanzamt") >= 0 && csTemp.Find("erstatt") >= 0 && csTemp.Find("umsatzsteuer") >= 0)
				einstellungen4->m_finanzamtserstattungen = einstellungen1->EinnahmenRechnungsposten[i];
		}
		bInitFinanzamtserstattungen = TRUE;
	}

	BOOL bInitFinanzamtszahlungen = FALSE;
	GetPrivateProfileString("Allgemein", "Finanzamtszahlungen", "<noch nicht angelegt>", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
	einstellungen4->m_finanzamtszahlungen = buffer;
	if (einstellungen4->m_finanzamtszahlungen == "<noch nicht angelegt>")		// noch kein Wert in der ini?
	{
		einstellungen4->m_finanzamtszahlungen == "";							// markieren als schon einmal geprüft
		for (i = 0; i < 100; i++)
		{
			CString csTemp = einstellungen1->AusgabenRechnungsposten[i];
			csTemp.MakeLower();
			if (csTemp.Find("finanzamt") >= 0 && csTemp.Find("zahl") >= 0 && csTemp.Find("umsatzsteuer") >= 0)
				einstellungen4->m_finanzamtszahlungen = einstellungen1->AusgabenRechnungsposten[i];				
		}
		bInitFinanzamtszahlungen = TRUE;
	}

	if (NetzwerkEasyCashIniFilenameBuffer[0] == '\\' && NetzwerkEasyCashIniFilenameBuffer[1] == '\\')	// UNC-Pfad ?
	{
		DeleteFile(EasyCashIniFilenameBuffer);	// temporäre Ini wieder löschen
	}

	if (bInitRechnungsposten || bInitPrivatSplit || bInitVSTSeparat || bInitFinanzamtserstattungen || bInitFinanzamtszahlungen)
		SaveProfile();
}

void CEasyCashView::SaveProfile()
{
	char buffer[10000];
	CEasyCashDoc *pDoc = GetDocument();
	extern CEasyCashApp theApp;
	char EasyCashIniFilenameBuffer[1000];
	char NetzwerkEasyCashIniFilenameBuffer[1000];
	*NetzwerkEasyCashIniFilenameBuffer = '\0';

	if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return; }

	if (!propdlg) return;

	if (EasyCashIniFilenameBuffer[0] == '\\' && EasyCashIniFilenameBuffer[1] == '\\')	// UNC-Pfad ?
	{
		strcpy(NetzwerkEasyCashIniFilenameBuffer, EasyCashIniFilenameBuffer);			// dann schnellere lokale Kopie der ini Datei erstellen
		if (GetTempPath(sizeof(EasyCashIniFilenameBuffer), EasyCashIniFilenameBuffer))
		{
			CString uuid = MakeUuidString(NULL);
			if (strlen(EasyCashIniFilenameBuffer) + uuid.GetLength() > sizeof(EasyCashIniFilenameBuffer))	// Pfad wird zu lang?
			{
				strcpy(EasyCashIniFilenameBuffer, NetzwerkEasyCashIniFilenameBuffer);			// dann langsame Methode verwenden
				*NetzwerkEasyCashIniFilenameBuffer = '\0';
			}
			else
			{
				strcat(EasyCashIniFilenameBuffer, uuid.GetBuffer(0));	// Dateinamen an Pfad anfügen

				if (!CopyFile(NetzwerkEasyCashIniFilenameBuffer, EasyCashIniFilenameBuffer, FALSE))	// kann nicht kopieren?
				{
					strcpy(EasyCashIniFilenameBuffer, NetzwerkEasyCashIniFilenameBuffer);			// dann langsame Methode verwenden
					*NetzwerkEasyCashIniFilenameBuffer = '\0';
				}
			}
		}
	}


	HCURSOR hcWait = theApp.LoadStandardCursor (IDC_WAIT);
	HCURSOR hcOld = SetCursor(hcWait);

	// Allgemein
	sprintf(buffer, "%d", (int)einstellungen1->m_monatliche_voranmeldung);
	WritePrivateProfileString("Allgemein", "monatliche_voranmeldung", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_taeglich_buchen);
	WritePrivateProfileString("Allgemein", "taeglich_buchen", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_BuchungsdatumBelassen);
	WritePrivateProfileString("Allgemein", "BuchungsdatumBelassen", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerEinnahmen);
	WritePrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerEinnahmen", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerAusgaben);
	WritePrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerAusgaben", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerBank);
	WritePrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerBank", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_bErzeugeLaufendeBuchungsnummernFuerKasse);
	WritePrivateProfileString("Allgemein", "ErzeugeLaufendeBuchungsnummernFuerKasse", (LPCSTR)buffer, EasyCashIniFilenameBuffer);	
	sprintf(buffer, "%d", (int)einstellungen1->m_bMwstFeldAktiviert);
	WritePrivateProfileString("Allgemein", "MwstFeldAktiviert", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_bJahresfeldAktiviert);
	WritePrivateProfileString("Allgemein", "JahresfeldAktiviert", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Allgemein", "tausender_mwst_saetze", "1", EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_AbschreibungGenauigkeit);
	WritePrivateProfileString("Allgemein", "AbschreibungGenauigkeit", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)m_nJahreswechselAbfrage);
	WritePrivateProfileString("Allgemein", "JahreswechselAbfrage", (LPCSTR)buffer, EasyCashIniFilenameBuffer);

	// Druck
	sprintf(buffer, "%d", (int)m_umstvoranmeldung_h);
	WritePrivateProfileString("Druck", "umstvoranmeldung_h", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)m_umstvoranmeldung_h2);
	WritePrivateProfileString("Druck", "umstvoranmeldung_h2", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)m_umstvoranmeldung_v);
	WritePrivateProfileString("Druck", "umstvoranmeldung_v", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)m_formular_nicht_mitdrucken);
	WritePrivateProfileString("Druck", "formular_nicht_mitdrucken", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Druck", "Bildschirmschrift", (LPCSTR)einstellungen1->m_Bildschirmschrift, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_Bildschirmschriftgroesse);
	WritePrivateProfileString("Druck", "Bildschirmschriftgroesse", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_BildschirmschriftFett);
	WritePrivateProfileString("Druck", "BildschirmschriftFett", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_BildschirmschriftKursiv);
	WritePrivateProfileString("Druck", "BildschirmschriftKursiv", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Druck", "Druckerschrift", (LPCSTR)einstellungen1->m_Druckerschrift, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_Druckerschriftgroesse);
	WritePrivateProfileString("Druck", "Druckerschriftgroesse", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_DruckerschriftFett);
	WritePrivateProfileString("Druck", "DruckerschriftFett", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", (int)einstellungen1->m_DruckerschriftKursiv);
	WritePrivateProfileString("Druck", "DruckerschriftKursiv", (LPCSTR)buffer, EasyCashIniFilenameBuffer);
	
	// Buchungsposten
	int i;
	for (i = 0; i < 100; i++)
	{
		char key_buffer[100];
		
		sprintf(key_buffer, "%2.2dText", i);
		WritePrivateProfileString("Buchungsposten", key_buffer, (LPCSTR)einstellungen1->Buchungsposten[i].Beschreibung, EasyCashIniFilenameBuffer);

		sprintf(key_buffer, "%2.2dAusg", i);
		sprintf(buffer, "%d", (int)(LPCSTR)einstellungen1->Buchungsposten[i].Ausgaben);
		WritePrivateProfileString("Buchungsposten", key_buffer, buffer, EasyCashIniFilenameBuffer);

		sprintf(key_buffer, "%2.2dMWSt", i);
		sprintf(buffer, "%d", (int)einstellungen1->Buchungsposten[i].MWSt);
		WritePrivateProfileString("Buchungsposten", key_buffer, buffer, EasyCashIniFilenameBuffer);

		sprintf(key_buffer, "%2.2dAfAJ", i);
		sprintf(buffer, "%d", (int)einstellungen1->Buchungsposten[i].Abschreibungsjahre);
		WritePrivateProfileString("Buchungsposten", key_buffer, buffer, EasyCashIniFilenameBuffer);

		sprintf(key_buffer, "%2.2dRech", i);
		WritePrivateProfileString("Buchungsposten", key_buffer, (LPCSTR)einstellungen1->Buchungsposten[i].Rechnungsposten, EasyCashIniFilenameBuffer);

	}

	WritePrivateProfileString("Persoenliche_Daten", "name", (LPCSTR)einstellungen2->m_name, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "vorname", (LPCSTR)einstellungen2->m_vorname, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "unternehmensart1", (LPCSTR)einstellungen2->m_unternehmensart1, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "unternehmensart2", (LPCSTR)einstellungen2->m_unternehmensart2, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "strasse", (LPCSTR)einstellungen2->m_strasse, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "plz", (LPCSTR)einstellungen2->m_plz, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "ort", (LPCSTR)einstellungen2->m_ort, EasyCashIniFilenameBuffer);
	sprintf(buffer, "%d", einstellungen2->m_land);
	WritePrivateProfileString("Persoenliche_Daten", "land", buffer, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "vat1", (LPCSTR)einstellungen2->m_vat1, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "vat2", (LPCSTR)einstellungen2->m_vat2, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "vat3", (LPCSTR)einstellungen2->m_vat3, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Persoenliche_Daten", "vat4", (LPCSTR)einstellungen2->m_vat4, EasyCashIniFilenameBuffer);

	WritePrivateProfileString("Finanzamt", "name", (LPCSTR)einstellungen3->m_name, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Finanzamt", "name2", (LPCSTR)einstellungen3->m_name2, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Finanzamt", "strasse", (LPCSTR)einstellungen3->m_strasse, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Finanzamt", "plz", (LPCSTR)einstellungen3->m_plz, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Finanzamt", "ort", (LPCSTR)einstellungen3->m_ort, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Finanzamt", "steuernummer", (LPCSTR)einstellungen3->m_steuernummer, EasyCashIniFilenameBuffer);

	for (i = 0; i < 100; i++)
	{
		char key_buffer[100];
		
		sprintf(key_buffer, "%2.2d", i);

		WritePrivateProfileString("EinnahmenRechnungsposten", key_buffer, (LPCSTR)einstellungen1->EinnahmenRechnungsposten[i], EasyCashIniFilenameBuffer);
		WritePrivateProfileString("EinnahmenFeldzuweisungen", key_buffer, (LPCSTR)einstellungen1->EinnahmenFeldzuweisungen[i], EasyCashIniFilenameBuffer);
		WritePrivateProfileString("EinnahmenUnterkategorien", key_buffer, (LPCSTR)einstellungen1->EinnahmenUnterkategorien[i], EasyCashIniFilenameBuffer);

		WritePrivateProfileString("AusgabenRechnungsposten", key_buffer, (LPCSTR)einstellungen1->AusgabenRechnungsposten[i], EasyCashIniFilenameBuffer);
		WritePrivateProfileString("AusgabenFeldzuweisungen", key_buffer, (LPCSTR)einstellungen1->AusgabenFeldzuweisungen[i], EasyCashIniFilenameBuffer);
		WritePrivateProfileString("AusgabenUnterkategorien", key_buffer, (LPCSTR)einstellungen1->AusgabenUnterkategorien[i], EasyCashIniFilenameBuffer);
	}

	sprintf(buffer, "%d", (int)einstellungen4->m_ustvst_gesondert);
	WritePrivateProfileString("Allgemein", "ustvst_gesondert", (LPCSTR)buffer, EasyCashIniFilenameBuffer);	

	sprintf(buffer, "%d", (int)einstellungen4->m_nach_konten_seitenumbruch);
	WritePrivateProfileString("Allgemein", "nach_konten_seitenumbruch", (LPCSTR)buffer, EasyCashIniFilenameBuffer);	

	WritePrivateProfileString("Allgemein", "Finanzamtserstattungen", (LPCSTR)einstellungen4->m_finanzamtserstattungen, EasyCashIniFilenameBuffer);
	WritePrivateProfileString("Allgemein", "Finanzamtszahlungen", (LPCSTR)einstellungen4->m_finanzamtszahlungen, EasyCashIniFilenameBuffer);

	for (i = 0; i < einstellungen5->m_privat_split_size; i++)
	{
		char key_buffer[100];
		
		sprintf(key_buffer, "Art%2.2d", i);
		WritePrivateProfileString("Privat-Split", key_buffer, (LPCSTR)(*(einstellungen5->m_part[i])), EasyCashIniFilenameBuffer);

		sprintf(key_buffer, "Satz%2.2d", i);
		WritePrivateProfileString("Privat-Split", key_buffer, (LPCSTR)(*(einstellungen5->m_psatz[i])), EasyCashIniFilenameBuffer);

		sprintf(key_buffer, "UstSatz%2.2d", i);
		WritePrivateProfileString("Privat-Split", key_buffer, (LPCSTR)(*(einstellungen5->m_pustsatz[i])), EasyCashIniFilenameBuffer);
	}

	if (NetzwerkEasyCashIniFilenameBuffer[0] == '\\' && NetzwerkEasyCashIniFilenameBuffer[1] == '\\')	// UNC-Pfad ?
	{
		if (!CopyFile(EasyCashIniFilenameBuffer, NetzwerkEasyCashIniFilenameBuffer, FALSE))	// kann nicht kopieren?
			AfxMessageBox("Fehler beim Speichern der Einstellungen im Netzwerkpfad.");
		DeleteFile(EasyCashIniFilenameBuffer);	// temporäre Ini wieder löschen
	}

	SetCursor(hcOld);
}

char *CEasyCashView::GetDefaultVAT()
{
	return einstellungen2->m_vat1.GetBuffer(0);
}

char *CEasyCashView::GetVATs(int n)
{
	switch (n)
	{
	case 0: return einstellungen2->m_vat1.GetBuffer(0);
	case 1: return einstellungen2->m_vat2.GetBuffer(0);
	case 2: return einstellungen2->m_vat3.GetBuffer(0);
	case 3: return einstellungen2->m_vat4.GetBuffer(0);
	}
	return "";
}

char *CEasyCashView::GetLandeskuerzel()
{
	switch (einstellungen2->m_land)
	{
	case 0: return "D";
	case 1: return "AT";
	case 2: return "CH";
	}
	return "";
}

/*
wurde ersetzt durch (LPCSTR)pDoc->csWaehrung
char *CEasyCashView::GetWaehrungskuerzel()
{
	switch (einstellungen2->m_land)
	{
	case 0: return "DEM";
	case 1: return "ATS";
	case 2: return "CHF";
	}
	return "?";
}
*/

void CEasyCashView::DestroyPlugin()
{
	if (pPluginWnd) 
	{
		CPluginElement *pPluginDaten = ((CMainFrame*)AfxGetMainWnd())->m_pPlugins;

		// Destroy-Aufruf im Plugin
		CEasyCashDoc* pDoc = GetDocument();
		CString csIDString;
		csIDString.Format("%u", (DWORD)pDoc);
		int pos;
		BOOL bPlatzhalterGefunden = FALSE;	// nur wenn nicht HTML-Plugin, Destroy-Member aufrufen
		if ((pos = m_csAufrufEscaped.Find("$$ID$$")) < 0) 
		{
			// ab hier: nur noch Destroy-Funktionsaufruf
			LPUNKNOWN pUnk;
			pPluginWnd->QueryControl(&pUnk);
			if (!pUnk)
			{
/*				CString csError;
				DWORD dwError = GetLastError();
				LPVOID lpMsgBuf;
 
				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
				);

				if (!lpMsgBuf) lpMsgBuf = "keine genauere Fehlerbeschreibung verfügbar";
				csError.Format("Fehler %lx beim Verlassen des ActiveX-Plugins (QueryControl): %s", (long)dwError, lpMsgBuf);
				AfxMessageBox(csError);
*/				goto exit_here;
			}
			IDispatch *spDispatch; 
			HRESULT hRes = pUnk->QueryInterface(__uuidof(spDispatch), (void **) &spDispatch); 	
			if (hRes != S_OK)
			{
				CString csError;
				csError.Format("Fehler %lx beim Verlassen des ActiveX-Plugins (QueryInterface).", (long)hRes);
//				AfxMessageBox(csError);
				goto exit_here;
			}

			// spDispatch->AddRef(); // rrrrrrrrrrrr....

			DISPID dispid;
			OLECHAR FAR szMember[8];
			MultiByteToWideChar(CP_ACP, 0, "Destroy", -1, szMember, 8);
			OLECHAR FAR *pszMember = szMember;
			DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
			VARIANT vRet;
			COleVariant vParam((long)GetDocument(),VT_I4);
			EXCEPINFO excepinfo;
			UINT nArgErr;
			dispparams.rgvarg = (LPVARIANT)vParam;
			dispparams.cArgs = 1;
			dispparams.cNamedArgs = 0;

			hRes = spDispatch->GetIDsOfNames(IID_NULL, &pszMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
			if (hRes == DISP_E_UNKNOWNNAME)
			{
//				AfxMessageBox("Destroy-Methode wurde im ActiveX-Plugin nicht gefunden.");
				goto exit_here;
			}
			else if (hRes == E_OUTOFMEMORY)
			{
				AfxMessageBox("Speicherprobleme beim Verlassen des ActiveX-Plugins.");
//				goto exit_here;
			}
			else if (hRes == DISP_E_UNKNOWNLCID)
			{
//				AfxMessageBox("Unbekannte Ländereinstellung beim Verlassen des ActiveX-Plugins.");
				goto exit_here;
			}
			else if (hRes != S_OK)
			{
//				AfxMessageBox("Unspezifizierbarer Fehler beim Verlassen des ActiveX-Plugins.");
				goto exit_here;
			}

			hRes = spDispatch->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparams, &vRet, &excepinfo, &nArgErr);
			if (hRes != S_OK)
			{	
				CString csError;
				if (hRes == DISP_E_EXCEPTION && excepinfo.bstrDescription && excepinfo.bstrSource)
					csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s' (Quelle: %s)", (int)excepinfo.wCode, (LPCSTR)(CString)excepinfo.bstrDescription, (LPCSTR)(CString)excepinfo.bstrSource);
				else if (hRes == DISP_E_EXCEPTION && excepinfo.bstrDescription)
					csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s'", (int)excepinfo.wCode, (LPCSTR)(CString)excepinfo.bstrDescription);
				else if (hRes == DISP_E_EXCEPTION && excepinfo.bstrSource)
					csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s'", (int)excepinfo.wCode, (LPCSTR)(CString)excepinfo.bstrSource);
				else if (hRes == DISP_E_EXCEPTION && excepinfo.wCode != 0)
					csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx", (int)excepinfo.wCode);
				else if (hRes == DISP_E_EXCEPTION)
					csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke Destroy).");
				else 
					csError.Format("Fehler %lx beim Verlassen des ActiveX-Plugins (Invoke).", (long)hRes);
//				AfxMessageBox(csError);
				goto exit_here;
			}

			spDispatch->Release(); // vermindert nicht den reference count m_dwRef in CCmdTarget...
		}

exit_here:
		if (pPluginWnd->IsWindow())
			pPluginWnd->DestroyWindow(); 
		delete pPluginWnd;
		pPluginWnd = NULL;

		((CExtSplitter*)GetParent())->ShowColumn(1);
	}
}

BOOL CEasyCashView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	CPluginElement *pPluginDaten = ((CMainFrame*)AfxGetMainWnd())->m_pPlugins;

	// Submenü Dauerbuchungen ausführen (bis Monat)
	if (nID >= ID_BUCHEN_DAUAUS_JANUAR && nID <= ID_BUCHEN_DAUAUS_DEZEMBER && nCode == CN_UPDATE_COMMAND_UI)
	{
		int nAktualisiertBisMonat = 0; 
		CEasyCashDoc* pDoc = GetDocument();
		CDauerbuchung* pdb = pDoc->Dauerbuchungen;

		while (pdb)
		{
			if ((pdb->AktualisiertBisDatum.GetMonth() > nAktualisiertBisMonat 
				&& pdb->AktualisiertBisDatum.GetYear() == pDoc->nJahr) 
				|| pdb->AktualisiertBisDatum.GetYear() > pDoc->nJahr)
					nAktualisiertBisMonat = pdb->AktualisiertBisDatum.GetMonth();
			pdb = pdb->next;
		}

		((CCmdUI*)pExtra)->SetCheck(nID-ID_BUCHEN_DAUAUS_JANUAR+1 < nAktualisiertBisMonat);
		((CCmdUI*)pExtra)->Enable(TRUE);
		return TRUE;
	}
	// Submenü Jounal für Monat
	else if (nID >= ID_VIEW_JOURNAL_MONATE_ALLE && nID <= ID_VIEW_JOURNAL_MONATE_QUARTAL4)
	{
		{
		if (nCode == CN_COMMAND)
		{
			switch (nID)
			{
			default:
			case ID_VIEW_JOURNAL_MONATE_ALLE:   	m_MonatsFilterDisplay = 0; break;
			case ID_VIEW_JOURNAL_MONATE_JANUAR:		m_MonatsFilterDisplay = 1; break;
			case ID_VIEW_JOURNAL_MONATE_FEBRUAR:	m_MonatsFilterDisplay = 2; break;
			case ID_VIEW_JOURNAL_MONATE_MAERZ:		m_MonatsFilterDisplay = 3; break;
			case ID_VIEW_JOURNAL_MONATE_APRIL:		m_MonatsFilterDisplay = 4; break;
			case ID_VIEW_JOURNAL_MONATE_MAI:		m_MonatsFilterDisplay = 5; break;
			case ID_VIEW_JOURNAL_MONATE_JUNI:		m_MonatsFilterDisplay = 6; break;
			case ID_VIEW_JOURNAL_MONATE_JULI:		m_MonatsFilterDisplay = 7; break;
			case ID_VIEW_JOURNAL_MONATE_AUGUST:		m_MonatsFilterDisplay = 8; break;
			case ID_VIEW_JOURNAL_MONATE_SEPTEMBER:	m_MonatsFilterDisplay = 9; break;
			case ID_VIEW_JOURNAL_MONATE_OKTOBER:	m_MonatsFilterDisplay = 10; break;
			case ID_VIEW_JOURNAL_MONATE_NOVEMBER:	m_MonatsFilterDisplay = 11; break;
			case ID_VIEW_JOURNAL_MONATE_DEZEMBER:	m_MonatsFilterDisplay = 12; break;
			case ID_VIEW_JOURNAL_MONATE_QUARTAL1:	m_MonatsFilterDisplay = 13; break;
			case ID_VIEW_JOURNAL_MONATE_QUARTAL2:	m_MonatsFilterDisplay = 14; break;
			case ID_VIEW_JOURNAL_MONATE_QUARTAL3:	m_MonatsFilterDisplay = 15; break;
			case ID_VIEW_JOURNAL_MONATE_QUARTAL4:	m_MonatsFilterDisplay = 16; break;
			}
			GetDocument()->UpdateAllViews(NULL);
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
			{
				((CCmdUI*)pExtra)->SetCheck(nID == ID_VIEW_JOURNAL_MONATE_ALLE + m_MonatsFilterDisplay);
				((CCmdUI*)pExtra)->Enable(TRUE);
				return TRUE;
			}
		}
	}
	// Jounalansicht nach Datum
	else if (nID == ID_VIEW_JOURNAL_DATUM && nCode == CN_UPDATE_COMMAND_UI)
	{
		((CCmdUI*)pExtra)->SetCheck(m_nAnzeige == 0);
		((CCmdUI*)pExtra)->Enable(TRUE);
		return TRUE;
	}
	// Jounalansicht nach Datum
	else if (nID == ID_VIEW_JOURNAL_KONTEN && nCode == CN_UPDATE_COMMAND_UI)
	{
		((CCmdUI*)pExtra)->SetCheck(m_nAnzeige == 1);
		((CCmdUI*)pExtra)->Enable(TRUE);
		return TRUE;
	}
	// Jounalansicht nach Datum
	else if (nID == ID_VIEW_JOURNAL_BESTKONTEN && nCode == CN_UPDATE_COMMAND_UI)
	{
		((CCmdUI*)pExtra)->SetCheck(m_nAnzeige == 2);
		((CCmdUI*)pExtra)->Enable(TRUE);
		return TRUE;
	}
	// Anlagenverzeichnis
	else if (nID == ID_VIEW_JOURNAL_ANLAGENVERZEICHNIS && nCode == CN_UPDATE_COMMAND_UI)
	{
		((CCmdUI*)pExtra)->SetCheck(m_nAnzeige == 3);
		((CCmdUI*)pExtra)->Enable(TRUE);
		return TRUE;
	}
	// Submenü Jounalfilter nach Konto
	else if (nID >= ID_VIEW_JOURNAL_FUER_KONTO_BASE && nID < ID_VIEW_JOURNAL_FUER_KONTO_BASE+200)
	{
		if (nCode == CN_COMMAND)
		{
			m_KontenFilterDisplay = m_KontenMitBuchungen[nID-ID_VIEW_JOURNAL_FUER_KONTO_BASE];	
			GetDocument()->UpdateAllViews(NULL);		
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->SetCheck(m_KontenMitBuchungen[nID-ID_VIEW_JOURNAL_FUER_KONTO_BASE] == m_KontenFilterDisplay);
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	// Submenü Jounalfilter nach  Betrieb
	else if (nID >= ID_BETRIEBEFILTER_BASE-1 && nID < ID_BETRIEBEFILTER_BASE+200)
	{
		if (nCode == CN_COMMAND)
		{
			if (nID >= ID_BETRIEBEFILTER_BASE)
				m_BetriebFilterDisplay = m_csaBetriebeNamen[nID-ID_BETRIEBEFILTER_BASE];	
			else
				m_BetriebFilterDisplay = "";
			GetDocument()->UpdateAllViews(NULL);		
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			if (nID >= ID_BETRIEBEFILTER_BASE)
			{
				((CCmdUI*)pExtra)->SetCheck(m_csaBetriebeNamen[nID-ID_BETRIEBEFILTER_BASE] == m_BetriebFilterDisplay);
				CMFCRibbonButton* pBtnFilterBetrieb;
				if (pBtnFilterBetrieb = ((CMainFrame*)AfxGetMainWnd())->m_pFilterBetriebButton)
				{
					int i;
					for (i = 0; i < pBtnFilterBetrieb->GetSubItems().GetCount(); i++)
						if (pBtnFilterBetrieb->GetSubItems()[i]->GetID() == nID)
						{
							((CMFCRibbonButtonEx*)pBtnFilterBetrieb->GetSubItems()[i])->SetCheck(m_csaBetriebeNamen[nID-ID_BETRIEBEFILTER_BASE] == m_BetriebFilterDisplay);
							break;
						}
				}
			}
			else
				((CCmdUI*)pExtra)->SetCheck(m_BetriebFilterDisplay == "");
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	// Submenü Jounalfilter nach Bestandskonto
	else if (nID >= ID_BESTANDSKONTENFILTER_BASE-1 && nID < ID_BESTANDSKONTENFILTER_BASE+200)
	{
		if (nCode == CN_COMMAND)
		{
			if (nID >= ID_BESTANDSKONTENFILTER_BASE)
				m_BestandskontoFilterDisplay = m_csaBestandskontenNamen[nID-ID_BESTANDSKONTENFILTER_BASE];	
			else
				m_BestandskontoFilterDisplay = "";
			GetDocument()->UpdateAllViews(NULL);		
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			if (nID >= ID_BESTANDSKONTENFILTER_BASE)
			{
				((CCmdUI*)pExtra)->SetCheck(m_csaBestandskontenNamen[nID-ID_BESTANDSKONTENFILTER_BASE] == m_BestandskontoFilterDisplay);
				CMFCRibbonButton* pBtnFilterBestandskonto;
				if (pBtnFilterBestandskonto = ((CMainFrame*)AfxGetMainWnd())->m_pFilterBestandskontoButton)
				{
					int i;
					for (i = 0; i < pBtnFilterBestandskonto->GetSubItems().GetCount(); i++)
						if (pBtnFilterBestandskonto->GetSubItems()[i]->GetID() == nID)
						{
							((CMFCRibbonButtonEx*)pBtnFilterBestandskonto->GetSubItems()[i])->SetCheck(m_csaBestandskontenNamen[nID-ID_BESTANDSKONTENFILTER_BASE] == m_BestandskontoFilterDisplay);
							break;
						}
				}
			}
			else
				((CCmdUI*)pExtra)->SetCheck(m_BestandskontoFilterDisplay == "");
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	// Formular-Knopf
	else if (nID == ID_ANSICHT_FORMULARE)
	{
		if (nCode == CN_COMMAND)
		{
			// plugin fenster schließen
			DestroyPlugin();
			
			if (pFormularfeldDlg) 
			{
				// ggf. Formularfeld-Edit-Fenster schließen
				pFormularfeldDlg->DestroyWindow();
				delete pFormularfeldDlg;
				pFormularfeldDlg = NULL;
			}

			// gleich Dauerfristverlängerung mit einberechnen, wenn Sondervorauszahlung gefunden wird
			CString Key, Daufri;
			int daufritage = 0;
			Key.Format("Sondervorauszahlung%-04.4d", GetDocument()->nJahr);
			char *cp = GetErweiterungKey(GetDocument()->Erweiterung, "Dauerfristverlängerung", Key);
			char *cp2;
			if (cp2 = strchr(cp, '|'))
			{ 
				strncpy(Daufri.GetBuffer(cp2-cp+1), cp, cp2-cp);
				Daufri.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
				Daufri.ReleaseBuffer();
			}
			if (atoi(Daufri) > 0) daufritage = 30;

			// aktuelles USt-VA-Formular herausbekommen
			CTime now = CTime::GetCurrentTime();
			CString csFromularnamenMatchDE, csFromularnamenMatchAT;
			if (!einstellungen1->m_monatliche_voranmeldung)
			{
				now -= CTimeSpan(28 + daufritage, 0, 0, 0);
				csFromularnamenMatchDE.Format("Umsatzsteuer-Voranmeldung %d %s (D)", (int)now.GetYear(), (LPCSTR)cpMonat[now.GetMonth()-1]);
				cpMonat[0] = "Jänner";
				csFromularnamenMatchAT.Format("U30 %d %s (AT)", (int)now.GetYear(), (LPCSTR)cpMonat[now.GetMonth()-1]);
			}
			else
			{
				now -= CTimeSpan(3*28 + daufritage, 0, 0, 0);
				csFromularnamenMatchDE.Format("Umsatzsteuer-Voranmeldung %d %d. Quartal (D)", (int)now.GetYear(), (int)(((now.GetMonth()-1)/3)+1));
				csFromularnamenMatchAT.Format("U30 %d %d. Quartal (AT)", (int)now.GetYear(), (int)(((now.GetMonth()-1)/3)+1));
			}
			int i;
			for (i = 0, m_GewaehltesFormular = 0; i < m_csaFormularnamen.GetCount(); i++)
			{
				// TRACE3("%02d: %s // %s\r\n", i, (LPCTSTR)m_csaFormularnamen[i], (LPCTSTR)m_csaFormulare[i]);
				if (m_csaFormularnamen[i] == csFromularnamenMatchDE || m_csaFormularnamen[i] == csFromularnamenMatchAT)
				{
					m_GewaehltesFormular = i;
					break;
				}
			}

			ShowWindow(SW_SHOW);
			GetDocument()->UpdateAllViews(NULL);
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	else if (nID >= ID_FORMULAR_BASE && nID < ID_FORMULAR_BASE+m_csaFormulare.GetSize())
	// Formular-Menüpunkt
	{
		if (nCode == CN_COMMAND)
		{
			// plugin fenster schließen
			DestroyPlugin();
			
			if (pFormularfeldDlg) 
			{
				// ggf. Formularfeld-Edit-Fenster schließen
				pFormularfeldDlg->DestroyWindow();
				delete pFormularfeldDlg;
				pFormularfeldDlg = NULL;
			}

			m_GewaehltesFormular = nID - ID_FORMULAR_BASE;
			ShowWindow(SW_SHOW);
			GetDocument()->UpdateAllViews(NULL);
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->SetCheck(nID-ID_FORMULAR_BASE == m_GewaehltesFormular);
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	else if (nID == ID_NEXT || nID == ID_PREV)
	// Find-Toolbar
	{
		if (nCode == CN_COMMAND)
		{
			ShowFindToolbar(SW_SHOW);
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			CString csText;
			//((CMainFrame*)AfxGetMainWnd())->m_wndFindToolBar.m_comboBox1.GetWindowText(csText);
			((CCmdUI*)pExtra)->Enable();		// CTRL-F muss immer gehen!      alt: ((CCmdUI*)pExtra)->Enable(!csText.IsEmpty());
			return TRUE;
		}
	}
	else if (nID == ID_FILE_MANDANTEN)
	{
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->Enable(FALSE);
			return TRUE;
		}
	}
	else if (nID == ID_VIEW_FINDTOOLBAR)
	// auch Find-Toolbar
	{
		if (nCode == CN_COMMAND)
		{
			
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			
		}
	}
	// Submenü Zoomlevel
	else if (nID >= ID_ZOOMFAKTOR_50 && nID <= ID_ZOOMFAKTOR_300 && nCode == CN_UPDATE_COMMAND_UI)
	{
		BOOL bCheck = FALSE;
		if (nID == ID_ZOOMFAKTOR_50  && m_wunschzoomfaktor ==  50) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_75  && m_wunschzoomfaktor ==  75) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_100 && m_wunschzoomfaktor == 100) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_125 && m_wunschzoomfaktor == 125) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_150 && m_wunschzoomfaktor == 150) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_175 && m_wunschzoomfaktor == 175) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_200 && m_wunschzoomfaktor == 200) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_250 && m_wunschzoomfaktor == 250) bCheck = true;
		if (nID == ID_ZOOMFAKTOR_300 && m_wunschzoomfaktor == 300) bCheck = true;
		((CCmdUI*)pExtra)->SetCheck(bCheck);
		((CCmdUI*)pExtra)->Enable(TRUE);
		return TRUE;
	}
	// plugins im Menü anzeigen
	else while (pPluginDaten)
	{
		if (pPluginDaten->id == nID)
		{
			if (nCode == CN_COMMAND)
			{
				DestroyPlugin();
				
				// Formularansicht zurücksetzen
				m_GewaehltesFormular = -1;

				AtlAxWinInit();	// registiere AtlAxWin80 Klasse -- sollte irgenwor unterhalb auch passieren, aber doppelt schadet nicht...
				pPluginWnd = new CAxWindow();
				CRect r;
				GetParent()->GetParent()->GetClientRect(&r);
				CEasyCashDoc* pDoc = GetDocument();
				CString csIDString;
				csIDString.Format("%u", (DWORD)pDoc);
				CString csAufrufEscaped;
				csAufrufEscaped = m_csAufrufEscaped = pPluginDaten->aufruf;
				csAufrufEscaped.ReleaseBuffer();
				int pos;
				BOOL bPlatzhalterGefunden = FALSE;	// dann nicht Meldung wg. fehlender Init-Member anzeigen
				while ((pos = csAufrufEscaped.Find("$$ID$$")) >= 0) // ggf. Platzhalter für Doc-Handle einfügen
				{
					CString csTail, csHead;
					if (csAufrufEscaped.GetLength() > pos+6)
						csTail = csAufrufEscaped.Mid(pos + 6);
					else 
						csTail = "";
					csHead = csAufrufEscaped.Left(pos);		// .Left tut es nicht :(
					csAufrufEscaped.Format("%s%s%s", csHead, csIDString, csTail);	
					
					bPlatzhalterGefunden = TRUE;
				}	

				if (!theApp.Check4EasyCTX())
				{
					DSAMessageBox(IDS_EASYCTX_FEHLT, MB_OK);
				}
			
//				if (csAufrufEscaped == "ECTImportX.ECTImportXCtrl.1")
//				{
//					DSAMessageBox(IDS_CSVIMPORT_INSTABIL, MB_OK);
//
//#if defined(NDEBUG)
//					// Uninstall crash reporting
//					crUninstall();
//#endif
//				}		fehler in v2.14 behoben!
//			
//				if (csAufrufEscaped == "EASYRTX.EasyRTXCtrl.1")
//					DSAMessageBox(IDS_EASYRTX_INSTABIL, MB_OK);

				if (!pPluginWnd->Create(GetParent()->m_hWnd, r, csAufrufEscaped, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (pPluginDaten->typ.Find("noscroll") >= 0 ? 0 : WS_VSCROLL | WS_HSCROLL)))
				{
					AfxMessageBox("Konnte Plugin-Fenster nicht erzeugen.");
					delete pPluginWnd;
					pPluginWnd = NULL;
					OnViewJournalKonto();
					return TRUE;
				}

				((CExtSplitter*)GetParent())->HideColumn(1);
				ShowWindow(SW_HIDE);

				// Wenn ein Platzhalter mit der Doc-ID ersetzt wurde, dann brauchen wir dies hier nicht:
				if (!bPlatzhalterGefunden)
				{
					// war Test, weil WM_CREATE beim Fahrtenbuch-Plugin nicht ankam:
					// pPluginWnd->CreateControl(csAufrufEscaped.AllocSysString());

					// ab hier: nur noch Init-Funktionsaufruf
					LPUNKNOWN pUnk;
					pPluginWnd->QueryControl(&pUnk);
					IDispatch *spDispatch; 
					HRESULT hRes = pUnk->QueryInterface(__uuidof(spDispatch), (void **) &spDispatch); 	
					if (hRes != S_OK)
					{
						CString csError;
						csError.Format("Fehler %lx beim Initialisieren des ActiveX-Plugins (QueryInterface).", (long)hRes);
						AfxMessageBox(csError);
						return TRUE;
					}

					// spDispatch->AddRef(); // rrrrrrrrrrrr....

					DISPID dispid;
					OLECHAR FAR szMember[5];
					MultiByteToWideChar(CP_ACP, 0, "Init", -1, szMember, 5);
					OLECHAR FAR *pszMember = szMember;
					DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
					VARIANT vRet;
					COleVariant vParam((long)GetDocument(),VT_I4);
					EXCEPINFO excepinfo;
					UINT nArgErr;
					dispparams.rgvarg = (LPVARIANT)vParam;
					dispparams.cArgs = 1;
					dispparams.cNamedArgs = 0;

					hRes = spDispatch->GetIDsOfNames(IID_NULL, &pszMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
					if (hRes == DISP_E_UNKNOWNNAME)
					{
						AfxMessageBox("Init-Methode wurde im ActiveX-Plugin nicht gefunden. Bitte ggf. das Plugin neu installieren.");
						return TRUE;
					}
					else if (hRes == E_OUTOFMEMORY)
					{
						AfxMessageBox("Speicherprobleme beim Initialisieren des ActiveX-Plugins.");
						return TRUE;
					}
					else if (hRes == DISP_E_UNKNOWNLCID)
					{
						AfxMessageBox("Unbekannte Ländereinstellung beim Initialisieren des ActiveX-Plugins.");
						return TRUE;
					}
					else if (hRes != S_OK)
					{
						AfxMessageBox("Unspezifizierbarer Fehler beim Initialisieren des ActiveX-Plugins.");
						return TRUE;
					}

					hRes = spDispatch->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispparams, &vRet, &excepinfo, &nArgErr);
					if (hRes != S_OK)
					{	
						CString csError;
						if (hRes == DISP_E_EXCEPTION && excepinfo.bstrDescription && excepinfo.bstrSource)
							csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s' (Quelle: %s)", (int)excepinfo.wCode, (LPCSTR)(CString)excepinfo.bstrDescription, (LPCSTR)(CString)excepinfo.bstrSource);
						else if (hRes == DISP_E_EXCEPTION && excepinfo.bstrDescription)
							csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s'", (int)excepinfo.wCode, (LPCSTR)(CString)excepinfo.bstrDescription);
						else if (hRes == DISP_E_EXCEPTION && excepinfo.bstrSource)
							csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s'", (int)excepinfo.wCode, (LPCSTR)(CString)excepinfo.bstrSource);
						else if (hRes == DISP_E_EXCEPTION && excepinfo.wCode != 0)
							csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke). Code %lx: '%s'", (int)excepinfo.wCode);
						else if (hRes == DISP_E_EXCEPTION)
							csError.Format("Ausnahmefehler beim Verlassen des ActiveX-Plugins (Invoke Init).");
						else 
							csError.Format("Fehler %lx beim Verlassen des ActiveX-Plugins (Invoke Init).", (long)hRes);
						AfxMessageBox(csError);
					}

					spDispatch->Release(); // vermindert nicht den reference count m_dwRef in CCmdTarget...
				}

				return TRUE;
			}
			if (nCode == CN_UPDATE_COMMAND_UI)
			{
				//((CCmdUI*)pExtra)->SetCheck(m_wndPluginToolBar.IsWindowVisible());
				((CCmdUI*)pExtra)->Enable(TRUE);
				return TRUE;
			}
		}

		pPluginDaten = pPluginDaten->next;
	}

	return CScrollView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CEasyCashView::OnFormularInfo()
{
	if (AfxMessageBox("Hinweis: Die Formulare für EasyCash&Tax werden üblicherweise bis Ende Januar über ein Update des Hauptprogramms zur Verfügung gestellt. Um zu prüfen, ob es schon ein Update gibt, einfach auf das rote Puzzlestück im Ansicht-Menübereich drücken. Bis zur Release des Updates kann man für eine Vorschau gern das Formular des Vorjahres benutzen. Registrierte User werden per E-Mail benachrichtigt, unregistrierte können sich auf der Homepage www.easyct.de auf dem Laufenden halten.\r\n\r\nÄltere Formulare können über Downloads -> Formulararchiv auf www.easyct.de bezogen werden. Jetzt das Formulararchiv im Browser öffnen?", MB_YESNO) == IDYES)
		ShellExecute(m_hWnd, "open", "https://www.easyct.de/downloads.php?cat_id=7", NULL, ".", SW_SHOWNORMAL);
}

void CEasyCashView::OnFormularNeu() 
{
	CNeuesFormular dlg;
	dlg.m_name = "Mein Formular";
	dlg.m_anzeigename = "Mein Formular (Sommersaison)";
	dlg.m_dateiname = "MeinFormular.ecf";
	dlg.m_seiten = 2;
	dlg.m_schriftart = "Courier New";
	dlg.m_schriftgroesse = 24;
	while (dlg.DoModal() == IDOK)
	{
		int i;
		for (i = 0; i < m_csaFormulare.GetSize(); i++)
			if (!stricmp(m_csaFormulare[i], dlg.m_dateiname))
			{
				AfxMessageBox("Formulardatei existiert bereits. Bitte einen anderen Dateinamen benutzen.");
				goto cont;
			}

		for (i = 0; i < m_csaFormularnamen.GetSize(); i++)
			if (!stricmp(m_csaFormularnamen[i], dlg.m_name))
			{
				AfxMessageBox("Formularname existiert bereits. Bitte einen anderen Formularnamen benutzen.");
				goto cont;
			}

		// neue Formulardatei generieren
		{
			CString csPfad;
			GetProgrammverzeichnis(csPfad.GetBuffer(1000), 1000);
			csPfad.ReleaseBuffer();
			csPfad += "\\" + dlg.m_dateiname;
			CStdioFile f(csPfad, CFile::modeCreate|CFile::modeWrite);

			f.WriteString("<?xml version='1.0' encoding='windows-1252'?>\r\n");
			CString csTemp;
			csTemp.Format("<formular name=\"%s\" anzeigename=\"%s\" seiten=\"%d\" schriftart=\"%s\" schriftgroesse=\"%d\">\r\n", (LPCSTR)dlg.m_name, (LPCSTR)dlg.m_anzeigename, (LPCSTR)dlg.m_seiten, (LPCSTR)dlg.m_schriftart, dlg.m_schriftgroesse);

			f.WriteString(csTemp);
			f.WriteString("\t<felder>\r\n");
			f.WriteString("\t</felder>\r\n");
			f.WriteString("\t<seiten>\r\n");
			for (int nSeite = 1, nZeichenpos = 0; nSeite <= dlg.m_seiten+1 && nZeichenpos < dlg.m_grafikdateien.GetLength(); nSeite++)
			{
				int nZeichenposNeu = dlg.m_grafikdateien.Find("\r\n", nZeichenpos);
				if (nZeichenposNeu == -1)
					csTemp = dlg.m_grafikdateien.Mid(nZeichenpos);
				else
					csTemp = dlg.m_grafikdateien.Mid(nZeichenpos, nZeichenposNeu-nZeichenpos);

				CString csTemp2;
				csTemp2.Format("\t\t<seite nr=\"%d\">%s</seite>\r\n", nSeite, (LPCSTR)csTemp);
				f.WriteString(csTemp2);

				if (nZeichenposNeu == -1) break;
				nZeichenpos = nZeichenposNeu + 2;
			}
			f.WriteString("\t</seiten>\r\n");
			f.WriteString("</formular>\r\n");
			f.Close();
//VS9
			//if (AfxGetMainWnd() && AfxGetMainWnd()->GetMenu() && AfxGetMainWnd()->GetMenu()->GetSubMenu(4))
			//	AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->InsertMenu(AfxGetMainWnd()->GetMenu()->GetSubMenu(4)->GetMenuItemCount()-1, MF_BYPOSITION, ID_FORMULAR_BASE+m_csaFormularnamen.GetSize(), dlg.m_name);
			//m_csaFormularnamen.Add(dlg.m_name);
			//m_csaFormulare.Add(csPfad);
			UpdateFormularMenu();  // <- besser so als die letzten vier Zeilen...
		}
		break;

cont:	;
	}	
}

void CEasyCashView::OnFormularMenuUpdate()
{
	UpdateFormularMenu();
}

#define ONKEY_WASDOWNBEFORE	0x4000
#define ONKEY_EXTENDEDKEY	0x0100

void CEasyCashView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
		/* Suchen jetzt mit Accelerators in den Ressourcen
	case 0x1d: // STRG-Taste
		m_bCtrlKeyPressed = TRUE; 
		break;
	case 'F':
		if (m_bCtrlKeyPressed)	//STRG-F - Find-Funktion
			ShowFindToolbar();		
		break;
	case 0x26:
		OnFindPrev();		// Arrow up
		break;
	case 0x28:
		OnFindNext();		// Arrow down
		break;
		*/
		
		// Scrolling
        case VK_HOME :
                OnVScroll(SB_TOP, 0, NULL);
                OnHScroll(SB_LEFT, 0, NULL);
                OnVScroll(SB_LINEDOWN, 0, NULL);
				if (m_GewaehltesFormular < 0 && nSelected >= 0)
				{
					int nSelectedNew = 0;
					while (++nSelectedNew < MAX_BUCHUNGEN)		// Buchungsindex erhöhen
						if (ppPosBuchungsliste[nSelectedNew])	// erste Buchung suchen ...
						{
							nSelected = nSelectedNew;
							RedrawSelection();					// ... und selektieren
							break;
						}
				}
                break;
        case VK_END :
                OnVScroll(SB_BOTTOM, 0, NULL);
                OnHScroll(SB_LEFT, 0, NULL);
				if (m_GewaehltesFormular < 0 && nSelected >= 0)
				{
					int nSelectedNew = MAX_BUCHUNGEN - 1;
					while (--nSelectedNew > 0)					// Buchungsindex erhöhen
						if (ppPosBuchungsliste[nSelectedNew])	// letzte Buchung suchen ...
						{
							nSelected = nSelectedNew;
							RedrawSelection();					// ... und selektieren
							break;
						}
				}
                break;
        case VK_UP :
                OnVScroll(SB_LINEUP, 0, NULL);
				if (m_GewaehltesFormular < 0 && nSelected >= 0)
				{
					int nSelectedNew = nSelected;
					while (--nSelectedNew > 0)					// Buchungsindex erhöhen
						if (ppPosBuchungsliste[nSelectedNew])	// vorige Buchung suchen ...
						{
							nSelected = nSelectedNew;
							RedrawSelection();					// ... und selektieren
							break;
						}
				}
                break;
        case VK_DOWN:
				OnVScroll(SB_LINEDOWN, 0, NULL);
				if (m_GewaehltesFormular < 0 && nSelected >= 0)
				{
					int nSelectedNew = nSelected;
					while (++nSelectedNew < MAX_BUCHUNGEN)		// Buchungsindex erhöhen
						if (ppPosBuchungsliste[nSelectedNew])	// nächste Buchung suchen ...
						{
							nSelected = nSelectedNew;
							RedrawSelection();					// ... und selektieren
							break;
						}
				}
                break;
        case VK_PRIOR:
				{
					int scrollPos1 = GetScrollPosition().y / charheight;
					OnVScroll(SB_PAGEUP, 0, NULL);
					int scrollPos2 = GetScrollPosition().y / charheight;
					if (m_GewaehltesFormular < 0 && nSelected >= 0)
					{
						if (scrollPos1 == scrollPos2)					// wind wir bereits auf der ersten Seite?
						{
							int nSelectedNew = 0;
							while (++nSelectedNew < MAX_BUCHUNGEN)		// Buchungsindex erhöhen,
								if (ppPosBuchungsliste[nSelectedNew])	// erste Buchung suchen ...
								{
									nSelected = nSelectedNew;
									RedrawSelection();					// ... und selektieren
									break;
								}
						}
						else
						{
							int nSelectedNew = nSelected;
							nSelectedNew += scrollPos2 - scrollPos1;
							while (nSelectedNew > 0)	// Buchungsindex verringern
								if (ppPosBuchungsliste[nSelectedNew])				// beste Buchung für vorige Seite suchen ...
								{
									nSelected = nSelectedNew;
									RedrawSelection();								// ... und selektieren
									break;
								}
								else
									nSelectedNew--;
						}
					}
				}
                break;
        case VK_NEXT:
				{
					int scrollPos1 = GetScrollPosition().y / charheight;
					OnVScroll(SB_PAGEDOWN, 0, NULL);
					int scrollPos2 = GetScrollPosition().y / charheight;
					if (m_GewaehltesFormular < 0 && nSelected >= 0)
					{
						if (scrollPos1 == scrollPos2)					// wind wir bereits auf der letzten Seite?
						{
							int nSelectedNew = MAX_BUCHUNGEN - 1;
							while (--nSelectedNew > 0)					// dann Buchungsindex erhöhen,
								if (ppPosBuchungsliste[nSelectedNew])	// letzte Buchung suchen ...
								{
									nSelected = nSelectedNew;
									RedrawSelection();					// ... und selektieren
									break;
								}
						}
						else
						{
							int nSelectedNew = nSelected;
							nSelectedNew += scrollPos2 - scrollPos1;
							while (nSelectedNew < MAX_BUCHUNGEN)	// Buchungsindex erhöhen
								if (ppPosBuchungsliste[nSelectedNew])							// beste Buchung für nächste Seite suchen ...
								{
									nSelected = nSelectedNew;
									RedrawSelection();											// ... und selektieren
									break;
								}
								else
									nSelectedNew++;
						}
					}
				}
                break;
        case VK_LEFT:
                OnHScroll(SB_LINELEFT, 0, NULL);
                break;
        case VK_RIGHT:
                OnHScroll(SB_LINERIGHT, 0, NULL);
                break;
		case VK_RETURN:
				if (m_GewaehltesFormular < 0 && nSelected >= 0)
				{
					PopUpPosition = CPoint(0, nSelected * charheight);
					PostMessage(WM_COMMAND, POPUP_AENDERN, 0);
				}
                break;

		// zoom
		case VK_ADD:
        case VK_OEM_PLUS:
                m_zoomfaktor += 25;
				SetzeZoomfaktor();
                break;
		case VK_SUBTRACT:
        case VK_OEM_MINUS:
                m_zoomfaktor -= 25;
				SetzeZoomfaktor();
                break;
	}

	//AfxTrace("CEasyCashView::OnKeyDown: %0x (%c) %0x %d\r\n", (int)nChar, (int)nChar, nFlags, nRepCnt);
	
	
	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

// für bewusstes Setzen des Zoomfaktors durch den User (deshalb Wunschzoomfaktor, der durch Fensterverkleinerungen nicht betroffen ist)
void CEasyCashView::SetzeZoomfaktor()
{
	if (m_zoomfaktor > 1000) m_zoomfaktor = 1000;
	if (m_zoomfaktor < 25) m_zoomfaktor = 25;	
	m_wunschzoomfaktor = m_zoomfaktor;
	theApp.WriteProfileInt("Fenster", "JournalZoomFaktor", m_wunschzoomfaktor);
	SetupScroll();
	RedrawWindow();
	CString csStatusMessage;
	csStatusMessage.Format("Zoomfaktor auf %d%%", m_zoomfaktor);
	((CMainFrame*)AfxGetMainWnd())->SetStatus(csStatusMessage);
}

void CEasyCashView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	AfxTrace("CEasyCashView::OnKeyUp: %0x (%c) %0x %d\r\n", (int)nChar, (int)nChar, nFlags, nRepCnt);

	switch (nChar)
	{
	case 0x1d: // STRG-Taste
		m_bCtrlKeyPressed = FALSE; 
		break;
	}
	
	CScrollView::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CEasyCashView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// CSplitterWnd verhindert mousewheel irgendwie. Deshalb explizit nochmal:
	CPoint pos = GetScrollPosition();
	pos.y -= zDelta;
	ScrollToPosition(pos);

	return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}
