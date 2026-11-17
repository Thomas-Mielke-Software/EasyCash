# USt-Abgleich Voranmeldung / Erklärung -- Portierung nach v4 (Übergabe)

## 0. Stand der Umsetzung (2026-11-16, Opus 5.5)

Abschnitte 1-9 sind die ursprüngliche Arbeitsanweisung und bleiben als
Referenz stehen. Erledigt:

- Schritt 1 (statisch statt am laufenden Programm): Der gemergte MFC-Weg taugt
  in v4 NICHT. `UStAbgleichMeldung()` zählt auf `einstellungen1->...Feldzuweisungen[]`,
  die im WPF-Pfad nur beim Öffnen der Ansicht geladen werden
  (`VerstecktEinstellungenWpf` lädt nicht nach) -- nach Änderungen in der
  Konten-Seite also veraltet. Schlimmer: `OnUStAbgleich()` ruft `SaveProfile()`,
  das das GANZE Profil aus diesen Kopien zurückschreibt und damit jede seither
  in den WPF-Einstellungen gemachte Änderung überschriebe. Beides läuft in v4
  jetzt managed (siehe unten); `SaveProfile()` fällt im `USE_ECTENGINE`-Zweig weg.
- Entscheidungen aus Abschnitt 7 (Thomas, 2026-11-16): Hinweis in der nativen
  CaptionBar (Text/Zählung managed); Konten-Seite mit rotem Anschluss +
  Kopfzeile "Abgleichen..."; automatisch angelegte Konten bekommen die
  Erklärungs-Zuweisung still, wenn sie eindeutig ist (Spezifikationen bleiben
  unverändert).
- Schritt 2: `ECTEngine/UstAbgleich.cs` + `ECTEngine.Tests/UstAbgleichTests.cs`
  (alle Fälle aus `test.cpp`, dazu Zählung, Hinweistext, Pipe-Roundtrip des
  Vermerks, Konto-Anlage). 432/432 grün. Die Zahlen aus Abschnitt 3 (47/46
  uvafeld-Felder) sind als Test festgehalten.
- Schritt 3: Konten-Seite (`KontenPageViewModel.WeiseFeldZu` -> `UstMitfuehren`,
  `AnschlussZustand.UstUnstimmig`, `AnzahlKontenUnstimmig`/`UnstimmigText`,
  Rückfragen per MessageBox, Feldauswahl als Kontextmenü an der Maus).
- Schritt 4: `ECTViews/UstAbgleich/UstAbgleichView` (+ViewModel); Bridge
  `ECT_UstAbgleichMeldung` / `ECT_ZeigeUstAbgleichDialog` (ViewExports);
  `easycashview.cpp` `UStAbgleichMeldung`/`OnUStAbgleich` mit `USE_ECTENGINE`-Zweig.
- Nachtrag 2026-11-17 (Abnahme Thomas: Hinweis erschien nicht): Im WPF-Pfad
  läuft die Formularwahl über `ZeigeFormularWpf()` OHNE `UpdateAllViews()`,
  `CaptionBoxCheckOnUpdate()` kam also nie. Jetzt am Ende von
  `ZeigeFormularWpf()` (Neuaufbau und Wechsel) und `VerstecktFormularWpf()`
  aufgerufen; `UStAbgleichMeldung()` meldet in v4 nur bei aktiver
  Formular-Ansicht. Rote Anschlüsse heller (#FF3B30).
- v3 schreibt eine entfernte Zuweisung als "0"; `EUKonten.LadeKonten` liest
  das für die beiden USt-Formulare als "keine Zuweisung" (sonst "Kz 0" in der
  Konten-Seite).

Geprüft: Engine-Tests; ECTViews baut; `ViewExports.cpp` und `easycashview.cpp`
kompilieren (Debug|Win32, nur USE_ECTENGINE); Dialog und Konten-Seite im
Offscreen-Test-Aufbau gerendert und durchgespielt (synthetische Konten, echte
.ecf 2026). NICHT geprüft: Linken, Programmstart, CaptionBar-Hinweis über der
WPF-Formularansicht, Rückfragen/Kontextmenü mit echter Maus, USE_ECTIFACE-Build.

Offen:
- Schritt 5: Abnahme durch Thomas (Abschnitt 8).
- `UpdateFormularMenu` verwirft in v3 den gemerkten Abgleich
  (`GetUStAbgleich().Verwerfen()`); das managed Pendant `UstAbgleich.Verwerfen()`
  hat keinen Export -- neue Formulare zur Laufzeit sieht v4 erst nach Neustart.
- v3 (`ECTIFace`-`HoleKontoMitFeldern`, master) führt die Erklärung bei der
  Konto-Anlage noch NICHT mit; v4 tut es (eindeutige Fälle). Format bleibt
  gleich, nur das Verhalten weicht ab.
- Nebenbefund: In `USt-d-2025.ecf` und `USt-d-2026.ecf` heißt Feld 744 "...neuer Fahrezeuge..."
  (Tippfehler; Quelle ist die Feldtabelle im Plugin-Repository, nicht die .ecf).

Stand: 2026-11-16. Geschrieben als Arbeitsanweisung für eine spätere Session
(auch für ein anderes Modell). Alles, was hier als "geprüft" steht, ist geprüft;
alles andere ist als Vorschlag oder als offene Frage gekennzeichnet.

## 1. Worum es geht

Die Umsatzsteuererklärung (Formulare 2025 und 2026) hat seit Commit 73c590f einen
eigenen Formularnamen `Umsatzsteuererklärung` und als Feld-IDs die Kennzahlen des
Vordrucks USt 2 A (741, 752, 2371 ...). Vorher hieß sie intern
`Umsatzsteuer-Voranmeldung` und benutzte deren Feld-IDs (41, 43 ...), damit die
Kontenzuweisungen der Voranmeldung auch für die Erklärung galten.

Folge: Jedes E/Ü-Konto braucht jetzt ZWEI Zuweisungen (eine je Formularname), und
die beiden können auseinanderlaufen. Ohne Zuweisung für die Erklärung füllen sich
dort nur die berechneten Felder.

Der "Abgleich" hält beide Zuweisungen zusammen. In v3 (Branch `master`, MFC) ist er
fertig und vom Autor (Thomas) durchgetestet. Aufgabe hier: dieselbe Funktion in v4
(C#/WPF), mit DENSELBEN Daten und DENSELBEN Regeln, damit v3 und v4 dieselbe
easyct.ini verstehen.

## 2. Was schon da ist

- Merge-Commit 4a2360b (lokal, nicht gepusht) hat master in diesen Branch gebracht:
  Formulare mit `elsterfeldname` und `uvafeld`, dazu der komplette v3-Code.
- Die v3-Umsetzung liegt damit als Referenz IM SELBEN Verzeichnis:
  - `UStAbgleich.h/.cpp` -- reine Logik (Klasse `CUStAbgleich`). Das ist die Vorlage
    für die C#-Klasse; die Kommentare im Header erklären das Datenmodell.
  - `UStAbgleichDlg.h/.cpp` -- Rückfragen, Feldauswahl-Menü, Dialog mit Vorschlagsliste.
  - `einstellungen4.cpp` -- dritte Spalte der MFC-Kontenliste, `UStAbgleichMitfuehren()`.
  - `easycashview.cpp` -- `UStAbgleichMeldung()`, `OnUStAbgleich()`, Einbau in
    `CaptionBoxCheckOnUpdate()`.
- Tests der v3-Logik (82 Prüfungen gegen die echten Formulare 2024-2026) liegen im
  Plugin-Repository: `C:\Users\mielk\Work\ECTElster\Testrahmen\ust-abgleich\test.cpp`.
  Sie sind die Vorlage für die xUnit-Tests.
- In v4 abweichende Ressourcennummern (weil belegt): `ID_USTABGLEICH` = 32927
  (master 32925), `IDD_USTABGLEICH` = 444 (master 443).
- Geprüft nach dem Merge: die betroffenen C++-Dateien und die Ressourcen kompilieren
  (Debug|Win32), `dotnet test ECTEngine.Tests\ECTEngine.Tests.csproj` 399/399.
  NICHT geprüft: Linken, Programmstart, Verhalten an der Oberfläche in v4.

## 3. Datenmodell (nicht ändern -- v3 liest und schreibt dasselbe)

- Feldzuweisung eines Kontos: ini-Sektionen `[EinnahmenFeldzuweisungen]` /
  `[AusgabenFeldzuweisungen]`, Key = Slot `NN`, Wert = Erweiterungs-Blob. Darin im
  Namensraum `ECT` je Formularname ein Key mit der Feld-ID als Wert, z. B.
  `Umsatzsteuer-Voranmeldung=43` und `Umsatzsteuererklärung=2371`.
  In v4: `ECTEngine/EUKonten.cs`, `EUKonto.Feldzuweisungen` (Formularname -> Feld-ID
  als string), geschrieben von `EUKonten.SpeichereFeldzuweisungen`.
- Korrespondenz: Attribut `uvafeld="<Feld-ID der Voranmeldung>"` an den Feldern der
  Erklärungsformulare (nur typ Einnahmen/Ausgaben). 2026: 47 Felder, 2025: 46.
  Mehrere Erklärungsfelder dürfen auf dasselbe Voranmeldungsfeld zeigen:
  - 43 -> 752, 2371, 2372, 2373, 2374
  - 48 -> 286, 287, 236, 240
  - 64 -> 3571, 3572, 3573
  In der Gegenrichtung (Erklärung -> Voranmeldung) ist alles eindeutig.
  Formulare ohne `uvafeld` (bis 2024, Österreich, eigene) nehmen nicht teil.
  Quelle der Attribute ist die Feldtabelle im Plugin-Repository
  (`ECTElster\Testrahmen\ust-formular\USt-d-<Jahr>.spez.txt`, Schlüssel `uva=NN`);
  die .ecf werden daraus erzeugt, also NICHT von Hand in den .ecf pflegen.
- Vermerk "so gewollt": Key `USt-Abgleich` im selben Namensraum `ECT`, Wert
  `<UVA-Feld>/<USt-Feld>` (z. B. `0/741`). Er gilt nur für genau dieses Paar und
  verfällt, sobald eine der beiden Zuweisungen sich ändert.
  ACHTUNG v4: `EUKonten.LadeKonten` übernimmt ALLE `ECT`-Keys in
  `Feldzuweisungen`. Der Key `USt-Abgleich` steht dort also wie ein Formularname.
  Das ist erwünscht (so wird er beim Speichern zurückgeschrieben), er darf nur
  nirgends als Formular behandelt oder beim Aufräumen entfernt werden. Heute ist
  das unkritisch, weil die Oberfläche über `EUKonten.FormularGruppen` iteriert.
- Formularnamen als Konstanten: `Umsatzsteuer-Voranmeldung`, `Umsatzsteuererklärung`.

## 4. Regeln (Port von `CUStAbgleich`, bitte 1:1)

Formulare finden (`Lade(jahr)`):
- Erklärung: Formulare mit `name="Umsatzsteuererklärung"`; die des verlangten Jahres,
  sonst die neueste, die mindestens ein `uvafeld` hat. Gibt es keine: kein Abgleich.
- Voranmeldung: `name="Umsatzsteuer-Voranmeldung"` UND Anzeigename beginnt mit
  `Umsatzsteuer-Voranmeldung ` (mit Leerzeichen). Die Erklärungen bis 2024 heißen
  intern ebenfalls so und scheiden über den Anzeigenamen aus. Genommen wird eine aus
  dem Jahr der gefundenen Erklärung, sonst die neueste (der Zeitraum ist egal, alle
  Monats-/Quartalsformulare haben dieselben Felder).
- Geladen werden nur Felder vom Typ Einnahmen/Ausgaben, MIT dem Merkmal `veraltet`.

Hilfsbegriffe:
- `UvaFeldZu(ust)` = `uvafeld` des Erklärungsfelds, 0 = keines.
- `UStKandidaten(uva, einnahme)` = alle nicht veralteten Erklärungsfelder des
  passenden Typs mit diesem `uvafeld`.
- `UvaFeldZuweisbar(uva, einnahme)` = Feld existiert, Typ passt, nicht veraltet.

`PruefePaar(uva, ust, einnahme)` -> Befund:
1. Zuweisungen auf Felder, die es im geladenen Formular nicht gibt, zählen als 0.
2. Beide 0 -> STIMMIG.
3. `uva` == 0: UVA_FEHLT, wenn `UvaFeldZu(ust)` != 0 und zuweisbar; sonst STIMMIG.
4. `ust` == 0: keine Kandidaten -> STIMMIG; genau einer -> UST_FEHLT; mehrere ->
   UST_MEHRDEUTIG.
5. Beide gesetzt: `UvaFeldZu(ust)` == `uva` -> STIMMIG. `UvaFeldZu(ust)` == 0 und
   keine Kandidaten -> STIMMIG (beide Felder gibt es nur im eigenen Formular).
   Sonst WIDERSPRUCH.

`Pruefe(konto)` = `PruefePaar`, aber STIMMIG, wenn der Vermerk `USt-Abgleich` genau
dieses Paar nennt.

`NachZuweisung(konto, ustGeaendert, alt)` -- nach einer geänderten Zuweisung für
eines der beiden Formulare (alt = Feld-ID davor, 0 = keine):
- Nichts tun, wenn sich nichts geändert hat. Sonst zuerst den Vermerk löschen.
- Ziel im anderen Formular bestimmen: wurde die Erklärung geändert, ist es
  `UvaFeldZu(neu)` (falls zuweisbar); wurde die Voranmeldung geändert, sind es die
  Kandidaten -- genau einer ist das Ziel, mehrere heißen "mehrdeutig".
- Gegenstück leer: Ziel still eintragen (PARTNER_GESETZT); mehrdeutig -> wählen
  lassen (PARTNER_WAEHLEN); wurde die Zuweisung entfernt, bleibt es dabei.
- Gegenstück belegt und das neue Paar ist STIMMIG: nichts tun.
- Sonst: Lief das Gegenstück bisher im Gleichschritt mit der ALTEN Zuweisung
  (Erklärung geändert: `UvaFeldZu(alt)` == Gegenstück; Voranmeldung geändert:
  `UvaFeldZu(Gegenstück)` == alt)? Dann folgt es: Ziel eintragen, oder wählen lassen,
  oder -- ohne Ziel -- Rückfrage "entfernen?" (PARTNER_ENTFERNEN).
  War es nicht im Gleichschritt und es gibt eine neue Zuweisung: Rückfrage
  WIDERSPRUCH (mit eindeutigem Vorschlag oder mit Kandidaten). Wurde nur entfernt,
  bleibt das eigenständige Gegenstück unberührt.
- Jede Rückfrage hat "Nein: so lassen" -> Vermerk `USt-Abgleich` setzen.

Zählung für den Hinweis am Formular (nur was DIESES Formular verfälscht):
- Erklärung angezeigt: UST_FEHLT, UST_MEHRDEUTIG, WIDERSPRUCH.
- Voranmeldung angezeigt: UVA_FEHLT, WIDERSPRUCH.
- Dazu die Zahl der betroffenen Konten mit Buchungen im Buchungsjahr.
Text in v3 (bitte gleich lassen): "Bei 3 E/Ü-Konten fehlt die Zuordnung zur
Umsatzsteuererklärung oder passt nicht zu der für die Umsatzsteuer-Voranmeldung
(2 davon mit Buchungen in 2025). Beträge dieser Konten können hier fehlen oder im
falschen Feld stehen." Varianten: "Bei einem E/Ü-Konto ... (es hat Buchungen in
2025 | ohne Buchungen in 2025)", "keines davon mit Buchungen in 2025".

Dialog "Zuordnungen für Voranmeldung und Umsatzsteuererklärung abgleichen":
- Zeilen = alle Konten mit Befund != STIMMIG, Konten mit Buchungen im Buchungsjahr
  zuerst. Spalten: E/Ü-Konto, Art, Voranmeldung, Umsatzsteuererklärung,
  Buchungen <Jahr>, Vorschlag. Feldtexte als "Feld 43: <Name>".
- Vorschlag: UVA_FEHLT -> Voranmeldung auf `UvaFeldZu(ust)` setzen; genau ein
  Kandidat -> Erklärung darauf setzen; sonst kein Vorschlag ("mehrere Felder möglich
  - bitte wählen" bzw. "bitte entscheiden").
- Vorab angehakt sind nur UST_FEHLT und UVA_FEHLT. Eine Zeile ohne Vorschlag lässt
  sich nicht anhaken, stattdessen öffnet sich die Auswahl.
- Auswahl je Zeile: die Kandidaten der Erklärung, bei WIDERSPRUCH zusätzlich
  "Erklärung: Zuordnung entfernen" / "Voranmeldung: Zuordnung entfernen", ggf.
  "Voranmeldung: <Feld>", "So lassen und nicht mehr melden", "Vorerst nichts ändern".
- "Übernehmen" trägt die angehakten Vorschläge ein.

## 5. Anknüpfungspunkte in v4 (gelesen am 2026-11-16)

Engine (`ECTEngine/`):
- `EUKonten.cs`: `Formularfeld` kennt nur Id (string!), Bezeichnung, IstEinnahme.
  `LadeFormularGruppen()` liest `*.ecf` aus `AppDomain.CurrentDomain.BaseDirectory`,
  überspringt veraltete Felder und nimmt je Formularname und Jahr nur EIN Formular
  (`jg.First()`). Für den Abgleich reicht das nicht: Die Gruppe
  `Umsatzsteuer-Voranmeldung` enthält auch die Erklärungen bis 2024, veraltete Felder
  fehlen, `uvafeld` wird nicht gelesen. Vorschlag: eigene Klasse
  `ECTEngine/UstAbgleich.cs`, die ihre zwei Formulare selbst lädt (Dateiliste bzw.
  Verzeichnis als Parameter, damit Tests ohne Programmverzeichnis laufen) und auf
  `IDictionary<string,string>` (= `EUKonto.Feldzuweisungen`) arbeitet.
- `FormularDefinition.cs`: Der Designer schreibt Feldänderungen in das vorhandene
  XElement zurück (`UebernehmeFeld`), unbekannte Attribute wie `uvafeld` bleiben
  dabei stehen. `FormularFeldDef` hat aber keine Eigenschaft dafür und
  `FormularFeldDialog` zeigt es nicht -- für den Abgleich nicht nötig.
- `KontoVorschlaege.cs` / `KontoFeldSelektor.cs`: Die Spezifikationen für automatisch
  angelegte Konten nennen nur `Umsatzsteuer-Voranmeldung=NN`. Ein so angelegtes Konto
  hat danach keine Zuweisung für die Erklärung. Offen, siehe Abschnitt 7.
- Buchungen je Konto im Buchungsjahr: über `BuchungsDocument`.

Konten-Seite (`ECTViews/Einstellungen/Pages/KontenPage*`):
- Anders als in v3 stehen ALLE Formulare nebeneinander (je Formular eine Spalte mit
  Jahres-Auswahl), Zuweisung per Drag&Drop. Eine "dritte Spalte" wie in v3 braucht es
  nicht.
- `KontenPageViewModel.WeiseFeldZu(konto, formularName, feld)` ist die EINE Stelle
  für Zuweisen, Umhängen und Lösen -> hier `NachZuweisung` einhängen (vorher die alte
  Feld-ID merken). Danach `EUKonten.SpeichereFeldzuweisungen`.
- `EUKontoVM.AktualisiereAnschluesse` vergibt je Formularspalte einen
  `AnschlussZustand` (Verknuepft / Offen / FehltInVariante) und baut den Tooltip
  `ZuordnungsText`. Vorschlag: neuer Zustand (rot) für die beiden USt-Spalten bei
  Befund != STIMMIG, Befundtext in den Tooltip.
- Über der Kontenliste steht schon `OhneZuordnungText`; dort passt eine zweite Zeile
  mit der Zahl der unstimmigen Konten und einem Knopf "Abgleichen...".
- Welches Jahr? v3 nimmt das Jahr des gewählten Formulars. In v4 bietet sich die in
  der Spalte `Umsatzsteuererklärung` gewählte Variante an.
- Dialoge/Rückfragen aus gehosteten Views NUR über `ECTViews/DialogBesitzer.cs`
  (siehe CLAUDE.md, "Win32-Hosting-Stolperfallen"); `MessageBox.Show(null, ...)`
  wirft dort.

Formularansicht:
- Der Hinweis läuft in v3 über die native CaptionBar (`theApp.CaptionBox(...)`).
  In v4 wird `CaptionBoxCheckOnUpdate()` am Ende von `CEasyCashView::OnUpdate`
  auch im WPF-Formularmodus erreicht, `m_GewaehltesFormular` bleibt gepflegt.
  Der gemergte MFC-Hinweis samt MFC-Dialog KÖNNTE in v4 also schon funktionieren.
  Ungeprüft. Zwei Risiken: `UStAbgleichMeldung()` liest die nativen Felder
  `einstellungen1->EinnahmenFeldzuweisungen[]` (sind sie nach Änderungen in der
  WPF-Konten-Seite aktuell? `LoadProfile()` bei easycashview.cpp ca. Zeile 9070
  ansehen), und `OnUStAbgleich()` schreibt über `SaveProfile()` (kommt das im
  managed Einstellungs-Cache an?).

## 6. Vorgeschlagene Reihenfolge

1. v4 bauen, Umsatzsteuererklärung 2025 anzeigen, nachsehen: Erscheint der Hinweis,
   öffnet "Abgleichen" den MFC-Dialog, kommen Änderungen in der Konten-Seite an?
   Ergebnis Thomas berichten, bevor Oberfläche gebaut wird.
2. `ECTEngine/UstAbgleich.cs` + `ECTEngine.Tests/UstAbgleichTests.cs`: Logik aus
   Abschnitt 4, Tests aus `ECTElster\Testrahmen\ust-abgleich\test.cpp` übernehmen
   (Formulare wie in `VorlagenBibliothekFelderTests` über die Repo-Wurzel finden:
   `UST\D-2025`, `UST\D-2026`, `UVA\D-2026`, dazu ein Formular bis 2024 als
   Gegenprobe "nimmt nicht teil"). Fertig, wenn alle 82 Fälle grün sind.
3. Konten-Seite: Zustand/Tooltip, Mitführen in `WeiseFeldZu`, Rückfragen, Auswahl
   bei Mehrdeutigkeit.
4. Abgleich-Dialog als WPF-View (Muster: die anderen modalen Views, z. B.
   `Dauerbuchungen/`), aufrufbar von der Konten-Seite und von der CaptionBar
   (ECTBridge-Export, `OnUStAbgleich` im `USE_ECTENGINE`-Zweig).
5. Abnahme durch Thomas mit denselben sechs Schritten wie in v3 (Abschnitt 8).

## 7. Offene Entscheidungen (vor dem Bauen mit Thomas klären)

- Hinweis am Formular: native CaptionBar behalten (mein Vorschlag, sie ist auch in
  v4 der Ort für solche Hinweise) oder etwas Eigenes in der WPF-Formularansicht?
- Darstellung der Unstimmigkeit in der Konten-Seite (Farbe/Symbol des Anschlusses).
- `KontoVorschlaege`: Sollen automatisch angelegte Konten die Erklärungs-Zuweisung
  gleich mitbekommen (eindeutige still; für 43 und 48 müsste die Spezifikation das
  Erklärungsfeld nennen, z. B. Ausfuhr -> 752)?
- Idee von Thomas, ausdrücklich NICHT Teil dieser Portierung: "Verprobung" (Summe der
  Voranmeldungen gegen die Erklärung je Kennzahl), evtl. als Hintergrund-Task.

## 8. Abnahme: so hat Thomas v3 getestet (alles wie erwartet)

1. Umsatzsteuererklärung 2025/2026 anzeigen -> Hinweis mit Kontenzahl und Knopf
   "Abgleichen"; verschwindet beim Wechsel auf ein anderes Formular.
2. "Abgleichen" -> Liste, Fehlendes angehakt, Konten mit Buchungen oben; Doppelklick
   auf eine Zeile mit Feld 43/48/64 zeigt die Auswahl.
3. "Übernehmen" -> Formular füllt sich, der Zähler im Hinweis sinkt bis zum
   Verschwinden.
4. Konten-Seite: Unstimmiges ist markiert.
5. Voranmeldung 41 zuweisen -> Erklärung folgt still auf 741. Voranmeldung 43
   zuweisen -> Auswahl 752, 2371-2374.
6. Zuweisung entfernen -> Rückfrage; "Ja" entfernt auch das Gegenstück, "Nein" lässt
   es stehen und meldet es nicht mehr.

Nebenbefund aus dem Test: Die Paragrafen-Texte zu 240, 287 und 2371 sind bei ELSTER
keine Pflicht (lokal mit ERiC 45 geprüft); die "..." in diesen Feldnamen stehen so im
Vordruck und bleiben.

## 9. Arbeitsweise in diesem Verzeichnis

- Quelltexte `*.cpp/*.h` sind hier UTF-8 mit BOM, in master Windows-1252; `.rc` und
  `.ecf` sind in beiden Windows-1252. Bei weiteren Merges aus master die betroffenen
  Quelltexte nicht blind übernehmen, sondern Basis und master-Fassung nach UTF-8
  wandeln und mit `git merge-file` neu zusammenführen. `resource.h` konfligiert an
  den beiden abweichenden Nummern.
- master holen (es gibt hier keinen lokalen Branch `master`):
  `git fetch C:\Users\mielk\Work\EasyCash master`, dann `FETCH_HEAD` mergen.
- Probe-Kompilierung einzelner C++-Dateien ohne die Ausgabeverzeichnisse anzufassen:
  `MSBuild EasyCash.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=Win32
  /p:IntDir=<temp>\ /p:OutDir=<temp>\ /p:BuildProjectReferences=false
  /p:SelectedFiles=<Datei>` (erst `StdAfx.cpp`, dann je Aufruf EINE Datei).
- Commits und Pushes nur auf ausdrücklichen Auftrag von Thomas; Trailer
  `Assisted-by: Claude <Modellname>`.
- Kein v4-Release, bevor der Abgleich in v4 bedienbar ist (sonst stehen Anwender mit
  leeren Erklärungsfeldern da).
