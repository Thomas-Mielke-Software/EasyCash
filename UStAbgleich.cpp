// UStAbgleich.cpp : Abgleich der Kontenzuweisungen zwischen USt-Voranmeldung und Umsatzsteuererklärung
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

// Diese Datei enthält nur die Logik (keine Dialoge), damit sie sich ohne das Hauptprogramm testen lässt.
// Die Bedienoberfläche dazu steht in UStAbgleichDlg.cpp.

#include "stdafx.h"
#include "ECTIFace\EasyCashDoc.h"
#include "ECTIFace\XMLite.h"
#include "UStAbgleich.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CUStAbgleich &GetUStAbgleich()
{
	static CUStAbgleich abgleich;
	return abgleich;
}

int UStAbgleichJahrAusAnzeigename(LPCTSTR anzeigename)
{
	if (!anzeigename) return 0;
	for (const unsigned char *s = (const unsigned char *)anzeigename; *s; s++)
		if (((s[0] == '1' && s[1] == '9') || (s[0] == '2' && s[1] == '0')) && isdigit(s[2]) && isdigit(s[3]) && !isdigit(s[4]))
			return (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0');
	return 0;
}

// Wert eines Attributs aus dem Anfang einer Formulardatei, ohne sie als XML zu laden
static CString KopfAttribut(const CString &csKopf, LPCTSTR attribut)
{
	CString csSuch = (CString)" " + attribut + "=\"";	// das Leerzeichen trennt name= von anzeigename=
	int nStart = csKopf.Find(csSuch);
	if (nStart < 0) return "";
	nStart += csSuch.GetLength();
	int nEnde = csKopf.Find('"', nStart);
	if (nEnde < 0) return "";
	return csKopf.Mid(nStart, nEnde - nStart);
}

// Name und Anzeigename eines Formulars. Es gibt einige hundert installierte Formulare; sie alle als XML zu
// laden, nur um die zwei passenden zu finden, wäre zu langsam. Deshalb wird nur der Dateianfang gelesen.
BOOL UStAbgleichFormularkopf(LPCTSTR formulardatei, CString &csName, CString &csAnzeigename)
{
	csName.Empty();
	csAnzeigename.Empty();
	CFile f;
	if (!f.Open(formulardatei, CFile::modeRead | CFile::shareDenyNone)) return FALSE;
	char buffer[801];
	UINT n = 0;
	TRY
	{
		n = f.Read(buffer, sizeof(buffer) - 1);
	}
	CATCH(CFileException, e)
	{
		n = 0;
	}
	END_CATCH
	f.Close();
	buffer[n] = '\0';

	CString csKopf(buffer);
	int nFormular = csKopf.Find("<formular");
	if (nFormular < 0) return FALSE;
	csKopf = csKopf.Mid(nFormular);
	int nEnde = csKopf.Find('>');
	if (nEnde >= 0) csKopf = csKopf.Left(nEnde);
	csName = KopfAttribut(csKopf, "name");
	csAnzeigename = KopfAttribut(csKopf, "anzeigename");
	return !csName.IsEmpty();
}

/////////////////////////////////////////////////////////////////////////////
// CUStAbgleich

CUStAbgleich::CUStAbgleich()
{
	m_bGeladen = FALSE;
	m_bAktiv = FALSE;
	m_nJahr = 0;
}

void CUStAbgleich::Verwerfen()
{
	m_bGeladen = FALSE;
	m_bAktiv = FALSE;
	m_nJahr = 0;
	m_csUStDatei.Empty();
	m_csUvaDatei.Empty();
	m_uva.RemoveAll();
	m_ust.RemoveAll();
}

BOOL CUStAbgleich::LadeFelder(LPCTSTR formulardatei, CArray<Feld, Feld&> &felder, BOOL bUSt)
{
	felder.RemoveAll();
	XDoc xmldoc;
	xmldoc.LoadFile(formulardatei);
	LPXNode xml = xmldoc.GetRoot();
	if (!xml) return FALSE;
	LPXNode knoten = xml->Find("felder");
	if (!knoten) return FALSE;

	for (int i = 0; i < knoten->GetChildCount(); i++)
	{
		LPXNode child = knoten->GetChild(i);
		if (!child) continue;
		LPCTSTR typ = child->GetAttrValue("typ");
		if (!typ) continue;
		Feld feld;
		if (!stricmp(typ, "Einnahmen"))
			feld.bEinnahmen = TRUE;
		else if (!stricmp(typ, "Ausgaben"))
			feld.bEinnahmen = FALSE;
		else
			continue;	// nur Felder, denen Konten zugewiesen werden können
		LPCTSTR id = child->GetAttrValue("id");
		feld.nID = id ? atoi(id) : 0;
		if (!feld.nID) continue;
		LPCTSTR veraltet = child->GetAttrValue("veraltet");
		feld.bVeraltet = veraltet && !stricmp(veraltet, "ja");
		LPCTSTR uvafeld = bUSt ? child->GetAttrValue("uvafeld") : NULL;
		feld.nUvaFeld = uvafeld ? atoi(uvafeld) : 0;
		feld.csName = child->GetChildValue("name");
		felder.Add(feld);
	}
	return felder.GetSize() > 0;
}

BOOL CUStAbgleich::LadeAusDateien(LPCTSTR ustFormulardatei, LPCTSTR uvaFormulardatei)
{
	int nJahr = m_nJahr;
	Verwerfen();
	m_bGeladen = TRUE;
	m_nJahr = nJahr;

	if (!LadeFelder(ustFormulardatei, m_ust, TRUE)) return FALSE;
	BOOL bKorrespondenz = FALSE;
	for (int i = 0; i < m_ust.GetSize() && !bKorrespondenz; i++)
		if (m_ust[i].nUvaFeld) bKorrespondenz = TRUE;
	if (!bKorrespondenz) return FALSE;	// Formular ohne uvafeld-Attribute: kein Abgleich
	if (!LadeFelder(uvaFormulardatei, m_uva, FALSE)) return FALSE;

	m_csUStDatei = ustFormulardatei;
	m_csUvaDatei = uvaFormulardatei;
	m_bAktiv = TRUE;
	return TRUE;
}

BOOL CUStAbgleich::Lade(int nJahr)
{
	if (m_bGeladen && m_nJahr == nJahr) return m_bAktiv;

	Verwerfen();
	m_bGeladen = TRUE;
	m_nJahr = nJahr;

	// Kandidaten unter den installierten Formularen sammeln
	CStringArray csaFormulare, csaUSt, csaUva;
	CUIntArray jahreUSt, jahreUva;
	LadeECFormulare(csaFormulare);
	int i;
	for (i = 0; i < csaFormulare.GetSize(); i++)
	{
		CString csName, csAnzeigename;
		if (!UStAbgleichFormularkopf(csaFormulare[i], csName, csAnzeigename)) continue;
		if (csName == USTABGLEICH_FORMULAR_UST)
		{
			csaUSt.Add(csaFormulare[i]);
			jahreUSt.Add(UStAbgleichJahrAusAnzeigename(csAnzeigename));
		}
		else if (csName == USTABGLEICH_FORMULAR_UVA && csAnzeigename.Left((int)strlen(USTABGLEICH_FORMULAR_UVA) + 1) == USTABGLEICH_FORMULAR_UVA " ")
		{	// Die Erklärungsformulare bis 2024 heißen intern ebenfalls "Umsatzsteuer-Voranmeldung"; sie scheiden hier aus.
			csaUva.Add(csaFormulare[i]);
			jahreUva.Add(UStAbgleichJahrAusAnzeigename(csAnzeigename));
		}
	}

	// Erklärung: die des verlangten Jahres, sonst die neueste, die uvafeld-Attribute hat
	int nJahrUSt = 0;
	CArray<Feld, Feld&> felder;
	while (csaUSt.GetSize() > 0)
	{
		int nBeste = 0;
		for (i = 1; i < csaUSt.GetSize(); i++)
			if ((int)jahreUSt[nBeste] != nJahr && ((int)jahreUSt[i] == nJahr || jahreUSt[i] > jahreUSt[nBeste]))
				nBeste = i;
		BOOL bKorrespondenz = FALSE;
		if (LadeFelder(csaUSt[nBeste], felder, TRUE))
			for (i = 0; i < felder.GetSize() && !bKorrespondenz; i++)
				if (felder[i].nUvaFeld) bKorrespondenz = TRUE;
		if (bKorrespondenz)
		{
			m_csUStDatei = csaUSt[nBeste];
			nJahrUSt = jahreUSt[nBeste];
			break;
		}
		csaUSt.RemoveAt(nBeste);
		jahreUSt.RemoveAt(nBeste);
	}
	if (m_csUStDatei.IsEmpty() || csaUva.GetSize() == 0) return FALSE;

	// Voranmeldung: eine aus dem Jahr der Erklärung, sonst die neueste (der Zeitraum spielt keine Rolle)
	int nBeste = 0;
	for (i = 1; i < csaUva.GetSize(); i++)
		if ((int)jahreUva[nBeste] != nJahrUSt && ((int)jahreUva[i] == nJahrUSt || jahreUva[i] > jahreUva[nBeste]))
			nBeste = i;

	CString csUSt = m_csUStDatei, csUva = csaUva[nBeste];
	return LadeAusDateien(csUSt, csUva);
}

/////////////////////////////////////////////////////////////////////////////
// Zugriff auf die Feldzuweisung eines Kontos

// Wert eines Keys der Erweiterung "ECT" als Zeichenkette (GetErweiterungKey liefert ihn '|'-terminiert)
static CString LiesWert(const CString &csFeldzuweisung, LPCTSTR key)
{
	CString csKopie = csFeldzuweisung;
	CString csWert;
	for (const char *cp = GetErweiterungKey(csKopie, "ECT", key); *cp && *cp != '|'; cp++)
		csWert += *cp;
	return csWert;
}

int CUStAbgleich::LiesZuweisung(const CString &csFeldzuweisung, LPCTSTR formularname)
{
	return atoi(LiesWert(csFeldzuweisung, formularname));
}

void CUStAbgleich::SchreibeZuweisung(CString &csFeldzuweisung, LPCTSTR formularname, int nFeldID)
{
	CString csFeldID;
	csFeldID.Format("%d", nFeldID);
	SetErweiterungKey(csFeldzuweisung, "ECT", formularname, csFeldID);
}

BOOL CUStAbgleich::IstBelassen(const CString &csFeldzuweisung, int nUvaFeld, int nUStFeld)
{
	CString csPaar;
	csPaar.Format("%d/%d", nUvaFeld, nUStFeld);
	return LiesWert(csFeldzuweisung, USTABGLEICH_KEY_BELASSEN) == csPaar;
}

void CUStAbgleich::SetzeBelassen(CString &csFeldzuweisung, BOOL bBelassen)
{
	if (bBelassen)
	{
		CString csPaar;
		csPaar.Format("%d/%d", LiesZuweisung(csFeldzuweisung, USTABGLEICH_FORMULAR_UVA), LiesZuweisung(csFeldzuweisung, USTABGLEICH_FORMULAR_UST));
		SetErweiterungKey(csFeldzuweisung, "ECT", USTABGLEICH_KEY_BELASSEN, csPaar);
	}
	else if (!LiesWert(csFeldzuweisung, USTABGLEICH_KEY_BELASSEN).IsEmpty())	// keinen leeren Key anlegen, wo keiner war
		SetErweiterungKey(csFeldzuweisung, "ECT", USTABGLEICH_KEY_BELASSEN, "");
}

/////////////////////////////////////////////////////////////////////////////
// Korrespondenz

const CUStAbgleich::Feld *CUStAbgleich::Finde(const CArray<Feld, Feld&> &felder, int nID)
{
	if (!nID) return NULL;
	for (int i = 0; i < felder.GetSize(); i++)
		if (felder[i].nID == nID)
			return &felder[i];
	return NULL;
}

const CUStAbgleich::Feld *CUStAbgleich::UvaFeld(int nID) const
{
	return Finde(m_uva, nID);
}

const CUStAbgleich::Feld *CUStAbgleich::UStFeld(int nID) const
{
	return Finde(m_ust, nID);
}

int CUStAbgleich::UvaFeldZu(int nUStFeld) const
{
	const Feld *pFeld = UStFeld(nUStFeld);
	return pFeld ? pFeld->nUvaFeld : 0;
}

void CUStAbgleich::UStKandidaten(int nUvaFeld, BOOL bEinnahmen, CUIntArray &kandidaten) const
{
	kandidaten.RemoveAll();
	if (!nUvaFeld) return;
	for (int i = 0; i < m_ust.GetSize(); i++)
		if (m_ust[i].nUvaFeld == nUvaFeld && !m_ust[i].bEinnahmen == !bEinnahmen && !m_ust[i].bVeraltet)
			kandidaten.Add(m_ust[i].nID);
}

BOOL CUStAbgleich::UvaFeldZuweisbar(int nUvaFeld, BOOL bEinnahmen) const
{
	const Feld *pFeld = UvaFeld(nUvaFeld);
	return pFeld && !pFeld->bEinnahmen == !bEinnahmen && !pFeld->bVeraltet;
}

CString CUStAbgleich::FeldText(BOOL bUSt, int nFeldID) const
{
	CString csText;
	if (!nFeldID) return csText;
	const Feld *pFeld = bUSt ? UStFeld(nFeldID) : UvaFeld(nFeldID);
	if (pFeld)
		csText.Format("Feld %d: %s", nFeldID, (LPCTSTR)pFeld->csName);
	else
		csText.Format("Feld %d (im Formular nicht vorhanden)", nFeldID);
	return csText;
}

/////////////////////////////////////////////////////////////////////////////
// Prüfung

CUStAbgleich::Befund CUStAbgleich::PruefePaar(int nUvaFeld, int nUStFeld, BOOL bEinnahmen) const
{
	if (!m_bAktiv) return STIMMIG;

	// Zuweisungen auf Felder, die es im Formular nicht (mehr) gibt, zählen als nicht zugewiesen
	if (!UvaFeld(nUvaFeld)) nUvaFeld = 0;
	if (!UStFeld(nUStFeld)) nUStFeld = 0;
	if (!nUvaFeld && !nUStFeld) return STIMMIG;

	int nUvaZurErklaerung = UvaFeldZu(nUStFeld);	// 0: das Feld gibt es nur in der Erklärung
	if (!nUvaFeld)
		return nUvaZurErklaerung && UvaFeldZuweisbar(nUvaZurErklaerung, bEinnahmen) ? UVA_FEHLT : STIMMIG;

	CUIntArray kandidaten;
	UStKandidaten(nUvaFeld, bEinnahmen, kandidaten);	// leer: das Feld gibt es nur in der Voranmeldung
	if (!nUStFeld)
		return kandidaten.GetSize() == 0 ? STIMMIG : (kandidaten.GetSize() == 1 ? UST_FEHLT : UST_MEHRDEUTIG);

	if (nUvaZurErklaerung == nUvaFeld) return STIMMIG;
	if (!nUvaZurErklaerung && kandidaten.GetSize() == 0) return STIMMIG;	// beide Felder ohne Gegenstück im anderen Formular
	return WIDERSPRUCH;
}

CUStAbgleich::Befund CUStAbgleich::Pruefe(const CString &csFeldzuweisung, BOOL bEinnahmen) const
{
	if (!m_bAktiv || csFeldzuweisung.IsEmpty()) return STIMMIG;
	int nUvaFeld = LiesZuweisung(csFeldzuweisung, USTABGLEICH_FORMULAR_UVA);
	int nUStFeld = LiesZuweisung(csFeldzuweisung, USTABGLEICH_FORMULAR_UST);
	Befund befund = PruefePaar(nUvaFeld, nUStFeld, bEinnahmen);
	if (befund != STIMMIG && IstBelassen(csFeldzuweisung, nUvaFeld, nUStFeld)) return STIMMIG;
	return befund;
}

void CUStAbgleich::NachZuweisung(CString &csFeldzuweisung, BOOL bEinnahmen, BOOL bUStGeaendert, int nAlt, Mitfuehrung &m) const
{
	int nUvaFeld = LiesZuweisung(csFeldzuweisung, USTABGLEICH_FORMULAR_UVA);
	int nUStFeld = LiesZuweisung(csFeldzuweisung, USTABGLEICH_FORMULAR_UST);
	int nNeu = bUStGeaendert ? nUStFeld : nUvaFeld;
	int nPartner = bUStGeaendert ? nUvaFeld : nUStFeld;
	LPCTSTR partnerFormular = bUStGeaendert ? USTABGLEICH_FORMULAR_UVA : USTABGLEICH_FORMULAR_UST;

	m.art = Mitfuehrung::NICHTS;
	m.bPartnerIstUSt = !bUStGeaendert;
	m.nGeaendertNeu = nNeu;
	m.nPartnerAlt = nPartner;
	m.nPartnerNeu = nPartner;
	m.kandidaten.RemoveAll();
	if (!m_bAktiv || nNeu == nAlt) return;

	SetzeBelassen(csFeldzuweisung, FALSE);	// der Vermerk galt dem bisherigen Paar

	// Was gehört im anderen Formular zur neuen Zuweisung?
	int nZiel = 0;
	if (nNeu)
	{
		if (bUStGeaendert)
		{
			int nUva = UvaFeldZu(nNeu);
			if (nUva && UvaFeldZuweisbar(nUva, bEinnahmen)) nZiel = nUva;
		}
		else
		{
			UStKandidaten(nNeu, bEinnahmen, m.kandidaten);
			if (m.kandidaten.GetSize() == 1)
			{
				nZiel = m.kandidaten[0];
				m.kandidaten.RemoveAll();
			}
		}
	}
	BOOL bMehrdeutig = m.kandidaten.GetSize() > 1;

	if (!nPartner)
	{	// Gegenstück noch frei: Eindeutiges eintragen, sonst wählen lassen. Wurde die Zuweisung entfernt, bleibt es dabei.
		if (nZiel)
		{
			SchreibeZuweisung(csFeldzuweisung, partnerFormular, nZiel);
			m.nPartnerNeu = nZiel;
			m.art = Mitfuehrung::PARTNER_GESETZT;
		}
		else if (bMehrdeutig)
			m.art = Mitfuehrung::PARTNER_WAEHLEN;
		return;
	}

	if (PruefePaar(nUvaFeld, nUStFeld, bEinnahmen) == STIMMIG)
	{
		m.kandidaten.RemoveAll();
		return;
	}

	// Lief das Gegenstück bisher im Gleichschritt mit der alten Zuweisung? Dann folgt es ihr auch jetzt.
	BOOL bGleichschritt = nAlt && (bUStGeaendert ? UvaFeldZu(nAlt) == nPartner : UvaFeldZu(nPartner) == nAlt);
	if (bGleichschritt)
	{
		if (nZiel)
		{
			SchreibeZuweisung(csFeldzuweisung, partnerFormular, nZiel);
			m.nPartnerNeu = nZiel;
			m.art = Mitfuehrung::PARTNER_GESETZT;
		}
		else if (bMehrdeutig)
			m.art = Mitfuehrung::PARTNER_WAEHLEN;
		else
			m.art = Mitfuehrung::PARTNER_ENTFERNEN;	// Zuweisung entfernt oder neues Feld ohne Gegenstück
	}
	else if (nNeu)
	{	// das Gegenstück war schon vorher eigenständig belegt und passt auch zur neuen Zuweisung nicht
		m.art = Mitfuehrung::WIDERSPRUCH;
		m.nPartnerNeu = nZiel;
	}
	// sonst: Zuweisung entfernt, das eigenständige Gegenstück bleibt unberührt
}
