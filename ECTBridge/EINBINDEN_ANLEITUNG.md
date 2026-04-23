# ECTBridge in EasyCash einbinden — Schritt-für-Schritt

## Überblick

Nach diesen Schritten ist die Kette vollständig:

```
EasyCash.exe (nativ)
    ↓ linkt gegen ECTBridge.lib
    ↓ lädt ECTBridge.dll
ECTBridge.dll (/clr Mixed-Mode)
    ↓ #using "ECTEngine.dll"
    ↓ CLR startet automatisch beim ersten Managed-Aufruf
ECTEngine.dll (C#/.NET 4.8)
    → Betrag, Buchung, BuchungsDocument, ...
```

---

## Schritt 1: Projekte zur Solution hinzufügen

In Visual Studio: Rechtsklick auf die Solution → "Hinzufügen" →
"Vorhandenes Projekt..." und nacheinander hinzufügen:

1. `ECTEngine\ECTEngine\ECTEngine.csproj`
2. `ECTBridge\ECTBridge.vcxproj`

**Reihenfolge bestätigen** — Rechtsklick auf Solution →
"Projektbuildabhängigkeiten...":

| Projekt      | hängt ab von        |
|-------------|---------------------|
| ECTEngine   | (nichts)            |
| ECTBridge   | ECTEngine           |
| EasyCash    | ECTBridge, ECTIFace |

## Schritt 2: ECTBridge-GUID anpassen

Die GUID im mitgelieferten `ECTBridge.vcxproj` ist ein Platzhalter.
Du kannst sie behalten oder an dein Setup anpassen. Wichtig ist nur,
dass sie einmalig in der Solution ist.

## Schritt 3: EasyCash.vcxproj ändern

### 3a. Include-Pfad hinzufügen

In **jeder** ItemDefinitionGroup (Debug|Win32, Release|Win32, etc.)
den Include-Pfad für ECTBridge hinzufügen.

Suche in der vcxproj nach `<ClCompile>` innerhalb der
`<ItemDefinitionGroup>` und füge eine Zeile hinzu:

```xml
<ClCompile>
  ...
  <!-- NEU: ECTBridge-Header finden -->
  <AdditionalIncludeDirectories>ECTBridge;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
  ...
</ClCompile>
```

Falls `<AdditionalIncludeDirectories>` schon existiert, den Wert
`ECTBridge;` vorne anfügen:

```xml
<AdditionalIncludeDirectories>ECTBridge;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```

### 3b. Linker-Abhängigkeit hinzufügen

In den `<Link>`-Abschnitten die ECTBridge.lib ergänzen:

**Debug|Win32:**
```xml
<Link>
  <AdditionalDependencies>ECTBridge\Debug\ECTBridge.lib;ECTIFace\Debug\ECTIFace.lib;...restliche libs...</AdditionalDependencies>
```

**Release|Win32:**
```xml
<Link>
  <AdditionalDependencies>ECTBridge\Release\ECTBridge.lib;ECTIFace\Release\ECTIFace.lib;...restliche libs...</AdditionalDependencies>
```

(Analog für x64-Konfigurationen.)

**ECTIFace.lib bleibt vorerst drin!** Wir ersetzen sie erst, wenn die
Bridge alle benötigten Symbole exportiert.

### 3c. DLL-Kopie sicherstellen

Die ECTBridge.dll und ECTEngine.dll müssen im selben Verzeichnis wie
EasyCT.exe liegen. Am einfachsten per Post-Build-Event in
`ECTBridge.vcxproj`:

```xml
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
  <PostBuildEvent>
    <Command>
      copy /Y "$(OutDir)ECTBridge.dll" "$(SolutionDir)Debug\"
      copy /Y "$(OutDir)ECTEngine.dll" "$(SolutionDir)Debug\"
    </Command>
    <Message>ECTBridge + ECTEngine DLLs ins Ausgabeverzeichnis kopieren</Message>
  </PostBuildEvent>
</PropertyGroup>
```

Oder händisch: Nach dem Build die beiden DLLs neben die EasyCT.exe
kopieren (ins `Debug\`- bzw. `Release\`-Verzeichnis des
EasyCash-Projekts).

## Schritt 4: EasyCash.cpp — Bridge beim Start testen

In der Datei `EasyCash.cpp` (die `CEasyCashApp::InitInstance()` enthält):

### 4a. Include hinzufügen

Am Anfang der Datei, zu den anderen #includes:

```cpp
#include "Exports.h"  // ECTBridge
```

### 4b. Testaufruf in InitInstance()

In `CEasyCashApp::InitInstance()`, nach der MFC-Initialisierung
(nach `AfxEnableControlContainer()` oder ähnlichem), BEVOR das
erste Dokument geöffnet wird:

```cpp
BOOL CEasyCashApp::InitInstance()
{
    // ... bestehender MFC-Init-Code ...

    // ─── ECTBridge-Integrationstest (Phase 1) ───
    {
        const char* bridgeVersion = ECT_BridgeVersion();
        TRACE("ECTBridge: %s\n", bridgeVersion);

        int selfTestResult = ECT_BridgeSelfTest();
        if (selfTestResult == 0)
        {
            TRACE("ECTBridge: Selbsttest bestanden.\n");
        }
        else
        {
            CString msg;
            msg.Format("ECTBridge-Selbsttest fehlgeschlagen (Code %d).\n"
                       "Die .NET-Engine ist nicht korrekt eingebunden.",
                       selfTestResult);
            AfxMessageBox(msg, MB_ICONWARNING);
        }
    }
    // ─── Ende ECTBridge-Test ───

    // ... restlicher bestehender Code (DocTemplate, ParseCommandLine, ...) ...
```

## Schritt 5: Build und Test

1. **Solution neu builden** (F7 / Strg+Shift+B).
   Build-Reihenfolge: ECTEngine → ECTBridge → EasyCash.

2. **Debug starten** (F5).

3. **Im Output-Fenster** (Debug → Fenster → Ausgabe) nach folgenden
   Meldungen suchen:

```
ECTBridge.DLL: Initialisiere MFC-Extension-Modul     ← NativeInit.cpp
ECTBridge: ECTBridge 1.0 | ECTEngine 1.0.0.0 | CLR 4.0.xxxxx  ← Exports.cpp
ECTBridge: Selbsttest bestanden.                      ← Exports.cpp
```

4. **Wenn alles klappt**, erscheint KEINE MessageBox — nur die
   TRACE-Ausgaben im Debug-Output. Wenn der Selbsttest fehlschlägt,
   zeigt eine MessageBox den Fehlercode an.

## Troubleshooting

### `ECTBridge.dll wurde nicht gefunden`
Die DLL liegt nicht neben EasyCT.exe. Siehe Schritt 3c.

### `ECTEngine.dll wurde nicht gefunden` / `FileNotFoundException`
ECTEngine.dll muss NEBEN ECTBridge.dll liegen (nicht im GAC).
Beide DLLs müssen im selben Verzeichnis wie EasyCT.exe sein.

### `BadImageFormatException`
Plattform-Mismatch: ECTEngine ist "Any CPU", ECTBridge ist
Win32/x64. Stelle sicher, dass EasyCash und ECTBridge dieselbe
Plattform nutzen (typischerweise Win32). Falls ECTEngine als
"Any CPU" gebaut wird, funktioniert es mit beiden.

### `Mixed mode assembly is built against version ... of the runtime`
In der `App.config` von EasyCash (oder neben der EasyCT.exe)
eine Datei `EasyCT.exe.config` ablegen:

```xml
<?xml version="1.0" encoding="utf-8"?>
<configuration>
  <startup useLegacyV2RuntimeActivationPolicy="true">
    <supportedRuntime version="v4.0" sku=".NETFramework,Version=v4.8"/>
  </startup>
</configuration>
```

### Linker-Fehler `unresolved external symbol _ECT_BridgeVersion`
ECTBridge.lib wird nicht gefunden oder nicht gelinkt.
Prüfe den Pfad in `AdditionalDependencies` (Schritt 3b).

### Trace-Ausgaben erscheinen nicht
TRACE-Makros funktionieren nur im Debug-Build.
Im Release-Build kann man stattdessen `OutputDebugString` nutzen
oder den Selbsttest vorübergehend mit einer MessageBox bestätigen.
