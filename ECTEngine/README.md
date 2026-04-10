# ECTEngine - .NET Implementation von EasyCash&Tax

ECTEngine ist eine .NET-basierte Implementierung des EasyCash&Tax Kernmoduls. Es bietet alle erforderlichen Funktionen für die Verwaltung von Buchungen, Abschreibungen und Währungskonvertierungen.

## Funktionen

- **Buchungsverwaltung**: Verwaltung von Einnahmen und Ausgaben
- **Abschreibungen**: Lineare und degressive Abschreibungsberechnungen mit verschiedenen Genauigkeitsstufen
- **Währungskonvertierung**: Unterstützung für Euro-Konvertierung historischer Währungen
- **Daueraufträge**: Verwaltung wiederkehrender Buchungen
- **Serialisierung**: Kompatibilität mit dem bestehenden EasyCash-Dateiformat

## Architektur

### Namespace-Struktur
- `ECTEngine.Models` - Datenmodelle (Buchung, Dauerbuchung, Dokument)
- `ECTEngine.Helpers` - Hilfsfunktionen (Währung, Konfiguration)
- `ECTEngine.Serialization` - Datei-Ein/Ausgabe
- `ECTEngine.Calculations` - Geschäftslogik und Berechnungen

## Verwendungsbeispiel

```csharp
using ECTEngine.Models;
using ECTEngine.Helpers;

// Neues Dokument erstellen 
var doc = new EasyCashDocument { Jahr = 2024 };

// Buchung hinzufügen 
var buchung = new Buchung { Datum = DateTime.Now, Beschreibung = "Büroausstattung", Wert = 100000,  // 1000,00 EUR in Cents MWSt = 19000,    // 19% Konto = "4730" }; doc.Ausgaben.AddLast(buchung);

// Summen berechnen 
long einnahmenSumme = doc.BerechneEinnahmenSumme(); long ausgabenSumme = doc.BerechneAusgabenSumme();

// Zu Euro konvertieren 
if (doc.Waehrung != "EUR") { doc.ConvertToEuro(); }
```	

## Versionskompatibi lität

- **Aktuelle Version**: 13
- **Minimale unterstützte Version**: 1
- **Dateiformat**: Binär mit Magic-Key "ECDo"

## Lizenz

GNU General Public License v3.0 or later