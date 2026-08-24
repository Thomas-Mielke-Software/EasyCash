// VorlagenBibliothekViewModel.cs -- Baum + Stichwortsuche der mitgelieferten
// Vorlagen-Bibliothek (ECTEngine.VorlagenBibliothek).
//
// Die Engine liefert den Katalog als Abschnitts-/Eintrags-Baum; fuer die
// TreeView braucht es pro Knoten EINE Kinderliste, deshalb die duenne
// Knoten-Huelle hier. Bei jeder Suche wird der Baum neu aus dem gefilterten
// Engine-Ergebnis gebaut -- bei ein paar Dutzend Eintraegen ist das
// billiger und einfacher als Sichtbarkeits-Flags zu pflegen.

using System.Collections.Generic;
using System.Collections.ObjectModel;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi
{
    /// <summary>Ein Knoten im Bibliotheks-Baum: Abschnitt ODER Vorlage.</summary>
    public sealed class BibliothekKnoten
    {
        public string Titel { get; set; } = "";

        /// <summary>Gesetzt, wenn der Knoten eine Vorlage ist (Blatt).</summary>
        public BibliothekEintrag Eintrag { get; set; }

        public bool IstEintrag => Eintrag != null;

        public ObservableCollection<BibliothekKnoten> Kinder { get; }
            = new ObservableCollection<BibliothekKnoten>();

        /// <summary>Abschnitte starten aufgeklappt -- die Bibliothek ist klein
        /// genug, dass Zuklappen mehr Klicks kostet als es spart.</summary>
        public bool IstAufgeklappt { get; set; } = true;

        /// <summary>Kurzinfo rechts neben dem Titel ("Ausgabe, 2 Zusatzzeilen").</summary>
        public string Kurzinfo
        {
            get
            {
                if (Eintrag == null) return "";
                string art = Eintrag.IstEinnahme ? "Einnahme" : "Ausgabe";
                if (Eintrag.ZeilenAnzahl == 0) return art;
                return Eintrag.ZeilenAnzahl == 1
                    ? art + ", 1 Zusatzzeile"
                    : art + ", " + Eintrag.ZeilenAnzahl + " Zusatzzeilen";
            }
        }
    }

    public sealed class VorlagenBibliothekViewModel : ViewModelBase
    {
        public VorlagenBibliothekViewModel(IReadOnlyList<int> freieSlots)
        {
            FreieSlots = freieSlots ?? new int[0];
            _gewaehlterSlot = FreieSlots.Count > 0 ? FreieSlots[0] : -1;
            BaueBaum("");
        }

        /// <summary>Freie Vorlagen-Plätze zur Auswahl (aufsteigend).</summary>
        public IReadOnlyList<int> FreieSlots { get; }

        private int _gewaehlterSlot;
        /// <summary>Zielplatz für die Übernahme; -1 wenn keiner frei ist.</summary>
        public int GewaehlterSlot
        {
            get => _gewaehlterSlot;
            set => SetProperty(ref _gewaehlterSlot, value);
        }

        public bool HatFreienSlot => FreieSlots.Count > 0;

        /// <summary>Warnung statt Platz-Auswahl, wenn alles belegt ist.</summary>
        public string SlotHinweis => HatFreienSlot
            ? "" : "Alle 100 Vorlagen-Plätze sind belegt.";

        public ObservableCollection<BibliothekKnoten> Wurzeln { get; }
            = new ObservableCollection<BibliothekKnoten>();

        private string _suchtext = "";
        /// <summary>Stichwortsuche; mehrere Woerter werden UND-verknuepft.</summary>
        public string Suchtext
        {
            get => _suchtext;
            set
            {
                if (!SetProperty(ref _suchtext, value)) return;
                BaueBaum(value);
            }
        }

        private BibliothekKnoten _auswahl;
        public BibliothekKnoten Auswahl
        {
            get => _auswahl;
            set
            {
                if (!SetProperty(ref _auswahl, value)) return;
                OnPropertyChanged(nameof(HatVorlagenAuswahl));
                OnPropertyChanged(nameof(KannUebernehmen));
                OnPropertyChanged(nameof(AuswahlNotiz));
                OnPropertyChanged(nameof(AuswahlUeberschrift));
            }
        }

        /// <summary>True, wenn eine VORLAGE (kein Abschnitt) gewaehlt ist.</summary>
        public bool HatVorlagenAuswahl => _auswahl != null && _auswahl.IstEintrag;

        /// <summary>Übernehmen ist nur möglich mit Vorlage UND freiem Platz.</summary>
        public bool KannUebernehmen => HatVorlagenAuswahl && HatFreienSlot;

        public string AuswahlUeberschrift =>
            HatVorlagenAuswahl ? _auswahl.Eintrag.Titel : "";

        public string AuswahlNotiz =>
            HatVorlagenAuswahl ? _auswahl.Eintrag.Notiz : "";

        /// <summary>Meldung statt Baum: leere Bibliothek oder keine Treffer.</summary>
        public string HinweisText
        {
            get
            {
                if (VorlagenBibliothek.Fehler.Length > 0) return VorlagenBibliothek.Fehler;
                if (Wurzeln.Count > 0) return "";
                return _suchtext.Trim().Length > 0
                    ? "Keine Vorlage passt zu \"" + _suchtext.Trim() + "\"."
                    : "Die Vorlagen-Bibliothek ist leer.";
            }
        }

        public bool ZeigeHinweis => HinweisText.Length > 0;

        /// <summary>XML der gewaehlten Vorlage (Futter fuer den Import-Weg);
        /// null, wenn kein Blatt gewaehlt ist.</summary>
        public string GewaehltesVorlagenXml =>
            HatVorlagenAuswahl ? _auswahl.Eintrag.VorlagenXml : null;

        // ------------------------------------------------------------------

        private void BaueBaum(string suchtext)
        {
            Wurzeln.Clear();
            foreach (var abschnitt in VorlagenBibliothek.Suche(suchtext))
                Wurzeln.Add(Wandle(abschnitt));

            // Auswahl haelt einem Filterwechsel nicht stand (neue Knoten-Objekte)
            Auswahl = null;
            OnPropertyChanged(nameof(HinweisText));
            OnPropertyChanged(nameof(ZeigeHinweis));
        }

        private static BibliothekKnoten Wandle(BibliothekAbschnitt abschnitt)
        {
            var knoten = new BibliothekKnoten { Titel = abschnitt.Titel };
            foreach (var unter in abschnitt.Abschnitte)
                knoten.Kinder.Add(Wandle(unter));
            foreach (var e in abschnitt.Eintraege)
                knoten.Kinder.Add(new BibliothekKnoten
                {
                    Titel   = e.Titel.Length > 0 ? e.Titel : e.Name,
                    Eintrag = e
                });
            return knoten;
        }
    }
}
