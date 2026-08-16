# ============================================================================
# bildwerkzeug.ps1 -- gemeinsame Helfer für die Icon-/Bitmap-Erzeugung
#
# Wird von den baue-*.ps1 per Dot-Sourcing eingebunden:
#     . "$PSScriptRoot\bildwerkzeug.ps1"
#
# Kernstück ist die Verkleinerung: ein exakter Flächenfilter (area average)
# auf VORMULTIPLIZIERTEM Alpha. Das ist kein Selbstzweck --
#   * GDI+ und WPF interpolieren die RGB-Werte auch dort, wo Alpha 0 ist.
#     Da in den Quell-PNGs hinter den transparenten Pixeln Schwarz liegt,
#     entsteht sonst ein dunkler Saum um das Motiv.
#   * WPFs "HighQuality"-Skalierung liefert bei extremen Verkleinerungen
#     (512 -> 16) sichtbar ausgefranste Kanten.
# ============================================================================
Add-Type -AssemblyName System.Drawing

if (-not ("BildWerkzeug" -as [type])) {
Add-Type -TypeDefinition @'
using System;

public static class BildWerkzeug
{
    // Flaechen-Mittelung (area average) auf vormultipliziertem Alpha.
    public static byte[] Skaliere(byte[] quelle, int qb, int qh, int zb, int zh)
    {
        byte[] ziel = new byte[zb * zh * 4];
        double sx = (double)qb / zb, sy = (double)qh / zh;
        for (int zy = 0; zy < zh; zy++)
        {
            double y0 = zy * sy, y1 = (zy + 1) * sy;
            int iy0 = (int)Math.Floor(y0), iy1 = (int)Math.Ceiling(y1);
            for (int zx = 0; zx < zb; zx++)
            {
                double x0 = zx * sx, x1 = (zx + 1) * sx;
                int ix0 = (int)Math.Floor(x0), ix1 = (int)Math.Ceiling(x1);
                double sB = 0, sG = 0, sR = 0, sA = 0, sW = 0;
                for (int y = iy0; y < iy1 && y < qh; y++)
                {
                    double wy = Math.Min(y + 1, y1) - Math.Max(y, y0);
                    if (wy <= 0) continue;
                    for (int x = ix0; x < ix1 && x < qb; x++)
                    {
                        double wx = Math.Min(x + 1, x1) - Math.Max(x, x0);
                        if (wx <= 0) continue;
                        double w = wx * wy;
                        int o = (y * qb + x) * 4;
                        double a = quelle[o + 3] / 255.0;
                        sB += quelle[o]     * a * w;
                        sG += quelle[o + 1] * a * w;
                        sR += quelle[o + 2] * a * w;
                        sA += quelle[o + 3] * w;
                        sW += w;
                    }
                }
                int zo = (zy * zb + zx) * 4;
                if (sW <= 0) continue;
                double alpha = sA / sW;
                ziel[zo + 3] = Klemme(alpha);
                if (alpha > 0.0001)
                {
                    double f = sW * (alpha / 255.0);      // zurueck-dividieren
                    ziel[zo]     = Klemme(sB / f);
                    ziel[zo + 1] = Klemme(sG / f);
                    ziel[zo + 2] = Klemme(sR / f);
                }
            }
        }
        return ziel;
    }

    private static byte Klemme(double v)
    { return v < 0 ? (byte)0 : (v > 255 ? (byte)255 : (byte)Math.Round(v)); }
}
'@
}

# ----------------------------------------------------------------------------
# Bilder werden als Hashtable @{ Breite; Höhe; Pixel } durchgereicht.
# Pixel = BGRA, top-down, 4 Byte pro Pixel, gerades (nicht vormultipliziertes)
# Alpha -- also genau das Format von Format32bppArgb.
# ----------------------------------------------------------------------------

function Lies-Pixel([string]$pfad) {
    $bm = New-Object Drawing.Bitmap((Resolve-Path $pfad).Path)
    $r = New-Object Drawing.Rectangle(0, 0, $bm.Width, $bm.Height)
    $bd = $bm.LockBits($r, [Drawing.Imaging.ImageLockMode]::ReadOnly,
                       [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $pix = New-Object byte[] ($bd.Stride * $bm.Height)
    [Runtime.InteropServices.Marshal]::Copy($bd.Scan0, $pix, 0, $pix.Length)
    $bm.UnlockBits($bd)
    $erg = @{ Breite = $bm.Width; Hoehe = $bm.Height; Pixel = $pix }
    $bm.Dispose()
    return $erg
}

function Lies-Pixel-Aus-Bitmap($bitmap) {
    $r = New-Object Drawing.Rectangle(0, 0, $bitmap.Width, $bitmap.Height)
    $bd = $bitmap.LockBits($r, [Drawing.Imaging.ImageLockMode]::ReadOnly,
                           [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $pix = New-Object byte[] ($bd.Stride * $bitmap.Height)
    [Runtime.InteropServices.Marshal]::Copy($bd.Scan0, $pix, 0, $pix.Length)
    $bitmap.UnlockBits($bd)
    return @{ Breite = $bitmap.Width; Hoehe = $bitmap.Height; Pixel = $pix }
}

function Als-Bitmap($bild) {
    $bm = New-Object Drawing.Bitmap($bild.Breite, $bild.Hoehe,
                                    [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $r = New-Object Drawing.Rectangle(0, 0, $bild.Breite, $bild.Hoehe)
    $bd = $bm.LockBits($r, [Drawing.Imaging.ImageLockMode]::WriteOnly,
                       [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    [Runtime.InteropServices.Marshal]::Copy($bild.Pixel, 0, $bd.Scan0, $bild.Pixel.Length)
    $bm.UnlockBits($bd)
    return $bm
}

function Skaliere-Bild($bild, [int]$zb, [int]$zh) {
    return @{ Breite = $zb; Hoehe = $zh
              Pixel  = [BildWerkzeug]::Skaliere($bild.Pixel, $bild.Breite, $bild.Hoehe, $zb, $zh) }
}

function Beschneide-Bild($bild, [int]$x0, [int]$y0, [int]$b, [int]$h) {
    $neu = New-Object byte[] ($b * $h * 4)
    for ($y = 0; $y -lt $h; $y++) {
        [Array]::Copy($bild.Pixel, (($y + $y0) * $bild.Breite + $x0) * 4, $neu, $y * $b * 4, $b * 4)
    }
    return @{ Breite = $b; Hoehe = $h; Pixel = $neu }
}

function Stelle-Frei($bild, [int]$schwelle = 8) {
    # Motiv auf den sichtbaren Bereich zuschneiden. Die Quell-PNGs haben
    # ringsum Luft; bei 16-32 px Kachelgröße zählt jedes Pixel.
    $minX = $bild.Breite; $maxX = -1; $minY = $bild.Hoehe; $maxY = -1
    for ($y = 0; $y -lt $bild.Hoehe; $y++) {
        for ($x = 0; $x -lt $bild.Breite; $x++) {
            if ($bild.Pixel[($y * $bild.Breite + $x) * 4 + 3] -gt $schwelle) {
                if ($x -lt $minX) { $minX = $x }
                if ($x -gt $maxX) { $maxX = $x }
                if ($y -lt $minY) { $minY = $y }
                if ($y -gt $maxY) { $maxY = $y }
            }
        }
    }
    if ($maxX -lt 0) { return $bild }
    return Beschneide-Bild $bild $minX $minY ($maxX - $minX + 1) ($maxY - $minY + 1)
}

function Setze-In-Kachel($motiv, [int]$kante, [int]$rand = 0) {
    # Motiv seitenverhältnistreu zentriert in eine quadratische Kachel setzen.
    $nutz = $kante - 2 * $rand
    $f = [math]::Min($nutz / $motiv.Breite, $nutz / $motiv.Hoehe)
    $b = [math]::Max(1, [int][math]::Round($motiv.Breite * $f))
    $h = [math]::Max(1, [int][math]::Round($motiv.Hoehe  * $f))
    $klein = Skaliere-Bild $motiv $b $h
    $bm = New-Object Drawing.Bitmap($kante, $kante, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [Drawing.Graphics]::FromImage($bm)
    $g.Clear([Drawing.Color]::Transparent)
    $src = Als-Bitmap $klein
    $g.DrawImageUnscaled($src, [int](($kante - $b) / 2), [int](($kante - $h) / 2))
    $g.Dispose(); $src.Dispose()
    return $bm
}

function Matte-Auf-Weiss($bitmap) {
    # RGB der VOLL transparenten Pixel auf Weiß setzen, Alpha bleibt.
    # Die alte res\easycash.bmp macht das genauso: wertet ein Code-Pfad das
    # Alpha nicht aus, erscheint Weiß statt eines schwarzen Kastens.
    # Halbtransparente Randpixel behalten ihre echte Farbe, damit das
    # Alpha-Blending auf beliebigem Untergrund korrekt bleibt.
    for ($y = 0; $y -lt $bitmap.Height; $y++) {
        for ($x = 0; $x -lt $bitmap.Width; $x++) {
            if ($bitmap.GetPixel($x, $y).A -eq 0) {
                $bitmap.SetPixel($x, $y, [Drawing.Color]::FromArgb(0, 255, 255, 255))
            }
        }
    }
}

# ----------------------------------------------------------------------------
# Schreiber
# ----------------------------------------------------------------------------

function Schreibe-Bmp32($bitmap, [string]$ziel) {
    # 32bpp, BITMAPINFOHEADER (40 Byte), BI_RGB, bottom-up, gerades Alpha.
    # Exakt das Format der alten res\easycash.bmp -- CMFCToolBarImages
    # premultipliziert beim Laden selbst.
    $b = $bitmap.Width; $h = $bitmap.Height
    $r = New-Object Drawing.Rectangle(0, 0, $b, $h)
    $bd = $bitmap.LockBits($r, [Drawing.Imaging.ImageLockMode]::ReadOnly,
                           [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $pix = New-Object byte[] ($bd.Stride * $h)
    [Runtime.InteropServices.Marshal]::Copy($bd.Scan0, $pix, 0, $pix.Length)
    $bitmap.UnlockBits($bd)

    $stride = $b * 4
    $ms = New-Object IO.MemoryStream; $bw = New-Object IO.BinaryWriter($ms)
    $bw.Write([byte]0x42); $bw.Write([byte]0x4D)                    # "BM"
    $bw.Write([uint32](54 + $stride * $h)); $bw.Write([uint32]0); $bw.Write([uint32]54)
    $bw.Write([uint32]40); $bw.Write([int32]$b); $bw.Write([int32]$h)
    $bw.Write([uint16]1); $bw.Write([uint16]32); $bw.Write([uint32]0)
    $bw.Write([uint32]($stride * $h))
    $bw.Write([int32]0); $bw.Write([int32]0); $bw.Write([uint32]0); $bw.Write([uint32]0)
    for ($y = $h - 1; $y -ge 0; $y--) { $bw.Write($pix, $y * $stride, $stride) }
    $bw.Flush(); [IO.File]::WriteAllBytes((New-Object IO.FileInfo($ziel)).FullName, $ms.ToArray())
}

function Schreibe-Bmp24($bitmap, [string]$ziel) {
    # 24bpp opak, bottom-up, Zeilen auf 4 Byte gepadded.
    $b = $bitmap.Width; $h = $bitmap.Height
    $r = New-Object Drawing.Rectangle(0, 0, $b, $h)
    $bd = $bitmap.LockBits($r, [Drawing.Imaging.ImageLockMode]::ReadOnly,
                           [Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $pix = New-Object byte[] ($bd.Stride * $h)
    [Runtime.InteropServices.Marshal]::Copy($bd.Scan0, $pix, 0, $pix.Length)
    $quellStride = $bd.Stride
    $bitmap.UnlockBits($bd)

    $stride = [math]::Floor(($b * 3 + 3) / 4) * 4
    $ms = New-Object IO.MemoryStream; $bw = New-Object IO.BinaryWriter($ms)
    $bw.Write([byte]0x42); $bw.Write([byte]0x4D)
    $bw.Write([uint32](54 + $stride * $h)); $bw.Write([uint32]0); $bw.Write([uint32]54)
    $bw.Write([uint32]40); $bw.Write([int32]$b); $bw.Write([int32]$h)
    $bw.Write([uint16]1); $bw.Write([uint16]24); $bw.Write([uint32]0)
    $bw.Write([uint32]($stride * $h))
    $bw.Write([int32]0); $bw.Write([int32]0); $bw.Write([uint32]0); $bw.Write([uint32]0)
    $zeile = New-Object byte[] $stride
    for ($y = $h - 1; $y -ge 0; $y--) {
        [Array]::Clear($zeile, 0, $stride)
        [Array]::Copy($pix, $y * $quellStride, $zeile, 0, ($b * 3))
        $bw.Write($zeile, 0, $stride)
    }
    $bw.Flush(); [IO.File]::WriteAllBytes((New-Object IO.FileInfo($ziel)).FullName, $ms.ToArray())
}

function Schreibe-Ico($bild, [string]$ziel,
                      [int[]]$bmpKanten = @(16, 20, 24, 32, 40, 48, 64),
                      [int[]]$pngKanten = @(128, 256)) {
    # Kleine Größen als BMP (maximale Kompatibilität), 128/256 als PNG
    # (sonst bläht das ICO auf). rc.exe des v143-Toolsets verträgt beides.
    function BmpEintrag([byte[]]$pixel, [int]$kante) {
        $stride = $kante * 4
        $ms = New-Object IO.MemoryStream; $bw = New-Object IO.BinaryWriter($ms)
        $bw.Write([uint32]40); $bw.Write([int32]$kante); $bw.Write([int32]($kante * 2))
        $bw.Write([uint16]1); $bw.Write([uint16]32); $bw.Write([uint32]0)
        $bw.Write([uint32]($stride * $kante))
        $bw.Write([int32]0); $bw.Write([int32]0); $bw.Write([uint32]0); $bw.Write([uint32]0)
        for ($y = $kante - 1; $y -ge 0; $y--) { $bw.Write($pixel, $y * $stride, $stride) }
        # AND-Maske: bei 32bpp zählt eigentlich das Alpha, ältere Pfade
        # werten die Maske aber noch aus -- also korrekt mitschreiben.
        $maskStride = [math]::Floor(($kante + 31) / 32) * 4
        for ($y = $kante - 1; $y -ge 0; $y--) {
            $zeile = New-Object byte[] $maskStride
            for ($x = 0; $x -lt $kante; $x++) {
                if ($pixel[$y * $stride + $x * 4 + 3] -lt 128) {
                    $i = [math]::Floor($x / 8); $zeile[$i] = $zeile[$i] -bor (0x80 -shr ($x % 8))
                }
            }
            $bw.Write($zeile, 0, $maskStride)
        }
        $bw.Flush(); return $ms.ToArray()
    }
    function PngEintrag($skaliert) {
        $bm = Als-Bitmap $skaliert
        $ms = New-Object IO.MemoryStream
        $bm.Save($ms, [Drawing.Imaging.ImageFormat]::Png); $bm.Dispose()
        return $ms.ToArray()
    }

    $eintraege = @()
    foreach ($k in ($bmpKanten + $pngKanten)) {
        $s = Skaliere-Bild $bild $k $k
        if ($bmpKanten -contains $k) { $eintraege += ,@($k, (BmpEintrag $s.Pixel $k)) }
        else                         { $eintraege += ,@($k, (PngEintrag $s)) }
    }

    $out = New-Object IO.MemoryStream; $bw = New-Object IO.BinaryWriter($out)
    $bw.Write([uint16]0); $bw.Write([uint16]1); $bw.Write([uint16]$eintraege.Count)
    $offset = 6 + 16 * $eintraege.Count
    foreach ($e in $eintraege) {
        $mass = if ($e[0] -ge 256) { 0 } else { $e[0] }    # 0 bedeutet 256
        $bw.Write([byte]$mass); $bw.Write([byte]$mass); $bw.Write([byte]0); $bw.Write([byte]0)
        $bw.Write([uint16]1); $bw.Write([uint16]32)
        $bw.Write([uint32]$e[1].Length); $bw.Write([uint32]$offset)
        $offset += $e[1].Length
    }
    foreach ($e in $eintraege) { $bw.Write($e[1], 0, $e[1].Length) }
    $bw.Flush(); [IO.File]::WriteAllBytes((New-Object IO.FileInfo($ziel)).FullName, $out.ToArray())
    Write-Host ("  {0}: {1} Einträge, {2} Bytes" -f (Split-Path $ziel -Leaf), $eintraege.Count, $out.Length)
}
