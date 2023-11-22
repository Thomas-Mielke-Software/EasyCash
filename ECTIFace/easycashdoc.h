#if !defined(AFX_EASYCASHDOC_H__55B7ED18_9D51_4B8C_9F6B_7DAA4BAAB943__INCLUDED_)
#define AFX_EASYCASHDOC_H__55B7ED18_9D51_4B8C_9F6B_7DAA4BAAB943__INCLUDED_
// EasyCashDoc.h : interface of the CEasyCashDoc class
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

#include "XMLite.h"

// Version für gespeicherte Dokumente
#define VERSION 13

#define GANZJAHRES_AFA 0
#define HALBJAHRES_AFA 1 
#define MONATSGENAUE_AFA 2
#define ENTSPRECHEND_EINSTELLUNGEN_AFA 3

#if !defined(AFX_EXT_CLASS)
#define AFX_EXT_CLASS __declspec(dllimport)
#endif // kann passieren...

class AFX_EXT_CLASS CMyMultiDocTemplate : public CMultiDocTemplate
{
public:
	CMyMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);
	BOOL GetDocString(CString& rString, enum DocStringIndex i) const;
	CDocTemplate::Confidence MatchDocType(const char* pszPathName, CDocument*& rpDocMatch);
};

class CEasyCashDoc;

class AFX_EXT_CLASS CBetrag : public CObject
{
public:
	int Betrag;	// brutto, in Cents
	int MWSt;	// in Prozent*1000
	// diese Klasse könnte um die Währung erweitert werden, vielleicht...

	long GetNetto();								// Gesamt-Nettobetrag
	BOOL SetMWSt(char *s);
	void GetMWSt(char *s);
	BOOL ConvertToEuro(CString wkz);
	BOOL ConvertFromEuro(CString wkz);
};

class AFX_EXT_CLASS CBuchung : public CBetrag
{
public:
	DECLARE_SERIAL(CBuchung)
	int AbschreibungNr;			// Achtung: bei der Abschreibungsprozedur kann sich noch was 
	int AbschreibungJahre;		//          ändern: Halbjahresregel und degressive Abschreibung
	CString Beschreibung;		// Der Beschreibungstext
	CTime Datum;				// Buchungsdatum
	CString Konto;				// virtuelles Konto in Reintext
	CBuchung *next;				// Zeiger auf die nächste Buchung in der Liste
	// dazugekommen ab v1.16 oder CDoc-VERSION 7 (siehe oben #define VERSION)
	CString Belegnummer;		// auf vielfachen Wunsch dazugekommen in Version 7 der Doc-Klasse
	int  AbschreibungRestwert;	// Restbuchwert des Anlagegegenstands
	BOOL AbschreibungDegressiv;	// TRUE wenn Abschreibungsmodus degressiv ist
	int  AbschreibungSatz;		// zur Zeit nur bei degressiver Abschreibung benötigt
	CString Erweiterung;		// member für benutzerdefinierte Daten der Erweiterungs-DLLs
		// ACHTUNG: dieses member müssen sich ggf. mehrere Erweiterungen teilen, deshalb dieses member nie
		// komplett löschen, sondern nur selektiv. Ich schlage folgendes Format für Daten vor, die hier 
		// gespeichert werden: DLL-Erweiterungsname1|Schlüsselname1=Wert1|Schlüsselname2=Wert2||DLL-Erweiterungsname2|...|...||
		// Beispiel: "ECTEOffenePosten|Kundennr=1234|Zahlung1=1000,00|Zahldatum1=12.12.2003||..."
		// wer ein paar schicke Funktionen zum handling schreibt: ich nehme die gern in die ECTIFace.dll auf...
	
	CBuchung();
	~CBuchung();
	virtual void Serialize(CArchive& ar);	
	CBuchung& operator=(const CBuchung& buchungSrc);
	long GetBuchungsjahrNetto(CEasyCashDoc *pDoc);	// Nettobetrag des aktuellen Buchungsjahrs entsprechend den Einstellungen fürs Dokument (für AfA!)
	long GetBuchungsjahrNetto(int angewandte_Abschreibungsgenauigkeit); // Nettobetrag des aktuellen Buchungsjahrs mit diskreter Abschreibungsgenauigkeit (für AfA!)
	int AbschreibungGenauigkeit;		// dazugekommen ab v1.37 oder CDoc Version 10
	CString Bestandskonto;				// dazugekommen ab v1.49 oder CDoc Version 11
	CString Betrieb;					// dazugekommen ab v1.49 oder CDoc Version 11

};

class AFX_EXT_CLASS CDauerbuchung : public CBetrag
{

public:
	DECLARE_SERIAL(CDauerbuchung)
	CString Beschreibung;
	int Buchungstyp;	// Buchungstyp: TRUE = Ausgaben, False = ...
#define BUCHUNGSTYP_EINNAHMEN	0
#define BUCHUNGSTYP_AUSGABEN	1
	int Intervall;		// monatlich, ... , ... ?
#define INTERVALL_MONAT		1
#define INTERVALL_QUARTAL	2
#define INTERVALL_HALBJAHR	3
#define INTERVALL_JAHR		4
#define INTERVALL_2MONATE	5
	CTime VonDatum;
	CTime BisDatum;
	int Buchungstag;
	CTime AktualisiertBisDatum;
	CString Konto;
	CDauerbuchung *next;
	CString Belegnummer;	// dazugekommen in Version 7 der Doc-Klasse, zukünftig soll es eine Maske enthalten, nach der Buchungsnummern in CBuchung generiert werden
	CString Erweiterung;	// siehe CBuchung

	CDauerbuchung();
	~CDauerbuchung();
	virtual void Serialize(CArchive& ar);
	CDauerbuchung& operator=(const CDauerbuchung& buchungSrc);
	CString Bestandskonto;				// dazugekommen ab v1.56 oder CDoc Version 12
	CString Betrieb;					// dazugekommen ab v1.56 oder CDoc Version 12
};

// Bem.: nach Änderungen in den Buchungsdaten empfiehlt es sich CEasyCashDoc::Sort() CEasyCashDoc::SetModifiedFlag() und soetwas wie 
// ::RedrawWindow(hWndParent, NULL, NULL, RDW_ERASE|RDW_FRAME|RDW_INTERNALPAINT|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN) aufzurufen

//#undef  AFX_DATA				// So wird auch das CObject-RuntimeClass member 
//#define AFX_DATA AFX_EXT_DATA	// in DECLARE_SERIAL mit im- bzw. exportiert
class AFX_EXT_CLASS CEasyCashDoc : public CDocument
{
	friend class CFormularCtrl;

protected: // create from serialization only
	CEasyCashDoc();

	DECLARE_SERIAL(CEasyCashDoc)

// Attributes
public:
	CBuchung *Einnahmen;
	CBuchung *Ausgaben;
	int	Buchungszaehler;
	CDauerbuchung *Dauerbuchungen;
	int nLaufendeBuchungsnummerFuerEinnahmen;
	int nLaufendeBuchungsnummerFuerAusgaben;
	int nJahr;
	CString csWaehrung;
	CString csUrspruenglicheWaehrung;
	int AbschreibungGenauigkeit;	// 0 = Jahres-AfA, 1 = Halbjahres-AfA, 2 = monatsgenaue AfA, 3 entsp. den aktuellen Einstellungen
	// neue Variablen im Konstruktor, Serialize und in OnFileJahreswechsel nachtragen!

// Operations
public:
	void Sort();
	void SortSpecial(CBuchung **bpp);
	void InkrementBuchungszaehler();
	int  EinnahmenSumme(int MonatsFilter, LPCTSTR KontoFilter = "");
	int  AusgabenSumme(int MonatsFilter, LPCTSTR KontoFilter = "");
	int  EinnahmenSummeNetto(int MonatsFilter, LPCTSTR KontoFilter = "");
	int  AusgabenSummeNetto(int MonatsFilter, LPCTSTR KontoFilter = "");
	void SavePublic();
	BOOL ConvertToEuro();
	static char *GetWaehrungskuerzel(int i);
	static double GetUmrechnungskurs(int i);
	BOOL BuchungIstEinnahme(CBuchung *pBuchung);
	BOOL BuchungIstAusgabe(CBuchung *pBuchung);

// weiter Attributes
public:
	// dazugekommen ab v1.16 oder CDoc-VERSION 7 (siehe oben #define VERSION)
	CString Erweiterung;		// member für benutzerdefinierte Daten der Erweiterungs-DLLs
		// ACHTUNG: dieses member müssen sich ggf. mehrere Erweiterungen teilen, deshalb dieses member nie
		// komplett löschen, sondern nur selektiv. Ich schlage folgendes Format für Daten vor, die hier 
		// gespeichert werden: DLL-Erweiterungsname1|Schlüsselname1=Wert1|Schlüsselname2=Wert2||DLL-Erweiterungsname2|...|...||
		// Beispiel: "ECTEOffenePosten|Kundennr1=1234|Kundenname1=Meier||..."
	int	Version;	// hiermit kann man prüfen, welche Verison das CDoc hat

	// dazugekommen ab v1.18 oder CDoc-VERSION 8
	int nLaufendeBuchungsnummerFuerBank;
	int nLaufendeBuchungsnummerFuerKasse;

// HIER WEITERE MEMBERS ANFÜGEN: public functions und variablen auch gern gemischt...

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasyCashDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void SetModifiedFlag(LPCTSTR lpszAktion, BOOL bModified = TRUE);
	virtual void SetModifiedFlag(BOOL bModified = TRUE);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEasyCashDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Helper:
private:
	XDoc *GetFormular(CString &csFormularname);
	CStringArray m_csEinnahmenKonten;
	CStringArray m_csEinnahmenFeldzuweisungen;
	CStringArray m_csAusgabenKonten;
	CStringArray m_csAusgabenFeldzuweisungen;
	long m_laFeldWerte[10000];
	BOOL m_bFeldWertGueltig[10000];
	CString m_csBetriebFuerFormular;
	// für Formular-ActiveX-Control:
	CStringArray m_csaFormulare;					// enthält die Dateinamen der verfügbaren Formulare
	CStringArray m_csaFormularfeldwerte;			// Cache-Speicher für die Formularwerte
	CStringArray m_csaFormularfeldbeschreibungen;	// ... und Feldbeschreibungen

// weitere Operations
public:
	int GetFormularwertIDByIndex(CString &csFormularname, int nIndex);	// passend zu GetFormularwertByIndex
	void ReadKontenCache();	// benötigt für GetFormularwertXXX Funktionen
	CString GetFormularwertByID(CString &csFormularname, int nID, LPCSTR sFilter = NULL);	// dazugekommen ab v1.30 oder CDoc-VERSION 9
	CString GetFormularwertByIndex(CString &csFormularname, int nIndex, LPCSTR sFilter = NULL);	// dazugekommen ab v1.30 oder CDoc-VERSION 9
	CString GetFormularwertByID(XDoc *pFormular, int nID, LPCSTR sFilter = NULL);	// dazugekommen ab v1.30 oder CDoc-VERSION 9
	CString GetFormularwertByIndex(XDoc *pFormular, int nIndex, LPCSTR sFilter = NULL);	// dazugekommen ab v1.30 oder CDoc-VERSION 9
	void SetFormularVoranmeldungszeitraum(int nZeitraum); // dazugekommen ab v1.35
	BOOL FormularVerlangtVoranmeldungszeitraum(XDoc *pFormular); // dazugekommen ab v1.35
	int m_nZeitraum;
	void BerechneFormularfeldwerte(CString &Formular, CStringArray &csaFormularfeldwerte, int *pFormularfeldIDs, CStringArray *pcsaFormularfeldbeschreibungen, LPCSTR sFilter = NULL); // IDs nur von 1 - 9999, also int[10000], NULL -> wird dann ignoriert
	void SortQuick(void **bparray, int left, int right);
	void SortSingle(CBuchung **bpp_base, CBuchung **bpp_einzufuegen);

	CStringArray m_csaFeldStatustext;	// Statustexte für Formularfelder im freien Zugriff

// Generated message map functions
protected:
	//{{AFX_MSG(CEasyCashDoc)
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// weiter Attribute für Backup-Nachfrage VERSION 13:
public:
	BOOL Check4Backup();
	int nNachfrageIntervall;	// in Tagen
	CTime ctNachfrageTermin;
};
//#undef  AFX_DATA			
//#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////

// nützliche Hilfsfunktionen

// Wandelt einen Betrag von String in int (= Pfennige/Cents) um
extern "C" AFX_EXT_CLASS int currency_to_int(char *s);

// Wandelt einen Betrag von int in String um:
// i = zu konvertierender Wert
// n = Feldweite incl. Komma und 2 Nachkommastellen
// s = Ergebnisstring
extern "C" AFX_EXT_CLASS void int_to_currency(int i, int n, char *s);
extern "C" AFX_EXT_CLASS void int_to_currency_tausenderpunkt(int i, int n, char *s);

// gibt den Pfad zur INI-Datei bzw. zum Programmverzeichnis zurück. Mit WritePrivateProfileString & Co.
// kann man in der Ini-Datei Einstellungen eintragen. Bitte als Sektionsnamen den Namen der 
// Erweiterungs-DLL angeben, damit es kein Durcheinander gibt. Mit zukünftiger Mandantenfähigkeit
// kann das Verzeichnis der Ini-Datei vom Programmverzeichnis abweichen. Das Programmverzeichnis
// wird ohne abschließenden '\' geliefert.
// buffer = pointer auf ein char[300] (sollte reichen)
// size   = sizeof(buffer)
// return = TRUE wenn kein Fehler auftrat
extern "C" AFX_EXT_CLASS BOOL GetProgrammverzeichnis(char *buffer3, int size);
extern "C" AFX_EXT_CLASS void SetIniFileName(char *buffer3);
extern "C" AFX_EXT_CLASS BOOL GetIniFileName(char *buffer, int size);
extern "C" AFX_EXT_CLASS void SetMandant(int n);
extern "C" AFX_EXT_CLASS int GetMandant();

//--- ab hier Hilfsfunktionen für den Zugriff auf die Plugin-Erweiterungsdaten in der Dokument-Klasse ---
// ACHTUNG: Das Zeichen '|' darf in Key- bzw. Erweiterungsnamen und in Werten nicht verwendet werden
//          und '=' nicht in Key-Namen. In Zukunft sollte das Zeichen intern 'escaped' werden.

// Return: Pointer auf den gewünschten Key-Wert der Ereiterung oder "" wenn nicht gefunden wurde
//		   Der Key-Wert ist nicht Null-Terminiert, sondern '|'-Terminiert!!!
// Die Aufrufende Funktion sollte nach der Verarbeitung einen ReleaseBuffer auf m_pDoc->Erweiterung machen
extern "C" AFX_EXT_CLASS char *GetErweiterungKey(CString &csSpeicher, LPCTSTR sErweiterung, LPCTSTR sKey);
extern "C" AFX_EXT_CLASS CString *GetErweiterungKeyCS(CString &csSpeicher, LPCTSTR sErweiterung, LPCTSTR sKey);
extern "C" AFX_EXT_CLASS void SetErweiterungKey(CString &csSpeicher, LPCTSTR sErweiterung, LPCTSTR sKey, LPCTSTR sValue);	// einen Wert im Erweiterungs-Bereich der Dokumentklasse schreiben

// holt alle Dateipfade mit Dateien der Endung .ecf im Programmverzeichnis in ein String Array
extern "C" AFX_EXT_CLASS void LadeECFormulare(CStringArray &csa);

// Holt den Formularnamen aus einem Array und filtert nach Formularart
extern "C" AFX_EXT_CLASS void HoleFormularnamenAusCSA(long Index, LPCTSTR Filter, CStringArray &csa, CString *Result);

// berechnet die Ini-Sektion aus dem angeforderten-Einstellungs-Key ('f' voranstellen für Finanzamts-Daten)
extern "C" AFX_EXT_CLASS LPCSTR IniSektion(LPCSTR id);
extern "C" AFX_EXT_CLASS char *HoleKontoFuerFeld(char ea, LPCSTR eurech_feld, LPCSTR uva_feld = NULL);

////////////////////////////////////////////
// ExtensionDLLTable

class AFX_EXT_CLASS ExtensionDLLsClass
{
public:
	CString				name;
	HINSTANCE			hInst;
	CString				menu_item_string;
	int					menu_item_id;
	ExtensionDLLsClass *next;
};

// Hilfsfunktion für plugin-hooks
extern "C" AFX_EXT_CLASS void CLoadExtensionDLLs(char *version_string);
extern "C" AFX_EXT_CLASS void CFreeExtensionDLLs();

extern "C" AFX_EXT_CLASS void CIterateExtensionDLLs(LPCSTR function_name, void *p);
extern AFX_EXT_CLASS ExtensionDLLsClass *ExtensionDLLs;

extern AFX_EXT_CLASS BOOL shareware;

extern double kurse[];
extern char *waehrungskuerzel[];

// user-defined messages
#define WM_SETSTATUS WM_USER+2222

#endif // !defined(AFX_EASYCASHDOC_H__55B7ED18_9D51_4B8C_9F6B_7DAA4BAAB943__INCLUDED_)