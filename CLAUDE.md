# EasyCash&Tax — Migration MFC/C++ → C#/WPF

Benutze  zum Speichern das Windows-1252 Encoding für alle legacy .cpp- und .h-Dateien.

## Projekt-Kontext

EasyCash&Tax ist eine 25 Jahre alte MFC-MDI-Buchhaltungs-Anwendung
(EÜR-Fibu) von Thomas Mielke. Quelle:
https://github.com/Thomas-Mielke-Software/EasyCash. Ziel: Schritt für
Schritt zu einer hybriden C#/.NET-Architektur überführen, wobei das
alte MFC-Programm weiterhin baubar und lauffähig bleibt
(`USE_ECTENGINE`-Switch).

Lokales Arbeitsverzeichnis: `C:\Users\mielk\Work\EasyCash`

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
    Resources/
      icons.bmp                  — Sprite mit Betrieb-Icons (32x32 horiz.)
      icons_bestandskonten.bmp   — Sprite mit Bestandskonto-Icons
    IconSpriteSplitter.cs        — Auto-Detect Icon-Größe aus Sprite-Höhe
    ViewHost.cs                  — statische Listen (BetriebeNamen etc.)
  ECTBridge/                   — C++/CLI Mixed-Mode
    ViewExports.h(.cpp)          — ECT_ShowBuchungBearbeitenDialog,
                                   ECT_SetzeBetriebeUndBestandskonten, …
    JournalExports.h(.cpp)       — ECT_JournalEinbetten, ECT_NavigationEinbetten,
                                   ECT_JournalAktualisiere, ECT_JournalSetzeZoom
    EasyCashDocBridge.h          — CEasyCashDocBridge + GetEngine(bridge)
```

## Wichtige Konventionen

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

## Offene Punkte

Siehe `TODO.md`.
