// MacPaket.h: Erkennung des EC&T-Mac-Pakets (PortJump-/CrossOver-App-Bundle)
//
// Unter macOS laeuft EasyCT.exe in einem von CodeWeavers' PortJump erzeugten App-Bundle
// (EasyCash&Tax.app). Der Plugin-Manager aktualisiert nur die Windows-Programmdateien,
// die CrossOver-Schicht im Bundle bleibt dabei unveraendert. Die Version des Bundles steht
// in dessen Info.plist (CFBundleShortVersionString), die wir aus Wine heraus lesen.

#pragma once

#define MAC_PAKET_MINDESTVERSION	"3.9.0"
#define MAC_PAKET_DOWNLOAD_URL		"https://easyct.de/downloads.php?cat_id=1&download_id=128"

// TRUE, wenn EasyCT.exe unter Wine/CrossOver auf macOS laeuft.
BOOL LaeuftUnterMacOS();

// Liefert die Version des Mac-Pakets aus der Info.plist des App-Bundles.
// FALSE, wenn nicht unter macOS oder die Info.plist nicht gefunden/gelesen werden konnte.
BOOL GetMacPaketVersion(CString& csVersion);

// TRUE, wenn ein Mac-Paket gefunden wurde, dessen Version aelter als MAC_PAKET_MINDESTVERSION ist.
// In csVersion steht dann die gefundene Version.
BOOL IstMacPaketVeraltet(CString& csVersion);

// Unter CrossOver auf macOS zeigt PrintDlgA den nativen macOS-Druckdialog. Waehlt der Anwender dort
// "Als PDF sichern", liefert CrossOvers comdlg32 keine Druckerinfo und greift danach auf einen
// NULL-Zeiger zu. DruckdialogMitCrossOverSchutz() ruft pfnDruckdialog(pKontext) auf (das letztlich
// pPD->DoModal() aufruft), faengt diesen Absturz ab, raeumt den MFC-Zustand von pPD auf und weist
// den Anwender darauf hin. Liefert TRUE, wenn ein Absturz abgefangen wurde; sonst steht in nErgebnis
// der Rueckgabewert von pfnDruckdialog.
typedef INT_PTR (*PFN_DRUCKDIALOG)(void* pKontext);
BOOL DruckdialogMitCrossOverSchutz(CPrintDialog* pPD, PFN_DRUCKDIALOG pfnDruckdialog, void* pKontext, INT_PTR& nErgebnis);
