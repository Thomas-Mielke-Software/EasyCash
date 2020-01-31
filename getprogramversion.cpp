// getprogramversion.cpp: implementation file
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

/*	$Header:   D:/pvcsarch/easycash/getprogramversion.cpv   1.30   Feb 15 2007 06:34:10   tmielke  $
*
*	Beschreibung: GetProgramVerion implementation file
*	
*	Die erste gültige Versionsnummer aus den Resourcen einer MFC-Applikation holen:
*	
*	Beispiel für den Aufruf:
*
*		...
*
*		char version_string[50];
*		HMODULE hExe = GetModuleHandle("TELAR.EXE");
*		VS_FIXEDFILEINFO vs_ffi;
*		if (hExe) 
*		{
*			GetProgramVerion(hExe, &vs_ffi);	
*			sprintf(version_string, "v%0lu.%0lu", vs_ffi.dwProductVersionMS >> 16, vs_ffi.dwProductVersionMS & 0xffff);
*			// version_string enthält dann "v2.0" oder so ähnlich
*		}
*		else 
*			*version_string = '\0';
*
*		...
*
*	$Log:   D:/pvcsarch/easycash/getprogramversion.cpv  $
* 
*    Rev 1.30   Feb 15 2007 06:34:10   tmielke
* v1.31
* 
*    Rev 1.29   Feb 05 2007 05:05:38   tmielke
* v1.30 EÜR2006 + UVA2007 + Dauerbuchungen Features + Bugfixes
* 
*    Rev 1.28   Feb 03 2006 02:29:18   tmielke
* v1.29
* 
*    Rev 1.27   Jan 19 2006 13:49:10   tmielke
* v1.28 -- Bugfixes
* 
*    Rev 1.26   Jan 16 2006 09:52:28   tmielke
* v1.26 EÜR-Formular, Bugfix
* 
*    Rev 1.25   Dec 10 2005 12:09:42   tmielke
* v1.24: neuer Formelrechner und DSA-Messagebox
* 
*    Rev 1.24   Dec 07 2005 05:32:58   tmielke
* v1.22
* 
*    Rev 1.23   Nov 21 2005 20:19:24   tmielke
* v1.21 ActiveX Plugin Schnittstelle
* 
*    Rev 1.22   Jun 21 2005 23:40:28   tmielke
* v1.19
* 
*    Rev 1.21   Jan 20 2005 15:17:26   tmielke
* v1.18:
* Die Bemessungsgrundlage bei der Umsatzsteuervoranmeldung wird jetzt vom vollen Euro-Betrag berechnet.
* Die Textfarbe und entspr. die Fensterhintergrundfarbe werden nun aus den Windows-Einstellungen übernommen.
* Die Zeilenbreite im Journal wird nun dynamisch an die Fensterbreite angepasst. 
* MWSt und Belegnummernspalten im Journal werden nur dann angezeigt, wenn entsprechend relevante Buchungen gemacht wurden. 
* Schon ausgeführte Dauerbuchungen können jetzt nachträglich geändert und nochmal ausgeführt werden, indem das 'Ausgeführt Bis'-Datum zurückgesetzt wird. Die alten Buchungseinträge im Journal müssen allerdings von Hand gelöscht werden. Ich hoffe, damit wird der Mechanismus transparenter.
* Dauerbuchungen sind jetzt auch in 2-monatlichen Intervallen möglich.
* Die Eingabe von Buchungspresets mit einer AfA-Dauer von 0 Jahren wird jetzt unterbunden.
* Einen Fehler beseitigt, der in seltenen Fällen für einen Absturz beim Aufruf der Dauerbuchungen verantwortlich war.
* Die Anrechnung der festgesetzten Sondervorauszahlung für Dauerfristverlängerung (Feld 39) im deutschen USt-Voranmeldungsformular bzw. Sonstige Berichtigungen (Feld 90) im österreischischen Formular wird jetzt berücksichtigt.
* Feld 43 Der USt-Voranmeldung (Weitere Steuerfreie Umsätze mit Vorsteuerabzug) wird jetzt unterstützt.
* Steuerfreie Umsätze ohne Vorsteuerabzug wurden bei der Anlage UR der Einkommensteuererklärung vergessen.
* Knopf für die Seitenansicht in der Werkzeugleiste.
* Es gibt zwei weitere Nummernkreise für Buchungsnummern. Aktivierbar durch Drücken von 'B' oder 'K' im Belegnummernfeld.
* 
* 
*    Rev 1.20   Mar 25 2004 02:59:50   tmielke
* v1.17
* 
*    Rev 1.19   Feb 27 2004 21:13:18   tmielke
* v1.16
* 
*    Rev 1.18   Feb 08 2004 06:40:28   tmielke
* v1.16 alpha 3
* 
*    Rev 1.17   Feb 04 2002 17:42:16   tmielke
* v1.14
* 
*    Rev 1.16   Jan 07 2002 14:23:56   tmielke
* v1.13
* Seitenwechsel zw. Einnahmen und Ausgaben
* UmSt-Vorauszahlungen werden für UmSt-Erkl. gemerkt
* 
*    Rev 1.15   Oct 17 2001 19:54:44   tmielke
* v1.12a
* 
*    Rev 1.14   Oct 09 2001 07:51:00   tmielke
* v1.12
* 
*    Rev 1.13   15 May 2001 08:21:28   tmielke
* v1.11
* 
*    Rev 1.12   22 Jan 2001 02:43:36   tmielke
* v1.10
* 
*    Rev 1.11   15 Jan 2001 15:28:36   tmielke
* Bugfix: Einnahmen-Konto wird jetzt mit exportiert
* Kosmetik im Druckauswahl-Dialog
* Buchung: Cursor springt jetzt vom Währungsfeld ins Beschrei-
*       bungsfeld,wenn zwei Ziffern nach dem Komma eingegeben wurden
* Version C. Detjen geschickt
* 
* 
*    Rev 1.10   21 Dec 2000 02:07:34   tmielke
* Letzte Version vor Umstellung auf DS60
* 
*    Rev 1.9   30 Sep 2000 18:03:24   tmielke
* v1.9
* 
*    Rev 1.8   19 Jun 2000 01:35:00   tmielke
* v1.8
* 
*    Rev 1.7   03 Apr 2000 01:27:38   tmielke
* v1.7b
* 
*    Rev 1.6   18 Mar 2000 11:36:14   tmielke
* v1.7
* 
*    Rev 1.5   17 Mar 2000 05:36:24   tmielke
* v1.6 österr./schweitzer Version
* 
*    Rev 1.4   28 Jan 2000 03:10:36   tmielke
* v1.5
* 
*    Rev 1.3   27 Jan 2000 19:53:56   tmielke
* v1.4
* 
*    Rev 1.2   04 Dec 1999 15:01:12   tmielke
* v1.4
* 
*    Rev 1.1   18 Nov 1999 02:31:44   tmielke
* v1.3 UmSt-Erklärung und E/Ü-Rechnung
* 
*    Rev 1.0   18 Nov 1999 02:30:08   tmielke
* Initial Check In
* 
*    Rev 1.10   07 Jun 1999 18:36:56   tmielke
* Multi Language Version
* 
*    Rev 1.9   05 Apr 1999 09:39:32   tmielke
* Nachtrag v1.32
* 
*    Rev 1.8   05 Apr 1999 08:14:26   tmielke
* Info-Screen Logo verändert
* 
*    Rev 1.7   22 Mar 1999 16:26:28   tmielke
* v1.31 Letzte-Aufnahme-Button und Logo
* 
*    Rev 1.6   20 Mar 1999 18:52:32   tmielke
* v1.3
* 
*    Rev 1.5   18 Mar 1999 13:27:30   tmielke
* v0.8 - Demoversion fertig gemacht
* 
*    Rev 1.4   03 Mar 1999 13:40:24   tmielke
* Version Label v0.6
* 
*    Rev 1.3   03 Mar 1999 10:54:52   tmielke
* zur Sicherheit eingecheckt
* 
*    Rev 1.0   02 Mar 1999 10:01:30   tmielke
* GetProgramVersion eingebaut.
* 
* 
*/

#include "stdafx.h"
#include "resource.h"

// statics:
static char szBuffer[80]; // print buffer for EnumResourceTypes  
static DWORD cbWritten;   // number of bytes written to res. info. file 
static int cbString;      // length of string in sprintf 

// Prototypen:
// Declare callback functions. 
static int __stdcall EnumTypesFunc( 
	HANDLE hModule, 
	LPTSTR lpType, 
	LONG lParam); 

static int __stdcall EnumNamesFunc( 
	HANDLE hModule, 
	LPCTSTR lpType, 
	LPTSTR lpName, 
	LONG lParam); 

static int __stdcall EnumLangsFunc( 
	HANDLE hModule, 
	LPCTSTR lpType, 
	LPCTSTR lpName, 
	WORD wLang, 
	LONG lParam); 


void GetProgramVerion(HMODULE hExe, VS_FIXEDFILEINFO *vs)
{
	if (hExe != NULL)
	{
		EnumResourceTypes(hExe,              // module handle 
			(ENUMRESTYPEPROC)EnumTypesFunc,  // callback function 
			(LONG)vs);                       // extra parameter 
	}

}
 

// --- die callbacks -----

//    FUNCTION: EnumTypesFunc(HANDLE, LPSTR, LONG) 
// 
//    PURPOSE:  Resource type callback 
 
static int __stdcall EnumTypesFunc( 
    HANDLE hModule,   // module handle 
    LPTSTR lpType,    // address of resource type 
    LONG lParam)      // extra parameter, could be 
                      // used for error checking 
{ 
	BOOL bFound = FALSE;
 
    // Write the resource type to a resource information file. 
    // The type may be a string or an unsigned decimal 
    // integer, so test before printing. 
 
    if ((ULONG)lpType & 0xFFFF0000) 
	{ 
		if (!strcmp(lpType, "RT_VERSION")) bFound = TRUE;
    } 
    else 
	{ 
 		if ((USHORT)lpType == (USHORT)RT_VERSION) bFound = TRUE;
    } 
 
	if (bFound)
	{
		// Find the names of all resources of type lpType. 
		EnumResourceNames((HINSTANCE) hModule, 
			lpType, 
			(ENUMRESNAMEPROC)EnumNamesFunc, 
			lParam); 
	    return FALSE; // stop!
	}
	
    return TRUE; 
} 
 
//    FUNCTION: EnumNamesFunc(HANDLE, LPSTR, LPSTR, LONG) 
// 
//    PURPOSE:  Resource name callback 
 
static int __stdcall EnumNamesFunc( 
    HANDLE hModule,   // module handle 
    LPCTSTR lpType,   // address of resource type 
    LPTSTR lpName,    // address of resource name 
    LONG lParam)      // extra parameter, could be 
                      // used for error checking 
{ 
	BOOL bFound = FALSE;
 
     // Write the resource name to a resource information file. 
     // The name may be a string or an unsigned decimal 
     // integer, so test before printing. 
 
    if ((ULONG)lpName & 0xFFFF0000) 
	{ 
		if (!strcmp(lpName, "VS_VERSION_INFO")) bFound = TRUE;
    } 
    else 
	{ 
  		if ((USHORT)lpName == VS_VERSION_INFO) bFound = TRUE;
    } 
 
 
     // Find the the 1st languages of resources of type 
     // lpType and name lpName. 
 
	if (bFound)
	{
		EnumResourceLanguages((HINSTANCE) hModule, 
			lpType, 
			lpName, 
			(ENUMRESLANGPROC)EnumLangsFunc, 
			lParam); 
		return FALSE; 
	}
	
    return TRUE; 
} 
 
//    FUNCTION: EnumLangsFunc(HANDLE, LPSTR, LPSTR, WORD, LONG) 
// 
//    PURPOSE:  Resource language callback 
 
static int __stdcall EnumLangsFunc( 
    HANDLE hModule,  // module handle 
    LPCTSTR lpType,  // address of resource type 
    LPCTSTR lpName,  // address of resource name 
    WORD wLang,      // resource language 
    LONG lParam)     // extra parameter, could be 
{ 
    HANDLE hResInfo; 
 
    hResInfo = FindResourceEx((HINSTANCE)hModule, lpType, lpName, wLang); 
	
	if (hResInfo)
	{
		HGLOBAL hGlobal = LoadResource((HINSTANCE)hModule, (HRSRC)hResInfo);
		if (hGlobal)
		{
			WORD *p = (WORD *)LockResource(hGlobal);
			if (p)
			{
				// wLength
				// Specifies the length of the VS_VERSION_INFO structure. 
				// This length does not include any padding that aligns the 
				// subsequent version structure on a 32-bit boundary. 
				p++;
				
				// wValueLength
				// Specifies the length of the Value member. This value is zero 
				// if there is no Value member associated with the current version 
				// structure. 
				ASSERT(*p == sizeof(VS_FIXEDFILEINFO));
				p++;

				// bText
				// Specifies the type of data in the version resource. This member 
				// is 1 if the version resource contains text data and 0 if the 
				// version resource contains binary data. 
				ASSERT(*p == 0);
				p++;
				
				// szKey
				// Contains "VS_VERSION_INFO". 
				p += strlen("VS_VERSION_INFO");	// wide chars!

				// Padding1
				// Contains as many zero bytes as necessary to align the Value 
				// member on the _next_ 32-bit boundary. 
				p += 2 - (((DWORD)p & 0x0003) >> 1/*da WORD*/);
							
				memcpy((VS_FIXEDFILEINFO *)lParam, (VS_FIXEDFILEINFO *)p, sizeof(VS_FIXEDFILEINFO));
				return FALSE;	// stop!
			}
		}
	}
	
	return TRUE; // weiter!
} 











