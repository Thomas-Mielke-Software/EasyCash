// EinstellungenLiveSync.cs -- Reaktive Verteilung von Cache-Änderungen.
//
// Verteilt jede Änderung am globalen Einstellungs-Cache (ECTEngine.Einstellungen)
// an ALLE offenen Einstellungs-ViewModels -- auch über Dokumentfenster hinweg.
// So sieht ein zweites geöffnetes Einstellungs-Panel die Änderung des ersten
// sofort, ohne Neuladen.
//
// Bewusst mit SCHWACHEN Referenzen auf die ViewModels (kein IDisposable nötig):
// geschlossene Fenster werden beim nächsten Durchlauf automatisch ausgesondert.
// Der Koordinator selbst lebt prozessweit und hängt sich GENAU EINMAL an die
// (statischen) WertGeaendert/SektionGeaendert-Events des Caches.

using System;
using System.Collections.Generic;
using System.Windows.Threading;
using ECTEngine;
using ECTViews.ViewModels;

namespace ECTViews.EinstellungenUi
{
    /// <summary>
    /// Wird von Einstellungs-Seiten implementiert, die auf eine externe
    /// Cache-Änderung mit mehr reagieren müssen als nur "alle Bindings neu
    /// lesen" -- typisch listenbasierte Seiten (Konten, Presets), die ihre
    /// materialisierte Liste neu aufbauen und das gegen laufende Interaktion
    /// (Drag&amp;Drop, Editieren) schützen wollen. Wird auf dem UI-Thread gerufen.
    /// </summary>
    public interface IEinstellungenLiveZiel
    {
        void AufExterneEinstellungsaenderung();
    }

    public static class EinstellungenLiveSync
    {
        private static readonly object _gate = new object();
        private static readonly List<WeakReference<object>> _abonnenten
            = new List<WeakReference<object>>();
        private static bool _verdrahtet;
        private static Dispatcher _dispatcher;

        // Urheber der gerade laufenden Cache-Änderung (UI-Thread). Wird per
        // AlsUrheber() für die Dauer einer eigenen Schreib-Operation gesetzt,
        // damit sich das auslösende Fenster die Änderung nicht zurückspiegelt
        // (sonst Selbst-Rebuild, der z.B. frisch gezeichnete Rubber-Bands wieder
        // löscht).
        [ThreadStatic] private static object _urheber;

        /// <summary>Markiert für die Dauer des zurückgegebenen Scopes den Urheber
        /// von Cache-Änderungen. Ein als Urheber registriertes Live-Ziel wird in
        /// diesem Scope NICHT benachrichtigt.</summary>
        public static IDisposable AlsUrheber(object urheber)
        {
            var vorher = _urheber;
            _urheber = urheber;
            return new Ruecksetzer(() => _urheber = vorher);
        }

        private sealed class Ruecksetzer : IDisposable
        {
            private readonly Action _aktion;
            private bool _erledigt;
            public Ruecksetzer(Action aktion) { _aktion = aktion; }
            public void Dispose() { if (!_erledigt) { _erledigt = true; _aktion(); } }
        }

        /// <summary>Meldet ein Ziel für Live-Updates an: entweder ein
        /// <see cref="ViewModelBase"/> (dann wird bei Änderung
        /// <see cref="ViewModelBase.RaiseAllPropertiesChanged"/> gerufen) oder
        /// ein <see cref="IEinstellungenLiveZiel"/> (dann dessen
        /// <see cref="IEinstellungenLiveZiel.AufExterneEinstellungsaenderung"/>).
        /// Wird typischerweise im Konstruktor der Seite/des ViewModels gerufen.
        /// Mehrfach-Registrierung ist unschädlich.</summary>
        public static void Registriere(object ziel)
        {
            if (ziel == null) return;
            lock (_gate)
            {
                if (!_verdrahtet)
                {
                    // Dispatcher des UI-Threads merken (die Seiten werden auf dem
                    // UI-Thread erzeugt). Einmalige Verdrahtung an den Cache.
                    _dispatcher = Dispatcher.CurrentDispatcher;
                    Einstellungen.WertGeaendert    += (k, v) => Benachrichtige();
                    Einstellungen.SektionGeaendert += (s, d) => Benachrichtige();
                    _verdrahtet = true;
                }
                _abonnenten.Add(new WeakReference<object>(ziel));
            }
        }

        private static void Benachrichtige()
        {
            // Urheber JETZT (synchron) festhalten -- beim späteren Lauf auf dem
            // UI-Thread ist der Scope längst verlassen.
            var urheber = _urheber;
            // Immer asynchron auf den UI-Thread posten: entkoppelt vom Setter,
            // der die Änderung ausgelöst hat (keine Re-Entrancy während einer
            // laufenden Binding-Aktualisierung), und marshalt notfalls aus einem
            // Hintergrund-Thread.
            var disp = _dispatcher;
            if (disp == null) return;
            disp.BeginInvoke((Action)(() => BenachrichtigeAufUiThread(urheber)));
        }

        private static void BenachrichtigeAufUiThread(object urheber)
        {
            lock (_gate)
            {
                for (int i = _abonnenten.Count - 1; i >= 0; i--)
                {
                    if (_abonnenten[i].TryGetTarget(out var ziel))
                    {
                        // Eigene Änderung nicht ans auslösende Fenster zurückspiegeln.
                        if (ReferenceEquals(ziel, urheber)) continue;
                        // Eigener Reload (mit Interaktions-Schutz) hat Vorrang;
                        // sonst der generische "alle Bindings neu lesen"-Pfad.
                        if (ziel is IEinstellungenLiveZiel live)
                            live.AufExterneEinstellungsaenderung();
                        else if (ziel is ViewModelBase vm)
                            vm.RaiseAllPropertiesChanged();
                    }
                    else
                        _abonnenten.RemoveAt(i);   // geschlossenes Fenster aussondern
                }
            }
        }
    }
}
