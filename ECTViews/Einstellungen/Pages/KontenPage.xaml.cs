using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Shapes;
using ECTEngine;

namespace ECTViews.EinstellungenUi.Pages
{
    public partial class KontenPage : UserControl, IEinstellungenLiveZiel
    {
        public KontenPage()
        {
            InitializeComponent();
            // Beim Scrollen (egal welche innere Liste) die Rubber-Bands nachziehen.
            AddHandler(ScrollViewer.ScrollChangedEvent,
                new ScrollChangedEventHandler(OnIrgendwoGescrollt), true);
            // Verlässt der Tastaturfokus ein Eingabefeld, eine ggf. aufgeschobene
            // externe Änderung nachholen.
            AddHandler(LostKeyboardFocusEvent,
                new KeyboardFocusChangedEventHandler(OnFokusVerloren), true);
            EinstellungenLiveSync.Registriere(this);
        }

        // -----------------------------------------------------------------
        // Live-Sync: Änderung aus einem anderen Dokumentfenster übernehmen,
        // aber NICHT mitten in einem Drag&Drop oder einer Texteingabe (das
        // würde die gerade benutzte Liste/Selektion unter den Fingern wegziehen).
        // -----------------------------------------------------------------
        private bool _aktualisierungAusstehend;

        public void AufExterneEinstellungsaenderung()
        {
            if (InteraktionLaeuft()) { _aktualisierungAusstehend = true; return; }
            VM?.AktualisiereAusCache();
        }

        private bool InteraktionLaeuft()
        {
            if (_aktiv) return true;   // Drag&Drop läuft
            // Tastaturfokus in einem Eingabefeld dieser Seite (Unterkategorie-Editor)?
            if (Keyboard.FocusedElement is DependencyObject fe && fe is TextBox && IsAncestorOf(fe))
                return true;
            return false;
        }

        private void OnFokusVerloren(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_aktualisierungAusstehend)
                Dispatcher.BeginInvoke(new Action(VerarbeiteAusstehend),
                    System.Windows.Threading.DispatcherPriority.Background);
        }

        private void VerarbeiteAusstehend()
        {
            if (_aktualisierungAusstehend && !InteraktionLaeuft())
            {
                _aktualisierungAusstehend = false;
                VM?.AktualisiereAusCache();
            }
        }

        private void OnIrgendwoGescrollt(object sender, ScrollChangedEventArgs e)
        {
            if (_bandFeld != null && !_aktiv) ZeichneBaender();
        }

        private KontenPageViewModel VM => DataContext as KontenPageViewModel;

        // -----------------------------------------------------------------
        // Konto anlegen / umbenennen (Namensfeld) / löschen
        // -----------------------------------------------------------------
        private void OnNeuEinnahmenkonto(object sender, RoutedEventArgs e) => KontoAnlegen(true);
        private void OnNeuAusgabenkonto(object sender, RoutedEventArgs e) => KontoAnlegen(false);

        private void KontoAnlegen(bool einnahme)
        {
            var vm = VM;
            if (vm == null) return;
            // Eigene Änderung -> nicht an dieses Fenster zurückspiegeln.
            using (EinstellungenLiveSync.AlsUrheber(this))
                vm.KontoAnlegen(einnahme);

            // Direkt ins Namensfeld springen, damit man den Default-Namen
            // gleich überschreiben kann.
            Dispatcher.BeginInvoke(new Action(() =>
            {
                KontoNameEditor.Focus();
                KontoNameEditor.SelectAll();
            }), System.Windows.Threading.DispatcherPriority.Input);
        }

        private void OnKontoLoeschen(object sender, RoutedEventArgs e)
        {
            var vm = VM;
            var konto = vm?.SelektiertesKonto;
            if (konto == null) return;

            var antwort = MessageBox.Show(
                $"Konto \"{konto.Name}\" wirklich löschen?\n\n"
              + "Bereits gebuchte Beträge bleiben erhalten, fehlen aber in "
              + "den dann nicht mehr verknüpften Feldern von Formularen.",
                "Konto löschen",
                MessageBoxButton.YesNo, MessageBoxImage.Warning, MessageBoxResult.No);
            if (antwort != MessageBoxResult.Yes) return;

            using (EinstellungenLiveSync.AlsUrheber(this))
                vm.KontoLoeschen(konto);
        }

        // -----------------------------------------------------------------
        // Spaltenbreiten an die Client-Breite anpassen
        // -----------------------------------------------------------------
        private void OnSpaltenScrollSizeChanged(object sender, SizeChangedEventArgs e)
        {
            var vm = VM;
            if (vm == null) return;
            int spalten = vm.SpaltenAnzahl;
            if (spalten <= 0) return;
            double verfuegbar = e.NewSize.Width - (spalten * 8) - 6;
            vm.SpaltenBreite = verfuegbar / spalten;

            // Layout aendert sich -> Rubber-Bands nach dem Re-Layout neu zeichnen.
            if (_bandFeld != null && !_aktiv)
                Dispatcher.BeginInvoke(new Action(ZeichneBaender),
                    System.Windows.Threading.DispatcherPriority.Background);
        }

        // =================================================================
        // Custom Drag&Drop (Maus-Capture) mit schwebendem Ghost,
        // ausgegrauter Quelle und animierter "Platz machen"-Lücke.
        //   Feld  -> Konto / Konto -> Feld  = Zuweisung
        //   Konto -> Konto (gleiche Gruppe) = Sortieren
        // =================================================================
        private enum DragArt { None, Konto, Feld }

        private DragArt _art;
        private EUKontoVM _dragKonto;
        private EUKontoVM _klickKandidat;   // fuer reinen Klick (Selektion erst beim Up)
        private FormularfeldVM _dragFeld;
        private FormularKategorieVM _dragKat;
        private FrameworkElement _quelleContainer;
        private Point _start;
        private Point _grab;
        private bool _aktiv;

        private Border _ghost;
        private FrameworkElement _gapIndikator;
        private FrameworkElement _verbotenIndikator;
        private double _zeilenHoehe;
        private readonly List<GapEintrag> _gap = new List<GapEintrag>();
        private int _einfuegeIndex;
        private int _natuerlicherIndex;   // Einfügeposition = Ursprung -> keine Änderung
        private double _gruppeOben, _gruppeUnten;   // Y-Grenzen der gezogenen Gruppe (Page-Koord.)

        private struct GapEintrag
        {
            public ListBoxItem Item;
            public double Mitte;          // urspruengliche Mitte (Y) in Page-Koordinaten
        }

        // --- Druck auf ein Konto bzw. ein Feld merkt sich den Kandidaten ---
        private void OnKontoDown(object sender, MouseButtonEventArgs e)
        {
            ResetKandidat();
            var item = FindeAncestor<ListBoxItem>(e.OriginalSource as DependencyObject);
            var konto = item?.DataContext as EUKontoVM;
            if (konto == null) return;
            _art = DragArt.Konto;
            _dragKonto = konto;
            _klickKandidat = konto;
            _quelleContainer = item;
            _start = e.GetPosition(this);
            _grab = e.GetPosition(item);
            // Auto-Selektion der ListBox unterdruecken -> beim Sortieren aendert
            // sich die Selektion (und die Feld-Hervorhebung) NICHT. Ein reiner
            // Klick selektiert in OnDragUp.
            e.Handled = true;
        }

        private void OnFeldDown(object sender, MouseButtonEventArgs e)
        {
            ResetKandidat();
            var item = FindeAncestor<ListBoxItem>(e.OriginalSource as DependencyObject);
            var feld = item?.DataContext as FormularfeldVM;
            var kat  = (sender as ListBox)?.DataContext as FormularKategorieVM;
            if (feld == null || kat == null) return;
            _art = DragArt.Feld;
            _dragFeld = feld;
            _dragKat = kat;
            _quelleContainer = item;
            _start = e.GetPosition(this);
            _grab = e.GetPosition(item);
        }

        private void OnDragMove(object sender, MouseEventArgs e)
        {
            if (!_aktiv)
            {
                if (e.LeftButton != MouseButtonState.Pressed || _art == DragArt.None
                    || _quelleContainer == null) return;
                Point p = e.GetPosition(this);
                if (Math.Abs(p.X - _start.X) < SystemParameters.MinimumHorizontalDragDistance
                    && Math.Abs(p.Y - _start.Y) < SystemParameters.MinimumVerticalDragDistance)
                    return;
                StarteDrag((IInputElement)sender);
            }
            if (_aktiv) AktualisiereDrag(e.GetPosition(this));
        }

        private void OnDragUp(object sender, MouseButtonEventArgs e)
        {
            if (_aktiv) { BeendeDrag(e.GetPosition(this)); return; }
            // Kein Drag -> reiner Klick auf ein Konto: jetzt selektieren
            // (die Auto-Selektion war in OnKontoDown unterdrueckt).
            if (_klickKandidat != null && VM != null) VM.SelektiertesKonto = _klickKandidat;
            ResetKandidat();
        }

        // -----------------------------------------------------------------
        private void StarteDrag(IInputElement quelleZumCapturen)
        {
            _aktiv = true;
            _bandFeld = null; ZeichneBaender();   // Bands waehrend des Drags ausblenden
            Mouse.Capture(quelleZumCapturen);

            // Ghost erzeugen (Schnappschuss der Quelle), schwebt im Overlay.
            var brush = new VisualBrush(_quelleContainer)
            {
                Stretch = Stretch.None,
                AlignmentX = AlignmentX.Left,
                AlignmentY = AlignmentY.Top
            };
            _ghost = new Border
            {
                Width = _quelleContainer.ActualWidth,
                Height = _quelleContainer.ActualHeight,
                Background = brush,
                IsHitTestVisible = false,
                Opacity = 0.92,
                Effect = new DropShadowEffect { BlurRadius = 14, ShadowDepth = 4, Opacity = 0.45 },
                RenderTransformOrigin = new Point(0.5, 0.5),
                RenderTransform = new ScaleTransform(1.03, 1.03)
            };
            GhostLayer.Children.Add(_ghost);

            // Quelle ausgrauen.
            _quelleContainer.BeginAnimation(OpacityProperty,
                new DoubleAnimation(0.30, TimeSpan.FromMilliseconds(120)));

            var vm = VM;
            if (vm != null)
            {
                if (_art == DragArt.Feld) vm.ZieheFeld = true;
                else if (_art == DragArt.Konto)
                {
                    vm.ZieheKonto = true;
                    // Felder des unpassenden Typs ausgrauen (keine Zuweisung moeglich).
                    vm.GraueUnpassendeFelderAus(_dragKonto.Modell.IstEinnahme);
                }
            }

            if (_art == DragArt.Konto) { BereiteGapVor(); ErzeugeGapIndikator(); }
            _einfuegeIndex = -1;
        }

        private void AktualisiereDrag(Point p)
        {
            if (_ghost != null)
            {
                Canvas.SetLeft(_ghost, p.X - _grab.X);
                Canvas.SetTop(_ghost, p.Y - _grab.Y);
            }

            if (_art != DragArt.Konto) return;

            if (!UeberElement(KontenListe, p))
            {
                VerbotenAus();
                if (_einfuegeIndex != -1) { _einfuegeIndex = -1; AnimiereGap(-1); }
                // Ueber einer Formularspalte: deren Feldliste am Rand auto-scrollen,
                // damit auch verdeckte (z.B. die einzigen passenden) Felder als
                // Drop-Ziel erreichbar sind.
                AktualisiereFormularScroll(p);
                return;
            }
            StoppeFormularScroll();   // ueber den Konten -> kein Formular-Scroll

            // Gruppe robust ueber das Konto unter dem Cursor bestimmen (Hit-Test,
            // unabhaengig von der geoeffneten Luecke); Faelle ohne Treffer
            // (Header/Luecke) ueber die Y-Grenzen.
            var kontoUnter = FindeDataContext<EUKontoVM>(HitTestTopmost(p));
            bool andereGruppe = kontoUnter != null
                ? kontoUnter.Modell.IstEinnahme != _dragKonto.Modell.IstEinnahme
                : (p.Y < _gruppeOben || p.Y > _gruppeUnten + _zeilenHoehe);

            if (andereGruppe)
            {
                // Falsche Gruppe: keine Luecke, rotes Verbots-Zeichen am Cursor.
                if (_einfuegeIndex != -1) { _einfuegeIndex = -1; AnimiereGap(-1); }
                ZeigeVerboten(p);
                return;
            }

            VerbotenAus();
            EdgeScroll(p);
            int idx2 = BerechneEinfuegeIndex(p.Y);
            if (idx2 != _einfuegeIndex)
            {
                _einfuegeIndex = idx2;
                AnimiereGap(idx2);
            }
        }

        // Rotes Verbots-Zeichen (falsche Gruppe) am Cursor.
        private void ZeigeVerboten(Point p)
        {
            if (_verbotenIndikator == null)
            {
                var b = new Border
                {
                    Width = 26, Height = 26, CornerRadius = new CornerRadius(13),
                    Background = new SolidColorBrush(Color.FromRgb(0xD3, 0x2F, 0x2F)),
                    IsHitTestVisible = false,
                    Effect = new DropShadowEffect { BlurRadius = 8, ShadowDepth = 2, Opacity = 0.4 },
                    Child = new TextBlock
                    {
                        Text = "✕", Foreground = Brushes.White, FontWeight = FontWeights.Bold,
                        FontSize = 15, HorizontalAlignment = HorizontalAlignment.Center,
                        VerticalAlignment = VerticalAlignment.Center
                    }
                };
                GhostLayer.Children.Add(b);
                _verbotenIndikator = b;
            }
            Canvas.SetLeft(_verbotenIndikator, p.X + 12);
            Canvas.SetTop(_verbotenIndikator, p.Y + 12);
            _verbotenIndikator.Visibility = Visibility.Visible;
        }

        private void VerbotenAus()
        {
            if (_verbotenIndikator != null) _verbotenIndikator.Visibility = Visibility.Collapsed;
        }

        // Scrollt die Konten-Liste, wenn in den obersten/untersten 3 sichtbaren
        // Eintraegen gezogen wird. Erfasste Positionen werden um den Scroll-Delta
        // nachgezogen (kein Neu-Auslesen noetig).
        private void EdgeScroll(Point p)
        {
            var sv = FindeNachkommen<ScrollViewer>(KontenListe).FirstOrDefault();
            if (sv == null || sv.ScrollableHeight <= 0) return;

            double top    = sv.TransformToAncestor(this).Transform(new Point(0, 0)).Y;
            double bottom = top + sv.ViewportHeight;
            double rand   = 3 * _zeilenHoehe;

            double alt = sv.VerticalOffset, neu = alt;
            if (p.Y < top + rand && alt > 0)
                neu = Math.Max(0, alt - _zeilenHoehe);
            else if (p.Y > bottom - rand && alt < sv.ScrollableHeight)
                neu = Math.Min(sv.ScrollableHeight, alt + _zeilenHoehe);
            if (neu == alt) return;

            double delta = neu - alt;
            sv.ScrollToVerticalOffset(neu);
            for (int i = 0; i < _gap.Count; i++) { var g = _gap[i]; g.Mitte -= delta; _gap[i] = g; }
            _gruppeOben -= delta; _gruppeUnten -= delta;
        }

        // -----------------------------------------------------------------
        // Auto-Scroll der Formular-Feldliste unter dem Cursor (Konto-Drag).
        // Timer-basiert -> scrollt auch, wenn der Cursor in der Randzone
        // ruhig gehalten wird (nicht nur bei Mausbewegung).
        // -----------------------------------------------------------------
        private System.Windows.Threading.DispatcherTimer _formScrollTimer;
        private ScrollViewer _formScrollSv;
        private double _formScrollDelta;

        private void AktualisiereFormularScroll(Point p)
        {
            var lb = FindeNachkommen<ListBox>(FormulareListe)
                .FirstOrDefault(l => l.IsVisible && UeberElement(l, p));
            var sv = lb != null ? FindeNachkommen<ScrollViewer>(lb).FirstOrDefault() : null;
            if (sv == null || sv.ScrollableHeight <= 0) { StoppeFormularScroll(); return; }

            double top    = sv.TransformToAncestor(this).Transform(new Point(0, 0)).Y;
            double bottom = top + sv.ViewportHeight;
            const double rand = 30;   // Randzone in px

            double delta = 0;
            if (p.Y < top + rand)         delta = -Math.Max(6, (top + rand - p.Y) * 0.6);
            else if (p.Y > bottom - rand) delta =  Math.Max(6, (p.Y - (bottom - rand)) * 0.6);

            if (delta == 0) { StoppeFormularScroll(); return; }

            _formScrollSv    = sv;
            _formScrollDelta = delta;
            if (_formScrollTimer == null)
            {
                _formScrollTimer = new System.Windows.Threading.DispatcherTimer
                    { Interval = TimeSpan.FromMilliseconds(16) };
                _formScrollTimer.Tick += (s, e) =>
                {
                    if (_formScrollSv == null) return;
                    double neu = Math.Max(0, Math.Min(_formScrollSv.ScrollableHeight,
                        _formScrollSv.VerticalOffset + _formScrollDelta));
                    _formScrollSv.ScrollToVerticalOffset(neu);
                };
            }
            if (!_formScrollTimer.IsEnabled) _formScrollTimer.Start();
        }

        private void StoppeFormularScroll()
        {
            _formScrollSv = null; _formScrollDelta = 0;
            _formScrollTimer?.Stop();
        }

        private void BeendeDrag(Point p)
        {
            Mouse.Capture(null);

            // Ghost ausblenden + entfernen.
            if (_ghost != null)
            {
                var g = _ghost; _ghost = null;
                var fade = new DoubleAnimation(0, TimeSpan.FromMilliseconds(120));
                fade.Completed += (s, e) => GhostLayer.Children.Remove(g);
                g.BeginAnimation(OpacityProperty, fade);
            }

            // Gap-/Verbots-Indikator entfernen.
            if (_gapIndikator != null) { GhostLayer.Children.Remove(_gapIndikator); _gapIndikator = null; }
            if (_verbotenIndikator != null) { GhostLayer.Children.Remove(_verbotenIndikator); _verbotenIndikator = null; }

            // Quelle + Gap zuruecksetzen.
            _quelleContainer?.BeginAnimation(OpacityProperty,
                new DoubleAnimation(1.0, TimeSpan.FromMilliseconds(150)));
            foreach (var ge in _gap)
                AnimiereMargin(ge.Item, new Thickness(0));

            StoppeFormularScroll();

            var vm = VM;
            if (vm != null) { vm.ZieheFeld = false; vm.ZieheKonto = false; vm.GraueUnpassendeFelderAus(null); }

            // Drop auswerten.
            var hit  = HitTestTopmost(p);
            var konto = FindeDataContext<EUKontoVM>(hit);
            var feld  = FindeDataContext<FormularfeldVM>(hit);
            var kat   = FindeDataContext<FormularKategorieVM>(hit);
            bool ueberKonten = UeberElement(KontenListe, p);

            // Eigene Änderung als solche markieren -> der Live-Sync spiegelt sie
            // diesem Fenster nicht zurück (kein Selbst-Rebuild, der die gerade
            // gezeichneten Rubber-Bands sofort wieder löschen würde).
            using (EinstellungenLiveSync.AlsUrheber(this))
            {
                if (_art == DragArt.Feld)
                {
                    // Feld auf ein Konto gezogen -> zuweisen.
                    if (konto != null && _dragKat != null && _dragFeld != null)
                    {
                        vm?.WeiseFeldZu(konto, _dragKat.Name, _dragFeld);
                        ZeigeBaenderFuerFeld(_dragKat, _dragFeld);   // Verknuepfung visualisieren
                    }
                }
                else if (_art == DragArt.Konto)
                {
                    // Konto auf ein Formularfeld gezogen -> zuweisen (Vorrang vor
                    // Sortieren, falls die Maus ueber einer Formular-Spalte endet).
                    if (!ueberKonten && feld != null && kat != null)
                    {
                        vm?.WeiseFeldZu(_dragKonto, kat.Name, feld);
                        ZeigeBaenderFuerFeld(kat, feld);             // Verknuepfung visualisieren
                    }
                    else if (ueberKonten && _einfuegeIndex >= 0
                        && _einfuegeIndex != _natuerlicherIndex)   // Ursprungsposition -> nichts tun
                        vm?.VerschiebeKontoAnPosition(_dragKonto, _einfuegeIndex);
                }
            }

            ResetKandidat();

            // Nach dem Drag eine ggf. während des Drags aufgelaufene externe
            // Änderung (anderes Fenster) nachholen.
            if (_aktualisierungAusstehend)
                Dispatcher.BeginInvoke(new Action(VerarbeiteAusstehend),
                    System.Windows.Threading.DispatcherPriority.Background);
        }

        private void ResetKandidat()
        {
            _art = DragArt.None;
            _aktiv = false;
            _dragKonto = null; _dragFeld = null; _dragKat = null; _klickKandidat = null;
            _quelleContainer = null;
            _gap.Clear();
            _einfuegeIndex = -1;
        }

        // -----------------------------------------------------------------
        // Gap ("Platz machen") fuer das Konten-Sortieren
        // -----------------------------------------------------------------
        private void BereiteGapVor()
        {
            _gap.Clear();
            _zeilenHoehe = _quelleContainer.ActualHeight;
            if (_zeilenHoehe <= 0) _zeilenHoehe = 22;

            _gruppeOben = double.MaxValue;
            _gruppeUnten = double.MinValue;
            double mitteQuelle = 0;

            foreach (var k in VM.Konten.Where(x => x.Modell.IstEinnahme == _dragKonto.Modell.IstEinnahme))
            {
                if (!(KontenListe.ItemContainerGenerator.ContainerFromItem(k) is ListBoxItem c)) continue;
                c.Margin = new Thickness(0);   // Baseline fuer die Luecken-Animation

                double oben  = c.TransformToAncestor(this).Transform(new Point(0, 0)).Y;
                double mitte = oben + c.ActualHeight / 2.0;
                if (oben < _gruppeOben) _gruppeOben = oben;
                if (oben + c.ActualHeight > _gruppeUnten) _gruppeUnten = oben + c.ActualHeight;

                if (k == _dragKonto) { mitteQuelle = mitte; continue; }   // gezogenes Element nicht in _gap
                _gap.Add(new GapEintrag { Item = c, Mitte = mitte });
            }
            _gap.Sort((a, b) => a.Mitte.CompareTo(b.Mitte));

            // Natuerliche Position (Anzahl anderer Items darueber) -> keine Aenderung.
            _natuerlicherIndex = 0;
            foreach (var ge in _gap) { if (ge.Mitte < mitteQuelle) _natuerlicherIndex++; else break; }
        }

        private int BerechneEinfuegeIndex(double y)
        {
            int idx = 0;
            foreach (var ge in _gap) { if (ge.Mitte < y) idx++; else break; }
            return idx;
        }

        private void AnimiereGap(int einfuege)
        {
            // An der Ursprungsposition (vor/hinter dem gezogenen Item) keine
            // Luecke -- es waere keine Aenderung der Reihenfolge.
            int eff = (einfuege == _natuerlicherIndex) ? -1 : einfuege;

            // Luecke per Margin (echtes Reflow): EIN Item bekommt oben (bzw. am
            // Ende unten) Platz -- die nachfolgenden Eintraege UND die naechste
            // Gruppenueberschrift ruecken echt nach (kein Ueberlappen).
            for (int j = 0; j < _gap.Count; j++)
            {
                Thickness ziel;
                if (eff < 0) ziel = new Thickness(0);
                else if (j == eff) ziel = new Thickness(0, _zeilenHoehe, 0, 0);
                else if (eff >= _gap.Count && j == _gap.Count - 1) ziel = new Thickness(0, 0, 0, _zeilenHoehe);
                else ziel = new Thickness(0);
                AnimiereMargin(_gap[j].Item, ziel);
            }

            if (_gapIndikator == null) return;
            if (eff < 0)
            {
                _gapIndikator.Visibility = Visibility.Collapsed;
                return;
            }
            double left = KontenListe.TransformToAncestor(this).Transform(new Point(0, 0)).X;
            Canvas.SetLeft(_gapIndikator, left);
            Canvas.SetTop(_gapIndikator, GapTopY(eff));
            // Breite um die Scrollbalken-Breite kuerzen (Overlay nicht ueber dem Scrollbalken).
            _gapIndikator.Width = Math.Max(0, KontenListe.ActualWidth - SystemParameters.VerticalScrollBarWidth);
            if (_gapIndikator.Visibility != Visibility.Visible)
            {
                _gapIndikator.Visibility = Visibility.Visible;
                _gapIndikator.BeginAnimation(OpacityProperty,
                    new DoubleAnimation(1, TimeSpan.FromMilliseconds(120)));
            }
        }

        private static void AnimiereMargin(FrameworkElement el, Thickness ziel)
        {
            el.BeginAnimation(FrameworkElement.MarginProperty,
                new ThicknessAnimation(ziel, TimeSpan.FromMilliseconds(130))
                { EasingFunction = new CubicEase { EasingMode = EasingMode.EaseOut } });
        }

        private double GapTopY(int einfuege)
        {
            if (_gap.Count == 0)
                return KontenListe.TransformToAncestor(this).Transform(new Point(0, 0)).Y;
            if (einfuege < _gap.Count)
                return _gap[einfuege].Mitte - _zeilenHoehe / 2.0;
            return _gap[_gap.Count - 1].Mitte + _zeilenHoehe / 2.0;
        }

        // Zwei nach innen zeigende, leicht pendelnde Pfeile in der Luecke.
        private void ErzeugeGapIndikator()
        {
            var grid = new Grid
            {
                Height = _zeilenHoehe,
                IsHitTestVisible = false,
                Opacity = 0,
                Visibility = Visibility.Collapsed,
                Background = new SolidColorBrush(Color.FromArgb(40, 76, 175, 80))
            };
            var links = NeuerPfeil("▶", nachRechts: true);   // zeigt nach innen (rechts)
            links.HorizontalAlignment = HorizontalAlignment.Left;
            links.Margin = new Thickness(6, 0, 0, 0);
            var rechts = NeuerPfeil("◀", nachRechts: false); // zeigt nach innen (links)
            rechts.HorizontalAlignment = HorizontalAlignment.Right;
            rechts.Margin = new Thickness(0, 0, 6, 0);
            grid.Children.Add(links);
            grid.Children.Add(rechts);
            GhostLayer.Children.Add(grid);
            _gapIndikator = grid;
        }

        private static TextBlock NeuerPfeil(string glyph, bool nachRechts)
        {
            var tb = new TextBlock
            {
                Text = glyph,
                FontSize = 13,
                FontWeight = FontWeights.Bold,
                Foreground = new SolidColorBrush(Color.FromRgb(0x2E, 0x7D, 0x32)),
                VerticalAlignment = VerticalAlignment.Center
            };
            var tt = new TranslateTransform();
            tb.RenderTransform = tt;
            var anim = new DoubleAnimation(0, nachRechts ? 5 : -5, TimeSpan.FromMilliseconds(500))
            {
                AutoReverse = true,
                RepeatBehavior = RepeatBehavior.Forever,
                EasingFunction = new SineEase { EasingMode = EasingMode.EaseInOut }
            };
            tt.BeginAnimation(TranslateTransform.XProperty, anim);
            return tb;
        }

        // -----------------------------------------------------------------
        // Helfer
        // -----------------------------------------------------------------
        private bool UeberElement(FrameworkElement el, Point pImThis)
        {
            if (el == null || !el.IsVisible) return false;
            try
            {
                Rect r = el.TransformToAncestor(this)
                           .TransformBounds(new Rect(0, 0, el.ActualWidth, el.ActualHeight));
                return r.Contains(pImThis);
            }
            catch { return false; }
        }

        private Visual HitTestTopmost(Point p)
        {
            // WICHTIG: Die Callback-Variante von HitTest beachtet IsHitTestVisible
            // NICHT -- der schwebende Ghost (Kind von GhostLayer) liegt unter dem
            // Cursor und wuerde sonst jeden Treffer abfangen. Per Filter die
            // GhostLayer-Teilstruktur (Ghost, Gap-/Verbots-Indikator) ueberspringen.
            Visual treffer = null;
            VisualTreeHelper.HitTest(this,
                d => (d == GhostLayer)
                        ? HitTestFilterBehavior.ContinueSkipSelfAndChildren
                        : HitTestFilterBehavior.Continue,
                r => { treffer = r.VisualHit as Visual; return HitTestResultBehavior.Stop; },
                new PointHitTestParameters(p));
            return treffer;
        }

        private static T FindeDataContext<T>(object originalSource) where T : class
        {
            var dep = originalSource as DependencyObject;
            while (dep != null)
            {
                if (dep is FrameworkElement fe && fe.DataContext is T t) return t;
                dep = VisualTreeHelper.GetParent(dep);
            }
            return null;
        }

        private static T FindeAncestor<T>(DependencyObject d) where T : DependencyObject
        {
            while (d != null)
            {
                if (d is T t) return t;
                d = VisualTreeHelper.GetParent(d);
            }
            return null;
        }

        // -----------------------------------------------------------------
        // Scroll-Zentrierung: selektiertes Konto + zugewiesene Felder mittig
        // -----------------------------------------------------------------
        private void OnKontoSelektiert(object sender, SelectionChangedEventArgs e)
        {
            if (!IsInitialized) return;   // feuert sonst schon waehrend InitializeComponent
            if (_aktiv) return;   // waehrend Drag&Drop NICHT auto-zentrieren (sonst hakelig)
            var kontoVm = KontenListe.SelectedItem as EUKontoVM;
            if (kontoVm == null) return;
            _bandFeld = null; ZeichneBaender();   // Fokus wechselt zum Konto -> Bands weg
            ZentriereInListe(KontenListe, kontoVm);

            // Ziel-Feld je Formular DIREKT aus dem Konto-Modell bestimmen
            // (nicht ueber IstZugewiesen -- das wird vom SelektiertesKonto-Setter
            //  gesetzt und konkurriert sonst mit dem SelectionChanged-Event).
            foreach (var lb in FindeNachkommen<ListBox>(FormulareListe))
            {
                // Feld-Selektion (blau) aufheben -> Fokus liegt rein auf den
                // gruen hinterlegten, mit dem Konto verknuepften Feldern.
                lb.SelectedItem = null;

                var kat = lb.DataContext as FormularKategorieVM;
                if (kat == null) continue;
                string zielId;
                if (!kontoVm.Modell.Feldzuweisungen.TryGetValue(kat.Name, out zielId)
                    || string.IsNullOrEmpty(zielId))
                    continue;

                FormularfeldVM ziel = null;
                foreach (var it in lb.Items)
                    if (it is FormularfeldVM f && !f.IstKeine && f.Id == zielId) { ziel = f; break; }
                if (ziel != null) ZentriereInListe(lb, ziel);
            }
        }

        private void ZentriereInListe(ListBox lb, object item)
        {
            if (lb == null || item == null) return;
            lb.Dispatcher.BeginInvoke(new Action(() =>
            {
                lb.ScrollIntoView(item);   // Container sicherstellen
                if (!(lb.ItemContainerGenerator.ContainerFromItem(item) is ListBoxItem c)) return;
                var sv = FindeNachkommen<ScrollViewer>(lb).FirstOrDefault();
                if (sv == null) return;
                try
                {
                    double y = c.TransformToAncestor(sv).Transform(new Point(0, 0)).Y;
                    double ziel = sv.VerticalOffset + y - (sv.ViewportHeight - c.ActualHeight) / 2.0;
                    sv.ScrollToVerticalOffset(Math.Max(0, Math.Min(ziel, sv.ScrollableHeight)));
                }
                catch { }
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        // =================================================================
        // Rubber-Bands: vom zuletzt fokussierten Formularfeld geschwungene
        // Verbindungslinien zu ALLEN damit verknuepften Konten (ein Feld kann
        // mit mehreren Konten verbunden sein).
        // =================================================================
        private FormularfeldVM _bandFeld;
        private ListBox        _bandListe;
        private string         _bandKatName;

        private void OnFeldSelektiert(object sender, SelectionChangedEventArgs e)
        {
            if (!IsInitialized) return;   // feuert sonst schon waehrend InitializeComponent
            if (_aktiv) return;
            var lb   = sender as ListBox;
            var feld = lb?.SelectedItem as FormularfeldVM;
            var kat  = lb?.DataContext as FormularKategorieVM;
            if (lb == null || feld == null || kat == null || feld.IstKeine)
            {
                _bandFeld = null;
                ZeichneBaender();
                return;
            }
            _bandFeld    = feld;
            _bandListe   = lb;
            _bandKatName = kat.Name;
            // Konten-Liste so scrollen, dass moeglichst viele verknuepfte Konten
            // sichtbar sind, dann die Baender zeichnen (Positionen stehen erst
            // nach dem Layout).
            Dispatcher.BeginInvoke(new Action(() =>
            {
                ScrolleKontenZuVerknuepften(_bandKatName, _bandFeld);
                ZeichneBaender();
            }), System.Windows.Threading.DispatcherPriority.Background);
        }

        /// <summary>
        /// Scrollt die Konten-Liste so, dass moeglichst viele mit dem Feld
        /// verknuepfte Konten gleichzeitig sichtbar sind: das dichteste Fenster
        /// (Viewport-Hoehe) ueber den Konto-Positionen wird ermittelt und
        /// zentriert. Passen alle in den Viewport, werden sie mittig gezeigt.
        /// </summary>
        private void ScrolleKontenZuVerknuepften(string katName, FormularfeldVM feld)
        {
            if (feld == null || feld.IstKeine) return;
            var vm = VM;
            if (vm == null) return;
            var sv = FindeNachkommen<ScrollViewer>(KontenListe).FirstOrDefault();
            if (sv == null || sv.ScrollableHeight <= 0) return;

            var mitten = new List<double>();
            foreach (var kvm in vm.Konten)
            {
                if (kvm.Modell.IstEinnahme != feld.IstEinnahme) continue;
                if (!kvm.Modell.Feldzuweisungen.TryGetValue(katName, out var id) || id != feld.Id) continue;
                if (!(KontenListe.ItemContainerGenerator.ContainerFromItem(kvm) is ListBoxItem c)) continue;
                if (c.ActualHeight <= 0) continue;
                double y = c.TransformToAncestor(sv).Transform(new Point(0, 0)).Y + sv.VerticalOffset;
                mitten.Add(y + c.ActualHeight / 2.0);
            }
            if (mitten.Count == 0) return;
            mitten.Sort();

            double vp = sv.ViewportHeight;
            // Dichtestes Fenster der Groesse vp (gleitendes Fenster ueber sortierten Mitten).
            int bestAnzahl = 0;
            double bestOben = mitten[0], bestUnten = mitten[0];
            for (int i = 0; i < mitten.Count; i++)
            {
                int j = i;
                while (j < mitten.Count && mitten[j] - mitten[i] <= vp) j++;
                int anzahl = j - i;
                if (anzahl > bestAnzahl)
                {
                    bestAnzahl = anzahl;
                    bestOben = mitten[i];
                    bestUnten = mitten[j - 1];
                }
            }
            double ziel = (bestOben + bestUnten) / 2.0 - vp / 2.0;
            sv.ScrollToVerticalOffset(Math.Max(0, Math.Min(ziel, sv.ScrollableHeight)));
        }

        /// <summary>Nach einer Zuweisung die Rubber-Bands des betroffenen Feldes
        /// zu allen damit verknüpften Konten anzeigen. Wird nach WeiseFeldZu
        /// aufgerufen -- dessen Konto-Selektion hat die Bänder gerade geleert,
        /// daher hier neu setzen (deferred, nach Selektion/Zentrierung).</summary>
        private void ZeigeBaenderFuerFeld(FormularKategorieVM kat, FormularfeldVM feld)
        {
            if (kat == null || feld == null || feld.IstKeine) return;
            var lb = FindeNachkommen<ListBox>(FormulareListe)
                .FirstOrDefault(l => ReferenceEquals(l.DataContext, kat));
            if (lb == null) return;
            _bandFeld    = feld;
            _bandListe   = lb;
            _bandKatName = kat.Name;
            Dispatcher.BeginInvoke(new Action(ZeichneBaender),
                System.Windows.Threading.DispatcherPriority.Background);
        }

        private void ZeichneBaender()
        {
            // Waehrend InitializeComponent feuert die ListBox-Synchronisation
            // bereits SelectionChanged -- da existiert BandLayer noch nicht.
            if (BandLayer == null) return;
            BandLayer.Children.Clear();
            var vm = VM;
            if (_bandFeld == null || _bandListe == null || vm == null || _aktiv) return;

            // Endpunkt am Feld (linke Kante, vertikal zentriert -- Konten liegen links).
            if (!TryRechtImThis(_bandListe, _bandFeld, out Rect feldRect)) return;
            Rect feldVp = ViewportRect(_bandListe);
            double feldX = feldRect.Left;
            double feldY = Clamp(feldRect.Top + feldRect.Height / 2.0, feldVp.Top, feldVp.Bottom);

            Rect kontoVp = ViewportRect(KontenListe);

            foreach (var kvm in vm.Konten)
            {
                if (kvm.Modell.IstEinnahme != _bandFeld.IstEinnahme) continue;
                if (!kvm.Modell.Feldzuweisungen.TryGetValue(_bandKatName, out var id)
                    || id != _bandFeld.Id) continue;

                if (!TryRechtImThis(KontenListe, kvm, out Rect kRect)) continue;
                double kX = kRect.Right;
                double kY = Clamp(kRect.Top + kRect.Height / 2.0, kontoVp.Top, kontoVp.Bottom);

                BandLayer.Children.Add(ErzeugeBand(new Point(kX, kY), new Point(feldX, feldY)));
                BandLayer.Children.Add(ErzeugePunkt(new Point(kX, kY)));
            }
            // Feld-Endpunkt einmal markieren (falls mind. eine Verbindung gezeichnet).
            if (BandLayer.Children.Count > 0)
                BandLayer.Children.Add(ErzeugePunkt(new Point(feldX, feldY)));
        }

        private static readonly Color BandFarbe = Color.FromRgb(0x2E, 0x7D, 0x32);

        private static Path ErzeugeBand(Point von, Point bis)
        {
            double dx = Math.Max(30, Math.Abs(bis.X - von.X) * 0.4);
            var fig = new PathFigure { StartPoint = von };
            fig.Segments.Add(new BezierSegment(
                new Point(von.X + dx, von.Y), new Point(bis.X - dx, bis.Y), bis, true));
            var geo = new PathGeometry();
            geo.Figures.Add(fig);

            var p = new Path
            {
                Data = geo,
                Stroke = new SolidColorBrush(BandFarbe),
                StrokeThickness = 2,
                StrokeEndLineCap = PenLineCap.Round,
                StrokeStartLineCap = PenLineCap.Round,
                IsHitTestVisible = false,
                Effect = new DropShadowEffect { BlurRadius = 6, ShadowDepth = 1, Opacity = 0.30 }
            };
            p.BeginAnimation(OpacityProperty, new DoubleAnimation(0, 1, TimeSpan.FromMilliseconds(160)));
            return p;
        }

        private static Ellipse ErzeugePunkt(Point mitte)
        {
            var e = new Ellipse
            {
                Width = 7, Height = 7,
                Fill = new SolidColorBrush(BandFarbe),
                IsHitTestVisible = false
            };
            Canvas.SetLeft(e, mitte.X - 3.5);
            Canvas.SetTop(e, mitte.Y - 3.5);
            return e;
        }

        private static double Clamp(double v, double lo, double hi)
            => v < lo ? lo : (v > hi ? hi : v);

        /// <summary>Bounding-Rect des Item-Containers in Page-Koordinaten.</summary>
        private bool TryRechtImThis(ListBox lb, object item, out Rect rect)
        {
            rect = default(Rect);
            if (lb == null || item == null) return false;
            if (!(lb.ItemContainerGenerator.ContainerFromItem(item) is ListBoxItem c)) return false;
            if (c.ActualHeight <= 0) return false;
            try
            {
                rect = c.TransformToAncestor(this)
                        .TransformBounds(new Rect(0, 0, c.ActualWidth, c.ActualHeight));
                return true;
            }
            catch { return false; }
        }

        /// <summary>Sichtbarer Bereich (Viewport) der Liste in Page-Koordinaten.</summary>
        private Rect ViewportRect(ListBox lb)
        {
            var sv = FindeNachkommen<ScrollViewer>(lb).FirstOrDefault();
            if (sv == null) return new Rect(0, 0, ActualWidth, ActualHeight);
            try
            {
                return sv.TransformToAncestor(this)
                         .TransformBounds(new Rect(0, 0, sv.ViewportWidth, sv.ViewportHeight));
            }
            catch { return new Rect(0, 0, ActualWidth, ActualHeight); }
        }

        private static IEnumerable<T> FindeNachkommen<T>(DependencyObject root) where T : DependencyObject
        {
            if (root == null) yield break;
            int n = VisualTreeHelper.GetChildrenCount(root);
            for (int i = 0; i < n; i++)
            {
                var c = VisualTreeHelper.GetChild(root, i);
                if (c is T t) yield return t;
                foreach (var d in FindeNachkommen<T>(c)) yield return d;
            }
        }
    }
}
