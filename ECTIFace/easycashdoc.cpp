// EasyCashDoc.cpp : implementation of the CEasyCashDoc class
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
#include <ctype.h>

#include "resource.h"
#include "Datensicherungsoptionen.h"
#include "Startoptionen.h"
#define EXPORT_ECT_DLL
#include "EasyCashDoc.h"
#include "BuchungsjahrWaehlen.h"
#include "Konvertierung.h"
#include "AfAGenauigkeit.h"
#include "ectiface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

double kurse[] = { 1.0, 1.95583, 40.3399, 166.386, 6.55957, 0.787564, 1936.27, 2.20371, 13.7603, 200.482, 5.94573, 1.0};
char *waehrungskuerzel[] = { "EUR", "DEM", "BEF", "PTE", "FRF", "IEP", "ITL", "HFL", "ATS", "PTE", "FIM", "" };


/////////////////////////////////////////////////////////////////////////////
// CMyMultiDocTemplate

CMyMultiDocTemplate::CMyMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass ) :
	CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{ };

BOOL CMyMultiDocTemplate::GetDocString(CString& rString, enum DocStringIndex i) const
{
    CString strTemp,strLeft,strRight;
    int nFindPos;
    AfxExtractSubString(strTemp, m_strDocStrings, (int)i);

    if (i == CDocTemplate::filterExt)  
	{
		nFindPos=strTemp.Find(';');
		if(-1 != nFindPos) 
		{
			//string contains two extensions
			strLeft=strTemp.Left(nFindPos+1);
			strRight=strTemp.Right(lstrlen((const
			char*)strTemp)-nFindPos-1);
			strTemp=strLeft+strRight;
		}
    }
	rString = strTemp;
	return TRUE;
}

CDocTemplate::Confidence CMyMultiDocTemplate::MatchDocType(const char* pszPathName, CDocument*& rpDocMatch)
{
    ASSERT(pszPathName != NULL);
    rpDocMatch = NULL;

    // go through all documents
    POSITION pos = GetFirstDocPosition();
    while (pos != NULL)
    {
       CDocument* pDoc = GetNextDoc(pos);
       if (pDoc->GetPathName() == pszPathName) {
          // already open
          rpDocMatch = pDoc;
          return yesAlreadyOpen;
       }
    }  // end while

    // see if it matches either suffix
    CString strFilterExt;
    if (GetDocString(strFilterExt, CDocTemplate::filterExt) &&
      !strFilterExt.IsEmpty())
    {
       // see if extension matches
       ASSERT(strFilterExt[0] == '.');
       CString ext1,ext2;
       int nDot = CString(pszPathName).ReverseFind('.');
       const char* pszDot = nDot < 0 ? NULL : pszPathName + nDot;

       int nSemi = strFilterExt.Find(';');
       if(-1 != nSemi)   {
         // string contains two extensions
         ext1=strFilterExt.Left(nSemi);
         ext2=strFilterExt.Mid(nSemi+2);
         // check for a match against either extension
         if (nDot >= 0 && (lstrcmpi(pszPathName+nDot, ext1) == 0
             || lstrcmpi(pszPathName+nDot,ext2) ==0))
           return yesAttemptNative; // extension matches
       }
       else
       { // string contains a single extension
         if (nDot >= 0 && (lstrcmpi(pszPathName+nDot,
             strFilterExt)==0))
         return yesAttemptNative;  // extension matches
       }
    }
    return yesAttemptForeign; //unknown document type
}




/////////////////////////////////////////////////////////////////////////////
// CBetrag

long CBetrag::GetNetto()
{
	long netto;
	netto = Betrag;
	if (Betrag <= 1000000 && Betrag >= -1000000 && MWSt % 1000 == 0)
	{
		netto *= 1000;
		netto /= (100 + (MWSt / 1000));
		if (Betrag >= 0) // halber Pfennig zum Runden
			netto += 5;		
		else
			netto -= 5;
		netto /= 10;
	}
	else // bei zu großen Zahlen lieber double-float rechnen!
	{
		double temp = (double)netto * 100.0;
		temp /= (100.0 + ((double)MWSt / 1000.0));
		if (Betrag >= 0) // halber Cent zum Runden
			netto = (long)(temp + 0.5);
		else
			netto = (long)(temp - 0.5);
	}

	return netto;
}

BOOL CBetrag::SetMWSt(char *s)
{
	char *cp = s;
	MWSt = 0;

	while(isdigit(*cp))
	{
		MWSt *= 10;
		MWSt += *cp++ - '0';
	}

	if (MWSt > 100)
	{
		MWSt = 100;
		return FALSE;
	}

	if (*cp++ != ',') 
	{
		MWSt *= 1000;	// kein Nachkommaanteil
		return TRUE;
	}

	int i;
	for (i = 0; i < 3; i++)
	{
		int n;
		if (isdigit(*cp) || cp-s < (int)strlen(s)) 
			n = *cp++ - '0';
		else
			n = 0;

		MWSt *= 10;
		MWSt += n;
	}
	
	if (MWSt > 100000) 
	{
		MWSt = 100000;
		return FALSE;
	}
	
	return TRUE;
}

void CBetrag::GetMWSt(char *s)
{
	if (MWSt % 1000 == 0)
		sprintf(s, "%d", MWSt / 1000);
	else if (MWSt % 100 == 0)
		sprintf(s, "%3.1f", (float)MWSt / 1000.0);
	else if (MWSt % 10 == 0)
		sprintf(s, "%4.2f", (float)MWSt / 1000.0);
	else 
		sprintf(s, "%5.3f", (float)MWSt / 1000.0);

	char *cp;
	if (cp = strrchr(s, '.'))
		*cp = ',';
}

// TRUE zurück wenn Währungskürzel gefunden und Konvertierung durchgeführt wurde
BOOL CBetrag::ConvertToEuro(CString wkz)
{
	int i;
	for (i = 1; i < 11; i++)
		if (wkz == waehrungskuerzel[i])
		{
			double d = (double)Betrag;			// auf double konvertieren
			d /= kurse[i];						// auf Euro umgerechnen
			Betrag = (int)((double)d + 0.5);	// kfm. runden und auf Festkomma konvertieren
			return TRUE;
		}
		
	return FALSE;
}

BOOL CBetrag::ConvertFromEuro(CString wkz)
{
	int i;
	for (i = 1; i < 11; i++)
		if (wkz == waehrungskuerzel[i])
		{
			double d = (double)Betrag;			// auf double konvertieren
			d *= kurse[i];						// von Euro nach 'wkz' umgerechnen
			Betrag = (int)((double)d + 0.5);	// kfm. runden und auf Festkomma konvertieren
			return TRUE;
		}
		
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBuchung

IMPLEMENT_SERIAL(CBuchung, CObject, VERSIONABLE_SCHEMA|VERSION)

CBuchung::CBuchung()
{
	Betrag					= 0;
	MWSt					= 0;
	AbschreibungNr			= 1;
	AbschreibungJahre		= 1;
	Beschreibung			= "";
	Datum					= CTime(2000, 1, 1, 0, 0, 0);
	Konto					= "";
	Belegnummer				= "";
	Erweiterung				= "";
	AbschreibungRestwert	= 0;
	AbschreibungDegressiv	= 0;
	AbschreibungSatz		= 0;
	AbschreibungGenauigkeit = ENTSPRECHEND_EINSTELLUNGEN_AFA;
	Bestandskonto			= "";	
	Betrieb					= "";		
	next = NULL;
}

CBuchung::~CBuchung()
{
	if (next) delete next;	// crash v2.51.0.1-6edbae21-2d63-4630-8e78-00df1ebe2ca2
}

void CBuchung::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) ar.SetObjectSchema(VERSION);

	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << Betrag;		// in Cents
		ar << MWSt;
		ar << AbschreibungNr;
		ar << AbschreibungJahre;
		ar << Beschreibung;
		ar << Datum;
		ar << Konto;		// ab VERSION 3
		ar << Belegnummer;	// ab VERSION 7
		ar << Erweiterung;				// ab VERSION 7
		ar << AbschreibungRestwert;		// ab VERSION 7
		ar << AbschreibungDegressiv;	// ab VERSION 7
		ar << AbschreibungSatz;			// ab VERSION 7
		ar << AbschreibungGenauigkeit;  // ab VERSION 10
		ar << Bestandskonto;			// ab VERSION 11
		ar << Betrieb;					// ab VERSION 11
		ar << next;
	}
	else
	{
		int nVersion = ar.GetObjectSchema();

		ar >> Betrag;	// in Pfennigen
		ar >> MWSt;
		if (nVersion < 5) MWSt *= 1000; // ab Dateiversion 5 MWSt mit Nachkommaanteil
		ar >> AbschreibungNr;
		ar >> AbschreibungJahre;
		ar >> Beschreibung;
		ar >> Datum;

		// ab VERSION 3
		if (nVersion >= 3)
			ar >> Konto;
		else
			Konto = "";

		// ab VERSION 7
		if (nVersion >= 7)
		{
			ar >> Belegnummer;
			ar >> Erweiterung;
			ar >> AbschreibungRestwert;	
			ar >> AbschreibungDegressiv;	
			ar >> AbschreibungSatz;		
		}
		else
		{
			Belegnummer = "";
			Erweiterung = "";
			// Restwert berechnen
			{ 
				// Netto-Betrag berechnen
				long netto = GetNetto();

				// MwSt-Betrag berechnen
				AbschreibungRestwert = 0;
				int i;
				for (i = AbschreibungNr; i <= AbschreibungJahre; i++)
				{
					AbschreibungRestwert += netto / AbschreibungJahre 
						  + (netto % AbschreibungJahre >= AbschreibungNr ? 1 : 0);	// Rundungsfehler auf die ersten Jahre aufteilen!
				}
			}
			AbschreibungDegressiv = FALSE;	
			AbschreibungSatz = 0;		
		}

		// ab VERSION 10
		if (nVersion >= 10)
		{
			ar >> AbschreibungGenauigkeit;
		}
		else
			AbschreibungGenauigkeit = ENTSPRECHEND_EINSTELLUNGEN_AFA;

		// ab VERSION 11
		if (nVersion >= 11)
		{
			ar >> Bestandskonto;			// ab VERSION 11
			ar >> Betrieb;					// ab VERSION 11
		}
		else
		{
			Bestandskonto = "";
			Betrieb = "";
		}
		
		// next natürlich in allen Versionen
		ar >> next;
	}
}

// ohne next!!!
CBuchung& CBuchung::operator=(const CBuchung& buchungSrc)
{
	Betrag					= buchungSrc.Betrag;					// in Cent
	MWSt					= buchungSrc.MWSt;
	AbschreibungNr			= buchungSrc.AbschreibungNr;
	AbschreibungJahre		= buchungSrc.AbschreibungJahre;
	Beschreibung			= buchungSrc.Beschreibung;
	Datum					= buchungSrc.Datum;
	Konto					= buchungSrc.Konto;
	Belegnummer				= buchungSrc.Belegnummer;
	Erweiterung				= buchungSrc.Erweiterung;
	AbschreibungRestwert	= buchungSrc.AbschreibungRestwert;	
	AbschreibungDegressiv	= buchungSrc.AbschreibungDegressiv;	
	AbschreibungSatz		= buchungSrc.AbschreibungSatz;		
	AbschreibungGenauigkeit	= buchungSrc.AbschreibungGenauigkeit;
	Bestandskonto			= buchungSrc.Bestandskonto;		
	Betrieb					= buchungSrc.Betrieb;		
	return *this;
}

// wie CBBetrag::GetNetto, berücksichtigt aber AfAs und gibt lediglich den aktuellen Abschreibungswert für diese zurück
long CBuchung::GetBuchungsjahrNetto(CEasyCashDoc *pDoc)
{
	int angewandte_Abschreibungsgenauigkeit;
	if (!pDoc && AbschreibungGenauigkeit == ENTSPRECHEND_EINSTELLUNGEN_AFA) // Informationsnotstand? monatsgenau annehmen!
		angewandte_Abschreibungsgenauigkeit = MONATSGENAUE_AFA;
	else if (AbschreibungGenauigkeit == ENTSPRECHEND_EINSTELLUNGEN_AFA)
		angewandte_Abschreibungsgenauigkeit = pDoc->AbschreibungGenauigkeit;
	else
		angewandte_Abschreibungsgenauigkeit = AbschreibungGenauigkeit;

	return GetBuchungsjahrNetto(angewandte_Abschreibungsgenauigkeit);
}


// gibt die jährliche Abschreibung für das aktuelle Buchungsjahr zurück, berücksichtigt dabei die Abschreibungsgenauigkeit
long CBuchung::GetBuchungsjahrNetto(int angewandte_Abschreibungsgenauigkeit)
{
	// Netto-Betrag berechnen
	long netto = GetNetto();

	// keine AfA? dann einfach netto zurückgeben
	if (AbschreibungJahre <= 1)
		return netto;
	
	if (AbschreibungDegressiv)
	{
		// Spezialfall: 75%ige AfA im Anschaffungsjahr bei Elektroautos bei gleichzeitier ganzjähriger AfA
		if (AbschreibungSatz == 75 && AbschreibungGenauigkeit == GANZJAHRES_AFA)
		{   
			static int eautoAfa[] = { 75, 10, 5, 5, 3, 2 };  // diese Raten beziehen sich auf den Netto-Anschaffungswert, nicht auf den Restwert
			if (AbschreibungNr >= 1 && AbschreibungNr <= 6)
				return GetNetto() * eautoAfa[AbschreibungNr - 1] / 100;
			else
				return 0L;  // nach 6 Jahren ist nichts mehr abzuschreiben, sollte eigenlich nicht vorkommen, da AbschreibungJahre normalerweise auf 6 eingestellt sein sollte
		}
		else
		{
			// im letzten Jahr den Restwert zurückgeben (ist ein hypothetischer Fall: in der Praxis wäre schon längst auf lineare AfA umgestellt worden)
			if (AbschreibungNr == AbschreibungJahre)
				if (angewandte_Abschreibungsgenauigkeit == GANZJAHRES_AFA ||
					(angewandte_Abschreibungsgenauigkeit == HALBJAHRES_AFA && Datum.GetMonth() <= 6) ||
					(angewandte_Abschreibungsgenauigkeit == MONATSGENAUE_AFA && Datum.GetMonth() <= 1))
				{   
					return AbschreibungRestwert;
				}
			if (AbschreibungNr > AbschreibungJahre)  // ggf. extra Jahr bei nicht ganzjähriger AfA-Genauigkeit
				return AbschreibungRestwert;		 // hier in jedem Fall den Restwert zurückgeben
			return BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen(AbschreibungRestwert * AbschreibungSatz / 100, angewandte_Abschreibungsgenauigkeit);
		}
	}
	else // lineare AfA
	{	
		// alte Methode zur Berechnung der linearen AfA -- geht so seit Einführung der degressiven 
		// AfA nicht mehr: jetzt geht nur noch strikt von den Restwerten auszugehen
		// jaehrliche_rate = netto / AbschreibungJahre
		//				  + (netto % AbschreibungJahre >= AbschreibungNr ? 1 : 0);	// Rundungsfehler auf die ersten Jahre aufteilen!

		// Restabschreibungsdauer bestimmen
		int gesamt_monate = 12 * AbschreibungJahre, verbleibende_monate;
		switch (angewandte_Abschreibungsgenauigkeit)
		{
		case GANZJAHRES_AFA:
			verbleibende_monate = (AbschreibungJahre - AbschreibungNr + 1) * 12;
			break;
		case HALBJAHRES_AFA:
			if (AbschreibungNr == 1)
				verbleibende_monate = AbschreibungJahre * 12;
			else
				if (Datum.GetMonth() < 7)
					verbleibende_monate = (AbschreibungJahre - AbschreibungNr + 1) * 12;
				else
					verbleibende_monate = (AbschreibungJahre - AbschreibungNr + 2) * 12 - 6;
			break;
		case MONATSGENAUE_AFA:
			if (AbschreibungNr == 1)
				verbleibende_monate = AbschreibungJahre * 12;
			else
				verbleibende_monate = (AbschreibungJahre - AbschreibungNr + 2) * 12 - (Datum.GetMonth() - 1);
			break;
		}

		// jetzt verbleibende_monate auf den Restwert herunterbrechen und die Jahresrate bestimmen
		long jaehrliche_rate = AbschreibungRestwert * 12 / verbleibende_monate;
		if (jaehrliche_rate > AbschreibungRestwert)
			jaehrliche_rate = AbschreibungRestwert;
		return BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen(jaehrliche_rate, angewandte_Abschreibungsgenauigkeit);
	}
}

// hier wird die Abschreibungsgenauigkeit berücksichtigt: im ersten Jahr entsprechend der Einstellungen, im letzten Jahr entsprechend der verbleibenden Monate
long CBuchung::BuchungsjahrNettoAbschreibungsgenauigkeitBeruecksichtigen(long jaehrliche_rate, int angewandte_Abschreibungsgenauigkeit)
{
	if (AbschreibungNr == 1)
	{
		switch (angewandte_Abschreibungsgenauigkeit)
		{
		case GANZJAHRES_AFA:
			return jaehrliche_rate;
		case HALBJAHRES_AFA:
			if (Datum.GetMonth() < 7)
				return jaehrliche_rate;
			else
				return jaehrliche_rate / 2;
		case MONATSGENAUE_AFA:
			return jaehrliche_rate * (13 - Datum.GetMonth()) / 12;
		}
	}
	else if (AbschreibungNr <= AbschreibungJahre)	// die Jahre dazwischen (oder im Fall einer vollen 12-Monats-Rate kann 
	{												// es auch das letzte Jahr sein, macht dann aber keinen Unterschied)
		return jaehrliche_rate;
	}
	else // if (AbschreibungNr > AbschreibungJahre)
	{	 // letztes Jahr (bei nicht ganzjähriger AfA): hier muss die Anzahl der verbleibenden Monate berücksichtigt werden
		switch (angewandte_Abschreibungsgenauigkeit)
		{
		case GANZJAHRES_AFA:
			return 0L;
		case HALBJAHRES_AFA:
			if (Datum.GetMonth() < 7)
				return 0L;
			else
				return jaehrliche_rate / 2;
		case MONATSGENAUE_AFA:
			long verbleibendeMonate = 13 - Datum.GetMonth();
			return jaehrliche_rate * verbleibendeMonate / 12;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDauerbuchung

IMPLEMENT_SERIAL(CDauerbuchung, CObject, VERSIONABLE_SCHEMA|VERSION)

CDauerbuchung::CDauerbuchung()
{
	next = NULL;
}

CDauerbuchung::~CDauerbuchung()
{
	if (next) delete next;
}

void CDauerbuchung::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) ar.SetObjectSchema(VERSION);

	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar << Betrag;	// in Pfennigen
		ar << MWSt;
		ar << Beschreibung;
		ar << Buchungstyp;			// Buchungstyp
		ar << Intervall;	// ... oder monatlich
		ar << VonDatum;
		ar << BisDatum;
		ar << Buchungstag;
		ar << AktualisiertBisDatum;
		ar << Konto;
		ar << Belegnummer;	// ab VERSION 7
		ar << Erweiterung;	// ab VERSION 7
		ar << Bestandskonto;				// dazugekommen ab v1.56 oder CDoc Version 12
		ar << Betrieb;					// dazugekommen ab v1.56 oder CDoc Version 12
		ar << next;
	}
	else
	{
		int nVersion = ar.GetObjectSchema();

		ar >> Betrag;	// in Pfennigen
		ar >> MWSt;
		if (nVersion < 5) MWSt *= 1000; // ab Dateiversion 5 MWSt mit Nachkommaanteil
		ar >> Beschreibung;
		ar >> Buchungstyp;			// Buchungstyp
		ar >> Intervall;	// ... oder monatlich
		ar >> VonDatum;
		ar >> BisDatum;
		ar >> Buchungstag;
		ar >> AktualisiertBisDatum;

		if (nVersion >= 3)
			ar >> Konto;
		else
			Konto = "";

		// ab VERSION 7
		if (nVersion >= 7)
		{
			ar >> Belegnummer;
			ar >> Erweiterung;
		}
			
		// ab VERSION 12
		if (nVersion >= 12)
		{
			ar >> Bestandskonto;
			ar >> Betrieb;
		}
			
		ar >> next;
	}
}

// ohne next!!!
CDauerbuchung& CDauerbuchung::operator=(const CDauerbuchung& dauerbuchungSrc)
{
	Betrag					= dauerbuchungSrc.Betrag;				// in Pfennigen
	MWSt					= dauerbuchungSrc.MWSt;
	Beschreibung			= dauerbuchungSrc.Beschreibung;
	Buchungstyp				= dauerbuchungSrc.Buchungstyp;			// Buchungstyp
	Intervall				= dauerbuchungSrc.Intervall;			// ... oder monatlich
	VonDatum				= dauerbuchungSrc.VonDatum;
	BisDatum				= dauerbuchungSrc.BisDatum;
	Buchungstag				= dauerbuchungSrc.Buchungstag;
	AktualisiertBisDatum	= dauerbuchungSrc.AktualisiertBisDatum;
	Konto					= dauerbuchungSrc.Konto;
	Belegnummer				= dauerbuchungSrc.Belegnummer;
	Erweiterung				= dauerbuchungSrc.Erweiterung;
	Bestandskonto			= dauerbuchungSrc.Bestandskonto;		
	Betrieb					= dauerbuchungSrc.Betrieb;		
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
// CEasyCashDoc

IMPLEMENT_SERIAL(CEasyCashDoc, CDocument, VERSIONABLE_SCHEMA|VERSION)

BEGIN_MESSAGE_MAP(CEasyCashDoc, CDocument)
	//{{AFX_MSG_MAP(CEasyCashDoc)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasyCashDoc construction/destruction

CEasyCashDoc::CEasyCashDoc()
{
	CTime now = CTime::GetCurrentTime();

	Einnahmen = NULL;
	Ausgaben = NULL;
	Dauerbuchungen = NULL;
	Buchungszaehler = 0;
	nLaufendeBuchungsnummerFuerEinnahmen = 1;
	nLaufendeBuchungsnummerFuerAusgaben = 1;
	nLaufendeBuchungsnummerFuerBank = 1;
	nLaufendeBuchungsnummerFuerKasse = 1;
	nJahr = 0;
	char LocaleWaehrungskuerzel[4];
	*LocaleWaehrungskuerzel = '\0';
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SINTLSYMBOL, LocaleWaehrungskuerzel, sizeof(LocaleWaehrungskuerzel));
	if (!*LocaleWaehrungskuerzel) strcpy(LocaleWaehrungskuerzel, "EUR");
	csWaehrung = LocaleWaehrungskuerzel;
	csUrspruenglicheWaehrung = LocaleWaehrungskuerzel;
	char buffer[1000];
	GetIniFileName(buffer, sizeof(buffer));
	AbschreibungGenauigkeit = GetPrivateProfileInt("Allgemein", "AbschreibungGenauigkeit", 2, buffer);	
	Version = VERSION;
	m_nZeitraum = -1;
	ctNachfrageTermin = now + CTimeSpan(7, 0, 0, 0);
}

CEasyCashDoc::~CEasyCashDoc()
{
	if (Einnahmen) delete Einnahmen;
	if (Ausgaben) delete Ausgaben;
	if (Dauerbuchungen) delete Dauerbuchungen;
}

BOOL CEasyCashDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	//if (theApp.m_istInStartupPhase)
	//	ZeigeStartoptionen();

	CBuchungsjahrWaehlen dlg;
	dlg.m_jahr = CTime::GetCurrentTime().GetYear();
	dlg.m_waehrung = csWaehrung;
	if (dlg.DoModal() != IDOK) return FALSE;
	nJahr = dlg.m_jahr;
	csWaehrung = dlg.m_waehrung;
	SetModifiedFlag("Neue Buchungsdatei erzeugt", FALSE);
/*
	// Währung wählen
	{
		CKonvertierung dlg;
		dlg.m_waehrung_auswahl = 0;
		dlg.m_konvertieren = -1;
		dlg.m_waehrungskuerzel = "SFR";

		while (TRUE)
		{
			dlg.DoModal();
			if (dlg.m_waehrung_auswahl >=0 && dlg.m_waehrung_auswahl <= 11)
				break;
			else
				AfxMessageBox("Es ist notwendig, dass eine Währung ausgewählt wird. Wenn die Währung, in der die Buchungsdaten vorliegen, nicht aufgeführt ist, bitte 'andere' wählen.");
		}

		if (dlg.m_waehrung_auswahl < 11)
			csWaehrung = waehrungskuerzel[dlg.m_waehrung_auswahl];
		else
			csWaehrung = dlg.m_waehrungskuerzel;
		csUrspruenglicheWaehrung = csWaehrung;

	}
*/
	char buf[300];
	char titlebuf[300];
	char ini_filename[500];
	char nummer[10];

	// normalen Dateinamen erzeugen
	sprintf(buf, "Jahr%04d.eca", nJahr);
	
	// schone einmal ECA-Datei mit diesem Namen erzeugt?
	GetIniFileName(ini_filename, sizeof(ini_filename));
	int n = GetPrivateProfileInt("NeuesDokumentNummer", buf, 0, ini_filename);
	sprintf(nummer, "%d", n+1);
	WritePrivateProfileString("NeuesDokumentNummer", buf, nummer, ini_filename);

	// dann ein "-1", "-2" ... an den Dateinamen anhängen
	if (n > 0)
		sprintf(buf, "Jahr%04d-%d.eca", nJahr, n);
	
	// // Versuch einen MRU list bug zu fixen: AddToRecentFileList wird mehrfach aufgerufen, das letzte mal mit ECTIFace-DLL-Pfad
	// m_strPathName = buf;
	// 
	// strcpy(titlebuf, buf);
	// if (strlen(titlebuf) > 4 && !strcmp(titlebuf + strlen(titlebuf) - 4, ".eca"))  // Dateiendung .eca im Titel entfernen
	// 	titlebuf[strlen(titlebuf) - 4] = _T('\0');
	// SetTitle(titlebuf);
	// OnFileSave();         --> bessere Lösung: SetPathName(buf, FALSE); statt SetPathName(buf);

	TRY
	{
		char* cp1 = strrchr(ini_filename, '\\');
		if (cp1)
		{
			strcpy(++cp1, buf);
			SetPathName(ini_filename, FALSE);
		}
	}
	CATCH_ALL(e)  // (CInvalidArgException, e) ... auch file not found abdecken --> CATCH_ALL
	{
		// ignore invalid argument exception or file not found, if file was not found in "add to MRU" code (OnNewDocument)
		SetTitle(buf);  // ensure a title is set
	}
	END_CATCH_ALL //  END_CATCH


	// Hook Erweiterungs-DLLs
	::CIterateExtensionDLLs("ECTE_OpenDocument", (void *)this);

	return TRUE;
}


// !!!! siehe auch CEasyCashApp::OnFileOpen()  !!!!
BOOL CEasyCashDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	char lpszDatenverzeichnis[500];
	if (!*lpszPathName)
	{
		GetIniFileName(lpszDatenverzeichnis, 500);
		char *cp1 = strrchr(lpszDatenverzeichnis, '\\');
		if (cp1) *cp1 = '\0';
	}
	else
		strcpy(lpszDatenverzeichnis, lpszPathName);
/*	
	// Sonderbehandlung Registry-Daten
	if (strlen(lpszPathName) >= 7 && !((CString)lpszPathName).Right(7).CompareNoCase(".ecareg"))
	{
		if (AfxMessageBox("EC&T-Registrierungsdaten einlesen?", MB_YESNO) == IDYES)
		{
			CString Datenverzeichnis;
			GetPrivateProfileString("Allgemein", "Datenverzeichnis", "", Datenverzeichnis.GetBuffer(1000), 1000, lpszPathName);
			Datenverzeichnis.ReleaseBuffer();

			CString RegKey;
			GetPrivateProfileString("Allgemein", "RegKey", "", RegKey.GetBuffer(1000), 1000, lpszPathName);
			RegKey.ReleaseBuffer();

			// Allgemein
			if (!Datenverzeichnis.IsEmpty()) 
			{
				WriteProfileString("Allgemein", "Datenverzeichnis", Datenverzeichnis);
				SetIniFileName(((CString)(Datenverzeichnis + "\\easyct.ini")).GetBuffer(0));
			}
			if (!RegKey.IsEmpty()) WriteProfileString("Allgemein", "RegKey", RegKey);

		}
		return FALSE;

habe ich besser eigene Menüpunkte für gemacht...
	}
*/
	CheckWiederherstellungsdatei(lpszDatenverzeichnis);

	if (!CDocument::OnOpenDocument(lpszDatenverzeichnis))
	{
		ZeigeStartoptionen();
		return FALSE;
	}

	if (Version == -1) return FALSE;

	char IniFileName[500];
	GetIniFileName(IniFileName, sizeof(IniFileName));
	if (*lpszPathName) WritePrivateProfileString("Allgemein", "LetzteDatei", lpszDatenverzeichnis, IniFileName);
	
	// Hook Erweiterungs-DLLs
	::CIterateExtensionDLLs("ECTE_OpenDocument", (void *)this);

	// bei Mandanten check, ob die Datei aus dem selben Datenverzeichnis kommt wie die easyct.ini
	if (GetMandant() >= 0)
	{
		int nBackslashPos, nBackslashPos2;
		CString csPathName = lpszDatenverzeichnis;
		CString csPathName2 = (CString)IniFileName;
		csPathName.MakeLower();
		csPathName2.MakeLower();
		if ((nBackslashPos = csPathName.ReverseFind('\\')) > 0 && (nBackslashPos2 = csPathName2.ReverseFind('\\')) > 0)
		{
			if (nBackslashPos != nBackslashPos2 ||
				csPathName.Left(nBackslashPos) != csPathName2.Left(nBackslashPos2))
				AfxMessageBox("Achtung: Die zu öffnende Datei befindet sich nicht im korrekten Datenverzeichnis für den Mandanten. Das kann zu Problemen führen, wenn z.B. der Kontenrahmen ein anderer ist wie der, mit dem die Buchungsdatei ursprünglich entstanden ist.");
		}
	}
		
	return TRUE;
}

void CEasyCashDoc::ZeigeStartoptionen()
{
	int nRet;
	CStartoptionen dlg;
	nRet = dlg.DoModal();

	switch (nRet)
	{
	case 0:
		PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_FILE_OPEN, 0L);
		return;
	case 1:
		PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_FILE_WAEHLE_DATENVERZEICHNIS, 0L);
		return;
	case 2:
		PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_FILE_NEW, 0L);
		return;
	case 3:
	default:
		// tu nichts
		return;
	case 4:
		PostMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_APP_EXIT, 0L);
		return;
	}
}

void CEasyCashDoc::OnCloseDocument() 
{
	// Hook Erweiterungs-DLLs
	::CIterateExtensionDLLs("ECTE_CloseDocument", (void *)this);

	LoescheWiederherstellungsdatei((LPCTSTR)m_strPathName);

	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////
// CEasyCashDoc serialization

#define MAGIC_KEY "ECDo"
static char magic[5] = { MAGIC_KEY };

void CEasyCashDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) ar.SetObjectSchema(VERSION);

    CDocument::Serialize(ar);      

	if (ar.IsStoring())
	{
		ar << magic[0] << magic[1] << magic[2] << magic[3];
		ar << (DWORD)VERSION;
		
		ar << (BOOL)(Einnahmen != 0);
		if (Einnahmen) ar << Einnahmen;
	
		ar << (BOOL)(Ausgaben != 0);	// crash v2.51.0.1-9d4cbb5d-c8e9-4452-9fe7-673648049f6d und v2.51.0.1-780f0825-abfb-4d23-b12d-26da2e2ccd06, siehe auch DoSave()
		if (Ausgaben) ar << Ausgaben;

		ar << Buchungszaehler;
		
		ar << (BOOL)(Dauerbuchungen != 0);
		if (Dauerbuchungen) ar << Dauerbuchungen;

		// ab VERSION 2
		ar << nLaufendeBuchungsnummerFuerEinnahmen;
		ar << nLaufendeBuchungsnummerFuerAusgaben;

		// ab VERSION 4
		ar << nJahr;

		// ab VERSION 6
		ar << csWaehrung;
		ar << csUrspruenglicheWaehrung;

		// ab VERSION 7
		ar << Erweiterung;
		ar << AbschreibungGenauigkeit;

		// ab VERSION 8
		ar << nLaufendeBuchungsnummerFuerBank;
		ar << nLaufendeBuchungsnummerFuerKasse;

		// Version 9 nicht wirklich neuer, nur wegen Bugfix in CEinstellungCtrl->SpeichereEinstellung
		
		// ab VERSION 13
		ar << nNachfrageIntervall;
		ar << ctNachfrageTermin;
	}
	else
	{
		Version = -1; // ungültige Datei
		char buf[5];
		DWORD file_version;

		ar >> buf[0] >> buf[1] >> buf[2] >> buf[3];
		if (strncmp(buf, magic, 4))
		{
			AfxGetMainWnd()->MessageBox("Fehler beim Öffnen: Kein EasyCash-Dokument!",NULL,MB_ICONSTOP);
			return;
		}
		ar >> (DWORD)file_version;
		if (file_version > VERSION)
		{
			AfxGetMainWnd()->MessageBox("Fehler beim Öffnen: Diese Version des Programms ist zu veraltet um das EasyCash-Dokument einzulesen. Die Installation einer aktuellen Version des Programms ist erforderlich!",NULL,MB_ICONSTOP);
			return;
		}
		BOOL e, a, d;
		ar >> (BOOL)e;
		if (e) 
		{			
			if (file_version < 3)
			{
				if (!Einnahmen) Einnahmen = new CBuchung;
				Einnahmen->Serialize(ar);
			}
			else
				ar >> Einnahmen; // ab Version 3 'richtig serialisieren'
		}

		ar >> (BOOL)a;
		if (a) 
		{
			if (file_version < 3)
			{
				if (!Ausgaben) Ausgaben = new CBuchung;
				Ausgaben->Serialize(ar);
			}
			else
			{
				TRY
				{
					ar >> Ausgaben; // ab Version 3 'richtig serialisieren'
				}
				CATCH(CArchiveException, e)
				{
					if (e->IsKindOf(RUNTIME_CLASS(CArchiveException)) && ((CArchiveException*)e)->m_cause == 5)
					{
						AfxMessageBox("ACHTUNG: Die Datei wurde beschädigt und konnte nicht wieder korrekt geladen werden. Die Einnahmen-Buchungen waren ok aber die Ausgaben-Buchungen konnten nicht eingelesen werden. Außerdem sind die Dauerbuchungen verloren. Es wäre besser mit einer Version aus einer Datensicherung weiterzuarbeiten. Es folgen ein paar Dialoge zur manuellen Rekonstruktion einzelner verlorener Daten.");
						file_version = 3;						
						SetModifiedFlag("Beschädigte Datei wurde teilweise rekonstruiert.");
						goto recover;
					}
					else
					{
						DeleteContents();   // remove failed contents

						TRY
						{
							ReportSaveLoadException(GetPathName(), e, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
						}
						END_TRY
						e->Delete();
						AfxGetMainWnd()->PostMessage(ID_FILE_CLOSE, 0, 0L);
						return;
					}
				}
				END_CATCH
			}
		}

		ar >> Buchungszaehler;

		ar >> (BOOL)d;
		if (d) 
		{
			if (file_version < 3)
			{
				if (!Dauerbuchungen) Dauerbuchungen = new CDauerbuchung;
				Dauerbuchungen->Serialize(ar);
			}
			else
				ar >> Dauerbuchungen; // ab Version 3 'richtig serialisieren'
		}

		// ab VERSION 2
		if (file_version < 2)
		{
			nLaufendeBuchungsnummerFuerEinnahmen = 1;
			nLaufendeBuchungsnummerFuerAusgaben  = 1;

			AfxMessageBox("Hinweis - Es gibt einige Neuerungen im Programm: \r\n\
1. Beim Buchen kann durch Eintippen einer Zweistelligen Buchungspostennummer in das Feld 'Beschreibung'  ein vorab vergebener Buchungstext abgerufen werden. Optional kann er auch über den Knopf am rechten Rand des Feldes 'Beschreibung' von einer Liste ausgewählt werden. Die Buchungstexte können unter Menü->Ansicht->Einstellungen geändert werden.\r\n\
2. Es können im Buchungstextfeld laufende Buchungsnummern eingeblendet werden. Dazu unter Menü->Ansicht->Einstellungen das entsprechende Funktionsfeld ankreuzen.\r\n\
3. Über das Datei-Menü können jetzt Buchungsdaten im CSV-Format (das z.B. MS-Excel versteht) exportiert werden.\r\n\
4. Die Konfigurationsdatei EasyCash.ini wird gleich aus dem Windows-Verzeichnis ins Programmverzeichnis von Easy C&T kopiert. Indem Easy C&T in verschiedenen Unterverzeichnissen installiert wird, ist so eine Quasi-Mandantenfähigkeit gegeben.\r\n\
5. Ein Fehler beim Druck des Buchungsjournals ist behoben.", MB_ICONINFORMATION);

			// Windows Verzeichnis herausfinden
			{
				char buffer2[256];
				HKEY hKey;
				
				*buffer2 = '\0';
				if (RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", &hKey) == ERROR_SUCCESS)
				{
					long lRetCode, lDummy;
					long lType = REG_SZ;
					long lCb = sizeof(buffer2);
					lDummy = 0L;
					
					lRetCode = RegQueryValueEx(hKey, TEXT("SystemRoot"), 
											   NULL, (ULONG *)&lType, 
											   (LPBYTE)buffer2, (ULONG *)&lCb);
					if (lRetCode != ERROR_SUCCESS)
					{
						*buffer2 = '\0';
					}

					RegCloseKey(hKey);
				}

				if (!*buffer2)
					AfxMessageBox("Konnte das Windows-Verzeichnis nicht finden. Bitte EasyCash.ini manuell in das Easy C&T Verzeichnis kopieren und in EasyCT.ini umbenennen!");
				else
				{
					char buffer3[1000];

					strcat(buffer2, "\\EasyCash.ini");
					
					if (!GetIniFileName(buffer3, sizeof(buffer3)))
						AfxMessageBox("Bitte EasyCash.ini manuell in das Easy C&T Verzeichnis kopieren und in EasyCT.ini umbenennen!");
					else
					{
						if (CopyFile(buffer2, buffer3, TRUE))
							AfxMessageBox("Die Konfigurationsdatei EasyCash.ini wurde in das Easy C&T Programmverzeichnis kopiert und in EasyCT.ini umbenannt.");
					}
				}
			}

			SetModifiedFlag("Änderungen im Buchungsdateiformat durchgeführt");
		}
		else
		{
			ar >> nLaufendeBuchungsnummerFuerEinnahmen;
			ar >> nLaufendeBuchungsnummerFuerAusgaben;
		}
recover:
		// ab VERSION 4
		if (file_version < 4)
		{
			if (Einnahmen) nJahr = Einnahmen->Datum.GetYear();
			else if (Ausgaben) nJahr = Ausgaben->Datum.GetYear();
			else nJahr = -1;
			if (nJahr == -1) 
			{
				CTime now = CTime::GetCurrentTime();
				nJahr = now.GetYear();
			}
		}
		else
		{
			ar >> nJahr;
			if (nJahr < 1900 || nJahr > 3000)
			{
				CBuchungsjahrWaehlen dlg;
				dlg.m_jahr = CTime::GetCurrentTime().GetYear();
				if (dlg.DoModal() == IDOK) 
				{
					nJahr = dlg.m_jahr;	
					if (nJahr < 1900) nJahr = 1900;
					if (nJahr > 3000) nJahr = 3000;
				}
				else
				{
					CTime now = CTime::GetCurrentTime();
					nJahr = now.GetYear();
				}
			}
		}

		// ab VERSION 6
		if (file_version < 6)
		{
			CKonvertierung dlg;
			dlg.m_konvertieren = 0;
			dlg.m_waehrungskuerzel = "SFR";

			while (TRUE)
			{
				dlg.DoModal();
				if (dlg.m_waehrung_auswahl >=0 && dlg.m_waehrung_auswahl <= 11)
					break;
				else
					AfxMessageBox("Es ist notwendig, dass eine Währung ausgewählt wird. Wenn die Währung, in der die Buchungsdaten vorliegen, nicht aufgeführt ist, bitte 'andere' wählen.");
			}

			if (dlg.m_waehrung_auswahl < 11)
				csWaehrung = waehrungskuerzel[dlg.m_waehrung_auswahl];
			else
				csWaehrung = dlg.m_waehrungskuerzel;
			csUrspruenglicheWaehrung = csWaehrung;

			if (dlg.m_konvertieren == 0)
			{
				if (dlg.m_waehrung_auswahl > 0 && dlg.m_waehrung_auswahl < 11)
				{
					if (ConvertToEuro())
						AfxMessageBox("Die Buchungsdaten wurden in Euro umgerechnet");
					else
						AfxMessageBox("Bei der Umrechnung in Euro ist ein Fehler aufgetreten. Daten sind evtl. inkonsistent.");
				}
			}
			SetModifiedFlag("Währung wurde ausgewählt");
		}
		else
		{
			ar >> csWaehrung;
			ar >> csUrspruenglicheWaehrung;
		}

		// ab VERSION 7
		if (file_version < 7)
		{
			Erweiterung = "";

			CAfAGenauigkeit dlg;
			if (nJahr < 2004)
				dlg.m_afa_genauigkeit = HALBJAHRES_AFA;
			else
				dlg.m_afa_genauigkeit = MONATSGENAUE_AFA;
			if (dlg.DoModal() == IDOK)
				AbschreibungGenauigkeit = dlg.m_afa_genauigkeit;
			else
			{
				if (nJahr < 2004)
				{
					AbschreibungGenauigkeit = HALBJAHRES_AFA;
					AfxMessageBox("Halbjahres-AfA wurde angenommen.");
				}
				else
				{
					AbschreibungGenauigkeit = MONATSGENAUE_AFA;
					AfxMessageBox("Monatsgenaue AfA wurde angenommen.");
				}
			}
		}
		else
		{
			ar >> Erweiterung;
			ar >> AbschreibungGenauigkeit;
		}

		// ab VERSION 8
		if (file_version < 8)
		{
			nLaufendeBuchungsnummerFuerBank = 1;
			nLaufendeBuchungsnummerFuerKasse  = 1;
		}
		else
		{
			ar >> nLaufendeBuchungsnummerFuerBank;
			ar >> nLaufendeBuchungsnummerFuerKasse;
		}

		// Version 9 nicht wirklich neuer, nur wegen Bugfix in CEinstellungCtrl->SpeichereEinstellung		

		// ab VERSION 13
		if (file_version < 13)
		{
			nNachfrageIntervall = 7;
			ctNachfrageTermin = CTime(2000, 1, 1, 0, 0, 0);
		}
		else
		{
			ar >> nNachfrageIntervall;
			ar >> ctNachfrageTermin;
		}

		Version = VERSION;	// jetzt ist das CDoc auf die aktuelle ECTIFace-Version gebracht
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEasyCashDoc diagnostics

#ifdef _DEBUG
void CEasyCashDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEasyCashDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Wine helper

BOOL DatenverzeichnisCheck(char* pfad1, char* pfad2)  // gibt true zurück, wenn Pfade übereinstimmen
{
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Wine", &hKey) == ERROR_SUCCESS)
	{
		typedef char* (CDECL* wine_get_unix_file_name_realpath)(LPCWSTR dosW);

		HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");
		wine_get_unix_file_name_realpath get_unix_fn = (wine_get_unix_file_name_realpath)GetProcAddress(kernel32, "wine_get_unix_file_name_realpath");

		// unter normalem Wine keinen Check machen (wine_get_unix_file_name_realpath existiert nur im PortJump package (easyct-2.53.0-unsigned-v4.zip)
		if (get_unix_fn == NULL)
			return TRUE;

		// vergleicht reale Wine-Pfade statt möglicherweise verlinktes Userverzeichnis (Y:)
		WCHAR wcPfad1[1000];
		WCHAR wcPfad2[1000];
		ZeroMemory(wcPfad1, sizeof(wcPfad1));
		ZeroMemory(wcPfad2, sizeof(wcPfad2));
		if (!MultiByteToWideChar(CP_ACP, 0, pfad1, (int)strlen(pfad1), wcPfad1, sizeof(wcPfad1)))
		{
			FreeLibrary(kernel32);
			return FALSE;
		}
		if (!MultiByteToWideChar(CP_ACP, 0, pfad2, (int)strlen(pfad2), wcPfad2, sizeof(wcPfad2)))
		{
			FreeLibrary(kernel32);
			return FALSE;
		}

		typedef WCHAR* (CDECL* wine_get_dos_file_name)(LPCSTR str);

		//LPWSTR path = L"C:\\Users\\crossover\\Documents\\test.txt";

		//wine_get_dos_file_name get_dos_fn = (wine_get_dos_file_name)GetProcAddress(kernel32, "wine_get_dos_file_name");

		char* cPfad1Unix = get_unix_fn(wcPfad1);
		char* cPfad2Unix = get_unix_fn(wcPfad2);
		// AfxMessageBox((CString)"1: " + (CString)cPfad1Unix + " -- 2: " + (CString)cPfad2Unix);

		//WCHAR* wcPfad1Unix = get_dos_fn(cPfad1Unix);
		//WCHAR* wcPfad2Unix = get_dos_fn(cPfad2Unix);
		//char cPfad1[1000];
		//char cPfad2[1000];
		//ZeroMemory(cPfad1, sizeof(cPfad1));
		//ZeroMemory(cPfad2, sizeof(cPfad2));

		//int nSize = (int)wcslen(wcPfad1Unix);
		//WideCharToMultiByte(CP_ACP, 0, wcPfad1Unix, nSize, cPfad1, 1000, NULL, NULL);
		//cPfad1[nSize] = '\0';
		//nSize = (int)wcslen(wcPfad2Unix);
		//WideCharToMultiByte(CP_ACP, 0, wcPfad2Unix, nSize, cPfad2, 1000, NULL, NULL);
		//cPfad2[nSize] = '\0';
		//AfxMessageBox((CString)"3: " + (CString)cPfad1 + " -- 4: " + (CString)cPfad2);

		// dosFn now contains "Y:\Documents\test.txt"

		BOOL bVergleich = strcmp(cPfad1Unix, cPfad2Unix);

		HeapFree(GetProcessHeap(), 0, cPfad1Unix);
		HeapFree(GetProcessHeap(), 0, cPfad2Unix);
		//HeapFree(GetProcessHeap(), 0, dosFn);
		FreeLibrary(kernel32);

		return !bVergleich;
	}
	else
		return !stricmp(pfad1, pfad2);
}


/////////////////////////////////////////////////////////////////////////////
// CEasyCashDoc commands


/////////////////////////////////////////////////////////
//
// Hilfsfunktionen
//

void CEasyCashDoc::SavePublic()
{
	char lpszPathName[500], IniFileName[500];
	CDocument::OnFileSaveAs();
	strcpy(lpszPathName, GetPathName());
	GetIniFileName(IniFileName, sizeof(IniFileName));
	if (*lpszPathName) WritePrivateProfileString("Allgemein", "LetzteDatei", lpszPathName, IniFileName);
}

void CEasyCashDoc::OnFileSaveAs() 
{
	char lpszPathName[500], IniFileName[500];
	char lpszDatenverzeichnis[500];
	GetIniFileName(lpszDatenverzeichnis, 500);
	char *cp1 = strrchr(lpszDatenverzeichnis, '\\');
	if (cp1)
	{
		strcpy(lpszPathName, (LPCTSTR)GetPathName());
		char *cp2 = strrchr(lpszPathName, '\\');
		if (cp2)
		{
			strcpy(cp1, cp2);
			SetPathName((CString)lpszDatenverzeichnis);
		}
	}
	CDocument::OnFileSaveAs();
	strcpy(lpszPathName, GetPathName());
	GetIniFileName(IniFileName, sizeof(IniFileName));
	if (*lpszPathName) WritePrivateProfileString("Allgemein", "LetzteDatei", lpszPathName, IniFileName);
}

void CEasyCashDoc::OnFileSave() 
{
	char lpszPathName[500], IniFileName[500];	
	/*nicht länger Speichern im Datenverzeichnis forcieren:
	char lpszDatenverzeichnis[500];
	GetIniFileName(lpszDatenverzeichnis, 500);
	char *cp1 = strrchr(lpszDatenverzeichnis, '\\');
	if (cp1)
	{
		strcpy(lpszPathName, (LPCTSTR)GetPathName());
		char *cp2 = strrchr(lpszPathName, '\\');
		if (cp2)
		{
			strcpy(cp1, cp2);
			SetPathName((CString)lpszDatenverzeichnis);
		}
	}*/
	CDocument::OnFileSave();
	strcpy(lpszPathName, GetPathName());
	GetIniFileName(IniFileName, sizeof(IniFileName));
	if (*lpszPathName) WritePrivateProfileString("Allgemein", "LetzteDatei", lpszPathName, IniFileName);
}

void CEasyCashDoc::OnWiederherstellungsdateiSave()
{
	char path[1000];
	strcpy(path, m_strPathName);
	char* pFileExtension = path + strlen(path) - 4;
	if (strlen(path) >= 4 && !strcmp(pFileExtension, ".eca"))
	{
		strcpy(pFileExtension, ".~eca");
		BOOL bAlterModifyStatus = IsModified();
		CDocument::OnSaveDocument(path);
		CDocument::SetModifiedFlag(bAlterModifyStatus);
	}
}

void CEasyCashDoc::CheckWiederherstellungsdatei(LPCTSTR dateipfad)
{
	char pfadWiederherstellungsdatei[1000];
	strcpy(pfadWiederherstellungsdatei, dateipfad);
	char* pFileExtension = pfadWiederherstellungsdatei + strlen(pfadWiederherstellungsdatei) - 4;
	if (strlen(pfadWiederherstellungsdatei) >= 4 && !strcmp(pFileExtension, ".eca"))
	{
		strcpy(pFileExtension, ".~eca");
		DWORD dw = GetFileAttributes(pfadWiederherstellungsdatei);
		if (dw == 0xFFFFFFFF) return;  // keine Wiederherstellungsdatei gefunden --> alles ok

		CTime dateiZuletztModifiziert = GetFileModifiedTime(dateipfad);
		CTime wiederherstellungsdateiZuletztModifiziert = GetFileModifiedTime(pfadWiederherstellungsdatei);

		CString csMessage;
		if (wiederherstellungsdateiZuletztModifiziert > dateiZuletztModifiziert)
		{
			csMessage.Format("Es wurde eine Wiederherstellungsdatei '%s' gefunden, die neuer ist als die zu öffnende Buchungsdatei '%s'. "
				"Die Wiederherstellungsdatei enthält normalerweise letzte Änderungen, die noch nicht in der Buchungsdatei gespeichert wurden, "
				"weil EC&T unerwartet beendet wurde, z.B. bei einem Stromausfall oder Systemabsturz. "
				"Soll die ältere Buchungsdatei jetzt mit der neueren Wiederherstellungsdatei ersetzt werden?", pfadWiederherstellungsdatei, dateipfad);
			if (AfxMessageBox(csMessage, MB_YESNO) == IDYES)
			{
				DeleteFile((CString)dateipfad + "_kann_geloescht_werden");  // ggf. uraltes Backup löschen
				if (MoveFile(dateipfad, (CString)dateipfad + "_kann_geloescht_werden"))
				{
					csMessage = "Die Datei '" + (CString)dateipfad + "' wurde in '" + (CString)dateipfad + "_kann_geloescht_werden' umbenannt.\r\n";
					if (MoveFile(pfadWiederherstellungsdatei, dateipfad))
						csMessage += "Die Wiederherstellungsdatei '" + (CString)pfadWiederherstellungsdatei + "' wurde in '" + (CString)dateipfad + "' umbenannt.\r\n";
					else
					{
						csMessage += "Die Datei '" + (CString)pfadWiederherstellungsdatei + "' konnte nicht umbenannt werden. Bitte versuche die .~eca-Datei manuell im Windows-Explorer in eine .eca-Datei umzubenennen.\r\n";
						if (MoveFile((CString)dateipfad + "_kann_geloescht_werden", dateipfad))
							csMessage += "Die Änderung des Dateinamens in '" + (CString)dateipfad + "_kann_geloescht_werden' wurde rückgängig gemacht.\r\n";
						else
							csMessage += "Die Änderung des Dateinamens in '" + (CString)dateipfad + "_kann_geloescht_werden' konnte nicht rückgängig gemacht werden, sorry.\r\n";
					}
				}
				else
					csMessage = "Die Datei '" + (CString)dateipfad + "' konnte nicht umbenannt werden, um sie mit der Wiederherstellungs-Version zu ersetzten. Bitte versuche die jetzige .eca-Datei manuell im Windows-Explorer umzubenennen und danach die .~eca-Datei in eine .eca-Datei. Die alte Datei ohne die letzten Änderungen wird jetzt gleich geöffnet.\r\n";

				AfxMessageBox(csMessage);
			}
			else
				if (AfxMessageBox("Ok, soll die Wiederherstellungsdatei dann jetzt gelöscht werden, damit diese Meldung beim nächsten Öffnen nicht mehr erscheint?", MB_YESNO) == IDYES)
					DeleteFile(pfadWiederherstellungsdatei);
		}
		else if (wiederherstellungsdateiZuletztModifiziert == dateiZuletztModifiziert)  // Dateien sind mutmaßlich identisch
			DeleteFile(pfadWiederherstellungsdatei);
		else // if (wiederherstellungsdateiZuletztModifiziert < dateiZuletztModifiziert)
		{
			csMessage.Format("Es wurde eine Wiederherstellungsdatei '%s' gefunden, die älter ist als die zu öffnende Buchungsdatei '%s'. "
				"(Die Wiederherstellungsdatei enthält normalerweise letzte Änderungen, die noch nicht in der Buchungsdatei gespeichert wurden, "
				"weil EC&&T unerwartet beendet wurde, z.B. bei einem Stromausfall oder Systemabsturz.) Die mutmaßlich neuere Buchungsdatei wird gleich geöffnet. "
				"Soll die wahrscheinlich veraltete Wiederherstellungsdatei jetzt gelöscht werden, damit diese Meldung beim nächsten Öffnen nicht mehr erscheint?", pfadWiederherstellungsdatei, dateipfad);
			if (AfxMessageBox(csMessage, MB_YESNO) == IDYES)
				DeleteFile(pfadWiederherstellungsdatei);
		}
	}
}

void CEasyCashDoc::LoescheWiederherstellungsdatei(LPCTSTR dateipfad)
{
	char pfadWiederherstellungsdatei[1000];
	strcpy(pfadWiederherstellungsdatei, dateipfad);
	char* pFileExtension = pfadWiederherstellungsdatei + strlen(pfadWiederherstellungsdatei) - 4;
	if (strlen(pfadWiederherstellungsdatei) >= 4 && !strcmp(pFileExtension, ".eca"))
	{
		strcpy(pFileExtension, ".~eca");
		DWORD dw = GetFileAttributes(pfadWiederherstellungsdatei);
		if (dw == 0xFFFFFFFF) return;  // keine Wiederherstellungsdatei gefunden --> alles ok
		DeleteFile(pfadWiederherstellungsdatei);
	}
}

CTime CEasyCashDoc::GetFileModifiedTime(LPCTSTR path)
{
	CTime ctReturn;
	CFile cfile;
	CFileStatus status;

	cfile.Open(path, CFile::modeRead);
	if (cfile.GetStatus(status))
		ctReturn = status.m_mtime;
	return ctReturn;
}

//HINSTANCE hEasyCTXP_DLL = NULL;

BOOL CEasyCashDoc::Check4Backup()
{
	int nRet = 5; // abbrechen, wenn Zeit noch nicht um...
	int nAlterNachfrageIntervall = nNachfrageIntervall;
	CTime now = CTime::GetCurrentTime();

	if (nNachfrageIntervall != 7 && nNachfrageIntervall != 30)
		nNachfrageIntervall = 7;

	if (now >= ctNachfrageTermin)
	{
/*		OSVERSIONINFOEX osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (GetVersionEx((OSVERSIONINFO *)&osvi) && osvi.dwMajorVersion >= 6) // Vista or higher?
		{
			if (!(hEasyCTXP_DLL = LoadLibrary("EasyCTU.dll")))
			{
				AfxMessageBox("Fehler beim Öffnen des Datensicherungs-Nachfragedialogs. EasyCTXP.dll ist evtl. nicht installiert.");
				return;
			}
			else
			{
				BOOL (__cdecl *pBackupOptions)(int *);
				pBackupOptions = (int (__cdecl *)(int *))GetProcAddress(hEasyCTXP_DLL, "BackupOptions");
				if (pBackupOptions) 
				{
					int nRet = (*pBackupOptions)(&nNachfrageIntervall);
*/				
				// Command-Buttons (ab Vista) in Unicode DLL funktionieren nicht, also häßliche Normal-Buttons benutzen:
				CDatensicherungsoptionen dlg(&nNachfrageIntervall);
				nRet = dlg.DoModal();

				switch (nRet)
					{
						case 1: 
							SendMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_FILE_BACKUP_SUBFOLDER, 0L);
							break;
						case 2: 
							SendMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_FILE_BACKUP_EXTERNAL, 0L);
							break;
						case 3: 
							SendMessage(AfxGetMainWnd()->m_hWnd, WM_COMMAND, ID_FILE_BACKUP, 0L);
							break;
					}

					if (nRet != 5) // es wurde nicht Abgebrochen?
					{
						ctNachfrageTermin = now - CTimeSpan(0, now.GetHour(), now.GetMinute(), now.GetSecond()) + CTimeSpan(nNachfrageIntervall, 0, 0, 0);
					}
/*				}
				else
				{
					AfxMessageBox("Fehler beim Öffnen des Datensicherungs-Nachfragedialogs. (SelectFolder() Einsprungpunkt nicht gefunden.) EasyCTXP.dll ist evtl. nicht richtig installiert.");
					return;
				}
			}	
		}
		else // XP
		{	
			; // sorry, XP kriegt kein Backup-Nacfragedialog mehr
		}
*/	}

	// haben sich nAlterNachfrageIntervall oder ctNachfrageTermin geändert? dann noch einmal speichern
	if (nRet != 5 || nAlterNachfrageIntervall != nNachfrageIntervall)
		return true;
	else
		return false;
}

BOOL CDocument::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is NULL then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty())
	{
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			newName = m_strTitle;
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(" #%;/\\"));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
				!strExt.IsEmpty())
			{
			  ASSERT(strExt[0] == '.');

			  int nSemi;                       
			  if((nSemi = strExt.Find(';')) > 0)
				strExt = strExt.Left(nSemi);   

			  newName += strExt;
			}
		}

		if (!AfxGetApp()->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
			return FALSE;       // don't even attempt to save
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName))	// access violation v2.51.0.1-4f3ed12a-3f18-4a7a-a24b-723ae643ef04 v2.51.0.1-5f693222-6021-4cf6-8186-16f2366c678a v2.51.0.1-6bc724f0-8faf-40dc-92af-2e5dc3f53cf9 v2.51.0.1-6ee46e2e-b0a4-48d5-b984-674212ee88a8 v2.51.0.1-20df0361-4d26-4c29-a5ea-35e6b25f93ad v2.51.0.1-58ebdf74-6b66-433e-9d6d-1c8a73d355a9 v2.51.0.1-7802dff4-f939-4120-8980-be4544a4124e
	{								//					v2.51.0.1-9d4cbb5d-c8e9-4452-9fe7-673648049f6d, v2.51.0.1-8656d890-3bf1-4898-80da-2d6124920760 und v2.51.0.1-780f0825-abfb-4d23-b12d-26da2e2ccd06 crasht etwas tiefer in Serialize
		if (lpszPathName == NULL)	
		{
			// be sure to delete the file
			TRY
			{
				CFile::Remove(newName);
			}
			CATCH_ALL(e)
			{
				TRACE0("Warning: failed to delete file after failed SaveAs.\n");
				// e->Delete();     <-- don't delete the exception when using macros!
			}
			END_CATCH_ALL
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);
	
	char lpszDatenverzeichnis[500];
	GetIniFileName(lpszDatenverzeichnis, 500);
	char *cp1 = strrchr(lpszDatenverzeichnis, '\\');
	if (cp1)
	{
		*cp1 = '\0';		
		char *cp2 = strrchr(newName.GetBuffer(0), '\\');
		if (cp2)
		{
			*cp2 = '\0';
			if (!DatenverzeichnisCheck(lpszDatenverzeichnis, newName.GetBuffer(0)))
			{
				newName.ReleaseBuffer();
				AfxMessageBox((CString)"Hinweis: Die Datei wurde nicht im gewählten Datenverzeichnis '" + lpszDatenverzeichnis + "' gespeichert, wo EasyCash&Tax sie erwartet. So etwas ist ok, wenn man etwas ausprobieren und mit einer Kopie arbeiten möchte, aber die offizielle Buchungsdatei sollte auf jeden Fall im Datenverzeichnis gespeichert werden, z.B. um bei der eingebauten Datensicherung berücksichtigt zu werden. Wenn dies die offiziellen Buchführungsdaten sind, wird dringend empfohlen sie von '" + newName + " nach '" + lpszDatenverzeichnis + "' zu verschieben. Eventuell ist aber auch nur das Datenverzeichnis falsch gesetzt. In diesem Fall kann das mit 'Datenverzeichnis neu wählen' im Appliaktionsmenü (der runde Knopf links oben) nachgeholt werden.");
			}
			else
			{
				if (((CEasyCashDoc *)this)->Check4Backup())
					CDocument::OnFileSave(); // noch einmal mit neuem Backup-Nachfragetermin speichern
			}
		}
	}

	return TRUE;        // success
}

//CFile debugfile;

void CEasyCashDoc::Sort()
{
//AfxMessageBox("C:\\ProgramData\\EasyCash&Tax\\ectdebug.txt");
//debugfile.Open("C:\\ProgramData\\EasyCash&Tax\\ectdebug.txt", CFile::modeCreate|CFile::modeWrite);
	SortSpecial(&Einnahmen);
	SortSpecial(&Ausgaben);
//debugfile.Close();
}

void CEasyCashDoc::SortSingle(CBuchung **bpp_base, CBuchung **bpp_einzufuegen)
{
	CBuchung **bpp, **bpp_hier_einfuegen = NULL;

	// wieviele Elemente in der Liste?
	bpp = bpp_base;
	while (*bpp)
	{
		if (*bpp != *bpp_einzufuegen) // wenn sich das Datum des einzufügenden Elements geändert hat, muss es bei der Berechnung der Position ignoriert werden!
		{	
			if ((*bpp)->Datum == (*bpp_einzufuegen)->Datum)
			{
				if ((*bpp)->Belegnummer + (CString)" " + (*bpp)->Beschreibung > (*bpp_einzufuegen)->Belegnummer + (CString)" " + (*bpp_einzufuegen)->Beschreibung)
				{
					if (!bpp_hier_einfuegen)
					{
						bpp_hier_einfuegen = bpp;
						break;
					}
				}
				
			}
			else if ((*bpp)->Datum > (*bpp_einzufuegen)->Datum)
			{
				if (!bpp_hier_einfuegen)
				{
					bpp_hier_einfuegen = bpp;
					break;
				}
			}
		}

		bpp = &((*bpp)->next);
	}

	if (bpp_hier_einfuegen)
	{
		if (*bpp_hier_einfuegen == (*bpp_einzufuegen)->next) return; // schon an richtiger Stelle
		CBuchung *bptemp;
		bptemp = *bpp_einzufuegen;	// zwischenspeichern
		*bpp_einzufuegen = (*bpp_einzufuegen)->next;	// aus der Kette genommen
		bptemp->next = (*bpp_hier_einfuegen);	// und wieder 
		*bpp_hier_einfuegen = bptemp;					// einfügen
	}
	else
		SortSpecial(bpp_base);	// irgendwas ist schief gelaufen, machen wir dann doch lieber 'old style'
}

void CEasyCashDoc::SortSpecial(CBuchung **bpp_base)
{
	int count = 0;
	int i;
	CBuchung *bp;

	// wieviele Elemente in der Liste?
	bp = *bpp_base;
	while (bp)
	{
		count++;
		bp = bp->next;
	}

	/* BubbleSort war zu langsam bei 10000en Buchungen, aber QuickSort ist da auch nicht mehr wirklich schnell :( 
	CBuchung **bpp, *bp2, *bp3;
	// Sortieren (Bubble)
	for (i = 0; i < count; i++)
	{
		bpp = bpp_base;
		
		while (*bpp)
		{
			if ((*bpp)->next)
			{
				if ((*bpp)->Datum == (*bpp)->next->Datum)
				{
					if ((*bpp)->Belegnummer + (CString)" " + (*bpp)->Beschreibung > (*bpp)->next->Belegnummer + (CString)" " + (*bpp)->next->Beschreibung)
					{
						bp = *bpp;
						bp2 = (*bpp)->next;
						bp3 = (*bpp)->next->next;

						*bpp = bp2;
						(*bpp)->next = bp;
						bp->next = bp3;
					}	
				}
				else if ((*bpp)->Datum > (*bpp)->next->Datum)
				{
					bp = *bpp;
					bp2 = (*bpp)->next;
					bp3 = (*bpp)->next->next;

					*bpp = bp2;
					(*bpp)->next = bp;
					bp->next = bp3;
				}	
			}

			bpp = &((*bpp)->next);
		}
	}
*/

	/* Quicsort */
	void **bparray = new PVOID[count];

	// array aus Kette erzeugen ...
	for (bp = *bpp_base, i = 0; bp; i++)
	{
		bparray[i] = (PVOID)bp;
		bp = bp->next;
		((CBuchung *)bparray[i])->next = NULL; // ... und dabei Kette dissoziieren
	}

	SortQuick(bparray, 0, count-1);

	// neue kette aus array erzeugen
	if (count)
	{
		*bpp_base = (CBuchung *)*bparray;	// erstes element in bpp_base speichern
		for (i = 0; i < count-1; i++)
		{
			PVOID pvtemp = bparray[i+1];
			((CBuchung *)bparray[i])->next = (CBuchung *)pvtemp;
		}
		((CBuchung *)bparray[i])->next = NULL;
	}

	delete[] bparray;

}

void CEasyCashDoc::SortQuick(void **bparray, int links, int rechts)
{
	if (links < rechts)
	{
//		int teiler;
		int i = links;
		int j = rechts;
		CTime pivot = ((CBuchung *)bparray[(rechts+links)/2])->Datum;
		CString pivot_plus;
		pivot_plus.Format("%04d-%02d-%02d %s %s", pivot.GetYear(), pivot.GetMonth(), pivot.GetDay(), ((CBuchung *)bparray[(rechts+links)/2])->Belegnummer, (LPCTSTR)((CBuchung *)bparray[(rechts+links)/2])->Beschreibung);
		CString iVal, jVal;

/*CString pivot_gesamt;
pivot_gesamt.Format("Links: %d  Rechts: %d  Pivot: %04d-%02d-%02d ", links, rechts, pivot.GetYear(), pivot.GetMonth(), pivot.GetDay());
pivot_gesamt += ((CBuchung *)bparray[rechts])->Belegnummer + (CString)" " + ((CBuchung *)bparray[rechts])->Beschreibung + "\r\n";
debugfile.Write(pivot_gesamt, pivot_gesamt.GetLength());*/

		while (i <= j) // solange i an j nicht vorbeigelaufen ist 
		{

			// Suche von links ein Element, welches größer als das Pivotelement ist
			while (TRUE)
			{
				iVal.Format("%04d-%02d-%02d %s %s", ((CBuchung *)bparray[i])->Datum.GetYear(), ((CBuchung *)bparray[i])->Datum.GetMonth(), ((CBuchung *)bparray[i])->Datum.GetDay(), (LPCTSTR)((CBuchung *)bparray[i])->Belegnummer, (LPCTSTR)((CBuchung *)bparray[i])->Beschreibung);
				if (iVal >= pivot_plus) break;
				i++;
			}

			// Suche von rechts ein Element, welches kleiner als das Pivotelement ist
			while (TRUE)
			{
				jVal.Format("%04d-%02d-%02d %s %s", ((CBuchung *)bparray[j])->Datum.GetYear(), ((CBuchung *)bparray[j])->Datum.GetMonth(), ((CBuchung *)bparray[j])->Datum.GetDay(), (LPCTSTR)((CBuchung *)bparray[j])->Belegnummer, (LPCTSTR)((CBuchung *)bparray[j])->Beschreibung);
				if (jVal <= pivot_plus) break;
				j--;
			}

			if (i <= j) // austauschen
			{
/*CString autausch;
CString istring, jstring;
istring.Format("%04d-%02d-%02d %s %s", ((CBuchung *)bparray[i])->Datum.GetYear(), ((CBuchung *)bparray[i])->Datum.GetMonth(), ((CBuchung *)bparray[i])->Datum.GetDay(), (LPCTSTR)((CBuchung *)bparray[i])->Belegnummer, (LPCTSTR)((CBuchung *)bparray[i])->Beschreibung);
jstring.Format("%04d-%02d-%02d %s %s", ((CBuchung *)bparray[j])->Datum.GetYear(), ((CBuchung *)bparray[j])->Datum.GetMonth(), ((CBuchung *)bparray[j])->Datum.GetDay(), (LPCTSTR)((CBuchung *)bparray[j])->Belegnummer, (LPCTSTR)((CBuchung *)bparray[j])->Beschreibung);
autausch.Format("   Ausgetauscht i: %03d (%s) mit j: %03d (%s)\r\n", i, istring, j, jstring);
debugfile.Write(autausch, autausch.GetLength());*/

				PVOID pvtemp = bparray[i];
				bparray[i++] = bparray[j];
				bparray[j--] = pvtemp;
			}		
		}		

		// Tausche Pivotelement (daten[rechts]) mit neuer endgültiger Position (daten[i])

/*		if (((CBuchung *)bparray[i])->Datum > pivot || ((CBuchung *)bparray[i])->Datum == pivot && ((CBuchung *)bparray[i])->Belegnummer + (CString)" " + ((CBuchung *)bparray[i])->Beschreibung >= pivot_plus)
		{
			PVOID pvtemp = bparray[i];
			bparray[i] = bparray[rechts];
			bparray[rechts] = pvtemp;
		}

		teiler = i;

		SortQuick(bparray, links, teiler-1);
		SortQuick(bparray, teiler+1, rechts);*/


		if (links < j)
			SortQuick(bparray, links, j);
		if (i < rechts)
			SortQuick(bparray, i, rechts);
	}
}

// Konvertiert das ganze dokument nach Euro
BOOL CEasyCashDoc::ConvertToEuro()
{
	CBuchung *bp;
	CDauerbuchung *pdb;

	for (bp = Einnahmen; bp; bp = bp->next)
		if (!bp->ConvertToEuro(csWaehrung)) return FALSE;

	for (bp = Ausgaben; bp; bp = bp->next)
		if (!bp->ConvertToEuro(csWaehrung)) return FALSE;

	for (pdb = Dauerbuchungen; pdb; pdb = pdb->next)
		if (!pdb->ConvertToEuro(csWaehrung)) return FALSE;

	SetModifiedFlag("Die Buchungsdaten wurden in Euro umgerechnet");
	csWaehrung = "EUR";
	return TRUE;
}

char *CEasyCashDoc::GetWaehrungskuerzel(int i)
{
	return waehrungskuerzel[i];
}

double CEasyCashDoc::GetUmrechnungskurs(int i)
{
	return kurse[i];
}

void CEasyCashDoc::InkrementBuchungszaehler()
{
	if (shareware && !(++Buchungszaehler % 20))
	{
		char buf[5000];

		sprintf(buf, 
"Es sind jetzt schon %d Buchungen mit diesem Programm durchgeführt worden. \
Um die Weiterentwicklung dieser Software zu fördern besteht die Möglichkeit, sich \
registrieren zu lassen und einen Registriercode zu erhalten. \
Das hat zwar erst einmal nur den Effekt, dass nicht bei jeder 20sten Buchung diese \
Meldung erscheint. Aber langfristig motiviert es den Autor, die Software weiter zu entwickeln und zu pflegen, \
wenn er weiß, dass sein Programm von vielen Menschen benutzt wird. Außerdem kann er besser schlafen, weil er \
für den Fall, dass er trotz aller Sorgfalt mal einen gravierenden Fehler einbaut, schnell alle Nutzer darüber \
informieren kann, sich ein Update zu installieren, der den Fehler behebt.\n\n\
Wer darüber hinaus seine Wertschätzung zum Ausdruck bringen möchte, kann eine Spenden-Registrierung durchführen \
(Vorschlag: 15€ wer die Software ganz ok findet; eine Rechnung zum Absetzen als Betriebsausgabe ist obligatorisch). \
Überweisung mit E-Mail-Adresse im Verwendungszweck + Bestätigungs-Mail an thomas@mielke.software (Bankverbindung: \
DAB, BLZ 70120400, Konto 7406319017, SWIFT-BIC: DABBDEMMXXX, IBAN: DE54701204007406319017 -- \
drücke einfach einen der Spenden-Knöpfe im Optionen-Bereich des Hauptmenüs). \
Die Registrierung gilt natürlich auch für alle Folgeversionen. Mehr Informationen zum Registrierungsprozess gibt es unter 'Info'.\n\n\
(Hinweis: Wer bereits einen Registriercode hat und die Software auf einem neuen Computer installiert hat, kann ihn unter 'Info -> \
Registrierung' eingeben -- oder wer einmal die 'Registrierungsinformationen sichern' Funktion ausgeführt und eine .ectreg-Datei \
hat kann mit 'Registrierungsinformationen wiederherstellen' die Software auch ohne Code-Eingabe wieder in den 'Registriert'-Zustand versetzen.)", Buchungszaehler);

		AfxMessageBox(buf, MB_ICONINFORMATION|MB_OK);
	}
}

// Es gibt zwei Listen für Buchungen (CBuchung) in CEasyCashDocument:
// eine für Einnahmen und eine für Ausgaben
int CEasyCashDoc::EinnahmenSumme(int MonatsFilter, LPCTSTR KontoFilter)
{
	int einnahmen_summe = 0;
	CBuchung *p;

	// Summe der Einnahmen bilden:
	for (p = Einnahmen; p; p = p->next)
	{
		if (MonatsFilter)
		{
			if (MonatsFilter <= 12)
			{
				if (p->Datum.GetMonth() != MonatsFilter) continue;
			}
			else
			{
				if (((p->Datum.GetMonth()-1)/3)+1 != MonatsFilter-12) continue;
			}
		}
		if (*KontoFilter && p->Konto == KontoFilter) continue;
		if (!p->Konto.IsEmpty() && p->Konto[0] == '/') continue;	// keine neutralen Konten mitzählen
		einnahmen_summe += p->Betrag;
	}

	return einnahmen_summe;
}

int CEasyCashDoc::AusgabenSumme(int MonatsFilter, LPCTSTR KontoFilter)
{
	int ausgaben_summe = 0;
	CBuchung *p;

	// Summe der Ausgaben bilden:
	for (p = Ausgaben; p; p = p->next)
	{
		if (MonatsFilter)
		{
			if (MonatsFilter <= 12)
			{
				if (p->Datum.GetMonth() != MonatsFilter) continue;
			}
			else
			{
				if (((p->Datum.GetMonth()-1)/3)+1 != MonatsFilter-12) continue;
			}
		}
		if (*KontoFilter && p->Konto == KontoFilter) continue;
		if (!p->Konto.IsEmpty() && p->Konto[0] == '/') continue;	// keine neutralen Konten mitzählen
		if (p->AbschreibungJahre > 1)
		{
			// Abschreibung berechnen
			long mwst_betrag, netto;
			netto = p->GetBuchungsjahrNetto(this);

			// MwSt-Betrag berechnen
			if (p->AbschreibungNr == 1)	// nur im erstern Jahr Vorsteuer berechnen!
				mwst_betrag = p->Betrag - p->GetNetto();
			else
				mwst_betrag = 0;
		
			ausgaben_summe += netto + mwst_betrag;
		}
		else
//			if (p->AbschreibungNr == 1) // nur rein monetäre Umsätze berücksichtigen, ohne Anlagevermögen
			ausgaben_summe += p->Betrag;
	}

	return ausgaben_summe;
}

int CEasyCashDoc::EinnahmenSummeNetto(int MonatsFilter, LPCTSTR KontoFilter)
{
	int einnahmen_summe = 0;
	CBuchung *p;

	// Summe der Einnahmen bilden:
	for (p = Einnahmen; p; p = p->next)
	{
		if (MonatsFilter)
		{
			if (MonatsFilter <= 12)
			{
				if (p->Datum.GetMonth() != MonatsFilter) continue;
			}
			else
			{
				if (((p->Datum.GetMonth()-1)/3)+1 != MonatsFilter-12) continue;
			}
		}
		if (*KontoFilter && p->Konto == KontoFilter) continue;
		if (!p->Konto.IsEmpty() && p->Konto[0] == '/') continue;	// keine neutralen Konten mitzählen
		einnahmen_summe += p->GetNetto();
	}

	return einnahmen_summe;
}

int CEasyCashDoc::AusgabenSummeNetto(int MonatsFilter, LPCTSTR KontoFilter)
{
	int ausgaben_summe = 0;
	CBuchung *p;

	// Summe der Ausgaben bilden:
	for (p = Ausgaben; p; p = p->next)
	{
		if (MonatsFilter)
		{
			if (MonatsFilter <= 12)
			{
				if (p->Datum.GetMonth() != MonatsFilter) continue;
			}
			else
			{
				if (((p->Datum.GetMonth()-1)/3)+1 != MonatsFilter-12) continue;
			}
		}
		if (*KontoFilter && p->Konto == KontoFilter) continue;
		if (!p->Konto.IsEmpty() && p->Konto[0] == '/') continue;	// keine neutralen Konten mitzählen
		ausgaben_summe += p->GetBuchungsjahrNetto(this);
	}

	return ausgaben_summe;
}

XDoc *CEasyCashDoc::GetFormular(CString &csFormularname)
{
	CString csFilename = csFormularname;

	// kein Pfad im Dateinamen gefunden? klebe Programmverzeichnis davor:
	if (csFormularname.Find("\\", 0) == -1)
	{
		char path[1000];
		if (GetProgrammverzeichnis(path, sizeof(path)))
		{
			csFilename = (CString)path + csFilename;
		}
	}

	// Formulardefinitionsdatei in xmldoc laden
	XDoc *pxmldoc = new XDoc;
	pxmldoc->Load(csFilename);
	return pxmldoc;
}

// benötigt für GetFormularwertXXX Funktionen
void CEasyCashDoc::ReadKontenCache()
{
	m_csEinnahmenKonten.RemoveAll();
	m_csEinnahmenFeldzuweisungen.RemoveAll();
	m_csAusgabenKonten.RemoveAll();
	m_csAusgabenFeldzuweisungen.RemoveAll();

	m_csEinnahmenKonten.SetSize(100);
	m_csEinnahmenFeldzuweisungen.SetSize(100);
	m_csAusgabenKonten.SetSize(100);
	m_csAusgabenFeldzuweisungen.SetSize(100);

	// Einlesen der Einnahmen- und Ausgaben-Konten und -Feldzuweisungen
	char buffer[10000];
	char EasyCashIniFilenameBuffer[1000];
	if (!GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer))) return;
	int i;
	for (i = 0; i < 100; i++)
	{
		char key_buffer[100];				
		sprintf(key_buffer, "%2.2d", i);

		GetPrivateProfileString("EinnahmenRechnungsposten", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		m_csEinnahmenKonten[i] = buffer;

		GetPrivateProfileString("EinnahmenFeldzuweisungen", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		m_csEinnahmenFeldzuweisungen[i] = buffer;

		GetPrivateProfileString("AusgabenRechnungsposten", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		m_csAusgabenKonten[i] = buffer;

		GetPrivateProfileString("AusgabenFeldzuweisungen", key_buffer, "", buffer, sizeof(buffer), EasyCashIniFilenameBuffer);
		m_csAusgabenFeldzuweisungen[i] = buffer;
	}

	// Felder werden hier gespeichert wenn die ID zwischen 0 und 9999 liegt
	for(i = 0; i < 10000; i++)
	{
		m_laFeldWerte[i] = 0L;
		m_bFeldWertGueltig[i] = FALSE;
	}
	m_csaFeldStatustext.RemoveAll();
	m_csaFeldStatustext.SetSize(10000);
}

int CEasyCashDoc::GetFormularwertIDByIndex(CString &csFormularname, int nIndex)
{
	XDoc *pxmldoc = GetFormular(csFormularname);
	int nID = 0;
	if (pxmldoc)
	{
		LPXNode xml = pxmldoc->GetRoot();
		LPXNode felder = NULL;
		felder = xml->Find("felder");
		if (felder)
		{
			LPXNode child;
			child = felder->GetChild(nIndex);
			nID = atoi(child->GetAttrValue("id"));
		}

		delete pxmldoc;
	}

	return nID;
}

// GetFormularwertXXXX -- liefert den Wert eines bestimmten Formularfeldes, 
// indiziert nach Formularnamen bzw. Felder-Node und Feld-ID bzw -Index
//
// dazugekommen ab v1.30 oder CDoc-VERSION 9
CString CEasyCashDoc::GetFormularwertByID(CString &csFormularname, int nID, LPCSTR sFilter)	// dazugekommen ab v1.30 oder CDoc-VERSION 9
{
	XDoc *pxmldoc = GetFormular(csFormularname);
	CString csRet;
	if (pxmldoc)
	{
		LPXNode xml = pxmldoc->GetRoot();
		LPXNode felder = NULL;
		felder = xml->Find("felder");

		if (felder) 
			csRet = GetFormularwertByID(pxmldoc, nID, sFilter);

		delete pxmldoc;
	}
 
	return csRet;
}


CString CEasyCashDoc::GetFormularwertByIndex(CString &csFormularname, int nIndex, LPCSTR sFilter)
{
	XDoc *pxmldoc = GetFormular(csFormularname);
	CString csRet;
	if (pxmldoc)
	{
		LPXNode xml = pxmldoc->GetRoot();
		LPXNode felder = NULL;
		felder = xml->Find("felder");

		if (felder) 
			csRet = GetFormularwertByIndex(pxmldoc, nIndex, sFilter);

		delete pxmldoc;
	}
 
	return csRet;
}

CString CEasyCashDoc::GetFormularwertByID(XDoc *pFormular, int nID, LPCSTR sFilter)
{
	if (!pFormular) return "";

	LPXNode xml = pFormular->GetRoot();
	LPXNode felder = NULL;
	felder = xml->Find("felder");
	LPXNode child;

	for(int i = 0; i < felder->GetChildCount(); i++)
	{
		child = felder->GetChild(i);
		
		if (child && !child->value.IsEmpty())
		{
			CString csAttr = child->GetAttrValue("id");
			if (nID == atoi(csAttr))
				return GetFormularwertByIndex(pFormular, i, sFilter);
		}
	}

	return "";
}

CString CEasyCashDoc::GetFormularwertByIndex(XDoc *pFormular, int nIndex, LPCSTR sFilter)
{
	if (!pFormular) return "";
	if (!sFilter) sFilter = "";

	LPXNode xml = pFormular->GetRoot();
	LPXNode felder = NULL;
	LPCTSTR attr_voranmeldungszeitraum = NULL;
	if (xml) attr_voranmeldungszeitraum = xml->GetAttrValue("voranmeldungszeitraum");
	BOOL bFilterNachVoranmeldungszeitraum = FALSE;
	if (attr_voranmeldungszeitraum && (m_nZeitraum = atoi(attr_voranmeldungszeitraum))) bFilterNachVoranmeldungszeitraum = TRUE;
	LPCTSTR attr_filter = NULL; 
	if (xml) attr_filter = xml->GetAttrValue("filter");
	BOOL bFilterNachBetrieb = FALSE;
	felder = xml->Find("felder");
	LPXNode child;
	child = felder->GetChild(nIndex);
	int nID = atoi(child->GetAttrValue("id"));	

	m_bFeldWertGueltig[nID] = TRUE; // um deadlocks durch rekursionen zu vermeiden, jetzt schon gültig machen!
	CTime von_datum, bis_datum;

	// m_nZeitraum: 1-12 Monat; 13-16 Quartal, alles andere: jährlich
	if (m_nZeitraum < 1 || m_nZeitraum > 16) bFilterNachVoranmeldungszeitraum = FALSE;	// valider Voranmeldungszeitraum?
	if (bFilterNachVoranmeldungszeitraum)
	{
		if (m_nZeitraum < 12)	// monatsweise bis November
		{
			von_datum = CTime(nJahr, m_nZeitraum, 1, 0, 0, 0);
			bis_datum = CTime(nJahr, 1+m_nZeitraum, 1, 0, 0, 0);
		}
		else if (m_nZeitraum == 12)	// monatsweise, Dezember
		{
			von_datum = CTime(nJahr, 12, 1, 0, 0, 0);
			bis_datum = CTime(nJahr+1, 1, 1, 0, 0, 0);
		}
		else if (m_nZeitraum < 16)	// quartalsweise bis 3. Quartal
		{
			von_datum = CTime(nJahr, ((m_nZeitraum-13)*3)+1, 1, 0, 0, 0);
			bis_datum = CTime(nJahr, ((m_nZeitraum-13)*3)+4, 1, 0, 0, 0);
		}
		else if (m_nZeitraum == 16)	// quartalsweise, 4. Quartal
		{
			von_datum = CTime(nJahr, 10, 1, 0, 0, 0);
			bis_datum = CTime(nJahr+1, 1, 1, 0, 0, 0);
		}
	}
	else
	{
		von_datum = CTime(nJahr, 1, 1, 0, 0, 0);
		bis_datum = CTime(nJahr+1, 1, 1, 0, 0, 0);
	}

	CString csFeldinhalt;

	if (child && !child->value.IsEmpty())
	{
		// entspr. Feldtyp behandeln:				
		if (!stricmp(child->GetAttrValue("typ"), "Einnahmen"))
		{
			// aus der Konten-Oberkategorie alle verknüpften Konten auflisten
			CStringArray csaVerknuepfteKonten;
			LPCTSTR attr_name = xml->GetAttrValue("name");
			int j;
			for (j = 0; j < 100 && !m_csEinnahmenKonten[j].IsEmpty(); j++)
			{
				if (nID == atoi(GetErweiterungKey(m_csEinnahmenFeldzuweisungen[j], "ECT", attr_name)))
					csaVerknuepfteKonten.Add(m_csEinnahmenKonten[j]);
			}

			// Statustext erzeugen
			if (csaVerknuepfteKonten.GetSize() == 0)
				m_csaFeldStatustext[nID].Format("Es gibt keine Einnahmenkonten, die mit Feld %d (%s) verknüpft sind. Das kann unter Einstellungen->E/Ü-Konten geändert werden.", nID, attr_name);
			else if (csaVerknuepfteKonten.GetSize() == 1)
				m_csaFeldStatustext[nID].Format("%s Feld %d ist mit dem Konto '%s' verknüpft.", child->GetChildValue("erweiterung"), nID, (LPCTSTR)csaVerknuepfteKonten[0]);
			else
			{
				m_csaFeldStatustext[nID].Format("%s Feld %d ist verknüpft mit den Konten ", child->GetChildValue("erweiterung"), nID);
				int i;
				for (i = 0; i < csaVerknuepfteKonten.GetSize(); i++)
					m_csaFeldStatustext[nID] += "'" + csaVerknuepfteKonten[i] + "'" + (i == csaVerknuepfteKonten.GetSize()-1 ? "" : ", ");
			}

			// jetzt summieren
			CBuchung *p;
			long summe = 0L;
			for (p = this->Einnahmen; p; p = p->next)
			{
				int j;
				for (j = 0; j < csaVerknuepfteKonten.GetSize(); j++)
					if (csaVerknuepfteKonten[j] == p->Konto 
						&& p->Datum >= von_datum && p->Datum < bis_datum 
						&& (!attr_filter || *attr_filter == '\0' ||	(!stricmp(attr_filter, "betrieb") && (!sFilter || *sFilter == '\0') || p->Betrieb == (CString)sFilter)))
					{
						if (!stricmp(child->GetAttrValue("anteil"), "netto"))
							summe += p->GetNetto();
						else if (!stricmp(child->GetAttrValue("anteil"),"mwst"))
							summe += p->Betrag - p->GetNetto();
						else // if (!child->GetAttrValue("anteil").CompareNoCase("brutto"))
							summe += p->Betrag;
						break;
					}
			}

			if (!stricmp(child->GetAttrValue("nachkommaanteil"),"ohne"))
			{
				csFeldinhalt.Format("%d", summe/100);

				// Ergebnis speichern
				if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = summe/100*100;
			}
			else //if (!child->GetAttrValue("nachkommaanteil").CompareNoCase("mit"))
			{
				int_to_currency(summe, 4, csFeldinhalt.GetBuffer(30));
				csFeldinhalt.ReleaseBuffer();

				// Ergebnis speichern
				if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = summe;
			}
		}
		else if (!stricmp(child->GetAttrValue("typ"), "Ausgaben"))
		{
			// aus der Konten-Oberkategorie alle verknüpften Konten auflisten
			LPCTSTR attr_name = xml->GetAttrValue("name");
			CStringArray csaVerknuepfteKonten;
			int j;
			for (j = 0; j < 100 && !m_csAusgabenKonten[j].IsEmpty(); j++)
			{
				if (nID == atoi(GetErweiterungKey(m_csAusgabenFeldzuweisungen[j], "ECT", attr_name)))
					csaVerknuepfteKonten.Add(m_csAusgabenKonten[j]);
			}			

			// Statustext erzeugen
			if (csaVerknuepfteKonten.GetSize() == 0)
				m_csaFeldStatustext[nID].Format("Es gibt keine Ausgabenkonten, die mit Feld %d (%s) verknüpft sind. Das kann unter Einstellungen->E/Ü-Konten geändert werden.", nID, attr_name);
			else if (csaVerknuepfteKonten.GetSize() == 1)
				m_csaFeldStatustext[nID].Format("%s Feld %d ist mit dem Konto '%s' verknüpft.", child->GetChildValue("erweiterung"), nID, csaVerknuepfteKonten[0]);
			else
			{
				m_csaFeldStatustext[nID].Format("%s Feld %d ist verknüpft mit den Konten ", child->GetChildValue("erweiterung"), nID);
				int i;
				for (i = 0; i < csaVerknuepfteKonten.GetSize(); i++)
					m_csaFeldStatustext[nID] += "'" + csaVerknuepfteKonten[i] + "'" + (i == csaVerknuepfteKonten.GetSize()-1 ? "" : ", ");
			}

			// jetzt summieren
			CBuchung *p;
			long summe = 0L;
			for (p = this->Ausgaben; p; p = p->next)
			{
				int j;
				for (j = 0; j < csaVerknuepfteKonten.GetSize(); j++)
					if (csaVerknuepfteKonten[j] == p->Konto 
						&& p->Datum >= von_datum && p->Datum < bis_datum 
						&& (!attr_filter || *attr_filter == '\0' ||	(!stricmp(attr_filter, "betrieb") && (!sFilter || *sFilter == '\0') || p->Betrieb == (CString)sFilter)))
					{
						if (!stricmp(child->GetAttrValue("anteil"), "netto"))
							summe += p->GetBuchungsjahrNetto(this);
						else if (!stricmp(child->GetAttrValue("anteil"),"mwst"))
						{
							if (p->AbschreibungNr == 1)	// nur im erstern Jahr Vorsteuer berechnen!
								summe += p->Betrag - p->GetNetto();
						}									
						else // if (!child->GetAttrValue("anteil").CompareNoCase("brutto"))
							summe += p->GetBuchungsjahrNetto(this) + (p->AbschreibungNr == 1 ? p->Betrag - p->GetNetto() : 0);
						break;
					}
			}

			if (!stricmp(child->GetAttrValue("nachkommaanteil"),"ohne"))
			{
				csFeldinhalt.Format("%d", summe/100);

				// Ergebnis speichern
				if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = summe/100*100;
			}
			else //if (!child->GetAttrValue("nachkommaanteil").CompareNoCase("mit"))
			{
				int_to_currency(summe, 4, csFeldinhalt.GetBuffer(30));
				csFeldinhalt.ReleaseBuffer();

				// Ergebnis speichern
				if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = summe;
			}
		}
		else if (!stricmp(child->GetAttrValue("typ"), "Summe"))
		{
			CString csBerechnung = child->GetChildValue("erweiterung");
			char *cp = csBerechnung.GetBuffer(0);
			char operation = '+';	// default

			// bei Vorzeichen entspr. Operation setzen:
			if (*cp == '+') cp++;
			else if (*cp == '-') { operation = '-'; cp++; }

			// einzelne Felder über ihre ID zusammenzählen
			long summe = 0L;
			while (cp && *cp)
			{
				char *cp2;
				
				if (isdigit(*cp))
				{
					int nFeldID = atoi(cp);

					while (isdigit(*cp))
						cp++;

					// Faktor und/oder Divisor zu Feld-Nr.?
					int faktor = 1;
					int divisor = 1;
					if (*cp == '*')
					{
						faktor = atoi(++cp);
						while (isdigit(*cp))
							cp++;
					}
					if (*cp == '/')
					{
						divisor = atoi(++cp);
						while (isdigit(*cp))
							cp++;
					}

					if (nFeldID >= 0 && nFeldID < 10000)
					{
						if (!m_bFeldWertGueltig[nFeldID]) GetFormularwertByID(pFormular, nFeldID, sFilter);
						if (operation == '+')
							summe += (int)((LONGLONG)m_laFeldWerte[nFeldID] * (LONGLONG)faktor / (LONGLONG)divisor);
						else // if (operation == '-')
							summe -= (int)((LONGLONG)m_laFeldWerte[nFeldID] * (LONGLONG)faktor / (LONGLONG)divisor);
					}
				}
				else
				{
					// Betragsanteil der Kontenkategorie herausbekommen:
					char anteil = ' ';
					if (!strncmp(cp, "netto", 5))		{ anteil = 'n'; cp += 5; }
					else if (!strncmp(cp, "mwst", 4))	{ anteil = 'm'; cp += 4; }
					else if (!strncmp(cp, "brutto", 6))	{ anteil = 'b'; cp += 6; }
					
					// MWSt-Satz nach dem gefiltert werden soll (in % * 1000)
					int mwstsatz = -1;
					if (isdigit(*cp))
					{
						mwstsatz = atoi(cp);
						while (isdigit(*cp))
							cp++;
					}

					// Klammer-auf für Kontenkategorie-Namen
					if (*cp != '(') { summe = -99999999; break; }
					cp++;

					// bis Klammer-zu suchen
					if (!(cp2 = strchr(cp, ')'))) { summe = -99999999; break; }

					// 'e:' oder 'a:' vorweg
					int nKontenFeldID;
					char kontenart = tolower(*cp++); // 'e' oder 'a'
					if (kontenart != 'e' && kontenart != 'a') { summe = -99999999; break; }
					if (*cp == ')')	// nur (e) oder (a) ?
					{
						nKontenFeldID = -1;	// --> alle Buchungen zusammenzählen
					}
					else
					{
						if (*cp++ != ':') { summe = -99999999; break; }

						// Feld-ID holen, entspr. der die Konten durchsucht werden
						nKontenFeldID = atoi(cp);	// Achtung: Schachtelung!
					}
					cp = cp2 + 1;

					long untersumme = 0L;
					if (kontenart == 'e')
					{	// Einnahmen

						CStringArray csaVerknuepfteKonten;
						// Keine Feld-ID angegeben? Dann Feldzuweisungen rausfummel unnötig: einfach alle Buchungen aufsummieren
						if (nKontenFeldID != -1)
						{
							// aus der Konten-Oberkategorie alle verknüpften Konten auflisten
							LPCTSTR attr_name = xml->GetAttrValue("name");
							int j;
							for (j = 0; j < 100 && !m_csEinnahmenKonten[j].IsEmpty(); j++)
							{
								if (nKontenFeldID == atoi(GetErweiterungKey(m_csEinnahmenFeldzuweisungen[j], "ECT", attr_name)))
									csaVerknuepfteKonten.Add(m_csEinnahmenKonten[j]);
							}
						}

						// jetzt summieren
						CBuchung *p;
						for (p = this->Einnahmen; p; p = p->next)
						{
							if (mwstsatz != -1 && mwstsatz != p->MWSt) continue;	// ggf. nach MWSt-Satz filtern
							if (p->Datum < von_datum || p->Datum >= bis_datum) continue;	// ggf. nach Voranmeldungszeitraum filtern
							if (!(!attr_filter || *attr_filter == '\0' || (!stricmp(attr_filter, "betrieb") && (!sFilter || *sFilter == '\0') || p->Betrieb == (CString)sFilter))) continue;
							
							int j;
							for (j = 0; j < csaVerknuepfteKonten.GetSize(); j++)
								if (csaVerknuepfteKonten[j] == p->Konto)
									break;

							if (nKontenFeldID == -1 || j < csaVerknuepfteKonten.GetSize())
							{
								if (anteil == 'n')
									untersumme += p->GetNetto();
								else if (anteil == 'm')
									untersumme += p->Betrag - p->GetNetto();
								else // if (anteil == 'b')
									untersumme += p->Betrag;
							}
						}
					}
					else
					{	// Ausgaben

						CStringArray csaVerknuepfteKonten;
						// Keine Feld-ID angegeben? Dann Feldzuweisungen rausfummel unnötig: einfach alle Buchungen aufsummieren
						if (nKontenFeldID != -1)
						{
							// aus der Konten-Oberkategorie alle verknüpften Konten auflisten
							LPCTSTR attr_name = xml->GetAttrValue("name");
							int j;
							for (j = 0; j < 100 && !m_csAusgabenKonten[j].IsEmpty(); j++)
							{
								if (nKontenFeldID == atoi(GetErweiterungKey(m_csAusgabenFeldzuweisungen[j], "ECT", attr_name)))
									csaVerknuepfteKonten.Add(m_csAusgabenKonten[j]);
							}
						}

						// jetzt summieren
						CBuchung *p;
						for (p = this->Ausgaben; p; p = p->next)
						{
							if (mwstsatz != -1 && mwstsatz != p->MWSt) continue;	// ggf. nach MWSt-Satz filtern
							if (p->Datum < von_datum || p->Datum >= bis_datum) continue;	// ggf. nach Voranmeldungszeitraum filtern
							if (!(!attr_filter || *attr_filter == '\0' || (!stricmp(attr_filter, "betrieb") && (!sFilter || *sFilter == '\0') || p->Betrieb == (CString)sFilter))) continue;

							int j;
							for (j = 0; j < csaVerknuepfteKonten.GetSize(); j++)
								if (csaVerknuepfteKonten[j] == p->Konto)
									break;

							if (nKontenFeldID == -1 || j < csaVerknuepfteKonten.GetSize())
							{
								if (anteil == 'n')
									untersumme += p->GetBuchungsjahrNetto(this);
								else if (anteil == 'm')
								{
									if (p->AbschreibungNr == 1)	// nur im erstern Jahr Vorsteuer berechnen!
										untersumme += p->Betrag - p->GetNetto();
								}									
								else // if (anteil == 'b')
									untersumme += p->GetBuchungsjahrNetto(this) + (p->AbschreibungNr == 1 ? p->Betrag - p->GetNetto() : 0);
							}
						}
					}

					// Summe für Kontenart summieren oder abziehen
					if (operation == '+')
						summe += untersumme;
					else if (operation == '-')
						summe -= untersumme;
					else if (operation == '*')
						summe *= untersumme;
					else 
						summe /= untersumme;
				}

				if (*cp == '+')
				{
					operation = '+';
					cp++;
				}
				else if (*cp == '-')
				{
					operation = '-';
					cp++;
				}
			}

			if (!stricmp(child->GetAttrValue("nachkommaanteil"),"ohne"))
			{
				csFeldinhalt.Format("%d", summe/100);

				// Ergebnis speichern
				if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = summe/100*100;
			}
			else //if (!child->GetAttrValue("nachkommaanteil").CompareNoCase("mit"))
			{
				int_to_currency(summe, 4, csFeldinhalt.GetBuffer(30));
				csFeldinhalt.ReleaseBuffer();

				// Ergebnis speichern
				if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = summe;
			}

			csBerechnung.ReleaseBuffer();
			m_csaFeldStatustext[nID].Format("Feld %d summiert sich aus %s", nID, (LPCSTR)csBerechnung);
		}
		else if (!stricmp(child->GetAttrValue("typ"), "Einstellungsdaten"))
		{
			CString IDs = child->GetChildValue("erweiterung");
			CString csFeldinhalte = "";  // für Verkettung von mehreren Einstellungsdaten, getrennt durch Leerzeichen

			do
			{
				CString ID;
				int posTrenner = IDs.Find(' ');
				if (posTrenner < 0)
				{
					ID = IDs;  // kein Trenner (Leerzeichen) enthalten? Erweiterung == Einstellungs-ID
					IDs = "";
				}
				else
				{
					ID = IDs.Left(posTrenner);  // Trenner (Leerzeichen) enthalten? Mehrere Einstellungs-IDs in Erweiterung-String enthalten
					IDs = IDs.Mid(posTrenner + 1);
				}
				if (ID == "unternehmensart1" && *sFilter)
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
							csFeldinhalt = "<Unternehmensart für Betrieb nicht gefunden>"; 
							break; 
						}
						else if (!strcmp(betriebe, sFilter)) 
						{
							csKey.Format("Betrieb%02dUnternehmensart", iBetriebe);
							GetPrivateProfileString("Betriebe", csKey, "", betriebe, sizeof(betriebe), inifile);
							char *cp = strchr(betriebe, '\t');	// Unternehmensart1, Unternehmensart2 (Rechtsform), Steuernummer und Wirtschaftsidentifikationsnummer sind durch Tabs getrennt
							if (cp)
								*cp = '\0';
							csFeldinhalt = betriebe;
							break;
						}
					}
				}
				else if (ID == "unternehmensart2" && *sFilter)
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
							csFeldinhalt = ""; 
							break; 
						}
						else if (!strcmp(betriebe, sFilter)) 
						{
							csKey.Format("Betrieb%02dUnternehmensart", iBetriebe);
							GetPrivateProfileString("Betriebe", csKey, "", betriebe, sizeof(betriebe), inifile);
							char *cp = strchr(betriebe, '\t');	// Unternehmensart1, Unternehmensart2 (Rechtsform), Steuernummer und Wirtschaftsidentifikationsnummer sind durch Tabs getrennt
							if (!cp || cp[1] == '\0' || cp[1] == '\t')
							{
								GetPrivateProfileString(IniSektion((LPCSTR)ID), !strcmp(IniSektion((LPCSTR)ID), "Finanzamt") || !strcmp(IniSektion((LPCSTR)ID), "EinnahmenRechnungsposten") || !strcmp(IniSektion((LPCSTR)ID), "AusgabenRechnungsposten")? ((LPCSTR)ID)+1 : (LPCSTR)ID, "", csFeldinhalt.GetBuffer(10000), 10000, inifile);
								csFeldinhalt.ReleaseBuffer();
							}
							else
							{
								char *cp2 = strchr(++cp, '\t');
								if (cp2)
									*cp2 = '\0';
								csFeldinhalt = cp;
							}
							break;
						}
					}
				}
				else if (ID == "fsteuernummer" && *sFilter)
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
							csFeldinhalt = ""; 
							break; 
						}
						else if (!strcmp(betriebe, sFilter)) 
						{
							csKey.Format("Betrieb%02dUnternehmensart", iBetriebe);
							GetPrivateProfileString("Betriebe", csKey, "", betriebe, sizeof(betriebe), inifile);
							char *cp = strchr(betriebe, '\t');	// Unternehmensart1, Unternehmensart2 (Rechtsform), Steuernummer und Wirtschaftsidentifikationsnummer sind durch Tabs getrennt
							if (cp) cp = strchr(cp+1, '\t');
							if (!cp || cp[1] == '\0' || cp[1] == '\t')
							{
								GetPrivateProfileString(IniSektion((LPCSTR)ID), !strcmp(IniSektion((LPCSTR)ID), "Finanzamt") || !strcmp(IniSektion((LPCSTR)ID), "EinnahmenRechnungsposten") || !strcmp(IniSektion((LPCSTR)ID), "AusgabenRechnungsposten")? ((LPCSTR)ID)+1 : (LPCSTR)ID, "", csFeldinhalt.GetBuffer(10000), 10000, inifile);
								csFeldinhalt.ReleaseBuffer();
							}
							else
							{
								char* cp2 = strchr(++cp, '\t');  // durch tab getrennte nachfolgende 
								if (cp2) *cp2 = '\0';				// wirtschaftsidnr noch entfernen
								csFeldinhalt = cp;
							}
							break;
						}
					}
				}
				else if (ID == "wirtschaftsidnr")
				{
					if (*sFilter)
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
								csFeldinhalt = "";
								break;
							}
							else if (!strcmp(betriebe, sFilter))
							{
								csKey.Format("Betrieb%02dUnternehmensart", iBetriebe);
								GetPrivateProfileString("Betriebe", csKey, "", betriebe, sizeof(betriebe), inifile);
								char* cp = strchr(betriebe, '\t');	// Unternehmensart1, Unternehmensart2 (Rechtsform), Steuernummer und Wirtschaftsidentifikationsnummer sind durch Tabs getrennt
								if (cp) cp = strchr(cp + 1, '\t');
								if (cp) cp = strchr(cp + 1, '\t');
								if (!cp || cp[1] == '\0' || cp[1] == '\t')
								{
									GetPrivateProfileString(IniSektion((LPCSTR)ID), !strcmp(IniSektion((LPCSTR)ID), "Finanzamt") || !strcmp(IniSektion((LPCSTR)ID), "EinnahmenRechnungsposten") || !strcmp(IniSektion((LPCSTR)ID), "AusgabenRechnungsposten") ? ((LPCSTR)ID) + 1 : (LPCSTR)ID, "", csFeldinhalt.GetBuffer(10000), 10000, inifile);
									csFeldinhalt.ReleaseBuffer();
								}
								else
								{
									csFeldinhalt = ++cp;
								}
								break;
							}
						}
					}
					else // wenn kein Filter gesetzt, nur erste 11 Zeichen zurückliefern (keine Unterscheidung nach Betrieb)
					{
						char inifile[1000];
						GetIniFileName(inifile, sizeof(inifile));
						GetPrivateProfileString("Finanzamt", "wirtschaftsidnr", "", csFeldinhalt.GetBuffer(10000), 10000, inifile);
						csFeldinhalt.ReleaseBuffer();
						if (csFeldinhalt.GetLength() > 11)
							csFeldinhalt = csFeldinhalt.Left(11);
					}
				}
				else 
				{
					char EasyCashIniFilenameBuffer[1000];
					GetIniFileName(EasyCashIniFilenameBuffer, sizeof(EasyCashIniFilenameBuffer));
					GetPrivateProfileString(IniSektion((LPCSTR)ID), !strcmp(IniSektion((LPCSTR)ID), "Finanzamt") || !strcmp(IniSektion((LPCSTR)ID), "EinnahmenRechnungsposten") || !strcmp(IniSektion((LPCSTR)ID), "AusgabenRechnungsposten")? ((LPCSTR)ID)+1 : (LPCSTR)ID, "", csFeldinhalt.GetBuffer(10000), 10000, EasyCashIniFilenameBuffer);
					csFeldinhalt.ReleaseBuffer();
				}

				if (csFeldinhalte == "")
					csFeldinhalte = csFeldinhalt;
				else
					csFeldinhalte += " " + csFeldinhalt;
			}
			while (IDs != "");
			csFeldinhalt = csFeldinhalte;

			if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = currency_to_int(csFeldinhalt.GetBuffer(0));
		}
		else if (!stricmp(child->GetAttrValue("typ"), "Dokumentdaten"))
		{
			CString ID = child->GetChildValue("erweiterung");
			CString Sektion, Key;
			Sektion = "Allgemein";
			Key = ID;

			// Platzhalter auswechesln gegen Wert
			{
				CString temp;
				temp.Format("%-4.4d", nJahr);	// Jahr Langform JJJJ
				ID.Replace("$J", temp);
				temp.Format("%-2.2d", nJahr);	// Jahr Kurzform jj
				ID.Replace("$j", temp);
				temp.Format("%d", attr_voranmeldungszeitraum ? atoi(attr_voranmeldungszeitraum) : 0);	// Voranmeldungszeitraum
				ID.Replace("$v", temp);				
			}


			if (*(ID.GetBuffer(0)) == '[')
			{
				int nPosKlammerZu = ID.Find(']');
				if (nPosKlammerZu > 1)
				{
					Sektion = ID.Mid(1, nPosKlammerZu-1);
					Key = ID.Mid(nPosKlammerZu + 1);
				}
			}
			

			char *cp = GetErweiterungKey(Erweiterung, Sektion, Key);
			char *cp2;
			if (cp2 = strchr(cp, '|'))
			{
				strncpy(csFeldinhalt.GetBuffer(cp2-cp+1), cp, cp2-cp);
				csFeldinhalt.GetBuffer(cp2-cp+1)[cp2-cp] = '\0';
				csFeldinhalt.ReleaseBuffer();
			}
			else
				csFeldinhalt = "";
			
			if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = currency_to_int(csFeldinhalt.GetBuffer(0));
		}
		else if (!stricmp(child->GetAttrValue("typ"), "Freitext"))
		{
			csFeldinhalt = child->GetChildValue("erweiterung");

			// Platzhalter auswechesln gegen Wert
			{
				CString temp;
				temp.Format("%-4.4d", nJahr);	// Jahr Langform JJJJ
				csFeldinhalt.Replace("$J", temp);
				temp.Format("%-2.2d", nJahr);	// Jahr Kurzform jj
				csFeldinhalt.Replace("$j", temp);
				temp.Format("%d", attr_voranmeldungszeitraum ? atoi(attr_voranmeldungszeitraum) : 0);	// Voranmeldungszeitraum
				csFeldinhalt.Replace("$v", temp);				
			}
			
			if (nID >= 0 && nID < 10000) m_laFeldWerte[nID] = currency_to_int(csFeldinhalt.GetBuffer(0));
		}

		// ggf. Nullwerte nicht darstellen
		LPCSTR pDarstellenBeiNullwert = child->GetAttrValue("nullwertdarstellung");
		if (!pDarstellenBeiNullwert || stricmp(pDarstellenBeiNullwert, "ja"))
			if (csFeldinhalt == (CString)"0" + DEZIMALKOMMA + "00" || csFeldinhalt == "0")
				csFeldinhalt = "";

		if (!m_csaFeldStatustext[nID].IsEmpty())
			m_csaFeldStatustext[nID] += " -- ";
		m_csaFeldStatustext[nID] += (CString)"Feldname: '" + (LPCTSTR)child->GetChildValue("name") + "'";
		return csFeldinhalt;
	}
	else
		return "";
}

void CEasyCashDoc::SetFormularVoranmeldungszeitraum(int nZeitraum)
{
	int m_nZeitraum = nZeitraum;
}

BOOL CEasyCashDoc::FormularVerlangtVoranmeldungszeitraum(XDoc *pFormular)
{
	if (!pFormular) return FALSE;

	LPXNode xml = pFormular->GetRoot();
	LPXNode felder = NULL;
	if (xml) 
	{
		LPCTSTR attr_voranmeldungszeitraum = xml->GetAttrValue("voranmeldungszeitraum");
		if (atoi(attr_voranmeldungszeitraum)) 
		{
			return TRUE;
		}
	}
	m_nZeitraum = -1;
	return FALSE;
}

BOOL CEasyCashDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// If pHandlerInfo is NULL, then handle the message
	if (pHandlerInfo == NULL)
	{
		ExtensionDLLsClass *pExtensionDLLs = ExtensionDLLs;
		while (pExtensionDLLs)
		{
			if (nID == (UINT)pExtensionDLLs->menu_item_id)
			{
				if (nCode == CN_COMMAND)
				{
					// Handle WM_COMMAND message
					void (__cdecl *pECTE_function)(void *);

					pECTE_function = (void (__cdecl *)(void *))GetProcAddress(pExtensionDLLs->hInst, "ECTE_Menu");
					if (pECTE_function) 
						(*pECTE_function)((void *)this);
				}
				else if (nCode == CN_UPDATE_COMMAND_UI)
				{
					// Update UI element state
					((CCmdUI*)pExtra)->Enable(TRUE);
				}
				return TRUE;
			}
			pExtensionDLLs = pExtensionDLLs->next;
		}
	}
		
	return CDocument::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CEasyCashDoc::SetModifiedFlag(BOOL bModified)
{
	CDocument::SetModifiedFlag(bModified);
	if (bModified)
		OnWiederherstellungsdateiSave();
}

void CEasyCashDoc::SetModifiedFlag(LPCTSTR lpszAktion, BOOL bModified, BOOL bSaveToWiederherstellungsdatei)
{
	HWND hWndMain = AfxGetMainWnd()->GetSafeHwnd();
	if (hWndMain) ::SendMessage(hWndMain, WM_SETSTATUS, 0x4712, (LPARAM)lpszAktion);
	CDocument::SetModifiedFlag(bModified);
	if (bModified && bSaveToWiederherstellungsdatei)
		OnWiederherstellungsdateiSave();
	//UpdateAllViews(NULL); --> bringt Plugins zum Abstürzen!
}

void CEasyCashDoc::BerechneFormularfeldwerte(CString &Formular, CStringArray &csaFormularfeldwerte, int *pFormularfeldIDs, CStringArray *pcsaFormularfeldbeschreibungen, LPCSTR sFilter) // IDs nur von 1 - 9999, also int[10000], NULL -> wird dann ignoriert
{
	// Formulardefinitionsdatei in xmldoc laden
	XDoc xmldoc;
	xmldoc.LoadFile(Formular);
	LPXNode xml = xmldoc.GetRoot();
	if (!xml) return;
	LPXNode felder = NULL;
	LPXNode seiten = NULL;
	felder = xml->Find("felder");

	// Feldwerte einblenden
	if (felder)
	{
		ReadKontenCache(); // benötigt für GetFormularwertXXX Funktionen

		LPXNode child;

		csaFormularfeldwerte.RemoveAll();
		csaFormularfeldwerte.SetSize(felder->GetChildCount());
		if (pFormularfeldIDs)
		{
			int i;
			for (i = 0; i < 10000; i++) 
				pFormularfeldIDs[i] = -1;
		}
		if (pcsaFormularfeldbeschreibungen)
		{
			pcsaFormularfeldbeschreibungen->RemoveAll();
			pcsaFormularfeldbeschreibungen->SetSize(felder->GetChildCount());
		}
		for(int i = 0; i < felder->GetChildCount(); i++)
		{
			child = felder->GetChild(i);
			
			if (child && !child->value.IsEmpty())
			{
				csaFormularfeldwerte[i] = GetFormularwertByIndex(&xmldoc, i, sFilter);
				if (pFormularfeldIDs)
					pFormularfeldIDs[atoi(child->GetAttrValue("id"))] = i;
				if (pcsaFormularfeldbeschreibungen)
					(*pcsaFormularfeldbeschreibungen)[i] = child->GetChildValue("name");
			}
		}
	}
}

BOOL CEasyCashDoc::BuchungIstEinnahme(CBuchung *pBuchung)
{
	CBuchung *pb;
	for (pb = Einnahmen; pb; pb = pb->next)
		if (pb == pBuchung)
			return TRUE;
	return FALSE;
}

BOOL CEasyCashDoc::BuchungIstAusgabe(CBuchung *pBuchung)
{
	CBuchung *pb;
	for (pb = Ausgaben; pb; pb = pb->next)
		if (pb == pBuchung)
			return TRUE;
	return FALSE;
}