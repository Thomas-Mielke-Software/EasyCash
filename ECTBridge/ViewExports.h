// ViewExports.h — Native C-Exportschnittstelle für ECTViews
//
// Wird von EasyCash.exe inkludiert, um WPF-Dialoge aus nativem
// MFC-Code aufzurufen. Die Implementierung in ViewExports.cpp
// (C++/CLI) ruft intern ECTViews::ViewHost auf.
//
// Einbindung in EasyCash:
//   #include "ViewExports.h"
//   // in einem Menü-Handler oder View:
//   ECT_ShowBuchungDialog(pDoc, TRUE, AfxGetMainWnd()->GetSafeHwnd());

#pragma once

#ifdef ECTBRIDGE_EXPORTS
#define ECTBRIDGE_API __declspec(dllexport)
#else
#define ECTBRIDGE_API __declspec(dllimport)
#endif

// Forward-Deklaration für die Pointer-basierten Funktionen.
// Die View-Dateien inkludieren EasyCashDoc.h ohnehin und haben
// dann die volle Definition; die forward decl hier hält den
// Header leichtgewichtig und funktioniert auch in reinen C-APIs.
class CBuchung;

#ifdef __cplusplus
extern "C" {
#endif

// ──────────────────────────────────────────────
// Index-basierte API (Grundfunktionen)
// ──────────────────────────────────────────────

/// <summary>
/// Zeigt den WPF-Buchungseingabe-Dialog.
///
/// Parameter:
///   pDocBridge - Pointer auf CEasyCashDocBridge (wird intern zu
///                BuchungsDocument^ aufgelöst)
///   bAusgaben  - TRUE für Ausgaben, FALSE für Einnahmen
///   hWndOwner  - HWND des MFC-Elternfensters (für modale Darstellung)
///
/// Rückgabe:
///   TRUE wenn eine Buchung erstellt wurde (sie wurde bereits in das
///   Dokument eingefügt und die Engine synchronisiert).
///   FALSE wenn der Dialog abgebrochen wurde.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungDialog(
    void* pDocBridge, BOOL bAusgaben, HWND hWndOwner);

/// <summary>
/// Wie ECT_ShowBuchungDialog, aber mit einer beim Oeffnen bereits
/// vorgewaehlten Buchungsvorlage (Buchungsposten). Aufgerufen ueber das
/// Dropdown-Menue der Einnahme-/Ausgabe-Ribbon-Knoepfe.
///
/// Parameter:
///   nVorlagenSlot - Slot 0-99 der Vorlage im Einstellungs-Cache. Passt die
///                   Vorlage nicht zur Buchungsart (bAusgaben) oder ist der
///                   Slot leer, wird sie ignoriert (leerer Dialog).
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungDialogMitVorlage(
    void* pDocBridge, BOOL bAusgaben, HWND hWndOwner, int nVorlagenSlot);

/// <summary>
/// Zeigt den WPF-Dialog zur Bearbeitung einer bestehenden Buchung.
///
/// Parameter:
///   pDocBridge  - Pointer auf CEasyCashDocBridge
///   nBuchungIdx - Index der Buchung in der Engine-Buchungsliste
///   hWndOwner   - HWND des MFC-Elternfensters
///
/// Rückgabe:
///   TRUE wenn die Buchung geändert wurde.
///   FALSE wenn abgebrochen.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungBearbeitenDialog(
    void* pDocBridge, int nBuchungIdx, HWND hWndOwner);

/// <summary>
/// Zeigt den Bearbeiten-Dialog mit einer beim Öffnen angewandten
/// Buchungsgruppen-Vorlage -- der Weg vom Journal-Kontextmenü
/// "Umwandeln in <Vorlage>".
///
/// Aus der Einzelbuchung wird beim Speichern eine Buchungsgruppe (Basis +
/// Zusatz-Buchungen der Vorlage). War die Buchung schon Gruppen-Mitglied,
/// wird die Gruppe unter derselben UUID auf die neue Vorlage umgestellt.
/// Datum, Betrag, Belegnummer und ein selbst getippter Beschreibungstext
/// bleiben erhalten; Konto, MWSt und AfA kommen aus der Vorlage.
///
/// Parameter:
///   nBuchungIdx   - Index der Buchung in der Engine-Buchungsliste
///   nVorlagenSlot - Slot 0-99 der Buchungsgruppen-Vorlage. Passt die
///                   Vorlage nicht zur Buchungsart oder ist sie nicht
///                   mehrzeilig, wird sie ignoriert (normale Bearbeitung).
///
/// Rückgabe: TRUE wenn gespeichert wurde, FALSE bei Abbruch.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungUmwandelnDialog(
    void* pDocBridge, int nBuchungIdx, int nVorlagenSlot, HWND hWndOwner);

// ──────────────────────────────────────────────
// Pointer-basierte API (komfortabel für bestehenden View-Code)
// ──────────────────────────────────────────────
//
// Die folgenden Funktionen nehmen einen nativen CBuchung*-Pointer
// entgegen, wie er in der existierenden MFC-View-Logik (z.B.
// ppPosBuchungsliste, *ppb) schon vorhanden ist. Die Bridge
// identifiziert intern die korrespondierende managed Buchung,
// führt die Operation aus und synchronisiert native ↔ managed.
//
// WICHTIG: Nach jedem dieser Aufrufe muss der View sein Display
// neu zeichnen, weil sich native Linked-List-Positionen verschoben
// haben können (Sort, Neueinfügen, Löschen).

/// <summary>
/// Zeigt den Bearbeiten-Dialog für eine bestehende Buchung,
/// die per CBuchung*-Pointer identifiziert ist.
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungBearbeitenDialogFuerPointer(
    void* pDocBridge, CBuchung* pNativeBuchung, HWND hWndOwner);

/// <summary>
/// Zeigt den Buchungsdialog zum Kopieren einer bestehenden Buchung.
/// Der Dialog startet mit den Feldwerten der Vorlage; beim OK-Klick
/// wird eine NEUE Buchung angelegt (nicht die Vorlage überschrieben).
///
/// Parameter:
///   bNeueBelegnummer - TRUE: Belegnummer wird auf die nächste freie
///                      Nummer gesetzt. FALSE: Belegnummer wird aus
///                      der Vorlage übernommen (inkl. evtl. Suffix).
/// </summary>
ECTBRIDGE_API BOOL ECT_ShowBuchungKopierenDialog(
    void* pDocBridge, CBuchung* pNativeBuchung,
    BOOL bNeueBelegnummer, HWND hWndOwner);

/// <summary>
/// Löscht eine Buchung, die per CBuchung*-Pointer identifiziert ist.
/// Nach der Löschung sind alle zuvor gehaltenen CBuchung*-Pointer
/// ungültig (auch der übergebene!), weil SyncManagedToNative die
/// nativen Linked Lists komplett neu aufbaut.
///
/// Rückgabe:
///   TRUE wenn gelöscht, FALSE wenn der Pointer nicht gefunden wurde.
/// </summary>
ECTBRIDGE_API BOOL ECT_LoescheBuchungPerPointer(
    void* pDocBridge, CBuchung* pNativeBuchung);

// ──────────────────────────────────────────────
// Listen-Initialisierung (Betrieb + Bestandskonto)
// ──────────────────────────────────────────────
//
// EasyCash hält die Betriebs-/Bestandskonten-Daten in den Structs
// CBetrieb (m_Betriebe) und CBestandskonto (m_Bestandskonten) in der View.
//
// Diese Funktion übergibt eine Schnappschuss-Kopie an den ViewHost,
// der ihn beim nächsten Dialog-/Journal-Aufruf nutzt. Wenn die Listen
// leer sind, werden die zugehörigen UI-Elemente ausgeblendet.

/// <summary>
/// Setzt die Betriebs- und Bestandskonten-Listen für nachfolgende
/// Buchungsdialog- und Journal-Aufrufe.
///
/// Icons werden als int-Indizes übergeben (kein atoi mehr nötig).
/// pBestandskontenSalden enthält Vorjahres-Anfangssalden in Cent.
/// Die Bridge kopiert die Inhalte intern, der Aufrufer kann die
/// Arrays nach dem Aufruf verwerfen.
///
/// Übergibt man nCount = 0, werden die Listen ausgeblendet.
/// </summary>
ECTBRIDGE_API void ECT_SetzeBetriebeUndBestandskonten(
    LPCSTR* pBetriebeNamen, const int* pBetriebeIcons, int nBetriebeCount,
    LPCSTR* pBestandskontenNamen, const int* pBestandskontenIcons,
    const int* pBestandskontenSalden, int nBestandskontenCount);

// ──────────────────────────────────────────────
// Dauerbuchungen
// ──────────────────────────────────────────────

/// <summary>
/// Zeigt den WPF-Dauerbuchungs-Verwaltungsdialog (ersetzt DauerbuchungenDlg;
/// modal statt modeless). Synchronisiert vorher Native->Managed, arbeitet
/// auf BuchungsDocument.Dauerbuchungen und synchronisiert bei Änderungen
/// zurück (inkl. SetModifiedFlag).
///
/// Rückgabe: TRUE wenn Dauerbuchungen geändert wurden.
/// </summary>
ECTBRIDGE_API BOOL ECT_ZeigeDauerbuchungenDialog(
    void* pDocBridge, HWND hWndOwner);

/// <summary>
/// Zeigt den kleinen "Dauerbuchungen ausführen bis Monat/Jahr"-Dialog
/// (ersetzt DauBuchAusfuehren). Die eigentliche Ausführung bleibt beim
/// Aufrufer: CEasyCashView::DauerbuchungenAusfuehren(*pnJahrOut, *pnMonatOut).
///
/// Rückgabe: TRUE wenn "Ausführen" geklickt wurde; *pnMonatOut/*pnJahrOut
/// sind dann gültig (Jahr bereits normalisiert).
/// </summary>
ECTBRIDGE_API BOOL ECT_ZeigeDauerbuchungenAusfuehrenDialog(
    int nBuchungsjahr, HWND hWndOwner, int* pnMonatOut, int* pnJahrOut);

/// <summary>
/// Zeigt den USt-Vorauszahlungen-Dialog (ersetzt CUstVorauszahlungenDlg):
/// 12 Monats- + 4 Quartals-Zahlbetraege plus Sondervorauszahlung der
/// Dauerfristverlaengerung, gespeichert im Dokument-ErweiterungStore
/// (Sektionen "Elster" bzw. "Dauerfristverlängerung").
///
/// Rueckgabe: TRUE wenn OK gedrueckt wurde UND sich Werte geaendert haben
/// (dann wurde bereits zurueck-synchronisiert und das Modified-Flag gesetzt).
/// </summary>
ECTBRIDGE_API BOOL ECT_ZeigeUstVorauszahlungenDialog(
    void* pDocBridge, HWND hWndOwner);

// ──────────────────────────────────────────────
// Stammdaten-Verwaltung (Betriebe + Bestandskonten)
// ──────────────────────────────────────────────

/// <summary>
/// Zeigt den WPF-Verwaltungs-/Auswahl-Dialog für Betriebe (ersetzt
/// CIconAuswahlBetrieb im Modus 1, siehe OnViewJournalBetrieb).
/// Anlegen/Löschen/Umbenennen/Icon/Unternehmensart werden sofort über
/// den globalen Einstellungs-Cache in die easyct.ini geschrieben --
/// der Aufrufer muss danach seine Betriebe-Liste neu einlesen
/// (UpdateBetriebeMenu ruft UpdateBetriebe).
///
/// Rückgabe:
///   >= 0  Index des gewählten Betriebs ("Sel. anzeigen" -> Filter setzen)
///   -1    "Alle anzeigen" / Abbruch -> Filter aufheben
/// </summary>
ECTBRIDGE_API int ECT_ZeigeBetriebeVerwaltenDialog(HWND hWndOwner);

/// <summary>
/// Wie ECT_ZeigeBetriebeVerwaltenDialog, für Bestandskonten (ersetzt
/// CIconAuswahlBestandskonto im Modus 1, siehe OnViewJournalBestandskonto).
///
/// Parameter:
///   nBuchungsjahr - Buchungsjahr des aktiven Dokuments (pDoc->nJahr);
///                   der Anfangssaldo-Dialog bearbeitet SaldoJJJJ des
///                   Vorjahres, wie CIconAuswahlBestandskonto::GetProperty.
/// </summary>
ECTBRIDGE_API int ECT_ZeigeBestandskontenVerwaltenDialog(
    int nBuchungsjahr, HWND hWndOwner);

/// <summary>
/// Zeigt den WPF-Verwaltungs-/Auswahl-Dialog für Mandanten (ersetzt
/// CIconAuswahlMandant im Modus 1, siehe CMainFrame::OnFileMandanten).
///
/// Die Mandanten liegen im Anwendungsprofil (theApp.GetProfileString),
/// das die Bridge nicht erreicht -- deshalb übergibt der Aufrufer die
/// aktuelle Liste und bekommt die (ggf. per Neu/Löschen/Umbenennen/Icon/
/// Datenverzeichnis geänderte) Liste über die Out-Puffer zurück. Er muss
/// sie IMMER zurückschreiben, wenn *pnAnzahlOut >= 0 ist -- auch bei
/// Abbruch, denn Verwaltungs-Änderungen sollen wie im Original erhalten
/// bleiben. Bei einer Exception ist *pnAnzahlOut == -1 (nichts schreiben).
///
/// Parameter:
///   pNamen/pIcons/pVerzeichnisse - aktuelle Mandanten (nAnzahl Stück)
///   pszNamenOut         - Puffer für nMaxAnzahlOut Namen, je nNamenStride Bytes
///   pIconsOut           - Puffer für nMaxAnzahlOut Icon-Indizes
///   pszVerzeichnisseOut - Puffer für nMaxAnzahlOut Pfade, je nVerzStride Bytes
///   pnAnzahlOut         - [out] Anzahl zurückgegebener Einträge (-1 bei Fehler)
///
/// Rückgabe:
///   >= 0  Index des gewählten Mandanten (bezogen auf die Out-Liste)
///   -1    Abbrechen -> kein Mandantenwechsel
///
///   pszNichtMandantVerzOut - [out] beim Loeschen des LETZTEN Mandanten
///                    dessen Datenverzeichnis (sonst leer); der Aufrufer
///                    wechselt dann in den Nicht-Mandanten-Modus und nutzt
///                    dieses Verzeichnis kuenftig als Datenverzeichnis.
/// </summary>
ECTBRIDGE_API int ECT_ZeigeMandantenVerwaltenDialog(
    LPCSTR* pNamen, const int* pIcons, LPCSTR* pVerzeichnisse, int nAnzahl,
    HWND hWndOwner,
    char* pszNamenOut, int nNamenStride,
    int* pIconsOut,
    char* pszVerzeichnisseOut, int nVerzStride,
    int nMaxAnzahlOut, int* pnAnzahlOut,
    char* pszNichtMandantVerzOut, int nNichtMandantVerzBufLen);

/// <summary>
/// Reiner Icon-Picker mit den Mandanten-Icons (= Betriebe-Sprite).
/// Für den Erstanlauf in OnFileMandanten ("Mandant 1" anlegen).
/// Rückgabe: Icon-Index, oder -1 bei Abbruch.
/// </summary>
ECTBRIDGE_API int ECT_ZeigeMandantIconAuswahlDialog(HWND hWndOwner);

// ──────────────────────────────────────────────
// "Buchungsjahr wählen"-Dialog (beim Anlegen eines neuen Dokuments)
// ──────────────────────────────────────────────

/// <summary>
/// Zeigt den WPF-Dialog "Buchungsjahr wählen", der beim Erzeugen eines
/// neuen Dokuments erscheint. Bietet zwei Aktionen: Jahreswechsel aus einer
/// bestehenden JahrXXXX.eca-Datei (empfohlen) oder Anlegen einer neuen,
/// leeren Buchungsdatei (Jahr + Währung).
///
/// Parameter:
///   pszDatenverzeichnis - Verzeichnis ohne abschließenden Backslash, in dem
///                         nach *.eca-Buchungsdateien gesucht wird
///   nDefaultJahr        - Vorbelegung des Jahr-Feldes
///   pszDefaultWaehrung  - Vorbelegung des Währungs-Feldes
///   hWndOwner           - HWND des MFC-Elternfensters (für modale Darstellung)
///   pnJahrOut           - [out] gewähltes Jahr (nur bei Rückgabe 1 gültig)
///   pszWaehrungOut      - [out] gewähltes Währungskürzel
///   nWaehrungBufLen     - Größe des Währungs-Puffers
///   pszQuelldateiOut    - [out] voller Pfad der Jahreswechsel-Quelldatei
///                         (nur bei Rückgabe 2 gültig)
///   nQuelldateiBufLen   - Größe des Quelldatei-Puffers
///
/// Rückgabe:
///   0 = Abbruch, 1 = neue Buchungsdatei, 2 = Jahreswechsel
/// </summary>
ECTBRIDGE_API int ECT_ShowBuchungsjahrWaehlenDialog(
    LPCSTR pszDatenverzeichnis, int nDefaultJahr, LPCSTR pszDefaultWaehrung,
    HWND hWndOwner,
    int* pnJahrOut, char* pszWaehrungOut, int nWaehrungBufLen,
    char* pszQuelldateiOut, int nQuelldateiBufLen);

/// <summary>
/// Merkt vor, dass nach dem nächsten OnInitialUpdate ein Jahreswechsel
/// angestoßen werden soll. Wird von CEasyCashDoc::OnNewDocument gesetzt, wenn
/// der Dialog die Jahreswechsel-Aktion liefert. Der Umweg über ein Bridge-
/// internes Flag (statt eines Doc-Members) vermeidet eine Layout-Divergenz
/// zwischen den parallel existierenden CEasyCashDoc-Definitionen in ECTIFace
/// und ECTBridge. Unkritisch, weil OnNewDocument und OnInitialUpdate eines
/// neuen Dokuments synchron innerhalb von OnFileNew nacheinander laufen.
/// </summary>
ECTBRIDGE_API void ECT_MerkeJahreswechselNachInit();

/// <summary>
/// Liefert das gemerkte Flag und löscht es (einmalige Konsumierung). Wird in
/// CEasyCashView::OnInitialUpdate abgefragt.
/// </summary>
ECTBRIDGE_API BOOL ECT_HoleUndLoescheJahreswechselNachInit();

// ──────────────────────────────────────────────
// Buchungsjournal
// ──────────────────────────────────────────────

/// <summary>
/// Öffnet das Buchungsjournal-Fenster (modeless) als WPF-View.
/// Replikat von DrawToDC_Datum/DrawToDC_Konten aus easycashview.cpp.
///
/// Der Aufrufer sollte vorher per ECT_SetzeBetriebeUndBestandskonten
/// die Listen für die Icons gesetzt haben.
///
/// Rückgabe:
///   TRUE wenn das Fenster geöffnet werden konnte.
/// </summary>
ECTBRIDGE_API BOOL ECT_ZeigeJournal(void* pDocBridge, HWND hWndOwner);

/// <summary>
/// Aktualisiert das offene Journal-Fenster — z.B. nach
/// Buchungsänderungen. Wenn kein Fenster offen ist, passiert nichts.
///
/// Parameter:
///   nAnzeigeModus      - 0 = nach Datum, 1 = nach Konten
///   pszKontenFilter    - "" oder "<alle Konten>" für kein Filter
///   nMonatsFilter      - 0=alle, 1-12=Monat, 13-16=Quartal
///   pszBetriebFilter   - "" für kein Filter
///   pszBestandskontoFilter - "" für kein Filter
///   dSchriftgroesse    - Zoom (z.B. 13.0 = Standard)
/// </summary>
ECTBRIDGE_API void ECT_AktualisiereJournal(
    int nAnzeigeModus,
    LPCSTR pszKontenFilter,
    int nMonatsFilter,
    LPCSTR pszBetriebFilter,
    LPCSTR pszBestandskontoFilter,
    double dSchriftgroesse);

// ──────────────────────────────────────────────
// Tastatur-Fokus
// ──────────────────────────────────────────────

/// <summary>
/// TRUE, wenn der WPF-Tastaturfokus gerade in einem Texteingabe-Element
/// steht (TextBox/RichTextBox/PasswordBox oder das Editierfeld einer
/// aufklappbaren ComboBox).
///
/// Gedacht für CMainFrame::PreTranslateMessage: die Accelerator-Tabelle
/// IDR_MAINFRAME belegt Strg+C/V/X/Z/A und die Einfg-/Entf-Varianten.
/// TranslateAccelerator würde sie schlucken, bevor ein eingebettetes
/// WPF-Textfeld sie sieht -- in den Einstellungen (und jedem anderen
/// WPF-Overlay) ließe sich dann weder kopieren noch einfügen. Steht der
/// Fokus in einem Textfeld, muss der Aufrufer den Accelerator auslassen.
///
/// Liegt der Fokus NICHT in einem Textfeld (z.B. in der Journal-Liste),
/// kommt FALSE zurück -- Strg+A bucht dort weiterhin eine Ausgabe.
/// </summary>
ECTBRIDGE_API BOOL ECT_WpfTextfeldHatFokus();

#ifdef __cplusplus
}
#endif
