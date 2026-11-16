// UStAbgleich.h : Abgleich der Kontenzuweisungen zwischen USt-Voranmeldung und Umsatzsteuererklärung
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

#if !defined(USTABGLEICH_H_INCLUDED)
#define USTABGLEICH_H_INCLUDED

#include <afxtempl.h>

// Ein E/Ü-Konto trägt je Formularname eine Feldzuweisung (Einstellungen -> E/Ü-Konten, ini-Sektionen
// [EinnahmenFeldzuweisungen]/[AusgabenFeldzuweisungen], Erweiterung "ECT", Key = Formularname, Wert = Feld-ID).
// Voranmeldung und Erklärung haben verschiedene Formularnamen und verschiedene Feld-IDs, ihre Zuweisungen
// können also auseinanderlaufen. Welches Feld der Erklärung welchem Feld der Voranmeldung entspricht, steht
// als Attribut uvafeld="<Feld-ID der Voranmeldung>" an den Feldern des Erklärungsformulars. Mehrere Felder
// der Erklärung dürfen auf dasselbe Feld der Voranmeldung zeigen (die Erklärung gliedert feiner).
// Formulare ohne dieses Attribut nehmen am Abgleich nicht teil.

#define USTABGLEICH_FORMULAR_UVA	"Umsatzsteuer-Voranmeldung"
#define USTABGLEICH_FORMULAR_UST	"Umsatzsteuererklärung"

// Vermerk "dieses Paar ist so gewollt" in der Feldzuweisung eines Kontos: Key in der Erweiterung "ECT",
// Wert "<Feld Voranmeldung>/<Feld Erklärung>". Er gilt nur für genau dieses Paar und verfällt damit von
// selbst, sobald eine der beiden Zuweisungen geändert wird. Der Key muss in "ECT" liegen: v4 schreibt beim
// Speichern nur diese Erweiterung zurück.
#define USTABGLEICH_KEY_BELASSEN	"USt-Abgleich"

class CUStAbgleich
{
public:
	enum Befund
	{
		STIMMIG = 0,		// nichts zu tun
		UST_FEHLT,			// Voranmeldung zugewiesen, Erklärung nicht; genau ein passendes Feld der Erklärung
		UST_MEHRDEUTIG,		// wie UST_FEHLT, aber mehrere Felder der Erklärung kommen in Frage
		UVA_FEHLT,			// Erklärung zugewiesen, Voranmeldung nicht
		WIDERSPRUCH			// beide zugewiesen, passen aber nicht zusammen
	};

	struct Feld
	{
		int nID;
		BOOL bEinnahmen;	// Feldtyp "Einnahmen" (sonst "Ausgaben"); andere Typen werden nicht geladen
		BOOL bVeraltet;
		int nUvaFeld;		// nur Erklärung: entsprechendes Feld der Voranmeldung, 0 = keines
		CString csName;
	};

	// Ergebnis von NachZuweisung()
	struct Mitfuehrung
	{
		enum Art
		{
			NICHTS,					// nichts zu tun
			PARTNER_GESETZT,		// Gegenstück wurde still mitgeführt (nPartnerNeu)
			PARTNER_WAEHLEN,		// mehrere Felder kommen in Frage (kandidaten), der Benutzer muss wählen
			PARTNER_ENTFERNEN,		// Gegenstück hat seine Entsprechung verloren: entfernen? (Rückfrage)
			WIDERSPRUCH				// Gegenstück war schon vorher abweichend belegt: angleichen? (Rückfrage)
		} art;
		BOOL bPartnerIstUSt;		// TRUE: das Gegenstück ist die Zuweisung für die Erklärung
		int nGeaendertNeu;			// neue Feld-ID im geänderten Formular
		int nPartnerAlt;			// bisherige Feld-ID des Gegenstücks
		int nPartnerNeu;			// PARTNER_GESETZT: gesetzte Feld-ID; WIDERSPRUCH: eindeutiger Vorschlag oder 0
		CUIntArray kandidaten;		// PARTNER_WAEHLEN und WIDERSPRUCH ohne eindeutigen Vorschlag
	};

	CUStAbgleich();

	// Sucht unter den installierten Formularen die Erklärung des Jahres (ersatzweise die neueste mit
	// uvafeld-Attributen) und eine Voranmeldung dazu. Das Ergebnis wird gemerkt, bis ein anderes Jahr verlangt
	// oder Verwerfen() aufgerufen wird. FALSE = kein Abgleich möglich.
	BOOL Lade(int nJahr);
	BOOL LadeAusDateien(LPCTSTR ustFormulardatei, LPCTSTR uvaFormulardatei);
	void Verwerfen();
	BOOL IstAktiv() const { return m_bAktiv; }
	const CString &UStFormulardatei() const { return m_csUStDatei; }

	// --- Zugriff auf die Feldzuweisung eines Kontos
	static int  LiesZuweisung(const CString &csFeldzuweisung, LPCTSTR formularname);
	static void SchreibeZuweisung(CString &csFeldzuweisung, LPCTSTR formularname, int nFeldID);
	static BOOL IstBelassen(const CString &csFeldzuweisung, int nUvaFeld, int nUStFeld);
	static void SetzeBelassen(CString &csFeldzuweisung, BOOL bBelassen);	// vermerkt bzw. löscht das aktuelle Paar

	// --- Korrespondenz
	const Feld *UvaFeld(int nID) const;
	const Feld *UStFeld(int nID) const;
	int  UvaFeldZu(int nUStFeld) const;											// 0 = keine Entsprechung
	void UStKandidaten(int nUvaFeld, BOOL bEinnahmen, CUIntArray &kandidaten) const;
	BOOL UvaFeldZuweisbar(int nUvaFeld, BOOL bEinnahmen) const;
	CString FeldText(BOOL bUSt, int nFeldID) const;								// "Feld 741: <Name>", "" bei 0

	// --- Prüfung
	Befund PruefePaar(int nUvaFeld, int nUStFeld, BOOL bEinnahmen) const;		// ohne Rücksicht auf den Vermerk
	Befund Pruefe(const CString &csFeldzuweisung, BOOL bEinnahmen) const;		// mit Vermerk "so gewollt"

	// Nach einer geänderten Zuweisung für eines der beiden Formulare das Gegenstück mitführen.
	// bUStGeaendert: geändert wurde die Zuweisung für die Erklärung (sonst die für die Voranmeldung);
	// nAlt: Feld-ID vor der Änderung. Eindeutiges wird sofort in csFeldzuweisung eingetragen, alles andere
	// meldet m.art für eine Rückfrage.
	void NachZuweisung(CString &csFeldzuweisung, BOOL bEinnahmen, BOOL bUStGeaendert, int nAlt, Mitfuehrung &m) const;

private:
	static const Feld *Finde(const CArray<Feld, Feld&> &felder, int nID);
	static BOOL LadeFelder(LPCTSTR formulardatei, CArray<Feld, Feld&> &felder, BOOL bUSt);

	BOOL m_bGeladen;
	BOOL m_bAktiv;
	int m_nJahr;
	CString m_csUStDatei;
	CString m_csUvaDatei;
	CArray<Feld, Feld&> m_uva;
	CArray<Feld, Feld&> m_ust;
};

// die eine, gemerkte Instanz für Ansicht und Einstellungen
CUStAbgleich &GetUStAbgleich();

// erste vierstellige Jahreszahl (19xx/20xx) im Anzeigenamen eines Formulars, 0 = keine
int UStAbgleichJahrAusAnzeigename(LPCTSTR anzeigename);

// Attribute name und anzeigename einer Formulardatei, gelesen aus dem Dateianfang (ohne XML-Parser)
BOOL UStAbgleichFormularkopf(LPCTSTR formulardatei, CString &csName, CString &csAnzeigename);

#endif // !defined(USTABGLEICH_H_INCLUDED)
