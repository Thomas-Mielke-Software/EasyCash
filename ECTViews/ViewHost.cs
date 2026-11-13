// ViewHost.cs -- Statischer Einstiegspunkt zum Anzeigen von WPF-Dialogen
//
// Wird von der C++/CLI-Bridge (ECTBridge) aufgerufen. Stellt sicher,
// dass ein WPF Application-Objekt und ein Dispatcher existieren,
// bevor ein WPF-Fenster erzeugt wird.
//
// Aufruf aus C++/CLI:
//   ECTViews::ViewHost::ZeigeBuchungDialog(engine, true);

using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Interop;
using ECTEngine;
using ECTViews.ViewModels;
using ECTViews.Views;

namespace ECTViews
{
    /// <summary>
    /// Statischer Host für WPF-Dialoge. Kümmert sich um:
    /// - WPF Application-Initialisierung (falls noch keine existiert)
    /// - Fenster-Erzeugung mit korrektem Owner (MFC HWND)
    /// - Rückgabe der Ergebnis-Buchung
    /// - Befüllen der Bestandskonto-/Betrieb-Listen mit Icons
    /// </summary>
    public static class ViewHost
    {
        private static bool _wpfInitialized;

        // ----------------------------------------------
        // Sprite-Bitmaps für Betrieb/Bestandskonto-Icons
        // Werden lazily aus den Embedded Resources geladen.
        // ----------------------------------------------

        private static System.Windows.Media.Imaging.BitmapSource _spriteBetriebe;
        private static System.Windows.Media.Imaging.BitmapSource _spriteBestandskonten;

        /// <summary>
        /// Sprite-Bitmap für Betrieb-Icons. Standardmäßig aus
        /// "Resources/icons.bmp" der ECTViews-Assembly geladen.
        /// Kann vom Aufrufer überschrieben werden.
        /// </summary>
        public static System.Windows.Media.Imaging.BitmapSource SpriteBetriebe
        {
            get
            {
                if (_spriteBetriebe == null)
                    _spriteBetriebe = IconSpriteSplitter.LoadFromResource(
                        "icons.bmp");
                return _spriteBetriebe;
            }
            set => _spriteBetriebe = value;
        }

        /// <summary>
        /// Sprite-Bitmap für Bestandskonto-Icons. Standardmäßig aus
        /// "Resources/icons_bestandskonten.bmp".
        /// </summary>
        public static System.Windows.Media.Imaging.BitmapSource SpriteBestandskonten
        {
            get
            {
                if (_spriteBestandskonten == null)
                    _spriteBestandskonten = IconSpriteSplitter.LoadFromResource(
                        "icons_bestandskonten.bmp");
                return _spriteBestandskonten;
            }
            set => _spriteBestandskonten = value;
        }

        /// <summary>
        /// Vom Aufrufer übergebene Listen -- werden bei jedem Dialog-Aufruf
        /// in das ViewModel kopiert. Zentrale Stelle, damit der C++/CLI-
        /// Aufrufer diese nur einmal setzen muss.
        /// </summary>
        public static System.Collections.Generic.IList<string> BetriebeNamen { get; set; }
        public static System.Collections.Generic.IList<int>    BetriebeIcons { get; set; }
        public static System.Collections.Generic.IList<string> BestandskontenNamen { get; set; }
        public static System.Collections.Generic.IList<int>    BestandskontenIcons { get; set; }
        public static System.Collections.Generic.IList<int>    BestandskontenSalden { get; set; }

        private static void BefuelleListen(BuchungViewModel vm)
        {
            vm.LadeBetriebe(BetriebeNamen, BetriebeIcons, SpriteBetriebe);
            vm.LadeBestandskonten(BestandskontenNamen, BestandskontenIcons,
                SpriteBestandskonten);
        }

        /// <summary>
        /// True, wenn der WPF-Tastaturfokus gerade in einem Texteingabe-
        /// Element steht (TextBox/RichTextBox/PasswordBox oder das
        /// Editierfeld einer aufklappbaren ComboBox) oder in einer
        /// Dokument-Ansicht mit markierbarem Text (Anleitung, Seitenansicht).
        ///
        /// Aufgerufen aus <c>CMainFrame::PreTranslateMessage</c> über
        /// <c>ECT_WpfTextfeldHatFokus</c>: die Accelerator-Tabelle
        /// IDR_MAINFRAME belegt Strg+C/V/X/Z/A und die Einfg-/Entf-
        /// Varianten. TranslateAccelerator schnappt sie sich, bevor ein
        /// eingebettetes WPF-Textfeld sie sieht -- in den Einstellungen
        /// (und jeder anderen eingebetteten Ansicht) ließe sich dann weder
        /// kopieren noch einfügen. Der native Aufrufer lässt den
        /// Accelerator deshalb aus, wenn diese Abfrage true liefert.
        ///
        /// Steht der Fokus NICHT in einem Textfeld (z.B. in der
        /// Journal-Liste), kommt false zurück -- Strg+A bucht dort
        /// weiterhin eine Ausgabe.
        /// </summary>
        public static bool TextfeldHatFokus()
        {
            var fokus = System.Windows.Input.Keyboard.FocusedElement;
            if (fokus == null) return false;

            // TextBox, RichTextBox und das PART_EditableTextBox einer
            // aufklappbaren ComboBox sind alle TextBoxBase.
            if (fokus is System.Windows.Controls.Primitives.TextBoxBase) return true;
            if (fokus is System.Windows.Controls.PasswordBox) return true;

            // Sicherheitsnetz: manche ComboBox-Stile lassen den Fokus auf
            // der ComboBox selbst stehen.
            if (fokus is System.Windows.Controls.ComboBox cb && cb.IsEditable) return true;

            // Lese-Ansichten mit markierbarem Text (Anleitung = FlowDocumentReader,
            // Seitenansicht = DocumentViewer): Strg+C muss dort kopieren, und
            // Strg+A soll markieren statt eine Ausgabe zu buchen. Der Fokus
            // steht dabei auf einem inneren Teil des Viewers (oder einem
            // ContentElement des Dokuments), darum die Vorfahren absuchen.
            return LiegtInDokumentAnsicht(fokus as System.Windows.DependencyObject);
        }

        private static bool LiegtInDokumentAnsicht(System.Windows.DependencyObject d)
        {
            for (int i = 0; d != null && i < 64; i++)
            {
                if (d is System.Windows.Controls.FlowDocumentReader
                    || d is System.Windows.Controls.FlowDocumentScrollViewer
                    || d is System.Windows.Controls.Primitives.DocumentViewerBase)
                    return true;

                d = d is System.Windows.Media.Visual || d is System.Windows.Media.Media3D.Visual3D
                    ? System.Windows.Media.VisualTreeHelper.GetParent(d)
                    : System.Windows.LogicalTreeHelper.GetParent(d);
            }
            return false;
        }

        /// <summary>
        /// Stellt sicher, dass ein WPF Application-Objekt existiert.
        /// Muss vor dem ersten WPF-Fenster aufgerufen werden.
        /// In einer MFC-Hostanwendung gibt es kein WPF App.xaml --
        /// daher erzeugen wir die Application manuell.
        /// </summary>
        private static void EnsureWpfInitialized()
        {
            if (_wpfInitialized) return;

            if (Application.Current == null)
            {
                // Minimale WPF-Application erzeugen.
                new Application();
            }

            // ShutdownMode IMMER auf OnExplicitShutdown ziehen, auch wenn
            // jemand anderes (z.B. JournalEmbed.EnsureWpfApplication) die
            // Application bereits mit dem Default OnLastWindowClose erzeugt
            // hat. Sonst beendet WPF die App, sobald der erste Dialog
            // geschlossen wird, und der nächste ShowDialog crasht mit
            // "Das Anwendungsobjekt wird beendet".
            Application.Current.ShutdownMode = ShutdownMode.OnExplicitShutdown;

            _wpfInitialized = true;
        }

        /// <summary>
        /// Zeigt den Buchungseingabe-Dialog als modales Fenster.
        ///
        /// Aufruf aus C++/CLI (Exports.cpp oder EasyCashDocBridge.cpp):
        ///   ECTViews::ViewHost::ZeigeBuchungDialog(engine, true);
        ///
        /// Aufruf mit Owner-HWND (damit der Dialog vor dem MFC-Fenster bleibt):
        ///   ECTViews::ViewHost::ZeigeBuchungDialog(engine, true, hwnd);
        /// </summary>
        /// <param name="doc">Das BuchungsDocument aus der Engine.</param>
        /// <param name="ausgaben">True für Ausgaben, False für Einnahmen.</param>
        /// <param name="ownerHwnd">
        /// HWND des MFC-Elternfensters (optional). Wenn angegeben,
        /// wird der Dialog modal zu diesem Fenster.
        /// </param>
        /// <returns>
        /// Alle erzeugten Buchungen (bei einer Buchungsgruppen-Vorlage
        /// mehrere, Basis zuerst; sonst genau eine), oder null wenn
        /// abgebrochen.
        /// </returns>
        /// <param name="vorgewaehltesPreset">
        /// Slot (0-99) einer Buchungsvorlage, die beim Oeffnen automatisch
        /// geladen wird (Auswahl aus dem Ribbon-Dropdown). -1 = keine Vorwahl.
        /// </param>
        public static IReadOnlyList<Buchung> ZeigeBuchungDialog(
            BuchungsDocument doc, bool ausgaben, IntPtr ownerHwnd = default,
            Action<IReadOnlyList<Buchung>> onWeiterbuchen = null,
            int vorgewaehltesPreset = -1)
        {
            EnsureWpfInitialized();

            var vm = new BuchungViewModel(doc, ausgaben)
            {
                VorgewaehltesPreset = vorgewaehltesPreset
            };

            // "Weiterbuchen": jeder Klick persistiert die Buchung(en) ueber
            // diesen Callback (nativer Aufrufer), der Dialog bleibt offen.
            if (onWeiterbuchen != null)
                vm.GebuchtUndWeiter += onWeiterbuchen;

            BefuelleListen(vm);
            var view = new BuchungView(vm);

            // Owner-Fenster setzen (MFC HWND --> WPF WindowInteropHelper)
            if (ownerHwnd != IntPtr.Zero)
            {
                var helper = new WindowInteropHelper(view)
                {
                    Owner = ownerHwnd
                };
            }

            view.ShowDialog();

            return vm.Bestaetigt ? vm.ErgebnisBuchungen : null;
        }

        /// <summary>
        /// Zeigt den Dialog zur Bearbeitung einer bestehenden Buchung
        /// (ohne "Abgang buchen"-Button -- z.B. beim Kopieren).
        /// </summary>
        public static Buchung ZeigeBuchungBearbeitenDialog(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd = default)
        {
            var vm = ZeigeBearbeitenInternal(doc, buchung, ownerHwnd, abgangErlaubt: false);
            return vm.Bestaetigt ? vm.Ergebnis : null;
        }

        /// <summary>
        /// Wie ZeigeBuchungBearbeitenDialog, aber mit aktivem
        /// "Abgang buchen"-Button. Das Ergebnis sagt zusaetzlich, ob der
        /// Benutzer den AfA-Abgang ausgeloest hat -- dann ist Buchung == null
        /// und der native Aufrufer fuehrt den Abgang aus.
        /// </summary>
        public static BuchungBearbeitenErgebnis ZeigeBuchungBearbeitenDialogMitAbgang(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd = default)
        {
            var vm = ZeigeBearbeitenInternal(doc, buchung, ownerHwnd, abgangErlaubt: true);
            return new BuchungBearbeitenErgebnis
            {
                Buchung = vm.Bestaetigt ? vm.Ergebnis : null,
                AbgangGewuenscht = vm.AbgangGewuenscht
            };
        }

        /// <summary>
        /// Bearbeiten-Dialog mit Buchungsgruppen-Unterstützung:
        /// Ist die angeklickte Buchung Mitglied einer Gruppe MIT auffindbarer
        /// mehrzeiliger Vorlage, wird die BASIS-Buchung mit der kompletten
        /// Gruppe geöffnet (wie bei der Neuerfassung; manuelle Felder aus den
        /// bestehenden Zusatz-Buchungen vorbelegt, Gruppen-UUID bleibt
        /// erhalten). Andernfalls normale Einzel-Bearbeitung (mit
        /// "Abgang buchen"). Der Aufrufer ersetzt <see cref="BuchungBearbeitenKombiErgebnis.ErsetzteBasis"/>
        /// durch Buchungen[0], entfernt bei WarGruppenBearbeitung alle alten
        /// Gruppen-Mitglieder und fügt Buchungen[1..] neu ein.
        /// </summary>
        /// <param name="umwandelVorlagenSlot">Slot 0-99 einer Buchungsgruppen-
        /// Vorlage, in die die Buchung beim Öffnen umgewandelt werden soll
        /// (Journal-Kontextmenü "Umwandeln in"); -1 = normale Bearbeitung.
        /// Ist die Buchung schon Gruppen-Mitglied, wird die Gruppe unter
        /// derselben UUID auf die neue Vorlage umgestellt.</param>
        public static BuchungBearbeitenKombiErgebnis ZeigeBuchungBearbeitenKombiDialog(
            BuchungsDocument doc, Buchung angeklickt, IntPtr ownerHwnd = default,
            int umwandelVorlagenSlot = -1)
        {
            EnsureWpfInitialized();

            // Vorlagen-Kontext auflösen: Basis, Mitglieder, Vorlage.
            // Zwei Fälle öffnen den Dialog MIT Vorlage:
            //   - Mitglied einer Buchungsgruppe, deren (mehrzeilige) Vorlage
            //     noch im gemerkten Platz steht;
            //   - Einzelbuchung, die mit einer erweiterten Vorlage gebucht
            //     wurde (Zusatz-Felder, Feld-Regeln; auch eine Gruppe, deren
            //     Zusatz-Zeilen alle entfallen sind).
            // In beiden Fällen muss es DIESELBE Vorlage noch geben: die
            // Buchung merkt sich deren feste Kennung (VorlageId), gefunden
            // wird sie auch umbenannt oder umnummeriert
            // (Einstellungen.FindeVorlagenSlot). Gibt es sie nicht mehr,
            // passen die gemerkten Feldwerte nicht -- dann
            // Einzel-Bearbeitung mit Hinweis.
            Buchung basis = null;
            List<Buchung> zusatz = null;
            Preset vorlage = null;
            int slot = -1;
            Buchung basisKandidat = null;   // auch bei nicht auflösbarer Vorlage
            bool bezugVeraltet = false;
            string uuid = angeklickt.GruppenUuid;
            if (uuid != null)
            {
                var alle = doc.Buchungen.Where(b => b.GruppenUuid == uuid).ToList();
                basisKandidat = alle.FirstOrDefault(b => b.GruppenRolle == 0);
                slot = Einstellungen.FindeVorlagenSlot(basisKandidat);
                if (basisKandidat != null
                    && IstVorlageDerBuchung(basisKandidat, slot, mehrzeilig: true,
                                            out bezugVeraltet))
                {
                    basis = basisKandidat;
                    vorlage = Einstellungen.Presets[slot];
                    zusatz = alle.Where(b => !ReferenceEquals(b, basisKandidat))
                                 .OrderBy(b => b.GruppenRolle).ToList();
                }
            }
            else if (angeklickt.GruppenVorlage >= 0)
            {
                slot = Einstellungen.FindeVorlagenSlot(angeklickt);
                if (IstVorlageDerBuchung(angeklickt, slot, mehrzeilig: false,
                                         out bezugVeraltet))
                {
                    basis = angeklickt;
                    vorlage = Einstellungen.Presets[slot];
                }
            }

            // "Umwandeln in" eine andere Vorlage braucht die alte nicht --
            // dann stört auch ein veralteter Bezug nicht.
            if (umwandelVorlagenSlot >= 0) bezugVeraltet = false;

            if (vorlage == null)
            {
                // Einzel-Fallback (Alt-Split ohne Vorlage, gelöschte/
                // geänderte Vorlage, Basis nicht auffindbar) -- bzw. der
                // Normalfall beim Umwandeln einer gewöhnlichen Buchung:
                // dann trägt der Dialog die Vorlage beim Öffnen ein.
                //
                // Beim Umwandeln eines Gruppen-Mitglieds, dessen Vorlage
                // nicht (mehr) auffindbar ist, setzt die Umwandlung auf der
                // BASIS-Buchung der Gruppe auf: sonst hinge die neue Gruppe
                // an einer Neben-Buchung und die alte Basis fiele beim
                // Ersetzen der Mitglieder weg.
                var ziel = (umwandelVorlagenSlot >= 0 && basisKandidat != null)
                    ? basisKandidat : angeklickt;
                var vmEinzel = ZeigeBearbeitenInternal(
                    doc, ziel, ownerHwnd,
                    abgangErlaubt: umwandelVorlagenSlot < 0,
                    vorgewaehltesPreset: umwandelVorlagenSlot,
                    bezugVeraltet: bezugVeraltet);
                var buchungenEinzel = vmEinzel.Bestaetigt
                    ? vmEinzel.ErgebnisBuchungen : null;
                return new BuchungBearbeitenKombiErgebnis
                {
                    Buchungen = buchungenEinzel,
                    AbgangGewuenscht = vmEinzel.AbgangGewuenscht,
                    // Aus der Einzelbuchung kann per "Umwandeln in" eine
                    // Buchungsgruppe geworden sein -- dann soll der Aufrufer
                    // sie wie eine Gruppen-Bearbeitung behandeln (Meldung,
                    // Journal-Selektion des ganzen Blocks).
                    WarGruppenBearbeitung =
                        buchungenEinzel != null && buchungenEinzel.Count > 1,
                    ErsetzteBasis = ziel
                };
            }

            // Bearbeitung mit Vorlage: Dialog auf der Basis öffnen, Vorlage
            // samt den gemerkten Eingaben laden. "Abgang buchen" gibt es
            // hier nur für die Einzelbuchung (eine Gruppen-Basis hat ihn
            // noch nie angeboten).
            var vm = new BuchungViewModel(doc, basis)
            {
                AbgangErlaubt = uuid == null && umwandelVorlagenSlot < 0
            };
            vm.LadeVorlageFuerBearbeitung(vorlage, slot, zusatz, uuid);
            // "Umwandeln in": beim Öffnen auf die neue Vorlage umstellen
            // (die Gruppen-UUID bleibt dabei erhalten).
            vm.VorgewaehltesPreset = umwandelVorlagenSlot;
            BefuelleListen(vm);
            var view = new BuchungView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();

            var buchungen = vm.Bestaetigt ? vm.ErgebnisBuchungen : null;
            return new BuchungBearbeitenKombiErgebnis
            {
                Buchungen = buchungen,
                AbgangGewuenscht = vm.AbgangGewuenscht,
                // Eine Einzelbuchung zählt als Gruppen-Bearbeitung, sobald
                // aus ihr mehrere Buchungen geworden sind (ein bisher leeres
                // Zusatz-Feld hat jetzt eine Zeile erzeugt).
                WarGruppenBearbeitung = uuid != null
                    || (buchungen != null && buchungen.Count > 1),
                ErsetzteBasis = basis
            };
        }

        /// <summary>
        /// Taugt die Vorlage im Platz <paramref name="slot"/> (Ergebnis von
        /// <see cref="Einstellungen.FindeVorlagenSlot"/>) zum Bearbeiten von
        /// <paramref name="buchung"/>? Name und Nummer dürfen sich seit dem
        /// Buchen geändert haben -- das klärt die Kennung.
        /// </summary>
        /// <param name="mehrzeilig">True: für eine Buchungsgruppe muss es
        /// eine Vorlage mit Zusatz-Zeilen sein; false: für eine
        /// Einzelbuchung genügt eine erweiterte Vorlage.</param>
        /// <param name="veraltet">True, wenn die Buchung auf eine Vorlage
        /// verweist, die es so nicht (mehr) gibt.</param>
        private static bool IstVorlageDerBuchung(Buchung buchung, int slot,
            bool mehrzeilig, out bool veraltet)
        {
            veraltet = false;
            if (slot < 0)
            {
                // Kennung gemerkt, aber keine Vorlage trägt sie mehr
                veraltet = buchung != null && buchung.GruppenVorlage >= 0;
                return false;
            }

            bool passt = slot < Einstellungen.Presets.Count;
            if (passt)
            {
                var p = Einstellungen.Presets[slot];
                passt = !p.IstLeer && (mehrzeilig ? p.IstMehrzeilig : p.IstErweitert);
            }
            veraltet = !passt;
            return passt;
        }

        private static BuchungViewModel ZeigeBearbeitenInternal(
            BuchungsDocument doc, Buchung buchung, IntPtr ownerHwnd, bool abgangErlaubt,
            int vorgewaehltesPreset = -1, bool bezugVeraltet = false)
        {
            EnsureWpfInitialized();

            var vm = new BuchungViewModel(doc, buchung)
            {
                AbgangErlaubt = abgangErlaubt,
                VorgewaehltesPreset = vorgewaehltesPreset,
                VorlagenBezugVeraltet = bezugVeraltet
            };
            if (bezugVeraltet)
                vm.VorlagenHinweis =
                    "Diese Buchung wurde mit einer Buchungsvorlage erfasst, die es "
                    + "so nicht mehr gibt (gelöscht, umbenannt oder der Platz ist "
                    + "anders belegt). Sie wird deshalb als einzelne Buchung "
                    + "bearbeitet -- ohne die Zusatz-Felder der Vorlage.";
            BefuelleListen(vm);
            var view = new BuchungView(vm);

            if (ownerHwnd != IntPtr.Zero)
            {
                new WindowInteropHelper(view) { Owner = ownerHwnd };
            }

            view.ShowDialog();
            return vm;
        }

        /// <summary>
        /// Zeigt den "Buchungsjahr wählen"-Dialog beim Anlegen eines neuen
        /// Dokuments. Scannt das Datenverzeichnis nach bestehenden
        /// JahrXXXX.eca-Dateien (Jahreswechsel-Quellen) und liefert die vom
        /// Benutzer gewählte Aktion zurück.
        /// </summary>
        /// <param name="datenverzeichnis">Verzeichnis ohne abschließenden Backslash.</param>
        /// <param name="defaultJahr">Vorbelegung des Jahr-Feldes.</param>
        /// <param name="defaultWaehrung">Vorbelegung des Währungs-Feldes.</param>
        /// <param name="ownerHwnd">HWND des MFC-Elternfensters (optional).</param>
        public static BuchungsjahrWaehlenErgebnis ZeigeBuchungsjahrWaehlenDialog(
            string datenverzeichnis, int defaultJahr, string defaultWaehrung,
            IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new BuchungsjahrWaehlenViewModel(defaultJahr, defaultWaehrung);
            vm.SetzeDateien(SucheBuchungsdateien(datenverzeichnis));

            var view = new BuchungsjahrWaehlenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();

            return new BuchungsjahrWaehlenErgebnis
            {
                Aktion = vm.Aktion,
                Jahr = vm.Jahr,
                Waehrung = vm.Waehrung ?? string.Empty,
                QuelldateiPfad = vm.QuelldateiPfad ?? string.Empty
            };
        }

        /// <summary>
        /// Zeigt den Dauerbuchungs-Verwaltungsdialog (WPF-Ersatz für
        /// DauerbuchungenDlg; modal statt modeless). Arbeitet direkt auf
        /// doc.Dauerbuchungen -- der Aufrufer synchronisiert vorher
        /// Native->Managed und bei Rückgabe true danach Managed->Native.
        /// </summary>
        /// <param name="dateiname">Anzeigename fürs Fenster ("Dauerbuchungen
        /// für ...", wie das Original mit GetPathName).</param>
        /// <returns>True wenn mindestens eine Änderung übernommen wurde.</returns>
        public static bool ZeigeDauerbuchungenDialog(
            BuchungsDocument doc, string dateiname, IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Dauerbuchungen.DauerbuchungenViewModel(doc, dateiname);
            var view = new Dauerbuchungen.DauerbuchungenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            return vm.Geaendert;
        }

        /// <summary>
        /// Zeigt den kleinen "Dauerbuchungen ausführen bis Monat/Jahr"-Dialog
        /// (WPF-Ersatz für DauBuchAusfuehren). Die Ausführung selbst bleibt
        /// beim nativen Aufrufer (DauerbuchungenAusfuehren).
        /// </summary>
        /// <returns>True wenn "Ausführen" geklickt wurde; monat/jahr sind
        /// dann gültig (Jahr bereits gefensterlt/geklemmt).</returns>
        public static bool ZeigeDauerbuchungenAusfuehrenDialog(
            int buchungsjahr, IntPtr ownerHwnd, out int monat, out int jahr)
        {
            EnsureWpfInitialized();

            var view = new Dauerbuchungen.DauerbuchungenAusfuehrenView(buchungsjahr);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            monat = view.Monat;
            jahr = view.Jahr;
            return view.Bestaetigt;
        }

        /// <summary>
        /// Zeigt den USt-Vorauszahlungen-Dialog (WPF-Ersatz fuer
        /// CUstVorauszahlungenDlg). Schreibt bei OK in den Dokument-
        /// ErweiterungStore der Engine; der Bridge-Aufrufer synchronisiert
        /// zurueck und setzt das Modified-Flag.
        /// </summary>
        /// <returns>true wenn OK gedrueckt UND sich Werte geaendert haben.</returns>
        public static bool ZeigeUstVorauszahlungenDialog(
            ECTEngine.BuchungsDocument doc, IntPtr ownerHwnd)
        {
            EnsureWpfInitialized();

            var vm = new ViewModels.UstVorauszahlungenViewModel(doc);
            var view = new Views.UstVorauszahlungenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            if (!view.Bestaetigt) return false;
            return vm.Uebernehme();
        }

        /// <summary>
        /// Zeigt den Verwaltungs-/Auswahl-Dialog für Betriebe (WPF-Ersatz für
        /// CIconAuswahlBetrieb im Modus 1). Änderungen werden sofort über den
        /// Einstellungs-Cache in die ini geschrieben.
        /// </summary>
        /// <returns>Index des gewählten Betriebs ("Sel. anzeigen"), oder -1
        /// für "Alle anzeigen"/Abbruch.</returns>
        public static int ZeigeBetriebeVerwaltenDialog(IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Stammdaten.BetriebeVerwaltenViewModel();
            var view = new Stammdaten.StammdatenVerwaltenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            return view.GewaehlterIndex;
        }

        /// <summary>
        /// Zeigt den Verwaltungs-/Auswahl-Dialog für Bestandskonten (WPF-Ersatz
        /// für CIconAuswahlBestandskonto im Modus 1).
        /// </summary>
        /// <param name="buchungsjahr">Buchungsjahr des aktiven Dokuments --
        /// der Anfangssaldo-Dialog bearbeitet den Saldo des Vorjahres.</param>
        /// <returns>Index des gewählten Bestandskontos, oder -1.</returns>
        public static int ZeigeBestandskontenVerwaltenDialog(
            int buchungsjahr, IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Stammdaten.BestandskontenVerwaltenViewModel(buchungsjahr);
            var view = new Stammdaten.StammdatenVerwaltenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            return view.GewaehlterIndex;
        }

        /// <summary>
        /// Zeigt den Verwaltungs-/Auswahl-Dialog für Mandanten (WPF-Ersatz für
        /// CIconAuswahlMandant im Modus 1). Die Mandanten liegen im
        /// App-Profil (Registry) -- deshalb übergibt der Aufrufer die Liste
        /// und bekommt die (ggf. geänderte) Liste im Ergebnis zurück, um sie
        /// selbst zu persistieren. Das gilt auch bei Abbruch, damit
        /// Verwaltungs-Änderungen wie im MFC-Original erhalten bleiben.
        /// </summary>
        /// <param name="aktuellesDatenverzeichnis">Aktuelles Daten-/
        /// Mandantenverzeichnis -- Vorauswahl-Fallback für den
        /// Ordner-Picker (optional).</param>
        public static MandantenVerwaltenErgebnis ZeigeMandantenVerwaltenDialog(
            System.Collections.Generic.IList<string> namen,
            System.Collections.Generic.IList<int> icons,
            System.Collections.Generic.IList<string> datenverzeichnisse,
            string aktuellesDatenverzeichnis = null,
            IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Stammdaten.MandantenVerwaltenViewModel(
                namen, icons, datenverzeichnisse, aktuellesDatenverzeichnis);
            var view = new Stammdaten.StammdatenVerwaltenView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();

            var ergebnis = new MandantenVerwaltenErgebnis
            {
                GewaehlterIndex = view.GewaehlterIndex,
                NichtMandantenModusDatenverzeichnis =
                    vm.NichtMandantenModusDatenverzeichnis
            };
            foreach (var e in vm.Eintraege)
            {
                ergebnis.Namen.Add(e.Name);
                ergebnis.Icons.Add(e.IconIndex);
                ergebnis.Datenverzeichnisse.Add(vm.HoleProperty(e));
            }
            return ergebnis;
        }

        /// <summary>
        /// Zeigt den reinen Icon-Picker mit den Mandanten-Icons (= Betriebe-
        /// Sprite). Für den Erstanlauf in OnFileMandanten, wenn "Mandant 1"
        /// aus dem bisherigen Datenbestand angelegt wird.
        /// </summary>
        /// <returns>Icon-Index, oder -1 bei Abbruch.</returns>
        public static int ZeigeMandantIconAuswahlDialog(IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var vm = new Stammdaten.IconAuswahlViewModel(
                "Icon für Mandant wählen",
                SpriteBetriebe, Stammdaten.IconKatalog.Betriebe);
            var view = new Stammdaten.IconAuswahlView(vm);
            if (ownerHwnd != IntPtr.Zero)
                new WindowInteropHelper(view) { Owner = ownerHwnd };

            view.ShowDialog();
            return view.GewaehlterIndex;
        }

        /// <summary>
        /// Plugin-API-Fassade für HoleKontoMitFeldern (ECTBridge): löst eine
        /// Feld-Spezifikation ("$de:Formular=Id|...||") zum erstbesten
        /// verknüpften Konto auf; existiert keines, öffnet der Anlage-Dialog
        /// (KontoAnlegenView). Liefert den Kontonamen oder "" (abgebrochen,
        /// Spezifikations-Fehler oder alle 100 Slots belegt -- Fehler jeweils
        /// als MessageBox, wie die Plugin-API es verspricht).
        /// </summary>
        public static string HoleKontoMitFeldern(string spez, IntPtr ownerHwnd = default)
        {
            EnsureWpfInitialized();

            var a = ECTEngine.KontoFeldSelektor.LoeseAuf(spez);
            if (!a.IstSpezifikation || a.Fehler.Length > 0)
            {
                MessageBox.Show(
                    a.IstSpezifikation
                        ? a.Fehler
                        : "Ungültige Feld-Spezifikation: \"" + spez + "\" "
                          + "(erwartet z.B. \"$de:E/Ü-Rechnung=1103||\").",
                    "HoleKontoMitFeldern",
                    MessageBoxButton.OK, MessageBoxImage.Warning);
                return "";
            }
            if (a.Konto != null) return a.Konto;

            return Stammdaten.KontoAnlegenView.ZeigeDialog(
                a.Bedarf, owner: null, ownerHwnd: ownerHwnd,
                nameVorschlag: a.NameVorschlag) ?? "";
        }

        /// <summary>
        /// Listet *.eca-Dateien im Verzeichnis auf, aufsteigend sortiert
        /// (jüngste zuletzt -- entspricht dem alten LBS_SORT der MFC-Liste).
        /// </summary>
        private static System.Collections.Generic.IEnumerable<BuchungsdateiItem>
            SucheBuchungsdateien(string datenverzeichnis)
        {
            var liste = new System.Collections.Generic.List<BuchungsdateiItem>();
            if (string.IsNullOrEmpty(datenverzeichnis) ||
                !System.IO.Directory.Exists(datenverzeichnis))
                return liste;

            try
            {
                foreach (var pfad in System.IO.Directory.GetFiles(datenverzeichnis, "*.eca"))
                {
                    liste.Add(new BuchungsdateiItem
                    {
                        Name = System.IO.Path.GetFileName(pfad),
                        VollerPfad = pfad
                    });
                }
            }
            catch (System.IO.IOException)
            {
                // Verzeichnis nicht lesbar -- leere Liste, Dialog zeigt Hinweis.
            }

            liste.Sort((a, b) => string.Compare(
                a.Name, b.Name, StringComparison.OrdinalIgnoreCase));
            return liste;
        }
    }

    /// <summary>
    /// Ergebnis von ViewHost.ZeigeMandantenVerwaltenDialog: die komplette
    /// (ggf. geänderte) Mandanten-Liste plus Auswahl-Index. Der native
    /// Aufrufer schreibt die Liste ins App-Profil zurück und wechselt bei
    /// GewaehlterIndex >= 0 zum gewählten Mandanten.
    /// </summary>
    public sealed class MandantenVerwaltenErgebnis
    {
        /// <summary>Index in der Liste, oder -1 bei Abbrechen.</summary>
        public int GewaehlterIndex { get; set; } = -1;

        /// <summary>Nicht null/leer, wenn im Dialog der LETZTE Mandant gelöscht
        /// wurde: das Datenverzeichnis dieses Mandanten. Der native Aufrufer
        /// wechselt dann in den Nicht-Mandanten-Modus und übernimmt dieses
        /// Verzeichnis als Datenverzeichnis (Allgemein\Datenverzeichnis +
        /// SetIniFileName).</summary>
        public string NichtMandantenModusDatenverzeichnis { get; set; }

        public System.Collections.Generic.List<string> Namen { get; }
            = new System.Collections.Generic.List<string>();
        public System.Collections.Generic.List<int> Icons { get; }
            = new System.Collections.Generic.List<int>();
        public System.Collections.Generic.List<string> Datenverzeichnisse { get; }
            = new System.Collections.Generic.List<string>();
    }

    /// <summary>
    /// Ergebnis von ViewHost.ZeigeBuchungsjahrWaehlenDialog.
    /// </summary>
    public sealed class BuchungsjahrWaehlenErgebnis
    {
        public BuchungsjahrAktion Aktion { get; set; }
        public int Jahr { get; set; }
        public string Waehrung { get; set; }
        public string QuelldateiPfad { get; set; }
    }

    /// <summary>
    /// Ergebnis von ViewHost.ZeigeBuchungBearbeitenKombiDialog.
    /// </summary>
    public sealed class BuchungBearbeitenKombiErgebnis
    {
        /// <summary>Neu gebaute Buchungen (Basis zuerst); null wenn
        /// abgebrochen. Bei aufgelöster Gruppe ("Vorlage entfernen") genau
        /// eine Buchung ohne Gruppen-Keys.</summary>
        public IReadOnlyList<Buchung> Buchungen { get; set; }

        /// <summary>True wenn "Abgang buchen" geklickt wurde (nur im
        /// Einzel-Fallback möglich).</summary>
        public bool AbgangGewuenscht { get; set; }

        /// <summary>True wenn der Gruppen-Editor gezeigt wurde -- der
        /// Aufrufer muss dann die alten Gruppen-Mitglieder entfernen.</summary>
        public bool WarGruppenBearbeitung { get; set; }

        /// <summary>Die Buchung im Dokument, die durch Buchungen[0] ersetzt
        /// wird (bei Gruppen die Basis, sonst die angeklickte Buchung).</summary>
        public Buchung ErsetzteBasis { get; set; }
    }

    /// <summary>
    /// Ergebnis von ViewHost.ZeigeBuchungBearbeitenDialogMitAbgang.
    /// </summary>
    public sealed class BuchungBearbeitenErgebnis
    {
        /// <summary>Geaenderte Buchung, oder null (abgebrochen oder Abgang).</summary>
        public Buchung Buchung { get; set; }

        /// <summary>True wenn "Abgang buchen" geklickt wurde.</summary>
        public bool AbgangGewuenscht { get; set; }
    }
}
