// DateiPromptVorbelegung.cs - belegt das Dateinamen-Feld des
// "Druckausgabe speichern unter"-Prompts vor (Microsoft Print to PDF
// und andere Datei-Drucker am Port "PORTPROMPT:").
//
// Hintergrund: Windows übernimmt den Druckjob-Namen NICHT als
// Dateinamens-Vorschlag in diesen Prompt (Feld bleibt leer; getestet
// auf Windows 11 26200 - der Jobname war dabei nachweislich korrekt
// gesetzt, sowohl über PrintDialog.PrintDocument als auch über
// PrintQueue.CurrentJobSettings.Description). Der Prompt läuft aber
// IM PROZESS der druckenden Anwendung (verifiziert für 32- und
// 64-bit): winspool zeigt ihn auf einem Callback-Thread an, während
// der druckende Thread in XpsDocumentWriter.Write blockiert. Ein
// Hintergrund-Thread desselben Prozesses kann das Feld darum per
// WM_SETTEXT vorbelegen.
//
// Erkennung sprachunabhängig: Toplevel-Dialog (Fensterklasse #32770)
// des EIGENEN Prozesses, der ein Edit-Control mit der Dialog-ID 1001
// enthält (das Dateiname-Feld der Explorer-Speichern-Dialoge).
// Vorbelegt wird nur ein LEERES Feld, und nur einmal pro Druckvorgang.
// Erscheint kein Prompt (normaler Drucker), läuft der Thread bis zum
// Dispose ins Leere und beendet sich.

using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace ECTViews.Druck
{
    internal sealed class DateiPromptVorbelegung : IDisposable
    {
        private readonly string _dateiname;
        private volatile bool _stop;

        /// <summary>
        /// Startet die Überwachung. Im using-Block um den blockierenden
        /// Druckaufruf legen; Dispose beendet die Überwachung.
        /// </summary>
        public static IDisposable Starte(string dateiname)
            => new DateiPromptVorbelegung(dateiname);

        private DateiPromptVorbelegung(string dateiname)
        {
            _dateiname = dateiname ?? "";
            var thread = new Thread(Ueberwache)
            {
                IsBackground = true,
                Name = "DateiPromptVorbelegung"
            };
            thread.Start();
        }

        public void Dispose()
        {
            _stop = true;   // Thread beendet sich selbst (kein Join nötig)
        }

        private void Ueberwache()
        {
            uint pid = GetCurrentProcessId();
            System.Diagnostics.Trace.WriteLine(
                "DateiPromptVorbelegung: Ueberwachung gestartet (pid " + pid + ")");
            while (!_stop)
            {
                IntPtr edit = FindeDateinamenFeld(pid);
                if (edit != IntPtr.Zero)
                {
                    var vorhanden = new StringBuilder(8);
                    SendMessageW(edit, WM_GETTEXT, (IntPtr)8, vorhanden);
                    if (vorhanden.Length == 0)
                    {
                        SendMessageW(edit, WM_SETTEXT, IntPtr.Zero, _dateiname);
                        // Der Dialog initialisiert sein Feld u.U. erst kurz
                        // nach dem Erscheinen und leert es dabei wieder -
                        // darum einmal nachpruefen und ggf. erneut setzen.
                        Thread.Sleep(500);
                        var kontrolle = new StringBuilder(8);
                        SendMessageW(edit, WM_GETTEXT, (IntPtr)8, kontrolle);
                        if (kontrolle.Length == 0 && !_stop)
                            SendMessageW(edit, WM_SETTEXT, IntPtr.Zero, _dateiname);
                    }
                    System.Diagnostics.Trace.WriteLine(
                        "DateiPromptVorbelegung: Prompt gefunden, Feld "
                        + (vorhanden.Length == 0 ? "vorbelegt" : "war schon belegt"));
                    return;   // nur einmal pro Druckvorgang
                }
                Thread.Sleep(150);
            }
            System.Diagnostics.Trace.WriteLine(
                "DateiPromptVorbelegung: beendet ohne Prompt");
        }

        /// <summary>
        /// Sucht in den Toplevel-Fenstern des eigenen Prozesses einen
        /// Dialog (#32770) mit Edit-Control der ID 1001 und liefert
        /// dessen HWND (oder IntPtr.Zero).
        /// </summary>
        private static IntPtr FindeDateinamenFeld(uint pid)
        {
            IntPtr gefunden = IntPtr.Zero;
            EnumWindows((hwnd, lParam) =>
            {
                GetWindowThreadProcessId(hwnd, out uint fensterPid);
                if (fensterPid != pid || Klasse(hwnd) != "#32770")
                    return true;

                IntPtr edit = IntPtr.Zero;
                EnumChildWindows(hwnd, (kind, l2) =>
                {
                    if (GetDlgCtrlID(kind) == 1001 && Klasse(kind) == "Edit")
                    {
                        edit = kind;
                        return false;
                    }
                    return true;
                }, IntPtr.Zero);

                if (edit != IntPtr.Zero)
                {
                    gefunden = edit;
                    return false;
                }
                return true;
            }, IntPtr.Zero);
            return gefunden;
        }

        private static string Klasse(IntPtr hwnd)
        {
            var sb = new StringBuilder(64);
            GetClassNameW(hwnd, sb, 64);
            return sb.ToString();
        }

        // ----------------------------------------------------------
        // Win32
        // ----------------------------------------------------------
        private const uint WM_SETTEXT = 0x000C;
        private const uint WM_GETTEXT = 0x000D;

        private delegate bool EnumProc(IntPtr hwnd, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern bool EnumWindows(EnumProc cb, IntPtr lParam);
        [DllImport("user32.dll")]
        private static extern bool EnumChildWindows(IntPtr parent, EnumProc cb, IntPtr lParam);
        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        private static extern int GetClassNameW(IntPtr hwnd, StringBuilder s, int n);
        [DllImport("user32.dll")]
        private static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint pid);
        [DllImport("user32.dll")]
        private static extern int GetDlgCtrlID(IntPtr hwnd);
        [DllImport("kernel32.dll")]
        private static extern uint GetCurrentProcessId();
        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        private static extern IntPtr SendMessageW(IntPtr hwnd, uint msg, IntPtr wParam, string text);
        [DllImport("user32.dll", CharSet = CharSet.Unicode)]
        private static extern IntPtr SendMessageW(IntPtr hwnd, uint msg, IntPtr wParam, StringBuilder text);
    }
}
