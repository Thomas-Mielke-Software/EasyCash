// IconSpriteSplitter.cs — Zerlegt ein horizontales Sprite-Bitmap in 32x32-Icons
//
// Das Original-MFC-Programm benutzt CImageList, die aus einer Bitmap mit
// horizontal aneinandergereihten 32x32-Icons gefüttert wird. Die WPF-
// Variante zerlegt das Sprite-Bitmap in einzelne CroppedBitmap-Objekte,
// die dann an Image-Controls in der ListView gebunden werden.

using System;
using System.Collections;
using System.IO;
using System.Reflection;
using System.Resources;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace ECTViews
{
    public static class IconSpriteSplitter
    {
        /// <summary>
        /// Schneidet ein Icon aus einer horizontalen Sprite-Bitmap.
        /// Die Icon-Kantenlänge wird automatisch aus der Bitmap-Höhe
        /// abgeleitet (also: bei einem horizontalen Strip ist jedes
        /// Icon so breit wie die Bitmap hoch ist).
        ///
        /// So funktioniert die Funktion sowohl für 16x16- als auch für
        /// 32x32- oder beliebige andere quadratische Icon-Größen,
        /// ohne dass der Aufrufer das wissen muss.
        /// </summary>
        public static ImageSource Crop(BitmapSource sprite, int index)
        {
            if (sprite == null) return null;
            if (index < 0) return null;

            int iconSize = sprite.PixelHeight;
            if (iconSize <= 0) return null;

            int x = index * iconSize;
            if (x + iconSize > sprite.PixelWidth)
                return null;

            var cropped = new CroppedBitmap(sprite,
                new Int32Rect(x, 0, iconSize, iconSize));
            if (cropped.CanFreeze) cropped.Freeze();
            return cropped;
        }

        /// <summary>
        /// Lädt eine Sprite-Bitmap aus einem Stream und friert sie ein
        /// (damit sie thread-übergreifend nutzbar ist).
        /// </summary>
        public static BitmapSource LoadFromStream(Stream stream)
        {
            if (stream == null) return null;

            var decoder = BitmapDecoder.Create(stream,
                BitmapCreateOptions.PreservePixelFormat,
                BitmapCacheOption.OnLoad);
            var frame = decoder.Frames.Count > 0 ? decoder.Frames[0] : null;
            if (frame != null && frame.CanFreeze) frame.Freeze();
            return frame;
        }

        /// <summary>
        /// Lädt eine Sprite-Bitmap aus den Embedded Resources der ECTViews-
        /// Assembly. Probiert nacheinander mehrere Strategien:
        ///
        ///   1) GetManifestResourceStream — funktioniert wenn die Datei
        ///      mit Build Action = "Embedded Resource" eingebunden ist.
        ///   2) WPF .g.resources — funktioniert wenn Build Action = "Resource"
        ///      (was der WPF-Standard ist). Liest direkt aus dem
        ///      g.resources-Container, ohne pack URI.
        ///   3) Pack URI über BitmapImage — der "offizielle" Weg; klappt
        ///      aber in hosted-Szenarien (WPF läuft in nativer EXE) oft
        ///      nicht zuverlässig, weil das pack-Schema nur teilweise
        ///      registriert ist.
        ///
        /// Bei Fehlschlag wird der Inhalt der Assembly im Debug-Output
        /// aufgelistet, damit man sehen kann, wie/ob die Bitmap eingebunden
        /// wurde.
        /// </summary>
        /// <param name="fileName">
        /// Dateiname der Bitmap (z.B. "icons.bmp"). Es wird angenommen,
        /// dass die Datei im Unterverzeichnis "Resources/" liegt.
        /// </param>
        public static BitmapSource LoadFromResource(string fileName)
        {
            var asm = typeof(IconSpriteSplitter).Assembly;
            var asmName = asm.GetName().Name;

            // ── Strategie 1: GetManifestResourceStream (Build = EmbeddedResource) ──
            var manifestKandidaten = new[]
            {
                $"{asmName}.Resources.{fileName}",
                $"{asmName}.{fileName}",
                fileName
            };
            foreach (var name in manifestKandidaten)
            {
                try
                {
                    using (var stream = asm.GetManifestResourceStream(name))
                    {
                        if (stream != null)
                        {
                            System.Diagnostics.Debug.WriteLine(
                                $"[IconSpriteSplitter] OK via GetManifestResourceStream('{name}')");
                            return LoadFromStream(stream);
                        }
                    }
                }
                catch (Exception ex)
                {
                    System.Diagnostics.Debug.WriteLine(
                        $"[IconSpriteSplitter] GetManifestResourceStream('{name}') Exception: {ex.GetType().Name}: {ex.Message}");
                }
            }

            // ── Strategie 2: WPF g.resources direkt lesen (Build = Resource) ──
            try
            {
                using (var grs = asm.GetManifestResourceStream($"{asmName}.g.resources"))
                {
                    if (grs != null)
                    {
                        using (var reader = new ResourceReader(grs))
                        {
                            string suchKey = $"resources/{fileName.ToLowerInvariant()}";
                            foreach (DictionaryEntry entry in reader)
                            {
                                string key = entry.Key as string ?? "";
                                if (string.Equals(key, suchKey,
                                        StringComparison.OrdinalIgnoreCase))
                                {
                                    System.Diagnostics.Debug.WriteLine(
                                        $"[IconSpriteSplitter] OK via g.resources['{key}']");
                                    if (entry.Value is Stream s)
                                        return LoadFromStream(s);
                                    if (entry.Value is byte[] bytes)
                                        return LoadFromStream(new MemoryStream(bytes));
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(
                    $"[IconSpriteSplitter] g.resources Exception: {ex.GetType().Name}: {ex.Message}");
            }

            // ── Strategie 3: Pack URI ──
            try
            {
                var uri = new Uri(
                    $"pack://application:,,,/{asmName};component/Resources/{fileName}",
                    UriKind.Absolute);
                var bmp = new BitmapImage();
                bmp.BeginInit();
                bmp.UriSource = uri;
                bmp.CacheOption = BitmapCacheOption.OnLoad;
                bmp.EndInit();
                if (bmp.CanFreeze) bmp.Freeze();
                System.Diagnostics.Debug.WriteLine(
                    $"[IconSpriteSplitter] OK via pack URI");
                return bmp;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(
                    $"[IconSpriteSplitter] Pack URI Exception: {ex.GetType().Name}: {ex.Message}");
            }

            // ── Alle Strategien gescheitert: Diagnose-Dump ──
            System.Diagnostics.Debug.WriteLine(
                $"[IconSpriteSplitter] '{fileName}' konnte NICHT geladen werden.");
            System.Diagnostics.Debug.WriteLine(
                $"[IconSpriteSplitter] Vorhandene Manifest-Resources in '{asmName}':");
            try
            {
                foreach (var n in asm.GetManifestResourceNames())
                {
                    System.Diagnostics.Debug.WriteLine($"  · {n}");

                    if (n.EndsWith(".g.resources", StringComparison.OrdinalIgnoreCase))
                    {
                        try
                        {
                            using (var s = asm.GetManifestResourceStream(n))
                            using (var rdr = new ResourceReader(s))
                            {
                                foreach (DictionaryEntry e in rdr)
                                    System.Diagnostics.Debug.WriteLine($"      [{e.Key}]");
                            }
                        }
                        catch (Exception ex)
                        {
                            System.Diagnostics.Debug.WriteLine($"      <Fehler: {ex.Message}>");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(
                    $"[IconSpriteSplitter] Diagnose-Dump fehlgeschlagen: {ex.Message}");
            }
            return null;
        }
    }
}
