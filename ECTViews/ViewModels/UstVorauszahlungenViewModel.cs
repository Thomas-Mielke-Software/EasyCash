// UstVorauszahlungenViewModel.cs - ViewModel des Dialogs
// "Umsatzsteuer-Vorauszahlungen" (WPF-Ersatz fuer CUstVorauszahlungenDlg).
//
// 12 Monats- + 4 Quartals-Zahlbetraege plus Sondervorauszahlung
// (Dauerfristverlaengerung). Werte kommen aus/gehen in den
// Dokument-ErweiterungStore ueber ECTEngine.UstVorauszahlungen; die
// Persistenz-Normalisierung (0/unlesbar -> leer) macht der Helfer.
//
// Abweichung vom MFC-Original (bewusst): ungueltige Eingaben blockieren
// das Speichern (rote Markierung) statt kommentarlos zu 0 zu werden.

using System.Collections.Generic;
using ECTEngine;

namespace ECTViews.ViewModels
{
    /// <summary>Ein Zahlbetrag-Eintrag (Monat oder Quartal).</summary>
    public class UstZeitraumEintrag : ViewModelBase
    {
        private readonly UstVorauszahlungenViewModel _eltern;

        public string Label { get; }
        /// <summary>Zeitraum-Code wie nativ: 1-12 Monat, 41-44 Quartal.</summary>
        public int Zeitraum { get; }

        private string _wert = "";
        public string Wert
        {
            get => _wert;
            set
            {
                if (SetProperty(ref _wert, value ?? ""))
                {
                    OnPropertyChanged(nameof(IstGueltig));
                    _eltern.MeldeEingabeGeaendert();
                }
            }
        }

        /// <summary>Leer oder parsebar = gueltig.</summary>
        public bool IstGueltig =>
            string.IsNullOrWhiteSpace(_wert)
            || Waehrungsformat.TryParse(_wert, out _);

        public UstZeitraumEintrag(UstVorauszahlungenViewModel eltern,
            string label, int zeitraum, string wert)
        {
            _eltern = eltern;
            Label = label;
            Zeitraum = zeitraum;
            _wert = wert ?? "";
        }
    }

    public class UstVorauszahlungenViewModel : ViewModelBase
    {
        private static readonly string[] Monatsnamen =
        {
            "Januar", "Februar", "März", "April", "Mai", "Juni",
            "Juli", "August", "September", "Oktober", "November", "Dezember"
        };

        private readonly BuchungsDocument _doc;

        public List<UstZeitraumEintrag> Monate { get; } = new List<UstZeitraumEintrag>();
        public List<UstZeitraumEintrag> Quartale { get; } = new List<UstZeitraumEintrag>();

        public string Titel => "Umsatzsteuer-Vorauszahlungen " + _doc.Jahr;

        private string _sondervorauszahlung = "";
        public string Sondervorauszahlung
        {
            get => _sondervorauszahlung;
            set
            {
                if (SetProperty(ref _sondervorauszahlung, value ?? ""))
                {
                    OnPropertyChanged(nameof(SondervorauszahlungGueltig));
                    MeldeEingabeGeaendert();
                }
            }
        }

        public bool SondervorauszahlungGueltig =>
            string.IsNullOrWhiteSpace(_sondervorauszahlung)
            || Waehrungsformat.TryParse(_sondervorauszahlung, out _);

        public bool AlleGueltig
        {
            get
            {
                if (!SondervorauszahlungGueltig) return false;
                foreach (var m in Monate) if (!m.IstGueltig) return false;
                foreach (var q in Quartale) if (!q.IstGueltig) return false;
                return true;
            }
        }

        internal void MeldeEingabeGeaendert() => OnPropertyChanged(nameof(AlleGueltig));

        public UstVorauszahlungenViewModel(BuchungsDocument doc)
        {
            _doc = doc;

            for (int monat = 1; monat <= 12; monat++)
                Monate.Add(new UstZeitraumEintrag(this,
                    Monatsnamen[monat - 1], monat,
                    UstVorauszahlungen.Hole(doc, monat)));

            for (int quartal = 1; quartal <= 4; quartal++)
                Quartale.Add(new UstZeitraumEintrag(this,
                    quartal + ". Quartal",
                    UstVorauszahlungen.QuartalZeitraum(quartal),
                    UstVorauszahlungen.Hole(doc,
                        UstVorauszahlungen.QuartalZeitraum(quartal))));

            _sondervorauszahlung = UstVorauszahlungen.HoleSondervorauszahlung(doc);
        }

        /// <summary>Schreibt alle Werte ins Dokument (bei OK).
        /// Liefert true, wenn sich mindestens ein Wert geaendert hat.</summary>
        public bool Uebernehme()
        {
            bool geaendert = false;
            foreach (var m in Monate)
                geaendert |= UstVorauszahlungen.Setze(_doc, m.Zeitraum, m.Wert);
            foreach (var q in Quartale)
                geaendert |= UstVorauszahlungen.Setze(_doc, q.Zeitraum, q.Wert);
            geaendert |= UstVorauszahlungen.SetzeSondervorauszahlung(
                _doc, _sondervorauszahlung.Trim());
            return geaendert;
        }
    }
}
