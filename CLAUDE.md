# EasyCash&Tax — Migration MFC/C++ → C#/WPF

## Zeichensatznutzung in-code

Verzichte nach Möglichkeit auf exotische Sonderzeichen wie das Pfeil-Symbol "→" und 
benutze eher aus ASCII-Zeichen zusammengesetzte Pendants wie "-->".
Oder in Kommentaren, um Code optisch zu trennen, benutze einfach eine Zeile mit vielen 
Bindestrichen oder Gleichheitszeichen, z.B.
"// ----------------------------------------------------------"

## Projekt-Kontext

EasyCash&Tax ist eine 25 Jahre alte MFC-MDI-Buchhaltungs-Anwendung
(EÜR-Fibu) von Thomas Mielke. Quelle:
https://github.com/Thomas-Mielke-Software/EasyCash. Ziel: Schritt für
Schritt zu einer hybriden C#/.NET-Architektur überführen, wobei das
alte MFC-Programm weiterhin baubar und lauffähig bleibt
(`USE_ECTENGINE`-Switch).

Lokales Arbeitsverzeichnis: `C:\Users\mielk\Work\EasyCash-ectengine-mit-opus46`
Mach die änderungen immer (auch) direkt im arbeitsverzeichnis.
Git-Branch, auf dem gearbeitet wird: `ectengine-mit-opus46` (Fork von `master`).

## Build-Umgebung

- **Visual Studio 2022**, Toolset v143
- **C++**: MultiByte (NICHT Unicode), Win32 (32-bit)
- **.NET Framework 4.8** für ECTEngine, ECTViews, ECTBridge
- Kommentare + TRACE-Ausgaben **in Deutsch**, plain ASCII / Latin-1
  (keine Umlaute in Strings, sonst Mojibake bei TRACE/Native-Concat)

## Architektur

```
EasyCash.exe (MFC, native)
    ↓ linkt
ECTBridge.lib (C++/CLI Mixed-Mode)
    ↓ enthält native ECTIFace + #using
ECTEngine.dll (C#, .NET 4.8)         — reine Logik (decimal-basiert)
ECTViews.dll  (C#/WPF, .NET 4.8)     — BuchungView + Journal+Navigation

EasyCTX.ocx (ActiveX, MFC)            — alter Plugin-Container
    ↓ linkt
ECTBridge.lib                         — ja, dieselbe lib
```

Das ECTIFace-Subprojekt ist deprecated und soll in diesem Branch i.d.R. nicht angerührt, sondern durch ECTBridge ersetzt werden.
Nur wenn es nicht anders geht, um Kompatibilität zu erhalten, darf ECTIFace noch geändert werden.

### Interop-Mechanik

- **Pointer-Map** in `EngineHost`: gcroot-Dictionary `<IntPtr, Buchung^>`
  wird beim Sync gefüllt. Free inline functions `GetEngine(bridge)`,
  `FindManagedFor(bridge, ptr)`, `FindManagedIndexFor(bridge, ptr)`
  in einem Header lösen das `dllexport`/`__clrcall`-Konflikt-Problem.
- **`CEasyCashDocBridge`** ist die Brücke zwischen native CEasyCashDoc
  und managed BuchungsDocument. `SyncNativeToManaged()` /
  `SyncManagedToNative()` synchronisiert beide Welten.
- **`NativeInit.cpp`** mit globalem ctor stellt sicher, dass die
  managed Engine vor dem ersten nativen Zugriff initialisiert ist.

## Verzeichnis-Layout

```
EasyCash/
  ECTEngine/ECTEngine/         — C# Engine (Buchung, Document, AfA, …)
  ECTViews/ECTViews/           — WPF UserControls + ViewModels
    Views/                       — XAML-Dialoge
      BuchungView.xaml           — Buchungs-Eingabe-Dialog
    ViewModels/
      BuchungViewModel.cs
      IconListItem.cs
    Journal/                     — Journal+Navigation
      JournalView.xaml(.cs)
      JournalRow.cs              — Datenklassen pro Zeilentyp
      JournalFilter.cs           — Filter-Optionen + Modus-Enum
      JournalViewModel.cs        — Filter, Build, Scroll-Methoden
      JournalRowStyleSelector.cs — Style+Zebra
      JournalEmbed.cs            — HwndSource-Hosting für MFC-Parent
      NavigationView.xaml(.cs)
      NavigationItem.cs
      NavigationViewModel.cs
    Dauerbuchungen/              — Dauerbuchungs-Verwaltung + Ausführen-Abfrage
      DauerbuchungenView.xaml(.cs)          — Liste + Editor (modal)
      DauerbuchungenViewModel.cs            — arbeitet auf doc.Dauerbuchungen
      DauerbuchungenAusfuehrenView.xaml(.cs) — "ausführen bis Monat/Jahr"
    Stammdaten/                  — Betriebe-/Bestandskonten-/Mandanten-Verwaltung
      StammdatenVerwaltenView.xaml(.cs) — Verwaltung + Auswahl (alle 3 Typen)
      StammdatenVerwaltenViewModel.cs   — abstrakt + Betriebe/Bestandskonten
      MandantenVerwaltenViewModel.cs    — In-Memory-Liste (App-Profil!),
                                          FolderBrowserDialog (WinForms)
      IconAuswahlView.xaml(.cs)  — wiederverwendbarer Icon-Picker
      IconKatalog.cs             — Icon-Namen (Spiegel von IconAuswahl*.cpp)
      UnternehmensartView.xaml   — Betriebsdaten (Tab-getrennter ini-Wert)
      AnfangssaldoView.xaml      — Anfangssaldo (SaldoJJJJ des Vorjahres)
    Druck/                       — WYSIWYG-Druck (WPF, FixedDocument)
      DruckDokument.cs           — Pagination, Kopf-/Fußzeile, PrintDialog,
                                   Seitenansicht-Aufruf
      JournalDruckBauer.cs       — druckt die JournalRow-Listen des VM
      BerichtDruckBauer.cs       — druckt die Formlos-Berichte
      DruckVorschauFenster.xaml  — DocumentViewer-Seitenansicht
                                   (In-Memory-XPS-Roundtrip, sonst findet
                                   das Such-Feld nichts; Druck-Kommando
                                   umgeleitet auf DruckDokument.Drucke)
      DateiPromptVorbelegung.cs  — belegt das Dateinamen-Feld des
                                   "Druckausgabe speichern unter"-Prompts
                                   (Microsoft Print to PDF) per WM_SETTEXT
                                   mit dem Jobnamen vor. Windows übernimmt
                                   den Jobnamen dort NICHT selbst (getestet
                                   Win11 26200); der Prompt läuft aber
                                   in-process (32+64bit verifiziert),
                                   darum kann ein Watcher-Thread das
                                   Edit-Feld (#32770/ID 1001) setzen
    Berichte/                    — Formlos-Ansicht (EÜR/USt/Kontenplan)
      BerichtView.xaml(.cs)      — Vollflächen-Ansicht + Zoom/Strg+P
      BerichtViewModel.cs        — projiziert ECTEngine.Bericht auf Zeilen-VMs
      BerichtEmbed.cs            — HwndSource-Hosting (Muster JournalEmbed)
    Resources/
      icons.bmp                  — Sprite mit Betrieb-Icons (32x32 horiz.)
      icons_bestandskonten.bmp   — Sprite mit Bestandskonto-Icons
    IconSpriteSplitter.cs        — Auto-Detect Icon-Größe aus Sprite-Höhe
    GemeinsameStile.xaml         — app-weite Button-Stile (PrimaryButtonStyle/
                                   SecondaryButtonStyle, per MergedDictionaries
                                   einbinden; genutzt von BuchungView +
                                   Stammdaten-Dialogen)
    ViewHost.cs                  — statische Listen (BetriebeNamen etc.)
  ECTBridge/                   — C++/CLI Mixed-Mode
    ViewExports.h(.cpp)          — ECT_ShowBuchungBearbeitenDialog,
                                   ECT_SetzeBetriebeUndBestandskonten, …
    JournalExports.h(.cpp)       — ECT_JournalEinbetten, ECT_NavigationEinbetten,
                                   ECT_JournalAktualisiere, ECT_JournalSetzeZoom,
                                   ECT_JournalDrucken + Druck-Callback (Strg+P)
    BerichtExports.h(.cpp)       — ECT_BerichtEinbetten/-Aktualisiere/-Drucken
                                   (Formlos-Ansicht)
    EasyCashDocBridge.h          — CEasyCashDocBridge + GetEngine(bridge)
```

## Wichtige Konventionen

### Beträge: Locale-Verhalten (Waehrungsformat)

Beträge/Prozente auf der C#-Seite laufen IMMER über
`ECTEngine.Waehrungsformat` — KEIN hartkodiertes de-DE:
- **Anzeige + Persistenz**: `Betrag()` / `BetragOhneGruppierung()` /
  `Zahl()` formatieren mit `CurrentCulture` (Pendant zur
  DEZIMALKOMMA/TAUSENDERTRENNER-Mechanik: NativeInit.cpp holt
  LOCALE_SDECIMAL etc., currency_to_int/int_to_currency nutzen sie —
  native Leser verstehen so die von WPF geschriebenen ini-Werte).
- **Parsen**: `TryParse()` / `TryParseProzent()` sind locale-FREI und
  tolerant (deutsch "1.234,56", schweizerisch "1'234.56", englisch
  "1,234.56") — wichtig beim Mandanten-Umzug zwischen Systemen.
  Regel: rechts stehender Trenner = Dezimal; einzelner Punkt/Komma =
  Dezimal; mehrfach = Gruppierung; Apostroph/Leerraum = Gruppierung.
- **Ausnahme**: die Formelsprache der Buchungsgruppen-Vorlagen
  (FormelParser) bleibt bewusst locale-fest — Formeln sind portable
  Vorlagen-Daten, Komma UND Punkt gelten dort immer als Dezimaltrenner.

### XAML-Stolperfallen

- `BooleanToVisibilityConverter` MUSS **ganz oben** in `<Resources>`
  stehen, weil WPF Resources sequenziell parst. DataTriggers, die ihn
  via `StaticResource` nutzen, müssen DANACH definiert werden.
- `XAML-Kommentare dürfen kein '--' enthalten** (XML-Strenge). Statt
  `--- Section ---` darum `[Section]` schreiben.
- **TextWrapping="Wrap" in einer ListBox** braucht drei Dinge zusammen:
  1. `ScrollViewer.HorizontalScrollBarVisibility="Disabled"` an der ListBox
  2. `HorizontalContentAlignment="Stretch"` am ListBoxItem-Style
  3. `Width="*"` an der Spalte (nicht `Auto`)
- **`VirtualizingPanel.ScrollUnit="Pixel"`** wenn vertikales Zentrieren
  per `ScrollViewer.ScrollToVerticalOffset()` funktionieren soll —
  Default ist `Item`, dann sind `VerticalOffset`/`ViewportHeight` in
  Item-Counts statt Pixel.

### C++/CLI-Stolperfallen

- **C++/CLI-Lambdas können keine managed Variablen capturen** und
  können auch nicht direkt einem `System::Action<T>`-Delegate
  zugewiesen werden. Lösung: `ref class` mit IntPtr-Feldern und
  Member-Methoden, die als Delegate-Targets dienen
  (`gcnew System::Action<T>(handler, &Klasse::OnXxx)`).
- **`#using <System.dll>`** in jedem File, das `INotifyPropertyChanged`
  oder andere System-Typen referenziert.
- **`/clr`** und **kein PCH** für jede C++/CLI-Datei.

### Win32-Hosting-Stolperfallen

- WPF in einer `CScrollView` hosten **funktioniert nicht zuverlässig**
  (Scroll-Mechanik kollidiert mit HwndSource). Stattdessen den
  **Splitter-Parent** nehmen und das WPF-HWND mit den
  Splitter-Koordinaten der Pane positionieren.
- **`WS_CLIPCHILDREN` darf NICHT am `CSplitterWnd` gesetzt sein** —
  `CSplitterWnd::StartTracking()` macht XOR-Drawing für die
  Drag-Vorschau, das funktioniert mit `WS_CLIPCHILDREN` nicht
  (ASSERT in winsplit.cpp 1143).
- **Native Pane-Views per `ShowWindow(SW_HIDE)` verstecken** wenn
  WPF sie überlagert, sonst malt ihr `OnDraw` ständig drüber.
- **`OnDraw` und `OnUpdate`** der nativen View müssen einen
  `IstJournalWpfAktiv()`-Frühausstieg haben, sonst Race Conditions.

### ViewHost / Listen-Cache

`ViewHost` hält statische Listen + Sprite-Bitmaps:
- `BetriebeNamen`, `BetriebeIcons` (Indizes als Strings)
- `BestandskontenNamen`, `BestandskontenIcons`
- `SpriteBetriebe`, `SpriteBestandskonten` (BitmapSource)

Werden via `ECT_SetzeBetriebeUndBestandskonten` einmal pro Session
gesetzt. ECTViews-Resources für die Sprites müssen **Build Action =
"Eingebettete Ressource"** haben (NICHT "Ressource") — funktioniert
robuster in hosted-WPF-Szenarien.

## Aktueller Stand

### ECTEngine (komplett)
- `Betrag` (decimal), `Buchung`, `BuchungsDocument`
- `AfaCalculator.GetBuchungsjahrNetto`, `Restwert`-Heuristik
- `ErweiterungStore` für `UrspruenglichesKonto` u.a.

### ECTViews — BuchungView (komplett)
- Validierung pro Feld + global
- AfA-Heuristik: Restwert auto + editierbar (User kann übersteuern)
- 0%-Hinweis bei degressiv+Satz=0+Jahre<99 (alle Setter triggern)
- Betriebe/Bestandskonten als ListBox mit Icons (statt ComboBox)
- Validierung "bitte auswählen" wenn Liste sichtbar aber leer
- Auto-Hide der Listen wenn keine Daten gesetzt
- `IstBearbeitung` als notifying Property mit `OkButtonText`
  ("Buchen" / "Speichern")
- `MinWidth=480 MinHeight=640 Width=800 SizeToContent=Height
  ResizeMode=CanResizeWithGrip`

### ECTViews — Journal+Navigation (komplett)
- 4 Modi: Datum (0), Konten (1), Bestandskonten (2), Anlagenverzeichnis (3)
- Adaptive Belegspalte: `Width="Auto"` mit `SharedSizeGroup`,
  MaxWidth = 1/4 der ListBox-Breite (gesetzt vom Code-Behind)
- Mehrzeilige Beschreibungs-Spalte (`TextWrapping="Wrap"`)
- Vertikales Zentrieren beim Navigations-Klick (eigener Algorithmus
  mit `ContainerFromItem` + `TransformToAncestor` + `ScrollToVerticalOffset`,
  funktioniert dank `ScrollUnit=Pixel`)
- Zebra-Streifen, gelbe Selektion, Doppelklick → Bearbeiten,
  Kontextmenü mit Ändern/Löschen/Kopieren/Kopieren-mit-Beleg
- Modus-abhängige Saldo-Spalte (Bestandskonten zeigt sie, andere
  Modi: 0px breit über `MinWidth=MaxWidth=0`-Binding an
  `JournalViewModel.SaldoSpaltenBreite`)
- Navigation rechts (passt zum CExtSplitter-Layout: Pane 0 = View,
  Pane 1 = Navigation)
- **Schriftgrößen/Zoom (Stand 2026-07-13)**: alle "festen" Spaltenbreiten
  (Icon/Datum/Brutto/AfA + Saldo/Steuer), Icon-Größen und die
  Abschnittstitel-Schriften kommen als skalierende VM-Properties
  (`Skala = Schriftgroesse/13`, `SkalierungAktualisieren()`) — nichts
  mehr hartkodiert, sonst Text-Abschneiden bei großer
  `Bildschirmschriftgroesse` bzw. Zoom > 100%. Die Icons werden auf
  exakt EINE Textzeilenhöhe skaliert (`IconGroesse` = FontSize x
  LineSpacing der FontFamily), damit die Textzeile die Zeilenhöhe
  bestimmt — größere Icons blähen die Zeile auf und der oben
  ausgerichtete Text säße bei kleinen Schriften sichtbar zu hoch.
  `JournalSchriftgroesse()` (easycashview.cpp) rechnet Punkt -> DIP
  (pt x 96/72), analog zum nativen `MulDiv(pt, LOGPIXELSY, 72)`.
  Zoom-Tasten (Strg-)'+'/'-' und Strg-Mausrad fängt das WPF-Journal
  selbst ab (die native View hat im WPF-Modus keinen Tastatur-Fokus
  mehr) und meldet sie über das `ZoomAendern`-Event ->
  `ECT_JournalRegistriereZoomAenderung`-Callback ->
  `CEasyCashView::JournalWpfZoomAenderung` an `SetzeZoomfaktor()`
  (Profil-Persistenz, Statuszeile, Broadcast an alle Journals).

### ECTBridge / Embedding (komplett)
- `ECT_JournalEinbetten(parent, x,y,w,h, doc, modus, zoom)` →
  HwndSource als Kind des Splitters, gibt das HWND zurück
- `ECT_NavigationEinbetten(parent, x,y,w,h, hwndJournal)` →
  zweite Pane, an dieselbe ViewModel-Hierarchie gehängt
- Eventhandler-Verdrahtung in `JournalEventHandler` (ref class)
- `ECT_JournalSetzeZoom(pt)` für `OnZoomfaktor*`
- `ECT_JournalAktualisiere(...)` für Filter-Änderung
- `ECT_JournalAlleAbloesen()` zum Aufräumen

### Integration in EasyCashView (in den 4 Journal-Modi aktiv)
- `OnViewJournalDatum/Konten/Bestandskonten/Anlagenverzeichnis`:
  per `USE_ECTENGINE` neuer Pfad ruft `ZeigeJournalWpf(modus)`
- `ZeigeJournalWpf` bettet WPF in Splitter-Parent ein, versteckt
  CEasyCashView+CNavigation per `SW_HIDE`
- `OnSize` → `GroessenAnpassungJournalWpf` repositioniert beide
  WPF-HWNDs (auch beim Splitter-Drag, weil
  `CNavigation::OnSize` denselben Hook ruft)
- `OnDraw` und `OnUpdate` haben Frühausstieg bei `IstJournalWpfAktiv()`
- `SetzeZoomfaktor` ruft `ECT_JournalSetzeZoom`

### ECTViews — Stammdaten-Verwaltung (komplett, Stand 2026-07-07)
- `StammdatenVerwaltenView` ersetzt `CIconAuswahlBetrieb`/`CIconAuswahlBestandskonto`
  (Modus 1): Verwaltung (Neu/Löschen/Umbenennen inline/Icon/Property)
  + Filter-Auswahl ("Sel. anzeigen" liefert Index, "Alle anzeigen" = -1).
- Persistenz: `ECTEngine.StammdatenVerwaltung` hält Einträge roh
  (Suffix->Wert), schreibt die ini-Sektion komplett neu über
  `Einstellungen.SchreibeSektion` — SaldoJJJJ-Keys und unbekannte
  Suffixe überleben, Reihenfolge bleibt beim Löschen stabil (anders
  als das alte Verschieben des letzten Eintrags in die Lücke).
- Exports: `ECT_ZeigeBetriebeVerwaltenDialog(hwnd)` /
  `ECT_ZeigeBestandskontenVerwaltenDialog(nJahr, hwnd)`; Aufrufer in
  `OnViewJournalBetrieb`/`OnViewJournalBestandskonto` (USE_ECTENGINE),
  danach liest `UpdateBetriebeMenu`/`UpdateBestandskontenMenu` die
  ini neu ein (Bridge schreibt synchron).
- W-IdNr-Prüfung nach `WIdNrPruefung.cs` extrahiert (geteilt von
  FinanzamtPage + UnternehmensartView).
- **Mandanten (Stand 2026-07-08)**: dritte VM-Subklasse, aber ANDERE
  Persistenz — Mandanten liegen im App-Profil (theApp/Registry), nicht
  in der easyct.ini. Listen-Roundtrip über
  `ECT_ZeigeMandantenVerwaltenDialog` (Liste rein, geänderte Liste über
  Out-Puffer zurück); `OnFileMandanten` (mainfrm.cpp) schreibt sie IMMER
  zurück (auch bei Abbrechen) und wechselt nur bei Index >= 0.
  Datenverzeichnis-Picker = WinForms FolderBrowserDialog. Erstanlauf:
  `ECT_ZeigeMandantIconAuswahlDialog`.
- **Moduswechsel beim Löschen (Stand 2026-07-15)**: Wird der VORLETZTE
  Mandant gelöscht (Liste danach = 1), meldet der Dialog vorab, dass das
  Löschen des letzten Mandanten in den Nicht-Mandanten-Modus zurückführt.
  Wird der LETZTE Mandant gelöscht (Liste danach = 0), meldet der Dialog
  den Moduswechsel und schließt sich; sein Datenverzeichnis wird über
  `MandantenVerwaltenErgebnis.NichtMandantenModusDatenverzeichnis` +
  neuen Out-Puffer (`ECT_ZeigeMandantenVerwaltenDialog`) zurückgereicht.
  Der native Aufrufer (`OnFileMandanten`; im Startup-Pfad `WaehleMandantWpf`
  mit Rückgabe -2) setzt es als `Allgemein\Datenverzeichnis`, ruft
  `SetMandant(-1)` + `SetIniFileName` und lädt die letzte Datei. Die
  Meldungslogik sitzt in `StammdatenVerwaltenViewModel.NachLoeschen`
  (virtuell; nur Mandanten überschreiben sie).

### ECTViews — Dauerbuchungen (komplett, Stand 2026-07-08)
- `DauerbuchungenView` (modal, ersetzt den modeless `DauerbuchungenDlg`):
  Liste + Editor mit Zustandsmodell wie das Original (Liste gesperrt
  während Bearbeitung; "ausgeführt bis" nur beim Ändern editierbar).
  Presets/Konten/MwSt (vat1..4)/Betriebe/Bestandskonten kommen aus dem
  Einstellungs-Cache der Engine.
- `DauerbuchungenAusfuehrenView` fragt nur Monat/Jahr ab — die
  **Ausführung bleibt nativ** in `CEasyCashView::DauerbuchungenAusfuehren`
  (Platzhalter inkl. $+m/$--M/$#, Buchungsjahr-Rückfrage,
  Journal-Selektion). Engine-`ResolvePlatzhalter` kennt nur die
  Basis-Platzhalter.
- Exports: `ECT_ZeigeDauerbuchungenDialog` (Sync beide Richtungen +
  SetModifiedFlag bei Änderung), `ECT_ZeigeDauerbuchungenAusfuehrenDialog`.
- MwSt-Skala beachten: Engine-"Promille" = Prozent x1000 (19 % = 19000).

### Buchungsgruppen (Phasen A+B fertig, Stand 2026-07-09)
- Generalisierung des Privat-Splits: mehrzeilige Presets
  ([Buchungsposten]-Slot + `NNZ<k><Feld>`-Keys, `NNBasisBetrag`-Formel)
  erzeugen mehrere per UUID verknüpfte Buchungen (Erweiterungs-Keys
  `Buchungsgruppe`/`BuchungsgruppeRolle`/`BuchungsgruppeVorlage`;
  Alt-Split-Keys werden lesend weiter erkannt).
- Engine: `FormelParser.cs` (Arithmetik + Interpolation, KEIN
  C#-Scripting — geteilte Vorlagen dürfen keinen Code ausführen),
  `Buchungsgruppen.cs` (`BuchungsgruppenRechner`), Tests in
  ECTEngine.Tests.
- UI: `BuchungViewModel.Gruppen.cs` (partial) + Zeilen-Bereich in
  BuchungView (kompakt/Maske je `Darstellung`-Override); Ergebnis des
  Buchen-Dialogs ist jetzt eine LISTE (`ErgebnisBuchungen`,
  `GebuchtUndWeiter` liefert Listen; ECT_ShowBuchungDialog fügt alle ein).
- **Gruppen-Bearbeitung**: Bearbeiten eines Gruppenmitglieds öffnet die
  Basis mit kompletter Gruppe (`ZeigeBuchungBearbeitenKombiDialog`);
  Speichern ersetzt die Zusatz-Buchungen unter gleicher UUID
  (`ECTBridge_BearbeiteBuchung` in ViewExports.cpp). Fallback
  Einzel-Bearbeitung bei Alt-Split/fehlender Vorlage. Kopieren entfernt
  Gruppen-Keys (`Buchung.EntferneGruppe`). Bearbeiten erhält seit dem
  Phase-A-Bugfix Erweiterungen + AfaGenauigkeit des Originals.
- **Phase C fertig (2026-07-09)**: Zeilen-Editor auf der PresetsPage
  (Live-Formelprüfung; Zeilen ohne Konto werden nicht gespeichert),
  einmaliger [Privat-Split]-Import beim Cache-Laden (Marker-Key
  `[Buchungsposten]PrivatSplitImportiert`, Formel-Variable `$mwstsatz`),
  XML-Export/Import über `PresetXml.cs` (*.ectvorlage.xml, reine Daten).
- **Phase D fertig (2026-07-12)**: Gruppen-Mitglieder tragen im Journal
  ein dezentes "[G]"-Präfix in der Beschreibungsspalte (Tooltip mit
  Rolle); Klick auf ein Mitglied markiert die ganze Gruppe als Block
  (Ctrl/Shift-Klick erlaubt weiter Einzelauswahl innerhalb der Gruppe;
  die Einzel-Kommandos Ändern/Kopieren/AfA-Abgang akzeptieren deshalb
  auch eine komplett selektierte Gruppe als "eine" Selektion und wirken
  auf die Primärzeile bzw. via Kombi-Dialog auf die Basis). Kontextmenü
  "Buchungsgruppe löschen" löscht alle Mitglieder (ein Sync, ein
  SetModifiedFlag). Beim Löschen einzelner Mitglieder fragt die geteilte
  Bridge-Funktion `ECTBridge_LoescheBuchungenMitGruppenAbfrage`
  (`BuchungenLoeschenShared.h`, Definition in JournalExports.cpp, genutzt
  von beiden JournalEventHandlern) per Ja/Nein/Abbrechen, ob die ganze
  Gruppe gelöscht werden soll (Kaskadenlöschen).

### Ad-hoc-Kontoselektor / HoleKontoMitFeldern (Stand 2026-07-15)
- Konto-Feld einer Vorlagen-Zeile (und Basis-Konto eines Presets) kann
  statt eines Kontonamens eine **Feld-Spezifikation** tragen:
  `$de:E/Ü-Rechnung=1103|Umsatzsteuer-Voranmeldung=48||at:Beilage E1a=9040|Umsatzsteuer=1020||`
  ('$' + Land-Blöcke mit "||" getrennt, Kürzel de/at/ch nach
  `[Persoenliche_Daten]land` 0/1/2; pro Block `Formularname=Feld-Id`-Paare
  mit '|' getrennt, UND-Kombination; aufgelöst wird das ERSTBESTE Konto
  in Slot-Reihenfolge, Einnahmen vor Ausgaben). Erkennungs-Heuristik:
  `$` + zwei Buchstaben + `:` (kollidiert nicht mit Template-Variablen).
- Engine: `KontoFeldSelektor.cs` (Parse/LoeseAuf/FindeKonto rein lesend —
  läuft live in `BuchungsgruppenRechner.Berechne`; `ErmittleFeldInfo`
  liest Feldnamen + E/A-Typ aus den .ecf via EUKonten; `ErzeugeKonto`
  persistiert über die EUKonten-Mechanik). Tests: KontoFeldSelektorTests.
- UI: fehlt das Konto, öffnet beim LADEN der Vorlage `KontoAnlegenView`
  (nur Kontoname einzugeben, vorbelegt mit den Feldnamen " / "-getrennt);
  Abbruch lässt den Zeilen-Fehler stehen und blockiert das Buchen.
  Verdrahtung über `BuchungViewModel.KontoAnlegenAbfrage` (Callback aus
  dem BuchungView-Code-Behind). Der PresetsPage-Zeilen-Editor prüft die
  Spezifikation live auf Syntaxfehler.
- Plugin-API: `HoleKontoMitFeldern(LPCSTR spez)` — V4 in
  `ECTBridge/KontoExports.cpp` (managed über `ViewHost.HoleKontoMitFeldern`),
  V3 nativ in `ECTIFace/ectifacemisc.cpp` (auf master implementiert,
  Commit 41b144b, hierher gemergt; eigener MFC-Dialog IDD_KONTO_ANLEGEN).
  Rückgabe: Kontoname oder "" (Abbruch, Spezifikations-Fehler oder alle
  100 Slots belegt — Fehler als MessageBox). Beide Welten müssen sich
  identisch verhalten (Plugin-Kompatibilität V3/V4).

### WYSIWYG-Druck + Formlos-Berichte (komplett, Stand 2026-07-19)
- **Prinzip**: Gedruckt wird exakt die aktuelle Ansicht mit den aktiven
  Ribbon-Filtern. Der alte `DruckauswahlDlg` (Parallel-Filterwelt
  `m_*FilterPrinter` + tagesgenauer von/bis-Bereich) ist im
  USE_ECTENGINE-Pfad abgeklemmt; tagesgenauer Zeitraum entfällt
  ersatzlos. `.ecf`-Formulardruck bleibt nativ (`DrawFormularToDC`).
- **Druck-Infrastruktur** (`ECTViews/Druck/`): `DruckDokument` baut aus
  `DruckBlock`-Listen ein A4-FixedDocument (Kopfzeile Titel+Filter+Datum,
  Seitennummern, Keep-with-next für Abschnittstitel, Spaltenkopf-
  Wiederholung nach Seitenumbruch). Journal: `JournalDruckBauer` rendert
  die JournalRow-Listen des ViewModels mit fester Druckschrift 10
  (zoom-unabhängig; Spaltenlayout wie am Bildschirm, Icon-Spalten nur
  wenn belegt). Seitenansicht = `DruckVorschauFenster` (DocumentViewer).
- **Formlos-Ribbon-Split-Button** ("Formlos\l", Ansicht-Panel, Default =
  Freestyle-EÜR; Menü `IDR_ANSICHT_FORMLOS`): vier Berichte als ANSICHT
  (Vollflächen-Overlay wie die Einstellungen, `ZeigeBerichtWpf(typ)` /
  `VerstecktBerichtWpf` in easycashview.cpp; Moduswechsel Journal/
  Formular/Plugin/Einstellungen räumen den Bericht ab). Engine-Logik
  decimal-basiert + getestet: `EuerBericht` (Port von DrawEURechungToDC),
  `UstErklaerungBericht` (Port von DrawUmStErklaerungToDC inkl.
  Elster-Vorauszahlungen), `KontenplanBericht` (neu, aus `EUKonten`),
  Modell `Berichte.cs` (`Bericht`/`BerichtZeile`/`BerichtZeitraum`).
- **Verdrahtung**: Strg+P im WPF-Journal/Bericht → Callback →
  `OnFilePrint2` (ein Einstiegspunkt, Wine-Guard
  `DruckenUnterWineGesperrt()`); `ID_FILE_PRINT`-Accelerator im
  USE_ECTENGINE-Pfad auf `OnFilePrint2`; Filter-Änderungen →
  `AktualisiereJournalFilter` → auch `ECT_BerichtAktualisiere`;
  Zoom → `ECT_BerichtSetzeZoom`. Buchen-Handler-Guards um
  `IstBerichtWpfAktiv()` erweitert.
- **Plugin-Druck**: `DruckePlugin(bVorschau)` (easycashview.cpp) —
  COM-Kaskade im Plugin-Modus: IE-`ExecWB` (HTML-Plugins, auch
  Seitenansicht) → `IOleCommandTarget` → `IPrint` → Fenster-Abbild
  per `WM_PRINT`+`PRF_CHILDREN` auf Bitmap, gestreckt auf Drucker-DC
  (Einseiten-Fallback; `IViewObject2::Draw` zeichnet bei
  fensterbasierten MFC-Controls wie ECTElster/Fahrtenbuch die
  Kindfenster NICHT — leere Seite — und ist nur noch Reserve für
  fensterlose Controls). Plugins
  laufen in-process, HDC-Übergabe wäre daher auch für eine spätere
  kooperative Plugin-Methode (`DruckeSeite(hdc, n)`) möglich.

### Persistenz
- `NavigationBreitenverhaeltnis` (Promille) wird vom existierenden
  `CExtSplitter`/`CChildFrame::OnSize` automatisch gehalten — kein
  Extra-Code nötig.

## Test-Daten
- 372 Buchungen, 4 Dauerbuchungen (typische Testdatei)

## Wichtige Code-Wegweiser im Original

```
easycashview.cpp:
  273-310    OnInitialUpdate, SetupScroll
  1051-1267  Navigations-Befüllungs-Block (USE_ECTENGINE skippt das)
  1482-1545  OnSize + SetupScroll
  1577-...   OnDraw (USE_ECTENGINE: weißer Hintergrund + return)
  2415       DrawToDC_Datum (Vorlage für JournalViewModel.BaueAnzeigeNachDatum)
  2706       DrawToDC_Konten
  3145       DrawToDC_Bestandskonten
  3457       DrawToDC_Anlagenverzeichnis
  9497ff     ActiveX-Plugin-Embedding (Vorlage für HwndSource-Embedding)
  9928       SetzeZoomfaktor

childfrm.cpp:
  118-122    Splitter-Setup (ViewWnd/NavigationWnd-Verknüpfung)
  178+       OnSize mit NavigationBreitenverhaeltnis-Persistenz

extsplitter.cpp:
  395        WriteProfileInt für NavigationBreitenverhaeltnis

navigation.cpp:                — alte CListCtrl-Navigation, im
                                 USE_ECTENGINE-Modus per SW_HIDE
                                 verdeckt durch WPF-Navigation

buchendlg.cpp:                 — Original-MFC-Buchungsdialog
                                 (durch BuchungView ersetzt im
                                 USE_ECTENGINE-Pfad)
```

## Konvention für ECTBridge-Patches

Wenn ich Änderungen an `easycashview.cpp` etc. liefere, ist der Code
im Patch immer **vollständig im `#ifdef USE_ECTENGINE`-Block**
gekapselt. Der alte MFC-Render-Pfad bleibt unverändert daneben
existieren und kann durch Wegnehmen des Switches reaktiviert werden.

**Ausnahme**: Beim Refactoring der Einstellungs-Architektur (siehe unten)
ist `USE_ECTENGINE` explizit nicht zu beachten — der Cache wird global,
und ein paralleler alter Pfad würde nur Inkonsistenzen schaffen.

## Globaler Einstellungs-Cache (Plugin-API-kompatibel)

Statt der alten Pro-Dokument-Verwaltung (`einstellungen1..5` als
`CPropertyPage`-Member in jeder `CEasyCashView`) gibt es jetzt einen
**globalen Key-Value-Cache** in `ECTEngine::Einstellungen`:

```
ECTEngine/Einstellungen.cs           — Static Dictionary, WertGeaendert-Event
ECTBridge/EinstellungenExports.h     — Native C-API
ECTBridge/EinstellungenExports.cpp   — C++/CLI-Bridge, ini-I/O
```

### Lifecycle
- App-Start: `SetIniFileName()` (in `ECTBridge/ectifacemisc.cpp`) ruft
  jetzt automatisch `ECT_LadeEinstellungen()` → Cache wird komplett
  aus easyct.ini befüllt.
- Mandantenwechsel: Selbiges, da `OnFileMandanten` / `OnFileWaehleDatenverzeichnis`
  in `mainfrm.cpp` `SetIniFileName()` aufrufen.
- Settings-Dialog OK: `SaveProfile()` ruft am Ende `ECT_LadeEinstellungen()`
  → andere offene MDI-Dokumente sehen die Änderung sofort.
- `ECT_SpeichereEinstellung()` schreibt synchron in die ini.

### Schlüsselformat (wie Plugin-API)
- Kurzform: `"fname"` → Sektion `Finanzamt`, Ini-Key `name`
            (Auflösung via `IniSektion()` aus `ectifacemisc.cpp`).
- Explizit: `"[Sektion]Key"`.
- **Variante X**: Cache-Key = exakt was der Aufrufer übergibt.
  Konvention: immer Kurzform verwenden (so wie Plugin-Interface).

### Zugriff aus nativem Code
```cpp
#include "ECTBridge\EinstellungenExports.h"
int g = ECT_HoleEinstellungInt("AbschreibungGenauigkeit", 0);
ECT_SpeichereEinstellung("fname", "Finanzamt Musterstadt");
```

### Stand der Migration
- Infrastruktur: ✅ fertig
- Bulk-Replacement von `einstellungen[1-5]->m_xxx`: nur an einer
  repräsentativen Stelle demonstriert (`buchendlg.cpp:1100`),
  Rest siehe TODO.md.
- Arrays (`EinnahmenRechnungsposten` etc.): explizit ausgenommen,
  später separater Plan.

## Offene Punkte

Siehe `TODO.md`.
