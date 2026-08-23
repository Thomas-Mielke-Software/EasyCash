// FormularViewModel.cs - ViewModel der WPF-Formular-Ansicht (.ecf-Formulare:
// EUeR, USt-Erklaerung, UVA/U30, AT-Formulare).
//
// Ersetzt die Anzeige-Seite von DrawFormularToDC (easycashview.cpp:4703):
// pro Seite ein PNG-Formularscan + absolut positionierte Feldwerte aus dem
// FormularRechner (ECTEngine). Das gewaehlte Formular (.ecf-Pfad) und der
// Betriebsfilter kommen vom nativen Aufrufer (m_csaFormulare/
// m_csaFormularfilter); der Zeitraum steckt im Formular selbst
// (voranmeldungszeitraum-Attribut), der Ribbon-Monatsfilter wirkt hier NICHT.
//
// Zoom: anders als Bericht/Journal (Schriftgroesse) skaliert die Ansicht
// ueber eine ScaleTransform (Skala) -- nur so bleibt die Deckung
// Feldtext/PNG-Hintergrund bei jedem Zoom exakt.

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.Formulare
{
    public class FormularViewModel : ViewModelBase
    {
        private BuchungsDocument _doc;
        private FormularDefinition _def;
        private string _betriebFilter = "";
        private int _zoomProzent = 100;

        /// <summary>Geladene Formular-Definition (null bei Ladefehler).</summary>
        public FormularDefinition Definition => _def;

        /// <summary>Zuletzt berechnete Feldwerte (Druck-Quelle, WYSIWYG).</summary>
        public IReadOnlyList<FormularFeldWert> LetzteWerte { get; private set; }

        public string BetriebFilter => _betriebFilter;

        public ObservableCollection<FormularSeiteVm> Seiten { get; } =
            new ObservableCollection<FormularSeiteVm>();

        public ObservableCollection<FormularNavigationItem> Navigation { get; } =
            new ObservableCollection<FormularNavigationItem>();

        // ----------------------------------------------------------
        // Kopf / Fehler
        // ----------------------------------------------------------
        private string _titel = "";
        public string Titel
        {
            get => _titel;
            private set => SetProperty(ref _titel, value);
        }

        private string _fehlertext = "";
        public string Fehlertext
        {
            get => _fehlertext;
            private set
            {
                if (SetProperty(ref _fehlertext, value))
                    OnPropertyChanged(nameof(HatFehler));
            }
        }
        public bool HatFehler => !string.IsNullOrEmpty(_fehlertext);

        public bool HatNavigation => Navigation.Count > 0;

        /// <summary>Schriftart des Formulars (schriftart-Attribut der .ecf).</summary>
        public string Schriftart =>
            string.IsNullOrWhiteSpace(_def?.Schriftart) ? "Courier New" : _def.Schriftart;

        // ----------------------------------------------------------
        // Designer-Modus (Pendant m_bFormularfelderAnzeigen +
        // Feld-Drag&amp;Drop, easycashview.cpp:7462ff). Felder bekommen
        // Rahmen (rosa; Auswahl blau), leere Felder einen Dummy-Wert,
        // Verschiebungen/AEnderungen gehen sofort in die .ecf.
        // ----------------------------------------------------------
        private bool _designerAktiv;
        public bool DesignerAktiv
        {
            get => _designerAktiv;
            set
            {
                if (_designerAktiv == value) return;
                _designerAktiv = value;
                OnPropertyChanged();
                Aktualisiere();   // Feld-VMs mit/ohne Designer-Zustand neu bauen
            }
        }

        /// <summary>Aktuell selektierte Felder (Designer).</summary>
        public IReadOnlyList<FormularFeldVm> Selektion
        {
            get
            {
                var liste = new List<FormularFeldVm>();
                foreach (var seite in Seiten)
                    foreach (var f in seite.Felder)
                        if (f.IstSelektiert) liste.Add(f);
                return liste;
            }
        }

        public void LeereSelektion()
        {
            foreach (var seite in Seiten)
                foreach (var f in seite.Felder)
                    f.IstSelektiert = false;
        }

        /// <summary>Verschiebt die Selektion um Promille-Deltas, schreibt
        /// in die .ecf und rechnet neu (Pendant MoveFormularfeld).</summary>
        public void VerschiebeSelektion(int deltaHorizontal, int deltaVertikal)
        {
            var selektiert = Selektion;
            if (selektiert.Count == 0 || _def == null) return;

            var ids = new List<int>();
            foreach (var vm in selektiert)
            {
                vm.FeldDef.Horizontal += deltaHorizontal;
                vm.FeldDef.Vertikal += deltaVertikal;
                if (vm.FeldDef.Horizontal < 0) vm.FeldDef.Horizontal = 0;
                if (vm.FeldDef.Vertikal < 0) vm.FeldDef.Vertikal = 0;
                _def.UebernehmeFeld(vm.FeldDef);
                ids.Add(vm.FeldDef.Id);
            }
            _def.Speichere();
            Aktualisiere();
            SelektiereFeldIds(ids);   // Selektion uebersteht den Neuaufbau
        }

        /// <summary>Selektion nach Feld-Ids wiederherstellen (nach Aktualisiere).</summary>
        public void SelektiereFeldIds(IList<int> ids)
        {
            foreach (var seite in Seiten)
                foreach (var f in seite.Felder)
                    f.IstSelektiert = ids.Contains(f.FeldDef.Id);
        }

        /// <summary>Feld-AEnderung aus dem Eigenschaften-Dialog uebernehmen.</summary>
        public void SpeichereFeld(FormularFeldDef feld)
        {
            if (_def == null || feld == null) return;
            _def.UebernehmeFeld(feld);
            _def.Speichere();
            Aktualisiere();
            SelektiereFeldIds(new List<int> { feld.Id });
        }

        public void LoescheFeld(FormularFeldDef feld)
        {
            if (_def == null || feld == null) return;
            _def.LoescheFeld(feld);
            _def.Speichere();
            Aktualisiere();
        }

        /// <summary>Neues Feld an Promille-Position anlegen; liefert die
        /// Definition fuer den anschliessenden Eigenschaften-Dialog.</summary>
        public FormularFeldDef NeuesFeldAn(int seite, int horizontal, int vertikal)
        {
            if (_def == null) return null;
            // naechste freie Id suchen (wie der native Dialog vorschlaegt)
            int id = 1;
            foreach (var f in _def.Felder)
                if (f.Id >= id) id = f.Id + 1;
            var feld = _def.NeuesFeld(id);
            feld.Seite = seite;
            feld.Horizontal = horizontal;
            feld.Vertikal = vertikal;
            _def.UebernehmeFeld(feld);
            _def.Speichere();
            Aktualisiere();
            SelektiereFeldIds(new List<int> { feld.Id });
            return feld;
        }

        public void SpeichereAbschnitt(FormularAbschnittDef abschnitt)
        {
            if (_def == null || abschnitt == null) return;
            _def.UebernehmeAbschnitt(abschnitt);
            _def.Speichere();
            Aktualisiere();
        }

        public FormularAbschnittDef NeuerAbschnittAn(string name, int seite, int vertikal)
        {
            if (_def == null) return null;
            var a = _def.NeuerAbschnitt(name, seite, vertikal);
            _def.Speichere();
            Aktualisiere();
            return a;
        }

        public void LoescheAbschnitt(FormularAbschnittDef abschnitt)
        {
            if (_def == null || abschnitt == null) return;
            _def.LoescheAbschnitt(abschnitt);
            _def.Speichere();
            Aktualisiere();
        }

        // ----------------------------------------------------------
        // Zoom (Prozent wie der native m_zoomfaktor; Anzeige skaliert
        // per ScaleTransform ueber Skala)
        // ----------------------------------------------------------
        public int ZoomProzent
        {
            get => _zoomProzent;
            set
            {
                if (value > 0 && value != _zoomProzent)
                {
                    _zoomProzent = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(Skala));
                }
            }
        }

        public double Skala => _zoomProzent / 100.0;

        // ----------------------------------------------------------
        // Events (Verdrahtung in der Bridge, Muster BerichtViewModel --
        // identische Signaturen, damit das Callback-Muster passt)
        // ----------------------------------------------------------
        public event Action<int> ZoomAendern;
        public void MeldeZoomAenderung(int deltaProzent) =>
            ZoomAendern?.Invoke(deltaProzent);

        public event Action DruckAnfordern;
        public void MeldeDruckAnforderung() => DruckAnfordern?.Invoke();

        // ----------------------------------------------------------
        // Aufbau
        // ----------------------------------------------------------

        /// <summary>Laedt die .ecf und rechnet die Ansicht. Ladefehler
        /// landen als Fehlertext in der Ansicht (Pendant zur nativen
        /// "Formular ... konnte nicht gefunden werden."-Zeile).</summary>
        public void Initialisiere(BuchungsDocument doc, string ecfPfad,
            string betriebFilter, int zoomProzent)
        {
            _doc = doc;
            _betriebFilter = betriebFilter ?? "";
            if (zoomProzent > 0) _zoomProzent = zoomProzent;

            LadeDefinition(ecfPfad);
            Aktualisiere();
        }

        /// <summary>Wechselt bei offener Ansicht auf ein anderes Formular
        /// (Ribbon-Menue) ohne Re-Embedding.</summary>
        public void WechsleFormular(string ecfPfad, string betriebFilter)
        {
            _betriebFilter = betriebFilter ?? "";
            LadeDefinition(ecfPfad);
            Aktualisiere();
        }

        private void LadeDefinition(string ecfPfad)
        {
            _def = null;
            Fehlertext = "";
            try
            {
                _def = FormularDefinition.Lade(ecfPfad);
            }
            catch (Exception)
            {
                Fehlertext = "Formular '" + ecfPfad + "' konnte nicht gefunden werden.";
            }
            Titel = _def?.Anzeigename ?? "";
            OnPropertyChanged(nameof(Schriftart));
        }

        /// <summary>Rechnet die Feldwerte neu und baut die Seiten-VMs
        /// (Buchungs-/Einstellungs-Aenderung, Formular-/Filterwechsel).</summary>
        public void Aktualisiere()
        {
            Seiten.Clear();
            Navigation.Clear();
            LetzteWerte = null;

            if (_def == null || _doc == null)
            {
                OnPropertyChanged(nameof(HatNavigation));
                return;
            }

            LetzteWerte = FormularRechner.Berechne(_doc, _def, _betriebFilter);

            double breite = FormularLayout.SeitenBreite(_def.Querformat);
            double hoehe = FormularLayout.SeitenHoehe(_def.Querformat);
            double fontDip = FormularLayout.FontDip(_def.Schriftgroesse, hoehe);

            // Seiten anlegen
            var seitenVms = new FormularSeiteVm[_def.Seitenzahl];
            for (int nr = 1; nr <= _def.Seitenzahl; nr++)
            {
                _def.Seitenbilder.TryGetValue(nr, out string bildDatei);
                seitenVms[nr - 1] = new FormularSeiteVm(nr, breite, hoehe,
                    LadeSeitenbild(bildDatei));
            }

            // Feldwerte auf die Seiten verteilen. Im Designer-Modus werden
            // auch leere Felder gezeigt (Dummy "0,00" zum Positionieren,
            // wie nativ easycashview.cpp:4929).
            foreach (var wert in LetzteWerte)
            {
                var feld = wert.Feld;
                if (!feld.HatInhalt) continue;
                if (!_designerAktiv && string.IsNullOrEmpty(wert.Text)) continue;
                if (feld.Seite < 1 || feld.Seite > _def.Seitenzahl) continue;

                seitenVms[feld.Seite - 1].Felder.Add(new FormularFeldVm(
                    wert, breite, hoehe, fontDip, _designerAktiv));
            }

            foreach (var vm in seitenVms)
                Seiten.Add(vm);

            // Navigations-Leiste: pro Seite ein Eintrag + die Abschnitte
            // (Pendant zur nativen Seitenleiste, easycashview.cpp:1294-1339)
            if (_def.Seitenzahl > 1 || _def.Abschnitte.Count > 0)
            {
                for (int nr = 1; nr <= _def.Seitenzahl; nr++)
                {
                    Navigation.Add(FormularNavigationItem.FuerSeite(nr));
                    foreach (var a in _def.Abschnitte)
                        if (a.Seite == nr)
                            Navigation.Add(FormularNavigationItem.FuerAbschnitt(a));
                }
            }

            OnPropertyChanged(nameof(HatNavigation));
        }

        // ----------------------------------------------------------
        // Seitenbilder: PNG-Scans neben der exe (Programmverzeichnis,
        // wie nativ GetProgrammverzeichnis + Dateiname). Prozessweiter
        // Cache -- die Scans sind gross und aendern sich nicht.
        // ----------------------------------------------------------
        private static readonly Dictionary<string, BitmapImage> _bildCache =
            new Dictionary<string, BitmapImage>(StringComparer.OrdinalIgnoreCase);

        internal static BitmapImage LadeSeitenbild(string dateiname)
        {
            if (string.IsNullOrEmpty(dateiname)) return null;
            string pfad = Path.IsPathRooted(dateiname)
                ? dateiname
                : Path.Combine(AppDomain.CurrentDomain.BaseDirectory, dateiname);

            lock (_bildCache)
            {
                if (_bildCache.TryGetValue(pfad, out var cached)) return cached;

                BitmapImage bild = null;
                try
                {
                    if (File.Exists(pfad))
                    {
                        bild = new BitmapImage();
                        bild.BeginInit();
                        bild.CacheOption = BitmapCacheOption.OnLoad;
                        bild.UriSource = new Uri(pfad, UriKind.Absolute);
                        bild.EndInit();
                        bild.Freeze();   // aus jedem Thread nutzbar (Druck!)
                    }
                }
                catch { bild = null; }   // defektes Bild: Seite bleibt weiss

                _bildCache[pfad] = bild;
                return bild;
            }
        }
    }

    /// <summary>Eine Formularseite: PNG-Hintergrund + positionierte Felder.
    /// Masse in unskalierten DIP (Zoom macht die ScaleTransform der View).</summary>
    public class FormularSeiteVm
    {
        public int Nummer { get; }
        public double Breite { get; }
        public double Hoehe { get; }
        public ImageSource Bild { get; }
        public bool HatBild => Bild != null;
        public List<FormularFeldVm> Felder { get; } = new List<FormularFeldVm>();

        public FormularSeiteVm(int nummer, double breite, double hoehe, ImageSource bild)
        {
            Nummer = nummer;
            Breite = breite;
            Hoehe = hoehe;
            Bild = bild;
        }
    }

    /// <summary>Ein positionierter Feldwert. Canvas-Positionierung:
    /// linksbuendige Felder setzen Left, rechtsbuendige Right (die jeweils
    /// andere Koordinate bleibt NaN = ungesetzt). Im Designer-Modus mit
    /// Rahmen (rosa; Auswahl blau) und Selektions-Zustand.</summary>
    public class FormularFeldVm : ViewModelBase
    {
        public string Text { get; }
        public double Top { get; }
        public double Left { get; }
        public double Right { get; }
        public double FontSize { get; }
        public string Tooltip { get; }

        /// <summary>Feld-Id, im Designer-Modus klein in der Ecke der
        /// Feldmarkierung eingeblendet (Pendant zum nativen Id-Text in der
        /// Feldmarke, easycashview.cpp:5016).</summary>
        public string IdText { get; }

        /// <summary>Halbe Feldschrift wie nativ (lfHeight /= 2), aber nicht
        /// unter 6 DIP -- bei kleinem Zoom waere die Id sonst unleserlich.</summary>
        public double IdFontSize { get; }

        /// <summary>Schwarz auf der Seite; WEISS fuer Felder, deren Anker
        /// rechts neben dem Seitenrand liegt (horizontal &gt; 1000 Promille,
        /// z.B. Feld 1111 der EUeR) -- die stehen auf dem dunklen
        /// Hintergrund der Ansicht und waeren schwarz kaum lesbar.
        /// Nur Bildschirm: der FormularDruckBauer nutzt dieses VM nicht
        /// (weisse Schrift wuerde auf Papier verschwinden).</summary>
        public System.Windows.Media.Brush Foreground { get; }

        /// <summary>Zugehoerige Feld-Definition (Designer: Drag, Dialog).</summary>
        public FormularFeldDef FeldDef { get; }
        public bool DesignerAktiv { get; }

        private bool _istSelektiert;
        public bool IstSelektiert
        {
            get => _istSelektiert;
            set
            {
                if (SetProperty(ref _istSelektiert, value))
                    OnPropertyChanged(nameof(RahmenBrush));
            }
        }

        private static readonly System.Windows.Media.Brush RosaBrush =
            ErzeugeBrush(0xE0, 0x80, 0x80);   // wie native Feldmarken (rosa)
        private static readonly System.Windows.Media.Brush BlauBrush =
            ErzeugeBrush(0x40, 0x80, 0xE0);   // Multiselect-Blau

        private static System.Windows.Media.Brush ErzeugeBrush(byte r, byte g, byte b)
        {
            var brush = new System.Windows.Media.SolidColorBrush(
                System.Windows.Media.Color.FromRgb(r, g, b));
            brush.Freeze();
            return brush;
        }

        /// <summary>Id-Marke nur im Designer-Modus ("Felder anzeigen").</summary>
        public System.Windows.Visibility IdSichtbar => DesignerAktiv
            ? System.Windows.Visibility.Visible
            : System.Windows.Visibility.Collapsed;

        /// <summary>Rahmen im Designer-Modus (sonst transparent).</summary>
        public System.Windows.Media.Brush RahmenBrush
        {
            get
            {
                if (!DesignerAktiv) return System.Windows.Media.Brushes.Transparent;
                return _istSelektiert ? BlauBrush : RosaBrush;
            }
        }

        public FormularFeldVm(FormularFeldWert wert, double seitenBreite,
            double seitenHoehe, double fontDip, bool designerAktiv = false)
        {
            var feld = wert.Feld;
            FeldDef = feld;
            DesignerAktiv = designerAktiv;

            // Designer: leere Felder mit Dummy-Wert zeigen, um sie
            // positionieren zu koennen (nativ "0,00", easycashview.cpp:4929)
            Text = (designerAktiv && string.IsNullOrEmpty(wert.Text))
                ? "0,00" : wert.Text;
            FontSize = fontDip;
            IdText = feld.Id.ToString(System.Globalization.CultureInfo.InvariantCulture);
            IdFontSize = Math.Max(6.0, fontDip / 2.0);
            Top = FormularLayout.YTextOben(feld.Vertikal, seitenHoehe);

            double x = FormularLayout.X(feld.Horizontal, seitenBreite);
            if (feld.RechtsBuendig)
            {
                Left = double.NaN;
                Right = seitenBreite - x;
            }
            else
            {
                Left = x;
                Right = double.NaN;
            }

            Foreground = x > seitenBreite
                ? System.Windows.Media.Brushes.White
                : System.Windows.Media.Brushes.Black;

            string tooltip = wert.Statustext ?? "";
            if (feld.Veraltet)
                tooltip = "VERALTET: Feld ist als veraltet markiert."
                    + (tooltip.Length > 0 ? " -- " + tooltip : "");
            if (designerAktiv)
                tooltip = "Feld " + feld.Id + " (" + feld.TypRoh + ")"
                    + (tooltip.Length > 0 ? " -- " + tooltip : "");
            Tooltip = tooltip.Length > 0 ? tooltip : null;
        }
    }

    /// <summary>Eintrag der Navigations-Leiste (Seite oder Abschnitt).</summary>
    public class FormularNavigationItem
    {
        public string Text { get; private set; }
        public int Seite { get; private set; }
        /// <summary>Promille-Vertikalposition (0 = Seitenanfang).</summary>
        public int Vertikal { get; private set; }
        public bool IstSeite { get; private set; }
        /// <summary>Bei Abschnitts-Eintraegen die Definition (Designer:
        /// Bearbeiten/Loeschen aus der Leiste), sonst null.</summary>
        public FormularAbschnittDef AbschnittDef { get; private set; }

        public static FormularNavigationItem FuerSeite(int nr) =>
            new FormularNavigationItem
            {
                Text = "Seite " + nr,
                Seite = nr,
                Vertikal = 0,
                IstSeite = true
            };

        public static FormularNavigationItem FuerAbschnitt(FormularAbschnittDef a) =>
            new FormularNavigationItem
            {
                Text = a.Name,
                Seite = a.Seite,
                Vertikal = a.Vertikal,
                IstSeite = false,
                AbschnittDef = a
            };
    }
}
