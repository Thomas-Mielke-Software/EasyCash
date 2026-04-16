# ECTBridge — C++/CLI Mixed-Mode Brücke zur ECTEngine

ECTBridge ersetzt die alte ECTIFace-DLL und verbindet die native MFC-Applikation
EasyCT.exe mit der neuen C#-Engine (ECTEngine.dll).

## Architektur

```
┌──────────────────────────────────────────────────┐
│  EasyCT.exe (native MFC)                         │
│    Views, Dialoge — unverändert                  │
│    include "EasyCashDocBridge.h" statt           │
│            "ECTIFace\EasyCashDoc.h"              │
└────────────────┬─────────────────────────────────┘
                 │ link: ECTBridge.lib
                 ▼
┌──────────────────────────────────────────────────┐
│  ECTBridge.dll (/clr Mixed-Mode)                 │
│  ┌────────────────────────────────────────────┐  │
│  │  Native Teil (wie altes ECTIFace)          │  │
│  │  • CBuchung, CDauerbuchung, CBetrag        │  │
│  │  • CEasyCashDoc  (MFC CDocument)           │  │
│  │  • currency_to_int, GetErweiterungKey, ... │  │
│  └────────────────┬───────────────────────────┘  │
│                   │ erbt                         │
│  ┌────────────────▼───────────────────────────┐  │
│  │  Bridge-Teil (/clr aktiviert)              │  │
│  │  • CEasyCashDocBridge : CEasyCashDoc       │  │
│  │  • BuchungConverter  (managed ↔ native)    │  │
│  │  • EngineHost  (gcroot<BuchungsDocument^>) │  │
│  └────────────────┬───────────────────────────┘  │
└───────────────────┼──────────────────────────────┘
                    │ #using "ECTEngine.dll"
                    ▼
┌──────────────────────────────────────────────────┐
│  ECTEngine.dll (C#, .NET Framework 4.8)          │
│    Geschäftslogik, Berechnungen                  │
└──────────────────────────────────────────────────┘
```

## Was wurde wo migriert?

| Datei | Alt | Neu |
|-------|-----|-----|
| `CBuchung`, `CDauerbuchung`, `CBetrag` (Deklaration) | `ECTIFace/EasyCashDoc.h` | `ECTBridge/EasyCashDoc.h` (unverändert) |
| `CEasyCashDoc`-Implementierung + Serialize | `ECTIFace/EasyCashDoc.cpp` | `ECTBridge/EasyCashDoc.cpp` (unverändert) |
| C-Hilfsfunktionen (`currency_to_int`, `GetErweiterungKey`, ...) | `ECTIFace/*.cpp` | `ECTBridge/*.cpp` (unverändert) |
| Neue Bridge-Klasse | — | `ECTBridge/EasyCashDocBridge.h/.cpp` |
| Managed-Engine-Handle | — | `ECTBridge/EngineHost.h` |
| Typ-Marshalling | — | `ECTBridge/Marshalling.h` |
| Buchungs-Konvertierung | — | `ECTBridge/BuchungConverter.h/.cpp` |

## Integrations-Schritte

### Schritt 1: ECTIFace-Dateien kopieren

Die Inhalte von `ECTIFace/*` werden in `ECTBridge/` kopiert. Das alte
ECTIFace-Projekt kann im Solution bleiben (archivarisch) oder gelöscht werden.

Dateien, die mit `<CompileAsManaged>false</CompileAsManaged>` in der Projektdatei
aufgeführt werden müssen (siehe `ECTBridge.vcxproj`):
- `EasyCashDoc.cpp`
- `AfAGenauigkeit.cpp`
- `Konvertierung.cpp`
- `BuchungsjahrWaehlen.cpp`
- `Datensicherungsoptionen.cpp`
- `Startoptionen.cpp`
- `XMLite.cpp`
- alle weiteren Dialog- und Hilfsdateien aus dem alten ECTIFace

### Schritt 2: Solution-Struktur

```
EasyCash.sln
├── EasyCash.vcxproj          # Hauptapplikation, nativ
├── ECTBridge.vcxproj         # Diese Bridge (C++/CLI)
├── ECTEngine\ECTEngine.csproj   # C# Engine
├── ECTEngine.Tests\ECTEngine.Tests.csproj
└── ECTIFace\                 # optional archivieren/löschen
```

Build-Reihenfolge (Project Dependencies):
1. `ECTEngine`
2. `ECTBridge` (abhängig von ECTEngine)
3. `EasyCash` (abhängig von ECTBridge)

### Schritt 3: Anpassungen in EasyCash.vcxproj

**Linker-Abhängigkeit umstellen:**
```xml
<!-- vorher -->
<AdditionalDependencies>ECTIFace\Debug\ECTIFace.lib;...

<!-- nachher -->
<AdditionalDependencies>..\ECTBridge\Debug\ECTBridge.lib;...
```

**Include-Pfade:** entweder
```cpp
// in allen Views und Dialogen:
#include "ECTIFace\EasyCashDoc.h"
```
unverändert lassen und Symbol-Link oder IncludePath ergänzen, oder global
ersetzen durch:
```cpp
#include "..\ECTBridge\EasyCashDoc.h"
```

### Schritt 4: Dokumenttemplate-Registrierung

In `EasyCash.cpp` (CEasyCashApp::InitInstance):

```cpp
// vorher
pDocTemplate = new CMyMultiDocTemplate(
    IDR_EASYCTYPE,
    RUNTIME_CLASS(CEasyCashDoc),       // <── alt
    RUNTIME_CLASS(CChildFrame),
    RUNTIME_CLASS(CEasyCashView));

// nachher
#include "..\ECTBridge\EasyCashDocBridge.h"

pDocTemplate = new CMyMultiDocTemplate(
    IDR_EASYCTYPE,
    RUNTIME_CLASS(CEasyCashDocBridge), // <── neu
    RUNTIME_CLASS(CChildFrame),
    RUNTIME_CLASS(CEasyCashView));
```

Da `CEasyCashDocBridge` von `CEasyCashDoc` erbt, funktionieren alle bestehenden
Zugriffe über `CEasyCashDoc* pDoc = GetDocument()` weiterhin — polymorphes
Verhalten genügt.

### Schritt 5: Deployment

Beim Laden der EXE muss zur Laufzeit erreichbar sein:
- `ECTBridge.dll` (im Exe-Verzeichnis)
- `ECTEngine.dll` (im Exe-Verzeichnis)
- .NET Framework 4.8 (standardmäßig auf Windows 10+ vorhanden)

Für das InnoSetup-Installationsskript `easycash.iss`:
```iss
Source: "ECTBridge\Release\ECTBridge.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "ECTEngine\bin\Release\net48\ECTEngine.dll"; DestDir: "{app}"; Flags: ignoreversion
```

## Funktionsweise der Synchronisation

### Beim Laden (`OnOpenDocument` / `Serialize(IsLoading)`)

```
CArchive liest .eca-Datei
        │
        ▼
CEasyCashDoc::Serialize (nativ, unverändert)
        │
        ▼ fertig eingelesen
        │
        ▼ Bridge-Hook:
CEasyCashDocBridge::NativeToManaged()
        • Iteriert Einnahmen-Linked-List → engine.Buchungen.Add(...)
        • Iteriert Ausgaben-Linked-List  → engine.Buchungen.Add(...)
        • Iteriert Dauerbuchungen        → engine.Dauerbuchungen.Add(...)
        • Kopiert Metadaten (nJahr, Waehrung, Zähler, etc.)
        • engine.Sort()
```

### Beim Speichern (`OnFileSave` / `Serialize(IsStoring)`)

```
Bridge-Hook:
CEasyCashDocBridge::ManagedToNative()
        • Metadaten aus engine zurück in native Felder
        • Linked-Lists freigeben
        • Aus engine.Buchungen neue Linked-Lists aufbauen
        • (Einnahmen getrennt von Ausgaben nach Buchung.Art)
        │
        ▼
CEasyCashDoc::Serialize (nativ, unverändert)
        │
        ▼
CArchive schreibt .eca-Datei im bekannten Format
```

### Während des Betriebs

Die nativen Linked-Lists (`pDoc->Einnahmen`, `pDoc->Ausgaben`) sind die primäre
Datenquelle. Views greifen wie gewohnt darauf zu.

**Hinweis:** Änderungen an den Linked-Lists während der Laufzeit werden NICHT
automatisch an die Engine propagiert. Für Phase 3 ist das akzeptabel, weil:
- Die Engine primär für Serialize-Operationen gebraucht wird
- Alle Änderungen durchlaufen früher oder später einen Save → dort wird synchronisiert
- Neue Features können die Engine-API direkt nutzen (über `GetEngineHost()`)

In Phase 5 (Aufräumen) können Views schrittweise auf die Engine-API umgestellt
werden, wodurch diese Lücke geschlossen wird.

## Bekannte Fallstricke

### Loader Lock
Mixed-Mode DLLs sind empfindlich gegenüber dem "Loader Lock"-Problem. Daher:
- **DllMain bleibt nativ** (keine managed Aufrufe darin)
- Die managed Initialisierung passiert lazy im `EngineHost`-Konstruktor

### PCH und /clr
Eine /clr-kompilierte Datei kann keinen nativen PCH verwenden. Daher haben
die Bridge-Dateien `<PrecompiledHeader>NotUsing</PrecompiledHeader>`.

### Inkompatible Compiler-Flags bei /clr
- `/GL` (Whole Program Optimization): deaktiviert
- `/LTCG` (Link Time Code Generation): deaktiviert
- `/RTC` (Runtime Checks): deaktiviert
- `Edit and Continue`: nicht möglich

### 32-bit vs 64-bit
Das bestehende EasyCash-Projekt unterstützt Win32 und x64. ECTBridge sollte für
beide Plattformen gebaut werden. Die ECTEngine-Assembly ist plattformunabhängig
(AnyCPU) und funktioniert mit beiden.

### COM-Plugin-Kompatibilität (Phase 4)
Plugins wie ECTElster, ECTDatev, ECTImport greifen über EasyCTX.ocx auf die
Buchungsdaten zu. Da `ECTBridge.dll` denselben Klassennamen `CEasyCashDoc`
(via Vererbung) und dieselben Datenstrukturen exportiert, funktionieren diese
Plugins ohne Änderung.

Eine leere `ECTIFace.dll`-Stub-Bibliothek könnte zusätzlich bereitgestellt
werden, falls Plugins via LoadLibrary darauf zugreifen (zu prüfen in Phase 4).
