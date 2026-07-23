// BuchungViewModel.Waehrung.cs -- Waehrungsrechner im Buchen-Dialog.
//
// Rechnet den im Betragsfeld stehenden FREMDwaehrungsbetrag ueber die
// ect-forex-API (Tageskurs zum Buchungsdatum) in die BUCHHALTUNGSwaehrung
// (doc.Waehrung, meist EUR) um und dokumentiert die Umrechnung im
// Beschreibungstext. Welche Waehrungen angeboten werden, bestimmt die
// Whitelist der Waehrungen-Einstellungsseite.
//
// Richtung der Umrechnung (empirisch verifiziert, siehe KursInfo): Fiat-Kurse
// der EZB sind "Fremdwaehrung pro EUR" (-> dividieren), Krypto-Kurse sind
// "EUR pro Einheit" (-> multiplizieren). KursInfo.EuroProEinheit kapselt das.
// Ist die Buchhaltungswaehrung NICHT EUR (z.B. Schweizer Franken), wird der
// Kurs der Zielwaehrung in einer zweiten, PARALLELEN Abfrage geholt.
//
// WICHTIG -- Issue #21, Befund Darkwing371 (12.02.2026) zur NATIVEN Umsetzung:
// die native Variante ruft CInternetSession SYNCHRON im UI-Thread -> die
// Oberflaeche fror ein ("Keine Rueckmeldung") und stuerzte gelegentlich ab.
// Hier: Abruf strikt ASYNCHRON + ABBRECHBAR, mit Fortschritts-Overlay ueber
// Betrags- und Beschreibungsfeld, alle Fehler abgefangen.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;
using ECTEngine;

namespace ECTViews.ViewModels
{
    public partial class BuchungViewModel
    {
        private CancellationTokenSource _umrechnungAbbruch;

        private bool _umrechnungLaeuft;
        /// <summary>True waehrend einer laufenden Kursabfrage. Steuert das
        /// Fortschritts-Overlay und sperrt den Umrechnen-Knopf.</summary>
        public bool UmrechnungLaeuft
        {
            get => _umrechnungLaeuft;
            private set
            {
                if (SetProperty(ref _umrechnungLaeuft, value))
                {
                    OnPropertyChanged(nameof(NichtAmUmrechnen));
                    CommandManager.InvalidateRequerySuggested();   // Abbrechen-Knopf
                }
            }
        }

        /// <summary>Fuer die IsEnabled-Bindung des Umrechnen-Knopfs.</summary>
        public bool NichtAmUmrechnen => !_umrechnungLaeuft;

        private string _umrechnungStatus = "";
        /// <summary>Fortschrittstext im Overlay ("... wird umgerechnet ...").</summary>
        public string UmrechnungStatus
        {
            get => _umrechnungStatus;
            private set => SetProperty(ref _umrechnungStatus, value);
        }

        /// <summary>Buchhaltungswaehrung (Ziel der Umrechnung), z.B. "EUR" oder
        /// "CHF". Kommt aus dem Dokument.</summary>
        public string Zielwaehrung
        {
            get
            {
                var w = _doc?.Waehrung;
                return string.IsNullOrWhiteSpace(w) ? "EUR" : w.Trim();
            }
        }

        /// <summary>Aufschrift des Umrechnen-Knopfs: Zielwaehrung + Dropdown-Pfeil
        /// (EUR -> "€").</summary>
        public string UmrechnenKnopfText
            => (Zielwaehrung.Equals("EUR", StringComparison.OrdinalIgnoreCase) ? "€" : Zielwaehrung)
               + " ▾";

        /// <summary>Tooltip des Umrechnen-Knopfs.</summary>
        public string UmrechnenKnopfTooltip
            => "Fremdwährungsbetrag über den Tageskurs in die Buchhaltungswährung ("
               + Zielwaehrung + ") umrechnen (Auswahl aus Einstellungen > Währungen)";

        /// <summary>Bricht eine laufende Kursabfrage ab.</summary>
        public ICommand AbbrechenUmrechnungCommand
            => _abbrechenUmrechnung ?? (_abbrechenUmrechnung = new RelayCommand(
                () => _umrechnungAbbruch?.Cancel(), () => _umrechnungLaeuft));
        private RelayCommand _abbrechenUmrechnung;

        /// <summary>Die in den Einstellungen ("Währungsumrechnung") angehakten
        /// Währungen -- Rohbestand für das Umrechnen-Menü.</summary>
        public IReadOnlyList<WaehrungsEintrag> AktiveWaehrungen()
            => Waehrungsliste.AktiveWaehrungen();

        /// <summary>Die tatsächlich im Umrechnen-Menü anzuzeigenden Währungen:
        /// die angehakten OHNE die Buchhaltungswährung selbst (X-&gt;X wäre
        /// sinnlos) und mit EUR als festem Zusatzeintrag, falls die
        /// Buchhaltungswährung nicht EUR ist.</summary>
        public IReadOnlyList<WaehrungsEintrag> WaehrungenFuerMenue()
            => Waehrungsliste.MenueWaehrungen(Waehrungsliste.AktiveWaehrungen(), Zielwaehrung);

        /// <summary>Callback für die DSGVO-Einwilligung vor dem ersten
        /// API-Zugriff (von der View gesetzt). Liefert true = einverstanden.</summary>
        public Func<bool> ApiEinwilligungAbfrage { get; set; }

        /// <summary>
        /// Rechnet den Betrag im Betragsfeld (in Währung <paramref name="code"/>)
        /// per Tageskurs (Buchungsdatum) in die Buchhaltungswährung um, schreibt
        /// das Ergebnis ins Betragsfeld und hängt die Umrechnung an den
        /// Beschreibungstext an. Blockiert die UI nicht, ist abbrechbar. Wirft
        /// bei ungültiger Eingabe / Abruf-Fehler (der Aufrufer zeigt die Meldung).
        /// </summary>
        public async Task WaehrungUmrechnenAsync(string code)
        {
            if (_umrechnungLaeuft) return;

            // 1) Betrag prüfen (locale-toleranter Parser wie das Betragsfeld).
            if (!Waehrungsformat.TryParse(BetragText, out decimal betrag) || betrag == 0m)
                throw new InvalidOperationException(
                    "Bitte zuerst den umzurechnenden Betrag im Betragsfeld eingeben.");

            // 2) Buchungsdatum bilden (2-stellige Jahre expandieren wie im Dialog).
            DateTime datum = BaueBuchungsdatumOderWirf();

            string ziel = Zielwaehrung;

            // 3) DSGVO-Einwilligung vor dem Netzzugriff sicherstellen.
            if (ApiEinwilligungAbfrage != null && !ApiEinwilligungAbfrage())
                return;   // ohne Einwilligung keine Abfrage (stillschweigend)

            // 4) Kurs(e) asynchron holen und umrechnen.
            var cts = new CancellationTokenSource();
            _umrechnungAbbruch = cts;
            UmrechnungStatus = code + " wird in " + ZielAnzeige(ziel) + " umgerechnet ...";
            UmrechnungLaeuft = true;
            try
            {
                decimal ergebnis = await Waehrungsliste.RechneUmAsync(
                    betrag, code, ziel, datum, 15, cts.Token);

                string originalBetrag = BetragText;   // Fremdbetrag (Eingabe)
                string ergebnisText = Waehrungsformat.BetragOhneGruppierung(ergebnis);
                BetragText = ergebnisText;

                // Umrechnung im Beschreibungstext dokumentieren (Issue #21).
                decimal proEinheit = ergebnis / betrag;
                string doku = Waehrungsliste.BaueUmrechnungsText(
                    originalBetrag, code, ergebnisText, ziel, datum, proEinheit);
                // Bereits angehaengte Umrechnung(en) ersetzen statt weiter
                // anhaengen (mehrfaches Umrechnen soll den Text nicht aufblaehen).
                string basis = Waehrungsliste.OhneUmrechnungsanhang(Beschreibung);
                string neu = string.IsNullOrWhiteSpace(basis) ? doku : basis + " -- " + doku;
                SetzeBeschreibungOhnePreset(neu);
            }
            catch (OperationCanceledException)
            {
                // Vom Anwender abgebrochen -> stillschweigend. Ein Timeout (der
                // in .NET 4.8 ebenfalls als OperationCanceledException kommt) ist
                // dagegen ein Fehler und wird als solcher gemeldet.
                if (cts.IsCancellationRequested) return;
                throw new TimeoutException(
                    "Zeitüberschreitung bei der Kursabfrage. Bitte erneut versuchen.");
            }
            finally
            {
                UmrechnungLaeuft = false;
                _umrechnungAbbruch = null;
                cts.Dispose();
            }
        }

        /// <summary>Setzt die Beschreibung, ohne die Preset-Erkennung
        /// auszulösen (die angehängte Umrechnung beginnt mit Ziffern und würde
        /// sonst als Preset-Nummer missverstanden -- das war die "Hakeligkeit").</summary>
        private void SetzeBeschreibungOhnePreset(string neu)
        {
            _presetLaden = true;
            try { Beschreibung = neu; }
            finally { _presetLaden = false; }
        }

        private static string ZielAnzeige(string ziel)
            => ziel.Equals("EUR", StringComparison.OrdinalIgnoreCase) ? "Euro" : ziel;

        private DateTime BaueBuchungsdatumOderWirf()
        {
            if (DatumTag < 1 || DatumTag > 31 || DatumMonat < 1 || DatumMonat > 12)
                throw new InvalidOperationException(
                    "Bitte zuerst ein gültiges Buchungsdatum (Tag/Monat/Jahr) eingeben -- "
                    + "der Kurs wird tagesgenau abgefragt.");
            int jahr = ExpandiereJahr(DatumJahr);
            try
            {
                return new DateTime(jahr, DatumMonat, DatumTag);
            }
            catch (ArgumentOutOfRangeException)
            {
                throw new InvalidOperationException(
                    $"Ungültiges Buchungsdatum: {DatumTag}.{DatumMonat}.{jahr}");
            }
        }
    }
}
