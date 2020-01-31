//////////////////////////////////////////////////////////////////
// ECTIFaceMisc - Verschiedene Hilfsfunktionen
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
#include "ECTIFace.h"
#include "EasyCashDoc.h"

// Wandelt einen Betrag von String in int (= Pfennige/Cents) um
extern "C" AFX_EXT_CLASS int currency_to_int(char *s)
{
	int i, n;
	char *cp;
	BOOL neg = FALSE;

	for (cp = s, n = 0, i = 200; *cp; cp++)
	{
		if (*cp < '0' || *cp > '9')
		{
			if (*cp == DEZIMALKOMMA) 
				i = 2;
			else if (*cp == MINUSZEICHEN && !n)
				neg = TRUE;
			else 
				if (*cp != TAUSENDERTRENNER) break;
		}
		else
		{
			n *= 10;
			n += *cp - '0';
			i--;
			if (i <= 0) break;
		}
	}
	if (i > 2) i = 2;
	int j;
	for (j = 0; j < i; j++)
		n *= 10;

	if (neg && n) n = -n;

	return n;
}

// i=zu konvertierender Wert
// n=Feldweite incl. Komma und 2 Nachkommastellen;
// s=Ergebnisstring
extern "C" AFX_EXT_CLASS void int_to_currency(int i, int n, char *s)
{
	char buf[300];

	if (n < 4) n = 4;
	sprintf(buf, "%%%d.2lf", n);
	sprintf(s, buf, (double)i/100);
	char *cp = strrchr(s, '.');
	if (cp) *cp = DEZIMALKOMMA;
}

// wie oben, jedoch mit Tausenderpunkten
extern "C" AFX_EXT_CLASS void int_to_currency_tausenderpunkt(int i, int n, char *s)
{
	int_to_currency(i, n, s);

	char *beginn;
	for (beginn = s; *beginn == ' '; beginn++)
		;

	char *cp = strrchr(beginn, DEZIMALKOMMA);
	if (cp)
		while (cp-beginn > (i<0?4:3))
		{
			cp-=TAUSENDERGRUPPIERUNG;
			for (char *cp2 = s+strlen(s); cp2 >= cp; cp2--)
				cp2[1] = *cp2;
			*cp = TAUSENDERTRENNER;
		}
}

extern "C" AFX_EXT_CLASS BOOL GetProgrammverzeichnis(char *buffer3, int size)
{
	char *cp;

	DWORD dw = GetModuleFileName(hInst, buffer3, size);

	if (dw < size && (cp = strrchr(buffer3, '\\')))
	{
		*cp = '\0';
		return TRUE;
	}
	else
		return FALSE;
}

static char IniFileName[1000] = "";

extern "C" AFX_EXT_CLASS void SetIniFileName(char *buffer3)
{
	strcpy(IniFileName, buffer3);
}

extern "C" AFX_EXT_CLASS BOOL GetIniFileName(char *buffer3, int size)
{
	if (*IniFileName)
	{
		strncpy(buffer3, IniFileName, min(size, strlen(IniFileName)));
		buffer3[min(size-1, strlen(IniFileName))] = '\0';
		return TRUE;
	}
	else
	{
		char *cp;

		DWORD dw = GetModuleFileName(hInst, buffer3, size);

		if (dw < size && (cp = strrchr(buffer3, '\\')))
		{
			strcpy(cp, "\\easyct.ini");
			return TRUE;
		}
		else
			return FALSE;
	}
}

// -1 -> Mandanten Modus aus, 0..x Mandantennummer
static int nMandant;

extern "C" AFX_EXT_CLASS void SetMandant(int n)
{
	nMandant = n;
}

extern "C" AFX_EXT_CLASS BOOL GetMandant()
{	
	return nMandant;
}

//--- ab hier Hilfsfunktionen für den Zugriff auf die Plugin-Erweiterungsdaten in der Dokument-Klasse ---

// Diese Funktionen werden mittelfristig in die Dokument-Klasse eingebaut

// ACHTUNG: Das Zeichen '|' darf in Key- bzw. Erweiterungsnamen und in Werten nicht verwendet werden
//          und '=' nicht in Key-Namen. In Zukunft sollte das Zeichen intern 'escaped' werden.

// Return: Pointer auf den gewünschten Key-Wert der Ereiterung oder "" wenn nicht gefunden wurde
//		   Der Key-Wert ist nicht Null-Terminiert, sondern '|'-Terminiert!!!
// Die Aufrufende Funktion sollte nach der Verarbeitung einen ReleaseBuffer auf csSpeicher machen
char *GetErweiterungKey(CString &csSpeicher, LPCTSTR sErweiterung, LPCTSTR sKey)
{
	char *cp = csSpeicher.GetBuffer(0);

	// Durchsuchen der Erweiterungsnamen (siehe Kommentar in easycashdoc.h unter Erweiterung)
	while (strncmp(sErweiterung, cp, strlen(sErweiterung)) || cp[strlen(sErweiterung)] != '|')
	{
		if (!(cp = strstr(cp, "||"))) 
		{
			csSpeicher.ReleaseBuffer();			
			return ""; // nichts gefunden
		}
		cp += 2;	// hinter "||" steht der Name der nächsten Erweiterung
	}

	// wenn Erweiterungsname gefunden: zum ersten key
	cp += strlen(sErweiterung) + 1;

	// Durchsuchen der zu der Erweiterung gehörenden keys 
	while (strncmp(sKey, cp, strlen(sKey)) || cp[strlen(sKey)] != '=')
	{
		if (!(cp = strchr(cp, '|'))) return ""; // nichts gefunden, sollte hier noch nicht passieren, da mit "||" abgeschlossen werden muss
		if (cp[1] == '|') 
		{
			csSpeicher.ReleaseBuffer();						
			return "";	// "||" erreicht = Ende des Bereichs für diese Erweiterung
		}
		cp ++;	// hinter "||" steht der Name des nächsten Key
	}

	cp += strlen(sKey) + 1;

	csSpeicher.ReleaseBuffer();
	
	return cp;
}


// einen Wert im Erweiterungs-Bereich der Dokumentklasse schreiben
void SetErweiterungKey(CString &csSpeicher, LPCTSTR sErweiterung, LPCTSTR sKey, LPCTSTR sValue)
{
	// CString fixieren für char-Operationen und noch etwas Luft reservieren
	// int maximale_groesse = csSpeicher.GetLength()+strlen(sErweiterung)+strlen(sKey)+strlen(sValue)+100;
	char *start = csSpeicher.GetBuffer(0);
	char *cp = start;

	// Durchsuchen der Erweiterungsnamen (siehe Kommentar in easycashdoc.h unter Erweiterung)
	while (strncmp(sErweiterung, cp, strlen(sErweiterung)) || cp[strlen(sErweiterung)] != '|')
	{
		// Wenn am Ende angekommen, ohne den Namen zu finden: einfach anhängen
		if (!(cp = strstr(cp, "||"))) 
		{
			// 1. Fall: Weder Erweiterung noch Key waren vorhanden
			csSpeicher.ReleaseBuffer();
			csSpeicher += (CString)sErweiterung + "|" + sKey + "=" + sValue + "||";
			return;
		}

		cp += 2;	// hinter "||" steht der Name der nächsten Erweiterung
	}

	// wenn Erweiterungsname gefunden: zum ersten key
	cp += strlen(sErweiterung) + 1;

	// Durchsuchen der zu der Erweiterung gehörenden keys 
	while (strncmp(sKey, cp, strlen(sKey)) || cp[strlen(sKey)] != '=')
	{
		cp = strchr(cp, '|');
		ASSERT(cp);	 // sollte hier noch nicht abbrechen, da mit "||" abgeschlossen werden muss!
		if (cp[1] == '|')	// "||" erreicht = Ende des Bereichs für diese Erweiterung
		{
			// 2. Fall: Erweiterung war vorhanden, aber Key noch nicht
			CString vorn, hinten;
			strncpy(vorn.GetBuffer(csSpeicher.GetLength()), (LPCSTR)csSpeicher, cp-(LPCSTR)csSpeicher);
			vorn.GetBuffer(csSpeicher.GetLength())[cp-(LPCSTR)csSpeicher] = '\0';
			hinten = cp;
			vorn.ReleaseBuffer(-1);
			csSpeicher.ReleaseBuffer();
			csSpeicher = vorn + "|" + sKey + "=" + sValue + hinten;
			return;
		}
		cp ++;	// hinter "||" steht der Name des nächsten Key
	}

	cp += strlen(sKey) + 1;	// Keynamen + '=' überspringen

	{
		// 3. Fall: Erweiterung und Key waren vorhanden und Wert muss ersetzt werden
		CString vorn, hinten;
		strncpy(vorn.GetBuffer(csSpeicher.GetLength()), (LPCSTR)csSpeicher, cp-(LPCSTR)csSpeicher);
		vorn.GetBuffer(csSpeicher.GetLength())[cp-(LPCSTR)csSpeicher] = '\0';
		cp = strchr(cp, '|');
		ASSERT(cp);
		hinten = cp;
		vorn.ReleaseBuffer(-1);
		csSpeicher.ReleaseBuffer();
		csSpeicher = vorn + sValue + hinten;
		return;
	}
}


/////////////////////////////////////////////
// ExtensionDLLsClass + CIterateExtensionDLLs

ExtensionDLLsClass *ExtensionDLLs;

extern "C" AFX_EXT_CLASS void CLoadExtensionDLLs(char *version_string)
{
	// Erweiterungs-DLLs laden:
	ExtensionDLLs = NULL;
	char filenamebuffer[300];
	GetIniFileName(filenamebuffer, sizeof(filenamebuffer));
	char *cp = strrchr(filenamebuffer, '\\');
	if (cp)
	{
		strcpy(cp, "\\ECTE2*.DLL");	// "\0"); 
		CFileFind ff;
		if (ff.FindFile(filenamebuffer))
		{
			ExtensionDLLsClass **ppExtensionDLLs = &ExtensionDLLs;
			int i = 40000;
			int bGoOn;
			do
			{
				bGoOn = ff.FindNextFile();

				{
					LPCSTR (*pECTE_Init)(LPCSTR);
					*ppExtensionDLLs = new ExtensionDLLsClass;
					(*ppExtensionDLLs)->name = ff.GetFilePath();
					(*ppExtensionDLLs)->hInst = AfxLoadLibrary((*ppExtensionDLLs)->name);
					pECTE_Init = (LPCSTR (*)(LPCSTR))GetProcAddress((*ppExtensionDLLs)->hInst, "ECTE_Init");
					if (pECTE_Init) 
					{
						(*ppExtensionDLLs)->menu_item_string = (CString)(*pECTE_Init)(version_string);
						if ((*ppExtensionDLLs)->menu_item_string.IsEmpty())
							(*ppExtensionDLLs)->menu_item_id = 0;
						else
							(*ppExtensionDLLs)->menu_item_id = i++;
					}
					else
						(*ppExtensionDLLs)->menu_item_id = i++;
					(*ppExtensionDLLs)->next = NULL;
					ppExtensionDLLs = &(*ppExtensionDLLs)->next;
				}
			}	
			while (bGoOn);
		}
	}
}

extern "C" AFX_EXT_CLASS void CFreeExtensionDLLs()
{
	// ExtensionDLLs-Liste entsorgen
	ExtensionDLLsClass *pExtensionDLLs = ExtensionDLLs;
	ExtensionDLLsClass *pTemp;
	while (pExtensionDLLs)
	{
		pTemp = pExtensionDLLs->next;
		AfxFreeLibrary(pExtensionDLLs->hInst);
		delete pExtensionDLLs;
		pExtensionDLLs = pTemp;
	}
}

extern "C" AFX_EXT_CLASS void CIterateExtensionDLLs(LPCSTR function_name, void *p)
{
	void (*pECTE_function)(void *);
	ExtensionDLLsClass *pExtensionDLLs = ExtensionDLLs;
	while (pExtensionDLLs)
	{
		pECTE_function = (void (*)(void *))GetProcAddress(pExtensionDLLs->hInst, function_name);
		if (pECTE_function) 
			(*pECTE_function)(p);
		pExtensionDLLs = pExtensionDLLs->next;
	}
}

// registrierte Version == FALSE
BOOL shareware; // flag aus der Anfangszeit, als ich noch nicht wusste, was ich mit der Software will..

extern "C" AFX_EXT_CLASS void LadeECFormulare(CStringArray &csa)
{
	csa.RemoveAll();

	char filenamebuffer[1000];

	if (GetProgrammverzeichnis(filenamebuffer, sizeof(filenamebuffer)))
	{
		strcat(filenamebuffer, "\\*.ecf");	// "\0"); 
		CFileFind ff;

		if (ff.FindFile(filenamebuffer))
		{
			BOOL bGoOn;

			do
			{
				bGoOn = ff.FindNextFile();

				csa.Add(ff.GetFilePath());
			}	
			while (bGoOn);
		}
	}
}

// Holt den Formularnamen aus einem Array und filtert nach Formularart
extern "C" void HoleFormularnamenAusCSA(long Index, LPCTSTR Filter, CStringArray &csa, CString *Result)
{
	*Result = "";

	if (Index >=0 && Index < csa.GetSize())
	{
		XDoc xmldoc;
		xmldoc.LoadFile(csa[Index]);
		LPXNode xml = xmldoc.GetRoot();
		if (xml) 
		{
			LPCTSTR attr_name = xml->GetAttrValue("name");

			if ((CString)attr_name == (CString)Filter)
				*Result = xml->GetAttrValue("anzeigename");
		}
	}
}

// berechnet die Ini-Sektion aus dem angeforderten-Einstellungs-Key ('f' voranstellen für Finanzamts-Daten)
extern "C" AFX_EXT_CLASS LPCSTR IniSektion(LPCSTR id)
{
	if (!strcmp(id, "monatliche_voranmeldung") ||
		!strcmp(id, "taeglich_buchen") ||
		!strcmp(id, "BuchungsdatumBelassen") ||
		!strcmp(id, "ErzeugeLaufendeBuchungsnummernFuerEinnahmen") ||
		!strcmp(id, "ErzeugeLaufendeBuchungsnummernFuerAusgaben") ||
		!strcmp(id, "JahresfeldAktiviert") ||
		!strcmp(id, "tausender_mwst_saetze") ||
		!strcmp(id, "AbschreibungGenauigkeit") ||
		!strcmp(id, "JahreswechselAbfrage") ||
		!strcmp(id, "telefon") ||
		!strcmp(id, "email")) return "Allgemein";

	if (!strcmp(id, "umstvoranmeldung_h") ||
		!strcmp(id, "umstvoranmeldung_h2") ||
		!strcmp(id, "umstvoranmeldung_v") ||
		!strcmp(id, "formular_nicht_mitdrucken") ||
		!strcmp(id, "Bildschirmschrift") ||
		!strcmp(id, "Bildschirmschriftgroesse") ||
		!strcmp(id, "Druckerschrift") ||
		!strcmp(id, "Druckerschriftgroesse")) return "Druck";

	if (*id == 'e' && isdigit(id[1]) && isdigit(id[2]) && !id[3]) return "EinnahmenRechnungsposten";

	if (*id == 'a' && isdigit(id[1]) && isdigit(id[2]) && !id[3]) return "AusgabenRechnungsposten";

	if (!strcmp(id+2, "Text") ||
		!strcmp(id+2, "Ausg") ||
		!strcmp(id+2, "MWSt") ||
		!strcmp(id+2, "AfAJ") ||
		!strcmp(id+2, "Rech")) return "Buchungsposten";

	if (!strcmp(id, "name") ||
		!strcmp(id, "vorname") ||
		!strcmp(id, "unternehmensart1") ||
		!strcmp(id, "unternehmensart2") ||
		!strcmp(id, "strasse") ||
		!strcmp(id, "plz") ||
		!strcmp(id, "ort") ||
		!strcmp(id, "land") ||
		!strcmp(id, "vat1") ||
		!strcmp(id, "vat2") ||
		!strcmp(id, "vat3")) return "Persoenliche_Daten";
		
	if (!strcmp(id, "fname") ||
		!strcmp(id, "fname2") ||
		!strcmp(id, "fstrasse") ||
		!strcmp(id, "fplz") ||
		!strcmp(id, "fort") ||
		!strcmp(id, "fsteuernummer")) return "Finanzamt";

	return "Allgemein";
}

