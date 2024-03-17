// EasyCashView.h : interface of the CEasyCashView class
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
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_EASYCASHVIEW_H_)
#define _EASYCASHVIEW_H_

#include "einstellungen1.h"
#include "einstellungen2.h"
#include "einstellungen3.h"
#include "einstellungen4.h"
#include "einstellungen5.h"
#include "Formularfeld.h"
#include "Navigation.h"
#include "..\GrafLib\cimage\cimage.h"

class BuchenDlg;
class DauerbuchungenDlg;

class DrawInfo
{
public:
	CPrintInfo* pInfo;
	BOOL pm;			/*Use Printer Metrix*/
	int line;
	int letzte_spalte_device;			// Journal-Layout
	int spalte_betriebicon;
	int spalte_bestandskontoicon;
	int spalte_datum;
	int spalte_belegnummer;
	int spalte_belegnummer_ende;
	int spalte_beschreibung; 
	int spalte_beschreibung_ende;
	int spalte_netto;
	int spalte_ust;
	int spalte_ust_betrag;
	int spalte_brutto;
	int spalte_afanr;
	int spalte_anschaffungskosten;
	int spalte_buchwert_beginn;
	int spalte_afa;
	int spalte_abgaenge;
	int spalte_buchwert_ende;

	SIZE printer_gesamtgroesse;
	int fontsize;	//  fontsize ist die max. erlaubte Fontgröße in Punkten
	float printer_charwidth, printer_charheight;	//  in Pixel
	int printer_fontsize;	// gleiches für Printer-Metrix
	long brutto_summe, netto_summe;
	long buchwert_beginn_summe, buchwert_ende_summe, abgaenge_summe;
	long vst, ust;
	CDC *m_pDC;	// device context
	BOOL zeige_betriebicon, zeige_bestandskontoicon, zeige_steuerspalte, zeige_belegnummernspalte;

	int querformat; // Bitmaps breiter drucken in Image2
};

class CToolBarWithCombo;

#define FORMULARSEITENCACHESIZE 1000

class CEasyCashView : public CScrollView
{
	friend CFormularfeld;
	friend CToolBarWithCombo;

protected: // create from serialization only
	CEasyCashView();
	DECLARE_DYNCREATE(CEasyCashView)

// Attributes
public:
	CEasyCashDoc* GetDocument();
	CNavigation *m_pNavigationWnd;
	//int CALLBACK GroupCompare(int Arg1, int Arg2, void *Arg3);  // callback für Navigation

	// Einstellungsdialogbox
	CPropertySheet *propdlg;
	CEinstellungen1 *einstellungen1;
	CEinstellungen2 *einstellungen2;
	CEinstellungen3 *einstellungen3;
	CEinstellungen4 *einstellungen4;
	CEinstellungen5 *einstellungen5;

	// Druckeinstellungen
	int m_umstvoranmeldung_h;
	int m_umstvoranmeldung_h2;
	int m_umstvoranmeldung_v;
	BOOL m_formular_nicht_mitdrucken;

	// für UVA-Druck
	CString m_umstvoranmeldung_sondervorauszahlung;
	BOOL bVorauszahlungssollMerken;
	BOOL m_nJahreswechselAbfrage;

	// Alles für die Journal-Anzeige
	int m_nAnzeige;
	CString m_KontenFilterPrinter;		// Variablen für gefilterten Journaldruck
	CTime voranmeldung_von_datum;
	CTime voranmeldung_bis_datum;
	CString m_BetriebFilterPrinter;		// 1 Variable für Freestyle-EÜR nach Betrieb und Journals nach Datum/Konto
	CString m_KontenFilterDisplay;		// 4 Variablen für Ansicht->Journal für ...
	int		m_MonatsFilterDisplay;
	CString m_BetriebFilterDisplay;		
	CString m_BestandskontoFilterDisplay;	
	CStringArray m_KontenMitBuchungen;		// für Menü-Aufbau, gesetzt in DrawToDC...
	CStringArray m_csaBestandskontenMitBuchungen;	// für Navigation
	OSVERSIONINFOEX m_osvi;				// für Win95/98/ME - Scrollbug-Message

	// Formularanzeige
	int m_GewaehltesFormular;			// aktuell gewähltes Formular, -1 wenn Journal Ansicht aktiv
	CStringArray m_csaFormulare;		// Dateipfade zu den Formulardateien
	CStringArray m_csaFormularnamen;	// Namen der Formulare
	CStringArray m_csaFormularseitenPfade;	// Dateipfade zu den Grafikdateien der Formularseiten
	::CImage *m_pFormularseitenImageCache[FORMULARSEITENCACHESIZE];
	CStringArray m_csaFormularfilter;	// Namen der zum Formular ausgewählten Betriebe etwa, wenn filter=betrieb im Formular
	CStringArray m_csaFormularfeldwerte;// mit BerechneFormularfeldwerte() berechnete Inhalte der Formularfelder
	BOOL m_bFormularfelderAnzeigen;		// Felder mit ID und Rahmen anzeigen (zum Bearbeiten)

// Operations
public:
	BuchenDlg *buchenDlg;
	BuchenDlg *pBuchungAendernDlg;
	DauerbuchungenDlg *dauerbuchungenDlg;
	CAxWindow *pPluginWnd;
	CString m_csAufrufEscaped;			// aktives Plugin
	CFormularfeld *pFormularfeldDlg;
	CPoint ptFeldmarke;
	BOOL m_bFeldmarkeRechtsbuendig;
	CPoint ptLetzteMousePosition;
	CPoint m_ptMittlererMausButtonDown;
	CPoint m_ptLinkerMausButtonDown;
	int m_nFeldMove;							// enthält die ID beim Bewegen eines Formularfeldes
	CArray<int> m_ptFeldMoveMultiselect;  // enthält die IDs beim Bewegen mehrerer Formularfelder wähend Multiselect mit mittlerer Maustaste
	int querformat_faktor;

	// Menü-Krempel
	CMenu PopUp;
	CMenu PopUpFormular;
	CPoint PopUpPosition;

#define MAX_BUCHUNGEN 655360
	CBuchung **ppPosBuchungsliste[MAX_BUCHUNGEN];	// zu faul...
	int nSelected;

	// für das Display-Layout
	SIZE gesamtgroesse;
	int letzte_zeile, letzte_spalte;	// letzte_spalte für variable Fensterbreite
	int charwidth, charheight;		//  in Pixel
	int page; // Hilfsvariable für DrawToDC: Seitenzähler
	int seitenzaehler; // Hilfsvariable für DrawToDC: anderer Seitenzähler
	int WasWirdGedruckt; // gesetzt in PreparePrint für OnPrint
	int max_seitenzahl;
	int m_vt;
	int m_vm;
	int m_bt;
	int m_bm;
	int m_bAfaKorrketuren;
	int m_anzahl_formularseiten;  // wird für die Navigations-Seitenleiste benutzt (ein bisschen redundant, aber dafür sicher, dass es keine Nebeneffekte hat)

	// Bitmaps fürs Menü
	CBitmap bmpEinBuch;
	 
	// Für die Anzeige der Betriebe/Bestandskonten-Icons
	CBitmap m_cbmIcons;
	CBitmap m_cbmIconsBestandskonten;
	CMFCToolBarImages m_tbiIcons;
	CMFCToolBarImages m_tbiIconsBestandskonten;
	CStringArray m_csaBetriebeNamen;
	CStringArray m_csaBetriebeIcons;
	CStringArray m_csaBetriebeUnternehmensarten;
	CStringArray m_csaBestandskontenNamen;
	CStringArray m_csaBestandskontenIcons;
	CStringArray m_csaBestandskontenSalden;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasyCashView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	int GetFeldindexFromMausposition(LPXNode felder);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
	
// Implementation
public:
	virtual ~CEasyCashView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DWORD m_TextColor;
	DWORD m_TextColorEinnahmen;
	DWORD m_TextColorAusgaben; 
	int m_zoomfaktor;		// in prozent
	int m_wunschzoomfaktor;	// wird in registry gespeichert
	CTime m_timeLetzteZwangsverkleinerung;  // um automatische Zoomvergrößerung auf den Wunschlevel zu unterbinden

	BOOL m_bCtrlKeyPressed;	// tracked in OnKeyUp/Down

	void SetupScroll();
	void Text(DrawInfo *pDrawInfo, int left, int top, char *s);
	void Text10(DrawInfo *pDrawInfo, int left, int top, char *s);
	void Box(DrawInfo *pDrawInfo, int left, int top, int right, int bottom);
	void GraueBox(DrawInfo *pDrawInfo, int left, int top, int right, int bottom);
	void Underline(DrawInfo *pDrawInfo, int left, int top, int right);
	void Underline10(DrawInfo *pDrawInfo, int left, int top, int right);
	RECT TextEx(DrawInfo *pDrawInfo, int left, int top, int right, int bottom, char *s);
	BOOL ScrollbugCheck(CDC *pDC, int top);
	void Image(DrawInfo *pDrawInfo, char *filename);
	void Image2(DrawInfo *pDrawInfo, char *filename);
	void Icon(DrawInfo *pDrawInfo, int left, int top, /*int right,*/ int bottom, CBitmap *bitmap, int n);
	void DrawToDC_Datum(CDC* pDC, DrawInfo *pDrawInfo);
	void DrawToDC_Konten(CDC* pDC, DrawInfo *pDrawInfo);
	BOOL BestandskontoExistiertInBuchungen(CString &bestandskontoname);
	void DrawToDC_Bestandskonten(CDC* pDC, DrawInfo *pDrawInfo);
	void DrawToDC_Anlagenverzeichnis(CDC* pDC, DrawInfo *pDrawInfo);
	void CheckLayout(DrawInfo *pDrawInfo);
	void CheckAnlagenverzeichnisLayout(DrawInfo *pDrawInfo);
	bool CheckPlatzFuerBeschreibung(DrawInfo *pDrawInfo);
	bool DrawToDC_EinnahmenHeader(DrawInfo *pDrawInfo);
	void DrawToDC_AusgabenHeader(DrawInfo *pDrawInfo);
	void DrawToDC_AnlagenverzeichnisHeader(DrawInfo *pDrawInfo);
	bool DrawToDC_BestandskontenHeader(DrawInfo *pDrawInfo, int nIcon, int nAnfangssaldo, int nBuchungenDavor, CTime von);
	void DrawToDC_EinnahmenFooter(DrawInfo *pDrawInfo);
	void DrawToDC_AusgabenFooter(DrawInfo *pDrawInfo);
	void DrawToDC_BestandskontenFooter(DrawInfo *pDrawInfo);
	void DrawToDC_AnlagenverzeichnisFooter(DrawInfo *pDrawInfo);
	void DrawToDC_EinnahmenLine(DrawInfo *pDrawInfo, CBuchung *p);
	void DrawToDC_AusgabenLine(DrawInfo *pDrawInfo, CBuchung *p);
	void DrawToDC_AnlagenverzeichnisLine(DrawInfo *pDrawInfo, CBuchung *p);
	void DrawToDC_BestandskontenLine(DrawInfo *pDrawInfo, CBuchung *p, char cBuchungsart);
	void DrawToDC_LineBreak(DrawInfo *pDrawInfo, int n);
	void DrawToDC_PrintLineNumber(DrawInfo *pDrawInfo);
	void DrawToDC_Selection(DrawInfo *pDrawInfo, CDC *pDC);
	void DrawToDC_Marker(DrawInfo *pDrawInfo, CDC *pDC, int nLine, COLORREF color);
	void RedrawSelection();
	void DrawUmStErklaerungToDC(CDC* pDC, DrawInfo *pDrawInfo);
	void DrawEURechungToDC(CDC* pDC, DrawInfo *pDrawInfo);
	void BerechneFormularfeldwerte();
	void DrawFormularToDC(CDC* pDC, DrawInfo *pDrawInfo);
	void OnFind(int nIncrement);
	void ShowFindToolbar(int nShowstate = SW_SHOW);
	void UpdateFormularMenu();
	void UpdateBetriebeMenu();
	void UpdateBestandskontenMenu();
	void UpdateBetriebe();
	void UpdateBestandskonten();
	BOOL Export(char *_Efilename, char *_Afilename, BOOL bExplorerOeffnen);
	BOOL Backup(char *backupdir);
	void DestroyPlugin();
	void GetUmsatzsteuervorauszahlung(int nZeitraum, CString& csValue);
	void SetUmsatzsteuervorauszahlung(int nZeitraum, CString& csValue);
	void DauerbuchungenAusfuehren(int jb, int mb);
	void CaptionBoxCheckOnUpdate();
	void SetzeZoomfaktor();
	void MoveFormularfeld(LPXNode child, CSize delta);

public:
	void LoadProfile();
	void SaveProfile();
	char *GetDefaultVAT();
	char *GetVATs(int n);
	char *GetLandeskuerzel(); 
	char *GetWaehrungskuerzel();
	void ScrolleZuBuchung(int b);
	void ScrolleZuSeite(int s, int vertikal = 0);
	void AfAAbgang(CBuchung **ppb);
	CUIntArray m_cuiaScrollPos;	// Array von uints für Formular-Abschnittsnavigation

// Generated message map functions
protected:
	//{{AFX_MSG(CEasyCashView)
	afx_msg void OnEditEinnahmeBuchen();
	afx_msg void OnEditAusgabeBuchen();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDestroy();
	afx_msg void OnEditDauerbuchungenEingeben();
	afx_msg void OnEditDauerbuchungenAusfuehren();
	afx_msg void OnEditUmsatzsteuervorauszahlungen();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnFileJahreswechsel();
	afx_msg void OnFileBackupSubfolder();
	afx_msg void OnFileBackupExternal();
	afx_msg void OnFileBackup();
	afx_msg void OnViewOptions();
	afx_msg void OnFileExport();
	afx_msg void OnViewJournalDatum();
	afx_msg void OnViewJournalKonten();
	afx_msg void OnViewJournalBestandskonten();
	afx_msg void OnViewJournalSwitch();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnFormularInfo();
	afx_msg void OnFormularNeu();
	afx_msg void OnFormularMenuUpdate();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnFindNext();
	afx_msg void OnFindPrev();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewFindtoolbar();
	afx_msg void OnViewJournalBestandskonto();
	afx_msg void OnViewJournalBetrieb();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnViewJournalBestkonten();
	afx_msg void OnViewJournalAnlagenverzeichnis();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFilePrint2();
public:
	afx_msg void OnBuchenDauausJanuar();
	afx_msg void OnBuchenDauausFebruar();
	afx_msg void OnBuchenDauausMaerz();
	afx_msg void OnBuchenDauausApril();
	afx_msg void OnBuchenDauausMai();
	afx_msg void OnBuchenDauausJuni();
	afx_msg void OnBuchenDauausJuli();
	afx_msg void OnBuchenDauausAugust();
	afx_msg void OnBuchenDauausSeptember();
	afx_msg void OnBuchenDauausOktober();
	afx_msg void OnBuchenDauausNovember();
	afx_msg void OnBuchenDauausDezember();
	afx_msg void OnViewJournalMonat();
	afx_msg void OnViewJournalKonto();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnViewZoomSwitch();
	afx_msg void OnZoomfaktorVergroessern();
	afx_msg void OnZoomfaktorVerkleinern();
	afx_msg void OnZoomfaktor50();
	afx_msg void OnZoomfaktor75();
	afx_msg void OnZoomfaktor100();
	afx_msg void OnZoomfaktor125();
	afx_msg void OnZoomfaktor150();
	afx_msg void OnZoomfaktor175();
	afx_msg void OnZoomfaktor200();
	afx_msg void OnZoomfaktor250();
	afx_msg void OnZoomfaktor300();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // debug version in EasyCashView.cpp
inline CEasyCashDoc* CEasyCashView::GetDocument()
   { return (CEasyCashDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

#define POPUP_AENDERN				10000
#define POPUP_LOESCHEN				10001
#define POPUP_KOPIEREN				10002
#define POPUP_KOPIEREN_BELEGNUMMER	10003
#define POPUP_AFA_ABGANG			10004

#define POPUPFORMULAR_NEUES_FELD				10010
#define POPUPFORMULAR_FELDER_BEARBEITEN			10011
#define POPUPFORMULAR_KALKULATION_BEARBEITEN	10012
#define POPUPFORMULAR_FOLMULARDATEI_OEFFNEN		10013
#define POPUPFORMULAR_FELDER_ANZEIGEN			10014
#define POPUPFORMULAR_FELDWERT_KOPIEREN			10015
#define POPUPFORMULAR_NEUER_ABSCHNITT			10016

#define POPUP_SPLIT		11000
//					bis	11009

#define ID_VIEW_JOURNAL_FUER_KONTO_BASE 12000
//								bis max 12199

#define ID_FORMULAR_BASE 12200
//					 bis 19999 -- wird wohl reichen...

#define ID_BETRIEBEFILTER_BASE 20000
//					   bis max 29999 -- völlig overdone...

#define ID_BESTANDSKONTENFILTER_BASE 30000
//							 bis max 39999 -- völlig overdone...

#define HCHARS 80
#define VCHARS 68
#define PAGE_GAP 4

#endif // _EASYCASHVIEW_H_
