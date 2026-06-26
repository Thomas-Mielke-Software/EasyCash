// KontenPageViewModel.cs -- E/Ü-Konten-Seite (M2).
//
// Links: ein gruppierter Konten-ListView (Einnahmen-/Ausgabenkonten).
// Rechts: pro Formular eine Liste mit dessen zuweisbaren Feldern (Kz + Name);
//         über jeder Liste eine ComboBox zur Auswahl der Formular-Variante
//         (Jahr), Default = neueste.
//
// Spaltenbreiten skalieren mit der Client-Breite (SpaltenBreite, vom
// Code-Behind gesetzt), Mindestbreite ~ Navigationsbreite.
//
// Selektions-Sync:
//  - Konto anklicken  -> alle ihm zugewiesenen Felder werden hervorgehoben.
//  - Feld anklicken    -> das zugewiesene Konto wird selektiert.
//
// Drag&Drop + Persistenz folgen in Phase 2b.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi.Pages
{
    public class KontenPageViewModel : ViewModelBase
    {
        public const string GruppeEinnahmen = "Einnahmenkonten";
        public const string GruppeAusgaben  = "Ausgabenkonten";

        public ObservableCollection<EUKontoVM> Konten { get; }
            = new ObservableCollection<EUKontoVM>();

        public ObservableCollection<FormularKategorieVM> Formulare { get; }
            = new ObservableCollection<FormularKategorieVM>();

        private EUKontoVM _selektiertesKonto;
        public EUKontoVM SelektiertesKonto
        {
            get => _selektiertesKonto;
            set
            {
                if (SetProperty(ref _selektiertesKonto, value))
                    MarkiereZugewieseneFelder(value);
            }
        }

        // Spaltenbreite (Konten + jede Formular-Spalte), vom Code-Behind aus
        // der Client-Breite berechnet. Mindestbreite ~ Navigationsbreite.
        public const double MinSpaltenBreite = 230;
        private double _spaltenBreite = MinSpaltenBreite;
        public double SpaltenBreite
        {
            get => _spaltenBreite;
            set => SetProperty(ref _spaltenBreite, value < MinSpaltenBreite ? MinSpaltenBreite : value);
        }

        /// <summary>Anzahl der Spalten (Konten + Formulare) -- für die Breitenberechnung.</summary>
        public int SpaltenAnzahl => Formulare.Count + 1;

        public KontenPageViewModel()
        {
            EUKonten.Lade();

            foreach (var k in EUKonten.EinnahmenKonten)
                Konten.Add(new EUKontoVM(k, GruppeEinnahmen));
            foreach (var k in EUKonten.AusgabenKonten)
                Konten.Add(new EUKontoVM(k, GruppeAusgaben));

            foreach (var g in EUKonten.FormularGruppen)
                Formulare.Add(new FormularKategorieVM(
                    g, FeldGewaehlt, () => MarkiereZugewieseneFelder(_selektiertesKonto)));
        }

        /// <summary>
        /// Baut die Konten-Liste aus dem (von außen geänderten) Cache neu auf --
        /// z.B. wenn ein anderes Dokumentfenster die Reihenfolge, eine
        /// Feldzuweisung oder eine Unterkategorie geändert hat. Die Selektion
        /// wird best möglich (über Name + Gruppe) wiederhergestellt. Die
        /// Formular-Spalten (aus den .ecf-Dateien) bleiben unberührt.
        /// Aufrufer (KontenPage) stellt sicher, dass das NICHT während einer
        /// laufenden Interaktion passiert.
        /// </summary>
        public void AktualisiereAusCache()
        {
            EUKonten.Lade();   // Cache neu einlesen -> frische EUKonto-Instanzen

            string selName     = _selektiertesKonto?.Name;
            bool   selEinnahme = _selektiertesKonto?.Modell.IstEinnahme ?? false;

            Konten.Clear();
            foreach (var k in EUKonten.EinnahmenKonten)
                Konten.Add(new EUKontoVM(k, GruppeEinnahmen));
            foreach (var k in EUKonten.AusgabenKonten)
                Konten.Add(new EUKontoVM(k, GruppeAusgaben));

            // Selektion (und damit die Feld-Hervorhebung) wiederherstellen.
            SelektiertesKonto = Konten.FirstOrDefault(
                v => v.Name == selName && v.Modell.IstEinnahme == selEinnahme);
        }

        private void MarkiereZugewieseneFelder(EUKontoVM konto)
        {
            foreach (var kat in Formulare)
            {
                string zielId = null;
                if (konto != null)
                    konto.Modell.Feldzuweisungen.TryGetValue(kat.Name, out zielId);

                foreach (var feld in kat.Felder)
                    feld.IstZugewiesen = feld.IstKeine
                        ? (konto != null && zielId == null)   // unverknüpft -> "<keine>" hervorheben
                        : (zielId != null && feld.Id == zielId);
            }
        }

        // Drag-Status fürs Drop-Target-Highlighting.
        private bool _zieheFeld;
        /// <summary>Es wird gerade ein Feld gezogen -> Konten als Drop-Ziele hervorheben.</summary>
        public bool ZieheFeld { get => _zieheFeld; set => SetProperty(ref _zieheFeld, value); }

        private bool _zieheKonto;
        /// <summary>Es wird gerade ein Konto gezogen -> Formular-Listen als Drop-Ziele hervorheben.</summary>
        public bool ZieheKonto { get => _zieheKonto; set => SetProperty(ref _zieheKonto, value); }

        /// <summary>
        /// Graut waehrend eines Konto-Drags alle Felder des unpassenden Typs aus
        /// (Einnahme-Konto -> Ausgaben-Felder grau und umgekehrt). Der
        /// "&lt;keine Feldzuweisung&gt;"-Eintrag bleibt aktiv (Zuweisung loesen
        /// ist immer erlaubt). <paramref name="istEinnahmeDesKontos"/> = null
        /// hebt die Ausgrauung wieder auf.
        /// </summary>
        public void GraueUnpassendeFelderAus(bool? istEinnahmeDesKontos)
        {
            foreach (var kat in Formulare)
                foreach (var feld in kat.Felder)
                    feld.IstAusgegraut = istEinnahmeDesKontos.HasValue
                        && !feld.IstKeine
                        && feld.IstEinnahme != istEinnahmeDesKontos.Value;
        }

        /// <summary>
        /// Weist dem Konto das Feld (in diesem Formular) zu (Drag&amp;Drop).
        /// Kardinalität: pro Konto/Formular höchstens EIN Feld; mehrere Konten
        /// dürfen dasselbe Feld teilen. Dasselbe Feld erneut auf das Konto
        /// ziehen entfernt die Zuweisung (Konto unverknüpft lassen).
        /// Persistiert sofort.
        /// </summary>
        public void WeiseFeldZu(EUKontoVM konto, string formularName, FormularfeldVM feld)
        {
            if (konto == null || feld == null || string.IsNullOrEmpty(formularName)) return;

            bool geloest;
            if (feld.IstKeine)
            {
                geloest = konto.Modell.Feldzuweisungen.Remove(formularName); // Zuweisung entfernen
            }
            else
            {
                if (konto.Modell.IstEinnahme != feld.IstEinnahme) return;   // Typ muss passen
                if (konto.Modell.Feldzuweisungen.TryGetValue(formularName, out var vorhanden)
                    && vorhanden == feld.Id)
                {
                    konto.Modell.Feldzuweisungen.Remove(formularName);     // Toggle -> lösen
                    geloest = true;
                }
                else
                {
                    konto.Modell.Feldzuweisungen[formularName] = feld.Id;  // pro Konto genau ein Feld
                    geloest = false;
                }
            }

            EUKonten.SpeichereFeldzuweisungen(konto.Modell);

            Statusleiste.Melde(geloest
                ? $"Verknüpfung von Konto \"{konto.Name}\" im Formular \"{formularName}\" gelöst."
                : $"Konto \"{konto.Name}\" mit Feld \"{feld.Bezeichnung}\" verknüpft.");

            SelektiertesKonto = konto;
            MarkiereZugewieseneFelder(konto);
        }

        /// <summary>
        /// Verschiebt ein Konto per Drag&amp;Drop innerhalb seiner Gruppe an die
        /// Position des Ziel-Kontos und persistiert die neue Reihenfolge.
        /// Gruppenübergreifend (Einnahmen ↔ Ausgaben) wird NICHT verschoben.
        /// </summary>
        /// <param name="zielIndex">Einfügeposition in der Gruppe OHNE das
        /// gezogene Konto (0 = ganz oben).</param>
        public void VerschiebeKontoAnPosition(EUKontoVM gezogen, int zielIndex)
        {
            if (gezogen == null) return;
            bool einnahme = gezogen.Modell.IstEinnahme;

            var gruppe = Konten.Where(k => k.Modell.IstEinnahme == einnahme).ToList();
            int alt = gruppe.IndexOf(gezogen);
            if (alt < 0) return;

            gruppe.RemoveAt(alt);
            int ziel = zielIndex;
            if (ziel < 0) ziel = 0;
            if (ziel > gruppe.Count) ziel = gruppe.Count;
            gruppe.Insert(ziel, gezogen);

            var einnahmen = einnahme ? gruppe : Konten.Where(k => k.Modell.IstEinnahme).ToList();
            var ausgaben  = einnahme ? Konten.Where(k => !k.Modell.IstEinnahme).ToList() : gruppe;

            var sw = System.Diagnostics.Stopwatch.StartNew();
            Konten.Clear();
            foreach (var k in einnahmen) Konten.Add(k);
            foreach (var k in ausgaben)  Konten.Add(k);
            sw.Stop();
            EUKonten.PerfLog($"Reorder-Rebuild (UI-Thread): {sw.ElapsedMilliseconds} ms, {Konten.Count} Konten");

            // Reihenfolge ändern -> Selektion/Hervorhebung bewusst NICHT ändern.
            PersistiereReihenfolge();

            Statusleiste.Melde($"Konto \"{gezogen.Name}\" neu einsortiert.");
        }

        private void PersistiereReihenfolge()
        {
            // Synchron: dank gebündeltem Sektions-Schreiben (6 Vorgänge) +
            // leichtem Konten-Rebuild schnell genug -- und GARANTIERT
            // abgeschlossen, bevor die App schließen kann (kein Hintergrund-
            // Thread, der verloren gehen könnte).
            var einnahmen = Konten.Where(k => k.Modell.IstEinnahme).Select(k => k.Modell).ToList();
            var ausgaben  = Konten.Where(k => !k.Modell.IstEinnahme).Select(k => k.Modell).ToList();
            EUKonten.SpeichereReihenfolge(einnahmen, ausgaben);
        }

        // Feld-Auswahl ändert die Konto-Selektion NICHT mehr: Ein Feld kann mit
        // mehreren Konten verknüpft sein -- die Verbindung wird stattdessen als
        // Rubber-Band-Linien zu allen verknüpften Konten visualisiert (Code-Behind
        // OnFeldSelektiert / ZeichneBaender).
        private void FeldGewaehlt(FormularKategorieVM kat, FormularfeldVM feld)
        {
        }
    }

    // ---------------------------------------------------------------------
    // View-Model-Wrapper
    // ---------------------------------------------------------------------

    public class EUKontoVM : ViewModelBase
    {
        public EUKonto Modell { get; }
        public string  Gruppe { get; }
        public EUKontoVM(EUKonto modell, string gruppe) { Modell = modell; Gruppe = gruppe; }

        public string Name => Modell.Name;

        /// <summary>Unterkategorie des Kontos -- strukturiert/rueckt die Konten
        /// in der EÜR ein. Wird sofort persistiert.</summary>
        public string Unterkategorie
        {
            get => Modell.Unterkategorie;
            set
            {
                var neu = (value ?? "").Trim();
                if (Modell.Unterkategorie == neu) return;
                Modell.Unterkategorie = neu;
                OnPropertyChanged();
                EUKonten.SpeichereUnterkategorie(Modell);
                Statusleiste.Melde(string.IsNullOrEmpty(neu)
                    ? $"Unterkategorie von Konto \"{Modell.Name}\" entfernt."
                    : $"Konto \"{Modell.Name}\" der Unterkategorie \"{neu}\" zugeordnet.");
            }
        }
    }

    public class FormularKategorieVM : ViewModelBase
    {
        public FormularGruppe Gruppe { get; }
        public string Name => Gruppe.Name;

        /// <summary>Auswählbare Varianten (Jahre), neueste zuerst.</summary>
        public IReadOnlyList<Formular> Varianten => Gruppe.Varianten;

        public ObservableCollection<FormularfeldVM> Felder { get; }
            = new ObservableCollection<FormularfeldVM>();

        private readonly Action<FormularKategorieVM, FormularfeldVM> _onFeldGewaehlt;
        private readonly Action _onVarianteGewechselt;

        public FormularKategorieVM(
            FormularGruppe gruppe,
            Action<FormularKategorieVM, FormularfeldVM> onFeldGewaehlt,
            Action onVarianteGewechselt)
        {
            Gruppe = gruppe;
            _onFeldGewaehlt = onFeldGewaehlt;
            _onVarianteGewechselt = onVarianteGewechselt;
            AusgewaehlteVariante = gruppe.Neueste;   // Default = neueste
        }

        private Formular _ausgewaehlteVariante;
        public Formular AusgewaehlteVariante
        {
            get => _ausgewaehlteVariante;
            set
            {
                if (!SetProperty(ref _ausgewaehlteVariante, value)) return;
                Felder.Clear();
                Felder.Add(FormularfeldVM.Keine());   // "<keine Feldzuweisung>" ganz oben
                if (value != null)
                    foreach (var f in value.Felder)
                        Felder.Add(new FormularfeldVM(f));
                _onVarianteGewechselt?.Invoke();   // Hervorhebung neu setzen
            }
        }

        private FormularfeldVM _ausgewaehltesFeld;
        public FormularfeldVM AusgewaehltesFeld
        {
            get => _ausgewaehltesFeld;
            set
            {
                _ausgewaehltesFeld = value;
                if (value != null) _onFeldGewaehlt?.Invoke(this, value);
            }
        }
    }

    public class FormularfeldVM : ViewModelBase
    {
        public Formularfeld Modell { get; }
        /// <summary>Sentinel-Eintrag "&lt;keine Feldzuweisung&gt;": Zuweisung auf
        /// diesen Eintrag entfernt die Verknüpfung des Kontos.</summary>
        public bool IstKeine { get; }

        public FormularfeldVM(Formularfeld modell) { Modell = modell; }
        private FormularfeldVM() { IstKeine = true; }
        public static FormularfeldVM Keine() => new FormularfeldVM();

        public string Id          => IstKeine ? "" : Modell.Id;
        public string Bezeichnung => IstKeine ? "<keine Feldzuweisung>" : Modell.Bezeichnung;
        public bool   IstEinnahme => !IstKeine && Modell.IstEinnahme;

        private bool _istZugewiesen;
        public bool IstZugewiesen
        {
            get => _istZugewiesen;
            set => SetProperty(ref _istZugewiesen, value);
        }

        // Waehrend ein Konto gezogen wird: Felder des unpassenden Typs (Einnahme
        // vs. Ausgabe) ausgrauen -- dorthin ist keine Zuweisung moeglich.
        private bool _istAusgegraut;
        public bool IstAusgegraut
        {
            get => _istAusgegraut;
            set => SetProperty(ref _istAusgegraut, value);
        }
    }
}
