// ErweiterungStore.cs — Typsicherer Key-Value-Store für Plugin-Erweiterungsdaten
//
// Ersetzt: CString Erweiterung (Pipe-Format) + GetErweiterungKey/SetErweiterungKey
// Originalformat: "DLLName|Key1=Val1|Key2=Val2||DLLName2|Key=Val||"
// Einschränkung: '|' darf nicht in Keys/Werten vorkommen, '=' nicht in Key-Namen
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
// Copyleft (GPLv3) 2024 Thomas Mielke

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ECTEngine
{
    /// <summary>
    /// Typsichere Alternative zu den C-Funktionen GetErweiterungKey/SetErweiterungKey.
    /// Liest und schreibt das Legacy-Pipe-Format für vollständige Rückwärtskompatibilität.
    /// </summary>
    public class ErweiterungStore
    {
        private readonly Dictionary<(string Namespace, string Key), string> _data
            = new Dictionary<(string, string), string>();

        // ──────────────────────────────────────────────
        // CRUD-Operationen
        // ──────────────────────────────────────────────

        /// <summary>Setzt einen Wert. Überschreibt, falls vorhanden.</summary>
        public void Setze(string ns, string key, string wert)
        {
            if (ns == null) throw new ArgumentNullException(nameof(ns));
            if (key == null) throw new ArgumentNullException(nameof(key));
            _data[(ns, key)] = wert ?? "";
        }

        /// <summary>Holt einen Wert. Gibt fallback zurück, wenn nicht vorhanden.</summary>
        public string Hole(string ns, string key, string fallback = "")
        {
            return _data.TryGetValue((ns, key), out var v) ? v : fallback;
        }

        /// <summary>Prüft, ob ein Schlüssel vorhanden ist.</summary>
        public bool Hat(string ns, string key) =>
            _data.ContainsKey((ns, key));

        /// <summary>Entfernt einen Schlüssel.</summary>
        public bool Entferne(string ns, string key) =>
            _data.Remove((ns, key));

        /// <summary>Gibt alle Einträge zurück.</summary>
        public IEnumerable<(string Ns, string Key, string Wert)> Alle() =>
            _data.Select(kv => (kv.Key.Namespace, kv.Key.Key, kv.Value));

        /// <summary>Gibt alle Namensräume zurück.</summary>
        public IEnumerable<string> Namespaces =>
            _data.Keys.Select(k => k.Namespace).Distinct();

        /// <summary>Anzahl der gespeicherten Einträge.</summary>
        public int Count => _data.Count;

        /// <summary>Entfernt alle Einträge.</summary>
        public void Clear() => _data.Clear();

        // ──────────────────────────────────────────────
        // Legacy-Pipe-Format
        // ──────────────────────────────────────────────

        /// <summary>
        /// Parst das Legacy-Pipe-Format aus CString Erweiterung.
        /// Format: "DLLName|Key1=Val1|Key2=Val2||DLLName2|Key=Val||"
        ///
        /// Trennzeichen: '|' zwischen Key=Value-Paaren
        ///               '||' zwischen Namensräumen (DLL-Namen)
        ///
        /// Hinweis: '|' darf nicht in Keys/Werten vorkommen,
        ///          '=' darf nicht in Key-Namen vorkommen.
        /// </summary>
        public static ErweiterungStore AusPipeFormat(string raw)
        {
            var store = new ErweiterungStore();
            if (string.IsNullOrEmpty(raw)) return store;

            // Aufteilen an "||" → Namensräume
            var namespaces = raw.Split(
                new[] { "||" }, StringSplitOptions.RemoveEmptyEntries);

            foreach (var nsBlock in namespaces)
            {
                var parts = nsBlock.Split('|');
                if (parts.Length < 1 || string.IsNullOrEmpty(parts[0]))
                    continue;

                string ns = parts[0]; // erstes Element = DLL-/Erweiterungsname

                for (int i = 1; i < parts.Length; i++)
                {
                    if (string.IsNullOrEmpty(parts[i])) continue;

                    int eq = parts[i].IndexOf('=');
                    if (eq > 0)
                    {
                        string key = parts[i].Substring(0, eq);
                        string val = parts[i].Substring(eq + 1);
                        store.Setze(ns, key, val);
                    }
                }
            }

            return store;
        }

        /// <summary>
        /// Serialisiert zurück ins Legacy-Pipe-Format.
        /// Garantiert Kompatibilität mit GetErweiterungKey/SetErweiterungKey.
        /// </summary>
        public string ZuPipeFormat()
        {
            if (_data.Count == 0) return "";

            var sb = new StringBuilder();
            var grouped = _data
                .GroupBy(kv => kv.Key.Namespace)
                .OrderBy(g => g.Key);

            foreach (var g in grouped)
            {
                sb.Append(g.Key);
                foreach (var kv in g)
                {
                    sb.Append('|')
                      .Append(kv.Key.Key)
                      .Append('=')
                      .Append(kv.Value);
                }
                sb.Append("||");
            }

            return sb.ToString();
        }

        /// <summary>Erstellt eine tiefe Kopie des Stores.</summary>
        public ErweiterungStore Clone()
        {
            var clone = new ErweiterungStore();
            foreach (var kv in _data)
                clone._data[kv.Key] = kv.Value;
            return clone;
        }
    }
}
