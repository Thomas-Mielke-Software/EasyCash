# RES\werkzeuge -- Icon- und Bitmap-Erzeugung

Skripte, die die Programmgrafiken aus den Master-PNGs neu erzeugen.
Reines PowerShell 5.1 + System.Drawing, keine externen Werkzeuge.

## Aufrufe

```
powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-programmicon.ps1
powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-bitmaps.ps1
powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-dokicon.ps1
powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-dokicon.ps1 -Vorschau
```

Die Pfade ermitteln die Skripte selbst (`$PSScriptRoot`), sie schreiben
direkt nach `RES\`. Am `.rc` ist nie etwas zu ändern -- alle Dateinamen
bleiben gleich.

## Was woher kommt

| Ziel | Ressource | Verwendung | Skript |
|---|---|---|---|
| `easycash.ico` | `IDR_MAINFRAME` | Programm-/Fenster-Icon | `baue-programmicon.ps1` |
| `easycashdoc.ico` | `IDR_EASYCATYPE` | Dokument-Icon | `baue-dokicon.ps1` |
| `easycash.bmp` | `IDB_EASYCASH` | Ribbon-Anwendungsknopf | `baue-bitmaps.ps1` |
| `EasyCash32x32.bmp` | `IDB_EASYCASH_32` | Platzhalter Plugin-Manager | `baue-bitmaps.ps1` |

Quelle ist jeweils `RES\EasyCash_v4_512x512.png` (512x512, RGBA); das
Dokument-Icon legt seinen eigenen Master als `RES\EasyCashDoc_v4_512x512.png`
daneben. Beide Master matchen das `.gitignore`-Muster `RES/EasyCash*x*.png`
-- zum Einchecken also `git add -f`.

## Warum eigener Skalierer

`bildwerkzeug.ps1` verkleinert mit einem exakten Flächenfilter auf
vormultipliziertem Alpha. Die naheliegenden Alternativen taugen hier nicht:

* **GDI+ / WPF** interpolieren die RGB-Werte auch dort, wo Alpha 0 ist.
  Hinter den transparenten Pixeln der Quell-PNGs liegt Schwarz, das ergibt
  einen dunklen Saum um das Motiv.
* **WPFs `BitmapScalingMode.HighQuality`** liefert bei der extremen
  Verkleinerung 512 -> 16 sichtbar ausgefranste Kanten.

## Formate, die eingehalten werden müssen

* **ICO**: 16/20/24/32/40/48/64 als 32bpp-BMP-Einträge (inkl. korrekt
  gesetzter AND-Maske), 128/256 als PNG-Einträge. `rc.exe` des
  v143-Toolsets verträgt beides.
* **`easycash.bmp`**: 32bpp, `BITMAPINFOHEADER`, **gerades** (nicht
  vormultipliziertes) Alpha -- `CMFCToolBarImages` premultipliziert beim
  Laden selbst. Voll transparente Pixel sind auf Weiß gemattet, damit ein
  Code-Pfad ohne Alpha-Auswertung Weiß statt eines schwarzen Kastens zeigt.
* **`EasyCash32x32.bmp`**: 24bpp opak auf Weiß. Die `CImageList` wird mit
  `ILC_COLOR24`, Maskenfarbe `RGB(255,0,255)` und weißer Hintergrundfarbe
  angelegt (`PluginManager.cpp`); Magenta als Bildhintergrund gäbe nur
  rosa Ränder.

## Nach dem Austausch

Explorer zeigt Dokument-Icons noch aus dem Icon-Cache. Auffrischen mit
`ie4uinit.exe -show` oder Ab-/Anmelden.
