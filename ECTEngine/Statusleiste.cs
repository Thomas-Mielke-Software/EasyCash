// Statusleiste.cs -- Managed Pendant zu CMainFrame::SetStatus.
//
// Kurze Hinweise ("Konto X wurde neu sortiert", "... mit Feld Z verknüpft")
// aus dem managed Code an die native MFC-Statusleiste melden. Die Bridge
// abonniert das Ereignis und reicht den Text an den vom Host (EasyCash.exe)
// registrierten nativen Callback weiter, der CMainFrame::SetStatus aufruft.
//
// Ohne registrierten Host (Designer, Unit-Tests) verpufft die Meldung
// folgenlos -- die UI bleibt also entkoppelt.

using System;

namespace ECTEngine
{
    public static class Statusleiste
    {
        /// <summary>Wird bei jeder nicht-leeren Meldung ausgelöst.</summary>
        public static event Action<string> Gemeldet;

        /// <summary>Meldet einen kurzen Hinweis an die Statusleiste.</summary>
        public static void Melde(string text)
        {
            if (string.IsNullOrEmpty(text)) return;
            Gemeldet?.Invoke(text);
        }
    }
}
