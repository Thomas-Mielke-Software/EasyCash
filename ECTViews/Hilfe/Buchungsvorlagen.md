# Buchungsvorlagen

Eine Buchungsvorlage nimmt Ihnen beim Buchen die Angaben ab, die sich bei gleichartigen Buchungen wiederholen. Im einfachsten Fall trägt sie Konto und Steuersatz ein. Sie kann aber auch aus einer Eingabe mehrere Buchungen machen, zusätzliche Angaben abfragen und den Buchen-Dialog auf das Nötige reduzieren.

Vorlagen verwalten Sie unter **Einstellungen > Buchungsvorlagen**. Diese Anleitung geht die Möglichkeiten der Reihe nach durch, von einfach nach umfangreich. Am Ende stehen sieben durchgerechnete Beispiele, die Sie auch fertig aus der Bibliothek übernehmen können.

> Alle Formeln und Platzhalter einer Vorlage sind reine Daten. Eine Vorlage kann nichts anderes tun, als Buchungen vorzubereiten. Vorlagen aus fremder Hand (Forum, Kollegen) lassen sich deshalb gefahrlos importieren.

## Die einfache Vorlage

Jede Vorlage hat eine zweistellige **Nummer** (00 bis 99), einen Namen im Feld **Beschreibung** und eine **Art** (Einnahme oder Ausgabe). Dazu kommen die Werte, die sie in den Buchen-Dialog einträgt: Konto, MwSt-Satz und bei Anlagegütern die Abschreibungsdauer.

Im Buchen-Dialog wählen Sie eine Vorlage über das Beschreibungsfeld:

- Die **Nummer** eintippen, z.B. `42`. Sobald die zweite Ziffer steht, wird die Vorlage geladen.
- Den **Namen** eintippen. Schon nach den ersten Buchstaben erscheint die Liste der passenden Vorlagen; mit den Pfeiltasten wählen, mit Enter übernehmen.
- Den **Pfeil-Knopf** rechts neben dem Feld anklicken (oder Pfeil ab bzw. F4 drücken). Dann erscheinen alle Vorlagen der Buchungsart.
- Über die Auswahlliste der Knöpfe **Einnahme** und **Ausgabe** im Menüband.

Die **Notiz** einer Vorlage wird beim Laden für einige Sekunden eingeblendet. Sie eignet sich für Erinnerungen wie "Beleg mit Teilnehmerliste aufbewahren".

Die Nummer lässt sich im Editor ändern. So können Sie Vorlagen thematisch ordnen, etwa Einnahmen auf 00 bis 29 und Ausgaben ab 30.

## Was eine Vorlage zusätzlich kann

| Baustein | Wozu |
|---|---|
| Zusatz-Zeilen | Aus einer Eingabe entstehen mehrere zusammengehörige Buchungen (eine Buchungsgruppe). |
| Zusatz-Felder | Der Buchen-Dialog fragt weitere Angaben ab, z.B. einen zweiten Betrieb oder einen Anteil in Prozent. |
| Felder des Buchen-Dialogs | Die üblichen Felder lassen sich sperren, ausblenden oder umbenennen. |
| Werte der Vorlage | Formeln und Textbausteine für Betrag, Beschreibung und Belegnummer der Buchung. |

Alle vier Bausteine arbeiten mit **Platzhaltern**. Darum zuerst dazu.

## Platzhalter und Formeln

Ein Platzhalter beginnt mit `$` und steht für einen Wert aus dem Buchen-Dialog. In einer **Formel** wird mit ihm gerechnet, in einem **Text** wird er durch seinen Wert ersetzt.

### Platzhalter für Formeln

| Platzhalter | Bedeutung |
|---|---|
| `$brutto` | Der Betrag aus dem Betragsfeld |
| `$netto` | Dieser Betrag ohne Mehrwertsteuer |
| `$mwst` | Der Mehrwertsteuer-Anteil dieses Betrags |
| `$mwstsatz` | Der Steuersatz aus dem MWSt-Feld (19 für 19 %) |
| `$rest` | Was vom Betrag übrig ist, nachdem die bisherigen Zusatz-Zeilen ihren Teil genommen haben |
| `$vat1` bis `$vat4` | Die Steuersätze aus Einstellungen > Unternehmer*in |
| `$vorlagenzeile` | Die Nummer der Zusatz-Zeile (1, 2, ...) |

Dazu kommen alle **Zusatz-Felder** der Vorlage, die eine Zahl liefern (siehe unten).

Eine Formel darf `+`, `-`, `*`, `/` und Klammern enthalten. Zahlen schreiben Sie mit Komma oder Punkt: `$brutto*0,3` und `$brutto*0.3` bedeuten dasselbe. Das Ergebnis wird kaufmännisch auf Cent gerundet.

```
$brutto * 0,3                30 % des eingegebenen Betrags
$netto * $vat1 / 100         die Steuer zum ersten Steuersatz auf den Nettobetrag
($brutto - $trinkgeld) * 0,3 erst abziehen, dann 30 % nehmen
$rest                        alles, was noch nicht verteilt ist
```

### Platzhalter für Texte

| Platzhalter | Bedeutung |
|---|---|
| `$beschreibung` | Der Inhalt des Beschreibungsfelds |
| `$beleg` | Die Belegnummer |
| `$konto` | Das Konto der Buchung |
| `$betrieb`, `$bestandskonto` | Der gewählte Betrieb, das gewählte Bestandskonto |
| `$datum` | Das Buchungsdatum (tt.mm.jjjj) |
| `$brutto`, `$netto`, `$mwst`, `$rest` | Die Beträge als Text |
| `$J`, `$j` | Jahr vierstellig, zweistellig |
| `$M`, `$m` | Monat zweistellig, ohne führende Null |
| `$q`, `$h` | Quartal (1 bis 4), Halbjahr (1 oder 2) |

Auch hier stehen alle Zusatz-Felder der Vorlage zur Verfügung. Ein Text wie `Miete $M/$J` ergibt bei einer Buchung im November 2026 "Miete 11/2026".

## Zusatz-Zeilen: mehrere Buchungen aus einer Eingabe

Mit Zusatz-Zeilen wird aus der Vorlage eine **Buchungsgruppe**. Die Buchung aus den Feldern des Dialogs heißt dann **Basis-Buchung**; jede Zusatz-Zeile erzeugt eine weitere Buchung mit demselben Datum.

Eine Zusatz-Zeile hat diese Angaben:

| Angabe | Bedeutung | Wenn leer |
|---|---|---|
| Buchungsart | Einnahme oder Ausgabe | wie die Basis |
| Konto | Kontoname, `$konto` oder ein Zusatz-Feld | Pflicht: ohne Konto wird die Zeile nicht gespeichert |
| MwSt | Fester Satz (`7`), `$vat1` bis `$vat4`, `$mwstsatz` oder ein Zusatz-Feld | Satz der Basis |
| Betrag (Formel) | Was die Zeile bucht | Pflicht: ohne Formel wird die Zeile übersprungen |
| Beschreibung | Text mit Platzhaltern | Beschreibung der Basis |
| Beleg | Text mit Platzhaltern, z.B. `$beleg-$vorlagenzeile` | Beleg der Basis |
| Betrieb, Bestandskonto | Name oder ein Zusatz-Feld | wie die Basis |

Drei Regeln bestimmen, was gebucht wird:

1. **Ergibt die Formel einer Zeile 0, wird die Zeile nicht gebucht.** Ein leer gelassenes Feld "Skonto" erzeugt also keine Buchung über 0,00.
2. **"Basis bucht"** legt fest, welchen Betrag die Basis-Buchung selbst bekommt. Leer heißt: den eingegebenen Betrag unverändert; die Zusatz-Zeilen kommen dann obendrauf. Mit `$rest` nimmt die Basis, was die Zeilen übrig lassen; der eingegebene Betrag wird also aufgeteilt.
3. **Die Summe muss aufgehen.** Wird der Betrag aufgeteilt und die Zusatz-Zeilen brauchen ihn ganz auf oder übersteigen ihn, lässt sich nicht buchen.

> Verwenden Sie beim Aufteilen `$rest` für die Basis und nicht eine zweite Prozent-Formel. So landet ein Rundungs-Cent in der Basis, und die Summe aller Buchungen entspricht exakt dem eingegebenen Betrag.

Im Buchen-Dialog zeigt der Bereich **Buchungsgruppe** unten, welche Buchungen entstehen. Zeilen, die nicht gebucht werden, erscheinen blass. Im Journal tragen die Mitglieder einer Gruppe ein `[G]`; ein Klick markiert die ganze Gruppe, und beim Bearbeiten öffnet sich immer die Basis mit der kompletten Gruppe.

## Zusatz-Felder: eigene Angaben abfragen

Ein Zusatz-Feld ist ein Eingabefeld, das nur bei dieser Vorlage im Buchen-Dialog erscheint. Sein Wert steht unter seinem Platzhalter in allen Formeln und Texten der Vorlage zur Verfügung.

| Angabe | Bedeutung |
|---|---|
| Platzhalter | Name des Felds, z.B. `anteil2`. Verwendet wird er als `$anteil2`. |
| Bezeichnung | Steht vor dem Eingabefeld, z.B. "Anteil Betrieb 2" |
| Typ | Welche Art von Eingabe (siehe Tabelle unten) |
| angefügt in Box | Die Box, in der das Feld unter den vorhandenen Feldern angefügt wird, z.B. "Betrag" |
| Vorbelegung | Womit das Feld startet |
| Pflichtfeld | Ohne Eingabe lässt sich nicht buchen. |
| letzten Wert merken | Der zuletzt gebuchte Wert wird beim nächsten Mal vorgeschlagen. |

Der Platzhalter besteht aus Buchstaben (a bis z), Ziffern und dem Unterstrich, beginnt mit einem Buchstaben und hat mindestens zwei Zeichen. Die eingebauten Namen wie `brutto` oder `rest` sind vergeben.

### Die Feld-Typen

| Typ | Eingabe im Dialog | Wert in Formeln | Wert in Texten |
|---|---|---|---|
| Betrag | Betragsfeld | Zahl | "12,50" |
| Zahl | Zahlenfeld (km, Tage, Stück) | Zahl | "150" |
| Prozent | Zahlenfeld mit %, 0 bis 100 | Zahl (30 für 30 %) | "30" |
| MWSt-Satz | Auswahl der Steuersätze, auch frei eintippbar | Zahl (7 für 7 %) | "7" |
| Text | Textfeld | nicht verwendbar | der Text |
| Auswahl | Auswahlliste mit festen Möglichkeiten | Zahl, wenn die Werte Zahlen sind | der Wert |
| Häkchen (ja/nein) | Kontrollkästchen | 1 oder 0 | "ja" oder "nein" |
| Konto | Auswahlliste der Konten | nicht verwendbar | der Kontoname |
| Betrieb | Liste der Betriebe mit Symbolen | nicht verwendbar | der Name |
| Bestandskonto | Liste der Bestandskonten mit Symbolen | nicht verwendbar | der Name |
| Datum | Datumsfeld (tt.mm.jjjj) | nicht verwendbar | "03.07.2026" |

Einige Hinweise zu einzelnen Typen:

- **Auswahl**: Die Möglichkeiten tragen Sie durch Semikolon getrennt ein, jeweils als `Anzeige=Wert`, zum Beispiel `1. Quartal=1Q;2. Quartal=2Q`. Ohne Gleichheitszeichen ist die Anzeige zugleich der Wert.
- **Häkchen**: Weil es 1 oder 0 liefert, eignet es sich als Schalter in einer Formel. `$brutto * 0,02 * $skonto` ergibt 0, solange das Häkchen nicht gesetzt ist; die Zeile entfällt dann.
- **Konto**: Bei den Auswahlmöglichkeiten legt `E` oder `A` fest, ob Einnahmen- oder Ausgaben-Konten angeboten werden. Leer heißt: die Konten der Buchungsart der Vorlage.
- **Betrieb und Bestandskonto**: Das Feld erscheint nur, wenn es überhaupt Betriebe bzw. Bestandskonten gibt.
- **Vorbelegung bei Zahlen**: Sie darf einen Steuersatz aus den Einstellungen nennen (`$vat2`) oder eine kleine Rechnung sein (`100/3`).

Ein Zusatz-Feld allein bewirkt noch nichts. Es wirkt erst dadurch, dass eine Formel oder ein Text der Vorlage seinen Platzhalter verwendet.

## Felder des Buchen-Dialogs: sperren, ausblenden, umbenennen

Im Abschnitt **Felder des Buchen-Dialogs** legen Sie fest, wie die üblichen Felder bei dieser Vorlage erscheinen:

- **normal**: sichtbar und änderbar
- **gesperrt**: sichtbar, aber nicht änderbar
- **ausgeblendet**: gar nicht zu sehen

Daneben lässt sich jedes Feld **umbenennen**, etwa "Brutto" in "Rechnungsbetrag" oder "Bestandskonto" in "Abbuchung von".

Ein gesperrtes oder ausgeblendetes Feld bekommt seinen Wert aus der Vorlage. Woher genau, zeigt die Tabelle:

| Feld | Wert bei "gesperrt" und "ausgeblendet" |
|---|---|
| Brutto | "Betrag (Formel)" im Abschnitt Werte der Vorlage |
| MWSt % | "MwSt %" der Vorlage |
| Konto | "Konto" der Vorlage |
| Beleg-Nr. | Die fortlaufende Belegnummer. Bei "gesperrt" formt "Beleg-Nr." der Vorlage sie auf Wunsch um; bei "ausgeblendet" bleibt sie, wie sie ist. |
| Bestandskonto | "Bestandskonto" der Vorlage |
| Betrieb | "Betrieb" der Vorlage |
| Abschreibung (AfA) | "AfA-Jahre" der Vorlage; bei "ausgeblendet" wird nie abgeschrieben |

Zwei Felder sind ausgenommen: Das **Datum** bleibt immer bedienbar. Die **Beschreibung** lässt sich nur umbenennen, weil über sie die Vorlage gewählt wird.

> Blenden Sie ein Feld nur aus, wenn die Vorlage seinen Wert liefert. Fehlt er, zeigt der Editor einen gelben Hinweis, und der Buchen-Dialog nennt beim Buchen das betroffene Feld.

### Betrag (Formel)

Ist das Betragsfeld gesperrt oder ausgeblendet, rechnet die Vorlage den Betrag selbst aus, und zwar aus den Zusatz-Feldern. Die Formel steht im Abschnitt **Werte der Vorlage** unter "Betrag (Formel)". Sie erscheint dort nur, solange das Betragsfeld nicht "normal" ist.

```
$tage8*14 + $tage24*28       Verpflegungspauschalen aus der Zahl der Tage
$km * 0,30                   Kilometerpauschale
```

Das Ergebnis ist das `$brutto` der Vorlage. Bei "gesperrt" sehen Sie es im Betragsfeld mitlaufen, während Sie die Zusatz-Felder ausfüllen.

## Werte der Vorlage

Dieser Abschnitt enthält, was die Vorlage in die Felder des Buchen-Dialogs einträgt. Bei normalen Feldern ist das eine Vorbelegung, die Sie beim Buchen ändern können.

**Beschreibung**: Bleibt das Feld leer, steht im Beschreibungsfeld des Dialogs der Name der Vorlage und wird so gebucht, sofern Sie ihn nicht überschreiben. Tragen Sie hier einen Text ein, setzt die Vorlage die gebuchte Beschreibung selbst zusammen:

```
Bewirtung $anlass ($personen)    nur aus Zusatz-Feldern
$beschreibung (Zinsen)           Inhalt des Beschreibungsfelds plus Zusatz
Fahrtkosten $beschreibung        fester Anfang plus Inhalt des Felds
```

Unter dem Beschreibungsfeld des Dialogs steht dann in grauer Schrift "Wird gebucht als: ...". Nennt der Text den Namen der Vorlage schon selbst, wie im dritten Beispiel bei einer Vorlage namens "Fahrtkosten", startet das Beschreibungsfeld leer und nimmt nur noch den eigentlichen Inhalt auf.

**Beleg-Nr.**: Leer heißt, die Belegnummer aus dem Dialog wird unverändert gebucht. Mit einem Text wie `$beleg-R` wird sie umgeformt.

**Bestandskonto** und **Betrieb**: Die Vorlage wählt beim Laden den genannten Eintrag aus.

### Konten in Vorlagen

Im Konto-Feld der Vorlage und ihrer Zusatz-Zeilen steht normalerweise ein Kontoname. Zwei Besonderheiten sind nützlich:

**Neutrale Konten.** Ein Konto, dessen Name mit einem Schrägstrich beginnt (z.B. `/Darlehenstilgung`), bleibt aus Gewinn und Einnahmenüberschussrechnung heraus. Es eignet sich für Geldbewegungen, die keine Betriebseinnahme oder -ausgabe sind: Umbuchungen, Tilgungen, durchlaufende Posten.

**Konto über ein Formularfeld finden.** Statt eines Namens kann im Konto-Feld eine Angabe dieser Art stehen:

```
$de:E/Ü-Rechnung=1175|$name=Bewirtungskosten (abziehbar)||
```

Das heißt: Nimm das erste Konto, das mit dem Feld 1175 des Formulars "E/Ü-Rechnung" verknüpft ist. Mehrere Felder werden mit `|` aneinandergereiht; das Konto muss dann mit allen verknüpft sein. Gibt es noch keines, bietet der Buchen-Dialog beim Laden der Vorlage an, es anzulegen, und schlägt den Namen hinter `$name=` vor. `de` steht für Deutschland; für Österreich und die Schweiz gibt es eigene Blöcke (`at`, `ch`), jeweils mit `||` abgeschlossen. So funktioniert eine weitergegebene Vorlage unabhängig davon, wie die Konten beim Empfänger heißen. Die mitgelieferten Vorlagen der Bibliothek arbeiten auf diese Weise.

**Die Auswahlliste am Konto-Feld** bietet neben den vorhandenen Konten solche Angaben für häufige Fälle fertig an. Sie stehen unter "Über Formularfelder" und betreffen Geschäfte, die außer einem Feld der Einnahmenüberschussrechnung auch eine bestimmte Kennzahl der Umsatzsteuer-Voranmeldung (Österreich: U30) brauchen. Für diese Kombinationen enthält der Standard-Kontenrahmen kein Konto:

| Eintrag | Deutschland | Österreich |
|---|---|---|
| Ausfuhrlieferungen (Drittland) | EÜR 1103, UVA 43 | E1a 9040, U30 011 |
| Innergemeinschaftliche Lieferungen | EÜR 1103, UVA 41 | E1a 9040, U30 017 |
| Sonstige Leistungen an EU-Unternehmer | EÜR 1103, UVA 21 | (keine Kennzahl in der U30) |
| Nicht steuerbare Umsätze (Leistungsort im Ausland) | EÜR 1103, UVA 45 | - |
| Steuerfreie Umsätze ohne Vorsteuerabzug | EÜR 1103, UVA 48 | - |
| Steuerfreie Grundstücksumsätze | - | E1a 9040, U30 019 |
| Umsätze zum Steuersatz 0 % (Photovoltaik) | EÜR 1112, UVA 87 | - |
| Entrichtete Einfuhrumsatzsteuer (Ausgabe) | EÜR 1185, UVA 62 | U30 061 |
| Steuerfreie innergemeinschaftliche Erwerbe (Ausgabe) | EÜR 1100, UVA 91 | E1a 9100, U30 071 |

Angeboten werden nur die Einträge für das in den Einstellungen gewählte Land und die Buchungsart. Fälle, die mehrere Buchungen brauchen, etwa der steuerpflichtige innergemeinschaftliche Erwerb oder Reverse Charge, stehen als fertige Vorlagen in der Bibliothek.

In den Zusatz-Zeilen enthält die Liste außerdem `$konto` (das Konto der Basis-Buchung) und die Zusatz-Felder vom Typ Konto.

## Beispiele

Alle sieben Beispiele finden Sie fertig in der Bibliothek (Knopf **Bibliothek...** unter der Vorlagen-Liste). Übernehmen Sie eines und sehen Sie es sich im Editor an; das ist der schnellste Weg, die Bausteine zu verstehen.

### 1. Umbuchung zwischen zwei Bestandskonten

*Aufgabe:* Das PayPal-Guthaben wird aufs Bankkonto ausgezahlt. Es soll auswählbar sein, wo das Geld ankommt.

- Art Ausgabe, Konto `/Umbuchung zwischen Bestandskonten`. Der Schrägstrich macht das Konto neutral: es berührt weder Gewinn noch Umsatzsteuer.
- Zusatz-Feld `zielkonto`, Typ Bestandskonto, Bezeichnung "Gutschrift auf", Pflichtfeld.
- Eine Zusatz-Zeile: Buchungsart Einnahme, Konto `$konto`, MwSt `0`, Betrag `$brutto`, Bestandskonto `$zielkonto`.
- Felder des Dialogs: Konto, MWSt und Abschreibung ausgeblendet; Bestandskonto umbenannt in "Abbuchung von".

Beim Buchen wählen Sie oben das Konto, von dem abgebucht wird, darunter das Zielkonto und geben den Betrag ein. Es entstehen zwei Buchungen über denselben Betrag.

### 2. Ausgabe auf zwei Betriebe aufteilen

*Aufgabe:* Eine Rechnung betrifft zwei Betriebe, meist im selben Verhältnis.

- Zusatz-Feld `betrieb2`, Typ Betrieb, "Zweiter Betrieb", Pflichtfeld, letzten Wert merken.
- Zusatz-Feld `anteil2`, Typ Prozent, "Anteil des zweiten Betriebs", Box Betrag, Vorbelegung `50`, letzten Wert merken.
- Eine Zusatz-Zeile: Konto `$konto`, Betrag `$brutto*$anteil2/100`, Betrieb `$betrieb2`.
- Basis bucht `$rest`.

Bei 200,00 Euro und 30 % bekommt der zweite Betrieb 60,00 Euro, der erste 140,00 Euro.

### 3. Rechnung mit zwei Steuersätzen

*Aufgabe:* Auf einer Rechnung stehen Positionen zu 19 % und zu 7 %.

- Zusatz-Feld `betrag2`, Typ Betrag, "davon zum zweiten Steuersatz".
- Zusatz-Feld `satz2`, Typ MWSt-Satz, "Zweiter Steuersatz", Vorbelegung `$vat2`.
- Eine Zusatz-Zeile: Konto `$konto`, MwSt `$satz2`, Betrag `$betrag2`, Beschreibung `$beschreibung ($satz2 %)`.
- Basis bucht `$rest`; das Betragsfeld heißt "Rechnungsbetrag".

Sie geben den Gesamtbetrag ein und darunter den Teil zum zweiten Satz. Bleibt dieser leer, entsteht eine einzige Buchung.

### 4. Wareneinkauf: Warenwert, Versand und Skonto getrennt

*Aufgabe:* Drei Buchungen mit je eigenem Text für Warenwert, Versandkosten und Skonto.

- Zusatz-Felder `versand` und `skonto`, beide Typ Betrag, Box Betrag.
- Zeile 1: Konto `$konto`, Betrag `$versand`, Beschreibung `$beschreibung (Versandkosten)`.
- Zeile 2: Konto `$konto`, Betrag `-$skonto`, Beschreibung `$beschreibung (Skonto)`. Das Minus bucht das Skonto als Minderung.
- Basis bucht `$brutto-$versand`; Beschreibung der Vorlage: `$beschreibung (Warenwert)`.

Sollen Versand oder Skonto auf eigene Konten, tragen Sie diese statt `$konto` in die Zeile ein.

### 5. Bewirtungskosten

*Aufgabe:* 70 % abziehbar, 30 % nicht; Anlass und Teilnehmer sollen in der Beschreibung stehen; das Trinkgeld hat keine Vorsteuer.

- Zusatz-Felder `anlass` und `personen` (Text, Pflichtfelder, Box Beschreibung) sowie `trinkgeld` (Betrag, Box Betrag).
- Beschreibung der Vorlage: `Bewirtung $anlass ($personen)`.
- Zeile 1 (nicht abziehbarer Teil der Rechnung): Betrag `($brutto-$trinkgeld)*0,3`, MwSt leer.
- Zeile 2 und 3 (Trinkgeld, abziehbar und nicht abziehbar): Betrag `$trinkgeld*0,7` und `$trinkgeld*0,3`, MwSt `0`.
- Basis bucht `$rest`; das Betragsfeld heißt "Gezahlt (inkl. Trinkgeld)".

### 6. Reisekosten: Betrag aus Mengen berechnen

*Aufgabe:* Verpflegungspauschale und Kilometergeld. Eingegeben werden Tage bzw. Kilometer, kein Betrag.

- Betragsfeld **gesperrt** und umbenannt in "Pauschale"; MWSt und Abschreibung ausgeblendet.
- Zusatz-Felder `tage8` und `tage24` (Typ Zahl) bzw. `km`.
- Betrag (Formel): `$tage8*14 + $tage24*28` bzw. `$km*0,30`.
- Beschreibung der Vorlage: `Fahrtkosten $beschreibung ($km km)`; das Beschreibungsfeld heißt "Fahrt (Ziel, Anlass)".

Die Sätze stehen sichtbar in der Formel. Ändern sie sich, passen Sie die Formel an.

### 7. Darlehensrate: Zinsen und Tilgung

*Aufgabe:* Von der abgebuchten Rate sind nur die Zinsen Betriebsausgabe.

- Zusatz-Feld `zinsen`, Typ Betrag, "davon Zinsen", Pflichtfeld; das Betragsfeld heißt "Rate".
- Basis bucht `$zinsen` auf das Zins-Konto.
- Eine Zusatz-Zeile: Konto `/Darlehenstilgung`, MwSt `0`, Betrag `$brutto-$zinsen`, Beschreibung `$beschreibung (Tilgung)`.
- Beschreibung der Vorlage: `$beschreibung (Zinsen)`.

## Gebuchte Buchungen später bearbeiten

Eine Buchung merkt sich, mit welcher Vorlage sie entstanden ist und was Sie in die Zusatz-Felder eingegeben haben. Öffnen Sie sie später zum Bearbeiten, erscheint der Dialog wieder mit der Vorlage und Ihren damaligen Eingaben. Ändern Sie zum Beispiel den Anteil von 30 auf 40 %, werden die zugehörigen Buchungen beim Speichern neu berechnet.

Das setzt voraus, dass die Vorlage noch unter derselben Nummer und demselben Namen existiert. Wurde sie gelöscht, umbenannt oder steht auf dem Platz inzwischen eine andere, öffnet sich die Buchung als gewöhnliche Einzelbuchung; ein Hinweis im Dialog sagt das.

Mit **Vorlage entfernen** unten im Dialog lösen Sie eine Buchung von ihrer Vorlage. Zusatz-Felder und Zusatz-Buchungen entfallen, alle Felder sind wieder frei bedienbar, und die schon eingetragenen Werte bleiben stehen.

## Prüfhinweise im Editor

Der Editor speichert jede Änderung sofort. Passt etwas nicht zusammen, erscheint oben ein gelber Kasten "Diese Vorlage ist noch nicht stimmig". Die häufigsten Meldungen:

| Meldung | Abhilfe |
|---|---|
| Unbekannte Variable `$xyz` | Der Platzhalter ist verschrieben, oder das Zusatz-Feld dazu fehlt noch. |
| Der Betrag ist im Buchen-Dialog nicht eingebbar, aber "Betrag (Formel)" ist leer | Formel eintragen oder das Betragsfeld wieder auf "normal" stellen. |
| Das Konto ist im Buchen-Dialog nicht wählbar, aber die Vorlage nennt kein Konto | Konto in der Vorlage eintragen. |
| Zusatz-Zeile hat keine Betrags-Formel und wird übersprungen | Formel eintragen oder die Zeile entfernen. |
| Der Platzhalter ist mehrfach vergeben | Jedes Zusatz-Feld braucht einen eigenen Namen. |
| Text-Feld in einer Formel | Nur Felder der Typen Betrag, Zahl, Prozent, MWSt-Satz und Häkchen liefern eine Zahl. |

Im Buchen-Dialog stehen Fehler direkt am betroffenen Feld. Fehler der Berechnung erscheinen unten im Bereich der Vorlage.

## Vorlagen weitergeben

**Exportieren...** speichert die gewählte Vorlage als Datei (`.ectvorlage.xml`), **Importieren...** liest eine solche Datei in einen freien Platz. Zusatz-Felder und Feld-Regeln sind enthalten.

Die **Bibliothek** enthält mitgelieferte Vorlagen, nach Themen gegliedert und durchsuchbar.

> Die mitgelieferten Vorlagen sind Vorschläge für typische Fälle und ersetzen keine steuerliche Beratung. Feste Sätze wie 70/30, 14 Euro oder 0,30 Euro stehen sichtbar in den Formeln; prüfen Sie, ob sie für Ihren Fall und das Buchungsjahr gelten.

## Änderungen gegenüber früheren Beta-Versionen

In früheren Beta-Versionen der Version 4 konnte eine Zusatz-Zeile ohne Betrags-Formel ein eigenes Eingabefeld für Betrag und MWSt im unteren Bereich des Dialogs öffnen. Das gibt es nicht mehr; an seine Stelle treten die Zusatz-Felder.

- Eine Zeile **ohne Betrags-Formel** wird beim Buchen übersprungen. Der Editor weist darauf hin.
- Ein **leeres MwSt-Feld** einer Zeile bedeutet jetzt immer "Satz der Basis".
- Die Auswahl **Darstellung** (automatisch, kompakt, Maske) ist entfallen.

So stellen Sie eine solche Vorlage um: Legen Sie ein Zusatz-Feld vom Typ Betrag an, zum Beispiel `betrag2`, und tragen Sie `$betrag2` als Betrags-Formel der Zeile ein. Für einen wählbaren Steuersatz nehmen Sie ein Zusatz-Feld vom Typ MWSt-Satz und tragen dessen Platzhalter im MwSt-Feld der Zeile ein. Beispiel 3 zeigt genau diesen Fall.
