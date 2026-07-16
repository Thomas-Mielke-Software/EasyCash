// KontoExports.cpp -- Plugin-API: HoleKontoMitFeldern (V4/ECTBridge)
//
// Diese Datei gehört ins ECTBridge-Projekt.
// Kompiliert mit /clr (Projektstandard), OHNE Precompiled Header.
//
// Generalisierung von HoleKontoFuerFeld (ectifacemisc.cpp): statt fest
// E/Ü-Rechnung + USt-Voranmeldung nimmt die Funktion eine
// Feld-Spezifikation mit Land-Blöcken entgegen, z.B.
//
//   HoleKontoMitFeldern("$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=48||"
//                       "at:Beilage E1a=9040|Umsatzsteuer=1020||")
//
// und liefert das erstbeste Konto, das (im Block des in den Einstellungen
// gewählten Landes) mit ALLEN angegebenen Formularfeldern verknüpft ist.
// Existiert keines, öffnet eine Eingabemaske (KontoAnlegenView) zur
// Anlage; nur der Kontoname ist einzugeben (vorbelegt mit den Feldnamen).
// Rückgabe: Kontoname oder "" (Abbruch, Spezifikations-Fehler oder alle
// 100 Konto-Slots belegt -- Fehler jeweils als MessageBox).
//
// Die Logik liegt in ECTEngine::KontoFeldSelektor (Lookup/Anlage) und
// ECTViews::ViewHost::HoleKontoMitFeldern (Dialog-Fluss); die native
// V3-Implementierung in ECTIFace/ectifacemisc.cpp verhält sich identisch,
// damit die Plugin-API beider Welten kompatibel bleibt.

#include "stdafx.h"
#include "Marshalling.h"

#using "ECTEngine.dll"
#using "ECTViews.dll"

using namespace System;

static char kontoMitFeldernBuffer[1000];

extern "C" AFX_EXT_CLASS char *HoleKontoMitFeldern(LPCSTR spez)
{
    kontoMitFeldernBuffer[0] = '\0';
    try
    {
        CString csSpez(spez ? spez : "");
        String^ s = ECTBridge::ToManaged(csSpez);

        // Aktives Fenster als Owner, damit der Anlage-Dialog vor dem
        // Aufrufer (MFC-Hauptfenster bzw. Plugin-Dialog) bleibt.
        String^ konto = ECTViews::ViewHost::HoleKontoMitFeldern(
            s, IntPtr((void*)::GetActiveWindow()));

        CString cs = ECTBridge::ToNative(konto);
        if (cs.GetLength() >= (int)sizeof(kontoMitFeldernBuffer))
            cs = cs.Left(sizeof(kontoMitFeldernBuffer) - 1);
        strcpy(kontoMitFeldernBuffer, (LPCTSTR)cs);
    }
    catch (Exception^ ex)
    {
        CString msg = "Fehler in HoleKontoMitFeldern: ";
        msg += ECTBridge::ToNative(ex->Message);
        AfxMessageBox(msg, MB_ICONERROR);
        kontoMitFeldernBuffer[0] = '\0';
    }
    return kontoMitFeldernBuffer;
}
