// UStAbgleichDlg.h : Bedienoberfläche zum Abgleich der Kontenzuweisungen von USt-Voranmeldung und Umsatzsteuererklärung
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2026  Thomas Mielke
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

#if !defined(USTABGLEICHDLG_H_INCLUDED)
#define USTABGLEICHDLG_H_INCLUDED

#include "UStAbgleich.h"

class CEinstellungen1;
class CEasyCashDoc;

// Feld über ein Kontextmenü an der Mausposition wählen lassen; kopfzeile steht als Erläuterung darüber.
// Rückgabe: Feld-ID oder 0, wenn nichts gewählt wurde.
int UStAbgleichWaehleFeld(CWnd *pWnd, const CUStAbgleich &abgleich, BOOL bUSt, const CUIntArray &kandidaten, LPCTSTR kopfzeile);

// Für Einstellungen -> E/Ü-Konten: nach einer geänderten Zuweisung das Gegenstück im anderen Formular
// mitführen. Eindeutiges geschieht still, sonst wird der Benutzer gefragt.
// formularname: Name des Formulars, dessen Zuweisung geändert wurde; nAlt: Feld-ID vor der Änderung.
void UStAbgleichNachZuweisung(CWnd *pWnd, CString &csFeldzuweisung, BOOL bEinnahmen, LPCTSTR konto, LPCTSTR formularname, int nJahr, int nAlt);

// Erläuterung zu einem Befund, z. B. als Tooltip. Leer bei STIMMIG.
CString UStAbgleichBefundText(const CUStAbgleich &abgleich, CUStAbgleich::Befund befund, int nUvaFeld, int nUStFeld, BOOL bEinnahmen);

// Zählt die Konten, deren Zuweisungen nicht zusammenpassen.
// nFuerFormular: 0 = was die Voranmeldung verfälscht, 1 = was die Erklärung verfälscht, -1 = alles.
// pnMitBuchungen: wie viele dieser Konten im Buchungsjahr des Dokuments Buchungen haben.
int UStAbgleichZaehleBefunde(const CUStAbgleich &abgleich, CEinstellungen1 *pEinstellungen1, CEasyCashDoc *pDoc, int nFuerFormular, int *pnMitBuchungen);

/////////////////////////////////////////////////////////////////////////////
// CUStAbgleichListe: Listenansicht, die zu jeder abgeschnittenen Zelle den vollen Text als Tooltip zeigt.
// (Von Haus aus kann das Listen-Control das nur für die erste Spalte.)

class CUStAbgleichListe : public CListCtrl
{
protected:
	virtual void PreSubclassWindow();
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	afx_msg BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CString m_csTooltip;		// der Tooltip-Text muss über die Benachrichtigung hinaus bestehen
	CStringW m_cswTooltip;
};

/////////////////////////////////////////////////////////////////////////////
// CUStAbgleichDlg: Liste aller Konten mit unstimmigen Zuweisungen samt Vorschlag

class CUStAbgleichDlg : public CDialog
{
public:
	CUStAbgleichDlg(CEinstellungen1 *pEinstellungen1, CEasyCashDoc *pDoc, int nJahr, CWnd* pParent = NULL);

	enum { IDD = IDD_USTABGLEICH };
	CUStAbgleichListe m_liste;

	BOOL m_bGeaendert;	// nach DoModal(): es wurden Zuweisungen geändert, der Aufrufer muss speichern

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAendern();
	afx_msg void OnDblclkListe(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListe(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

	enum Aktion { KEINE = 0, UST_SETZEN, UVA_SETZEN, BELASSEN };

	struct Zeile
	{
		BOOL bEinnahmen;
		int nKonto;			// Index in EinnahmenRechnungsposten[] bzw. AusgabenRechnungsposten[]
		int nUvaFeld;
		int nUStFeld;
		CUStAbgleich::Befund befund;
		int nBuchungen;		// Buchungen des Kontos im Buchungsjahr
		Aktion aktion;		// der Vorschlag
		int nZiel;			// UST_SETZEN/UVA_SETZEN: einzutragende Feld-ID, 0 = Zuweisung entfernen
	};

	CString &Feldzuweisung(const Zeile &zeile);
	CString Kontoname(const Zeile &zeile);
	CString VorschlagText(const Zeile &zeile);
	void ZeileAnzeigen(int nItem);
	void VorschlagAendern(int nItem);
	void SpaltenbreitenAnpassen();
	int m_nListenRand;	// Breite des Dialogs abzüglich der Breite der Liste, -1 = noch nicht gemessen

	CEinstellungen1 *m_pEinstellungen1;
	CEasyCashDoc *m_pDoc;
	int m_nJahr;
	CUStAbgleich &m_abgleich;
	CArray<Zeile, Zeile&> m_zeilen;
	BOOL m_bListeWirdGefuellt;
};

#endif // !defined(USTABGLEICHDLG_H_INCLUDED)
