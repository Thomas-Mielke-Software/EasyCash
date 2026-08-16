# ============================================================================
# baue-dokicon.ps1 -- erzeugt res\EasyCashDoc.ico (IDR_EASYCATYPE):
# weißes Blatt mit Eselsohr plus das Münz-Motiv des Programm-Icons.
#
#   -Variante 1   Münzen unten rechts, über die Blattkante hinaus
#   -Variante 2   Textzeilen oben, Münzen mittig im Blatt
#   -Variante 3   kleineres Blatt, großes Münz-Motiv (eingebaut, weil bei
#                 16 px am besten lesbar)
#   -Vorschau     baut alle drei und legt ein Vergleichsblatt ab, ohne
#                 res\EasyCashDoc.ico anzufassen
#
#     powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-dokicon.ps1
#     powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-dokicon.ps1 -Vorschau
#
# Gezeichnet wird auf einer 1024er Leinwand; verkleinert wird davon aus in
# einem Schritt per Flächenfilter (siehe bildwerkzeug.ps1).
# ============================================================================
param(
    [ValidateRange(1,3)][int]$Variante = 3,
    [switch]$Vorschau,
    [string]$VorschauDatei = (Join-Path $env:TEMP "dokicon-vorschau.png"),
    [string]$Quelle = "EasyCash_v4_512x512.png"
)

. "$PSScriptRoot\bildwerkzeug.ps1"
$res = Split-Path -Parent $PSScriptRoot

$motiv = Stelle-Frei (Lies-Pixel (Join-Path $res $Quelle))

$L = 1024                                                    # Leinwandkante
$RandFarbe   = [Drawing.Color]::FromArgb(255, 120, 128, 138) # Blattkontur
$FalzFarbe   = [Drawing.Color]::FromArgb(255, 228, 232, 237) # Eselsohr
$ZeilenFarbe = [Drawing.Color]::FromArgb(255, 198, 203, 210) # angedeuteter Text

function Neue-Leinwand {
    $bm = New-Object Drawing.Bitmap($L, $L, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [Drawing.Graphics]::FromImage($bm)
    $g.SmoothingMode = [Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.Clear([Drawing.Color]::Transparent)
    return @{ Bild = $bm; G = $g }
}

function Zeichne-Blatt($g, [int]$x0, [int]$y0, [int]$x1, [int]$y1, [int]$falz, [int]$strich) {
    # Die Kontur ist bewusst kräftig (120,128,138): ein zartgrauer Rand
    # verschwindet bei 16 px auf weißem Explorer-Hintergrund komplett.
    $pfad = New-Object Drawing.Drawing2D.GraphicsPath
    $pfad.AddPolygon(@(
        (New-Object Drawing.PointF($x0, $y0)),
        (New-Object Drawing.PointF(($x1 - $falz), $y0)),
        (New-Object Drawing.PointF($x1, ($y0 + $falz))),
        (New-Object Drawing.PointF($x1, $y1)),
        (New-Object Drawing.PointF($x0, $y1))
    ))
    $g.FillPath([Drawing.Brushes]::White, $pfad)
    $stift = New-Object Drawing.Pen($RandFarbe, $strich)
    $stift.LineJoin = [Drawing.Drawing2D.LineJoin]::Round
    $g.DrawPath($stift, $pfad)

    $ohr = New-Object Drawing.Drawing2D.GraphicsPath
    $ohr.AddPolygon(@(
        (New-Object Drawing.PointF(($x1 - $falz), $y0)),
        (New-Object Drawing.PointF($x1, ($y0 + $falz))),
        (New-Object Drawing.PointF(($x1 - $falz), ($y0 + $falz)))
    ))
    $g.FillPath((New-Object Drawing.SolidBrush($FalzFarbe)), $ohr)
    $g.DrawPath($stift, $ohr)
    $stift.Dispose(); $pfad.Dispose(); $ohr.Dispose()
}

function Zeichne-Zeilen($g, [int]$x0, [int]$y0, [int]$breite, [int]$anzahl, [int]$abstand, [int]$dicke) {
    $stift = New-Object Drawing.Pen($ZeilenFarbe, $dicke)
    $stift.StartCap = [Drawing.Drawing2D.LineCap]::Round
    $stift.EndCap   = [Drawing.Drawing2D.LineCap]::Round
    for ($i = 0; $i -lt $anzahl; $i++) {
        $b = if ($i -eq $anzahl - 1) { [int]($breite * 0.6) } else { $breite }
        $y = $y0 + $i * $abstand
        $g.DrawLine($stift, $x0, $y, ($x0 + $b), $y)
    }
    $stift.Dispose()
}

function Setze-Motiv($g, [int]$hoehe, [int]$mitteX, [int]$unten) {
    $b = [int][math]::Round($motiv.Breite * $hoehe / $motiv.Hoehe)
    $bm = Als-Bitmap (Skaliere-Bild $motiv $b $hoehe)
    $g.DrawImageUnscaled($bm, [int]($mitteX - $b / 2), [int]($unten - $hoehe))
    $bm.Dispose()
}

function Baue-Variante([int]$n) {
    $c = Neue-Leinwand
    switch ($n) {
        1 { Zeichne-Blatt $c.G 150 80 780 900 190 13
            Setze-Motiv $c.G 470 690 985 }
        2 { Zeichne-Blatt $c.G 190 60 850 964 190 13
            Zeichne-Zeilen $c.G 270 200 500 4 82 26
            Setze-Motiv $c.G 400 520 900 }
        3 { Zeichne-Blatt $c.G 210 40 810 800 170 13
            Setze-Motiv $c.G 560 610 1000 }
    }
    $c.G.Dispose()
    $bild = Lies-Pixel-Aus-Bitmap $c.Bild
    $c.Bild.Dispose()
    return $bild
}

if ($Vorschau) {
    # Vergleichsblatt: Master groß, daneben 16/24/32/48 in 1:1 und 3-fach
    $tmp = Join-Path $env:TEMP "dokicon-bau"
    New-Item -ItemType Directory -Force $tmp | Out-Null
    foreach ($n in 1..3) { Schreibe-Ico (Baue-Variante $n) (Join-Path $tmp "v$n.ico") }

    $kanten = @(16, 24, 32, 48)
    $blatt = New-Object Drawing.Bitmap(780, 720)
    $g = [Drawing.Graphics]::FromImage($blatt)
    $g.Clear([Drawing.Color]::White)
    $g.PixelOffsetMode = [Drawing.Drawing2D.PixelOffsetMode]::Half
    $g.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $sf = New-Object Drawing.Font("Segoe UI", 11, [Drawing.FontStyle]::Bold)
    $sk = New-Object Drawing.Font("Segoe UI", 8)
    foreach ($n in 1..3) {
        $y = 10 + ($n - 1) * 235
        $g.DrawString("Variante $n", $sf, [Drawing.Brushes]::Black, 10, $y)
        $x = 20
        foreach ($k in $kanten) {
            $ico = New-Object Drawing.Icon((Join-Path $tmp "v$n.ico"), $k, $k)
            $bm = $ico.ToBitmap()
            $g.DrawImage($bm, $x, ($y + 32), $k, $k)
            $g.DrawImage($bm, $x, ($y + 85), ($k * 3), ($k * 3))
            $g.DrawString("$k", $sk, [Drawing.Brushes]::Gray, $x, ($y + 56))
            $x += $k * 3 + 22
            $bm.Dispose(); $ico.Dispose()
        }
    }
    $g.Dispose()
    $blatt.Save($VorschauDatei, [Drawing.Imaging.ImageFormat]::Png)
    $blatt.Dispose()
    Write-Host "Vorschau: $VorschauDatei (nichts in res\ geändert)"
    return
}

$bild = Baue-Variante $Variante
Schreibe-Ico $bild (Join-Path $res "easycashdoc.ico")

# Master-PNG daneben ablegen, damit spätere Änderungen nicht wieder
# rekonstruiert werden müssen. Achtung: .gitignore schluckt RES\EasyCash*x*.png,
# fürs Einchecken braucht es ein "git add -f".
$master = Als-Bitmap (Skaliere-Bild $bild 512 512)
$master.Save((Join-Path $res "EasyCashDoc_v4_512x512.png"), [Drawing.Imaging.ImageFormat]::Png)
$master.Dispose()
Write-Host "  EasyCashDoc_v4_512x512.png: Master der Variante $Variante"
