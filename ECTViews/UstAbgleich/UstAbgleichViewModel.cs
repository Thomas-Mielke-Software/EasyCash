// UstAbgleichViewModel.cs -- Dialog "Zuordnungen für Voranmeldung und
// Umsatzsteuererklärung abgleichen" (Port von CUStAbgleichDlg, v3).
//
// Zeilen = alle E/Ü-Konten, deren Zuweisungen für die beiden USt-Formulare
// nicht zusammenpassen (Befund != Stimmig), Konten mit Buchungen im
// Buchungsjahr zuerst -- nur sie verfälschen die Formulare dieses Jahres.
// Vorab angehakt ist nur Fehlendes (UstFehlt, UvaFehlt); bei einem
// Widerspruch entscheidet der Benutzer. Eine Zeile ohne Vorschlag lässt sich
// nicht anhaken, stattdessen öffnet sich die Auswahl (AuswahlNoetig).
// "Übernehmen" trägt die angehakten Vorschläge in die Feldzuweisungen ein
// und speichert die geänderten Konten.
//
// Die Regeln selbst stehen in ECTEngine/UstAbgleich.cs.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.UstAbgleichUi
{
    public enum UstAbgleichAktion { Keine, UstSetzen, UvaSetzen, Belassen }

    /// <summary>Ein Eintrag der Auswahl je Zeile (Kontextmenü).</summary>
    public sealed class UstAbgleichOption
    {
        public string Text { get; }
        public bool IstKopf { get; }
        public bool IstTrenner { get; }
        public bool IstGewaehlt { get; }
        public Action Anwenden { get; }

        private UstAbgleichOption(string text, bool kopf, bool trenner, bool gewaehlt, Action anwenden)
        {
            Text = text ?? "";
            IstKopf = kopf;
            IstTrenner = trenner;
            IstGewaehlt = gewaehlt;
            Anwenden = anwenden;
        }

        public static UstAbgleichOption Kopf(string text) => new UstAbgleichOption(text, true, false, false, null);
        public static UstAbgleichOption Trenner() => new UstAbgleichOption("", false, true, false, null);
        public static UstAbgleichOption Eintrag(string text, bool gewaehlt, Action anwenden)
            => new UstAbgleichOption(text, false, false, gewaehlt, anwenden);
    }

    public sealed class UstAbgleichZeile : ViewModelBase
    {
        private readonly UstAbgleichViewModel _vm;

        public EUKonto Konto { get; }
        public UstAbgleichBefund Befund { get; }
        public int UvaFeld { get; }
        public int UstFeld { get; }
        public int Buchungen { get; }

        internal UstAbgleichZeile(UstAbgleichViewModel vm, EUKonto konto, UstAbgleichBefund befund,
            int uvaFeld, int ustFeld, int buchungen)
        {
            _vm = vm;
            Konto = konto;
            Befund = befund;
            UvaFeld = uvaFeld;
            UstFeld = ustFeld;
            Buchungen = buchungen;
        }

        public string KontoName => Konto.Name;
        public string Art => Konto.IstEinnahme ? "Einnahmen" : "Ausgaben";
        public string UvaText => _vm.Abgleich.FeldText(false, UvaFeld);
        public string UstText => _vm.Abgleich.FeldText(true, UstFeld);
        public string BuchungenText => Buchungen > 0 ? Buchungen.ToString() : "";
        public string BefundText => _vm.Abgleich.BefundText(Befund, UvaFeld, UstFeld, Konto.IstEinnahme);

        private UstAbgleichAktion _aktion;
        public UstAbgleichAktion Aktion
        {
            get => _aktion;
            set { if (SetProperty(ref _aktion, value)) OnPropertyChanged(nameof(VorschlagText)); }
        }

        private int _ziel;
        /// <summary>Feld-ID, die eingetragen wird (0 = Zuordnung entfernen).</summary>
        public int Ziel
        {
            get => _ziel;
            set { if (SetProperty(ref _ziel, value)) OnPropertyChanged(nameof(VorschlagText)); }
        }

        public string VorschlagText
        {
            get
            {
                switch (Aktion)
                {
                    case UstAbgleichAktion.UstSetzen:
                        return Ziel != 0 ? "Erklärung: " + _vm.Abgleich.FeldText(true, Ziel) : "Erklärung: Zuordnung entfernen";
                    case UstAbgleichAktion.UvaSetzen:
                        return Ziel != 0 ? "Voranmeldung: " + _vm.Abgleich.FeldText(false, Ziel) : "Voranmeldung: Zuordnung entfernen";
                    case UstAbgleichAktion.Belassen:
                        return "so lassen und nicht mehr melden";
                    default:
                        return Befund == UstAbgleichBefund.UstMehrdeutig
                            ? "mehrere Felder möglich - bitte wählen (Doppelklick)"
                            : "bitte entscheiden (Doppelklick)";
                }
            }
        }

        private bool _angehakt;
        /// <summary>Vorschlag übernehmen? Ohne Vorschlag nicht anhakbar --
        /// dann meldet die Zeile, dass erst gewählt werden muss.</summary>
        public bool Angehakt
        {
            get => _angehakt;
            set
            {
                if (value && Aktion == UstAbgleichAktion.Keine)
                {
                    OnPropertyChanged();   // Häkchen in der Oberfläche zurücknehmen
                    _vm.MeldeAuswahlNoetig(this);
                    return;
                }
                SetProperty(ref _angehakt, value);
            }
        }

        internal void SetzeAngehakt(bool wert) => SetProperty(ref _angehakt, wert, nameof(Angehakt));
    }

    public sealed class UstAbgleichViewModel : ViewModelBase
    {
        public UstAbgleich Abgleich { get; }
        public ObservableCollection<UstAbgleichZeile> Zeilen { get; } = new ObservableCollection<UstAbgleichZeile>();

        private readonly Action<EUKonto> _speichere;

        /// <summary>Spaltenkopf "Buchungen 2025" (Buchungsjahr des Dokuments).</summary>
        public string BuchungenKopf { get; }
        public string HinweisText { get; }
        /// <summary>Abgleich überhaupt möglich (Formular mit uvafeld installiert)?</summary>
        public bool IstMoeglich { get; }
        public bool HatZeilen => Zeilen.Count > 0;

        /// <summary>true, wenn "Übernehmen" etwas geändert (und gespeichert) hat.</summary>
        public bool Geaendert { get; private set; }

        /// <summary>Eine Zeile ohne Vorschlag soll angehakt werden: die
        /// Oberfläche öffnet die Auswahl.</summary>
        public event Action<UstAbgleichZeile> AuswahlNoetig;

        /// <param name="konten">die E/Ü-Konten (werden bei "Übernehmen" verändert)</param>
        /// <param name="doc">für die Buchungen je Konto im Buchungsjahr; darf null sein</param>
        /// <param name="jahr">Ersatz-Jahr für den Spaltenkopf ohne Dokument</param>
        /// <param name="speichere">schreibt ein geändertes Konto zurück (Default
        /// EUKonten.SpeichereFeldzuweisungen)</param>
        public UstAbgleichViewModel(IEnumerable<EUKonto> konten, BuchungsDocument doc, UstAbgleich abgleich,
            int jahr, Action<EUKonto> speichere = null)
        {
            Abgleich = abgleich ?? UstAbgleich.Inaktiv();
            _speichere = speichere ?? EUKonten.SpeichereFeldzuweisungen;
            int buchungsjahr = doc?.Jahr ?? jahr;
            BuchungenKopf = "Buchungen " + buchungsjahr.ToString("D4");

            IstMoeglich = Abgleich.IstAktiv;
            if (!IstMoeglich)
            {
                HinweisText = "Ein Abgleich ist nicht möglich: Es ist kein Formular '" + UstAbgleich.FormularUst
                            + "' installiert, das die Entsprechungen zur Voranmeldung enthält.";
                return;
            }

            var liste = (konten ?? Enumerable.Empty<EUKonto>()).ToList();
            // Konten mit Buchungen im Buchungsjahr zuerst: nur sie verfälschen die Formulare dieses Jahres tatsächlich
            for (int durchgang = 0; durchgang < 2; durchgang++)
                foreach (var einnahmen in new[] { true, false })
                    foreach (var k in liste.Where(x => x.IstEinnahme == einnahmen))
                    {
                        var befund = Abgleich.Pruefe(k.Feldzuweisungen, k.IstEinnahme);
                        if (befund == UstAbgleichBefund.Stimmig) continue;
                        int buchungen = UstAbgleich.ZaehleBuchungen(doc, k.IstEinnahme, k.Name);
                        if ((durchgang == 0) != (buchungen > 0)) continue;

                        int uva = UstAbgleich.LiesZuweisung(k.Feldzuweisungen, UstAbgleich.FormularUva);
                        int ust = UstAbgleich.LiesZuweisung(k.Feldzuweisungen, UstAbgleich.FormularUst);
                        var zeile = new UstAbgleichZeile(this, k, befund, uva, ust, buchungen);

                        var kandidaten = Abgleich.UstKandidaten(uva, k.IstEinnahme);
                        if (befund == UstAbgleichBefund.UvaFehlt)
                        {
                            zeile.Aktion = UstAbgleichAktion.UvaSetzen;
                            zeile.Ziel = Abgleich.UvaFeldZu(ust);
                        }
                        else if (kandidaten.Count == 1)   // UstFehlt, oder Widerspruch mit eindeutigem Gegenstück zur Voranmeldung
                        {
                            zeile.Aktion = UstAbgleichAktion.UstSetzen;
                            zeile.Ziel = kandidaten[0];
                        }
                        // Fehlendes ist unstrittig und wird vorab angehakt; bei einem Widerspruch soll der Benutzer selbst entscheiden
                        zeile.SetzeAngehakt(befund == UstAbgleichBefund.UstFehlt || befund == UstAbgleichBefund.UvaFehlt);
                        Zeilen.Add(zeile);
                    }

            HinweisText = Zeilen.Count == 0
                ? "Die Zuordnungen der E/Ü-Konten für Voranmeldung und Erklärung passen zusammen. Es ist nichts zu tun."
                : "Die Liste zeigt die E/Ü-Konten, deren Zuordnungen für die " + UstAbgleich.FormularUva + " und die "
                  + UstAbgleich.FormularUst + " nicht zusammenpassen. Konten mit Buchungen im Buchungsjahr stehen oben: "
                  + "Nur sie verfälschen die Formulare dieses Jahres. 'Übernehmen' trägt die angehakten Vorschläge ein. "
                  + "Ein Doppelklick auf eine Zeile bietet andere Möglichkeiten an, auch 'so lassen', wenn die "
                  + "Abweichung gewollt ist.";
        }

        internal void MeldeAuswahlNoetig(UstAbgleichZeile zeile) => AuswahlNoetig?.Invoke(zeile);

        /// <summary>Auswahl für eine Zeile (Pendant zum Vorschlagsmenü in
        /// CUStAbgleichDlg::VorschlagAendern).</summary>
        public List<UstAbgleichOption> Optionen(UstAbgleichZeile z)
        {
            var liste = new List<UstAbgleichOption>();
            if (z == null) return liste;
            bool widerspruch = z.Befund == UstAbgleichBefund.Widerspruch;

            var kandidaten = Abgleich.UstKandidaten(z.UvaFeld, z.Konto.IstEinnahme);
            int uvaZurErklaerung = Abgleich.UvaFeldZu(z.UstFeld);
            if (uvaZurErklaerung == z.UvaFeld || !Abgleich.UvaFeldZuweisbar(uvaZurErklaerung, z.Konto.IstEinnahme))
                uvaZurErklaerung = 0;

            liste.Add(UstAbgleichOption.Kopf($"Konto '{z.KontoName}':"));
            liste.Add(UstAbgleichOption.Trenner());
            foreach (int k in kandidaten)
            {
                if (k == z.UstFeld) continue;
                int feld = k;
                liste.Add(UstAbgleichOption.Eintrag("Erklärung: " + Abgleich.FeldText(true, feld),
                    z.Aktion == UstAbgleichAktion.UstSetzen && z.Ziel == feld,
                    () => Waehle(z, UstAbgleichAktion.UstSetzen, feld)));
            }
            if (widerspruch)
                liste.Add(UstAbgleichOption.Eintrag("Erklärung: Zuordnung entfernen",
                    z.Aktion == UstAbgleichAktion.UstSetzen && z.Ziel == 0,
                    () => Waehle(z, UstAbgleichAktion.UstSetzen, 0)));
            if (uvaZurErklaerung != 0)
                liste.Add(UstAbgleichOption.Eintrag("Voranmeldung: " + Abgleich.FeldText(false, uvaZurErklaerung),
                    z.Aktion == UstAbgleichAktion.UvaSetzen && z.Ziel == uvaZurErklaerung,
                    () => Waehle(z, UstAbgleichAktion.UvaSetzen, uvaZurErklaerung)));
            if (widerspruch)
                liste.Add(UstAbgleichOption.Eintrag("Voranmeldung: Zuordnung entfernen",
                    z.Aktion == UstAbgleichAktion.UvaSetzen && z.Ziel == 0,
                    () => Waehle(z, UstAbgleichAktion.UvaSetzen, 0)));
            liste.Add(UstAbgleichOption.Trenner());
            liste.Add(UstAbgleichOption.Eintrag("So lassen und nicht mehr melden",
                z.Aktion == UstAbgleichAktion.Belassen,
                () => Waehle(z, UstAbgleichAktion.Belassen, 0)));
            // "vorerst nichts ändern": Vorschlag bleibt stehen, wird aber nicht übernommen
            liste.Add(UstAbgleichOption.Eintrag("Vorerst nichts ändern", false, () => z.SetzeAngehakt(false)));
            return liste;
        }

        private static void Waehle(UstAbgleichZeile z, UstAbgleichAktion aktion, int ziel)
        {
            z.Aktion = aktion;
            z.Ziel = ziel;
            z.SetzeAngehakt(true);
        }

        /// <summary>Trägt die angehakten Vorschläge ein und speichert die
        /// betroffenen Konten.</summary>
        public void Uebernehmen()
        {
            var geaendert = new List<EUKonto>();
            foreach (var z in Zeilen)
            {
                if (!z.Angehakt || z.Aktion == UstAbgleichAktion.Keine) continue;
                var d = z.Konto.Feldzuweisungen;
                if (z.Aktion == UstAbgleichAktion.Belassen)
                    UstAbgleich.SetzeBelassen(d, true);
                else
                {
                    UstAbgleich.SetzeBelassen(d, false);
                    UstAbgleich.SchreibeZuweisung(d,
                        z.Aktion == UstAbgleichAktion.UstSetzen ? UstAbgleich.FormularUst : UstAbgleich.FormularUva,
                        z.Ziel);
                }
                if (!geaendert.Contains(z.Konto)) geaendert.Add(z.Konto);
            }
            foreach (var k in geaendert) _speichere(k);
            Geaendert = geaendert.Count > 0;
        }
    }
}
