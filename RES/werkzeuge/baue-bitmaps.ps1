# ============================================================================
# baue-bitmaps.ps1 -- erzeugt die beiden Logo-Bitmaps aus dem Master-PNG:
#
#   res\easycash.bmp       IDB_EASYCASH      26x26, 32bpp + Alpha
#                          Ribbon-Anwendungsknopf (mainfrm.cpp, Add_MainPanel)
#   res\EasyCash32x32.bmp  IDB_EASYCASH_32   32x32, 24bpp opak auf Weiß
#                          Platzhalter im Plugin-Manager (PluginManager.cpp)
#
# Warum die zwei Formate unterschiedlich sind:
#   * Der Ribbon-Knopf läuft über CMFCToolBarImages -- das wertet bei 32bpp
#     den Alphakanal aus und premultipliziert selbst. Also gerades Alpha
#     schreiben und die voll transparenten Pixel auf Weiß matten (siehe
#     Matte-Auf-Weiss in bildwerkzeug.ps1).
#   * Der Plugin-Platzhalter geht in eine CImageList mit ILC_COLOR24 und
#     RGB(255,0,255) als Maskenfarbe bei weißer Hintergrundfarbe. Magenta
#     als Bildhintergrund würde nur rosa Ränder erzeugen -- deshalb opak
#     auf Weiß, das deckt sich mit der Listenfarbe.
#
#     powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-bitmaps.ps1
# ============================================================================
param(
    [string]$Quelle = "EasyCash_v4_512x512.png"
)

. "$PSScriptRoot\bildwerkzeug.ps1"
$res = Split-Path -Parent $PSScriptRoot

$bild  = Lies-Pixel (Join-Path $res $Quelle)
$motiv = Stelle-Frei $bild      # Kacheln sind klein -- Luft wegschneiden
Write-Host ("Quelle: {0} ({1}x{2}), freigestellt {3}x{4}" -f `
            $Quelle, $bild.Breite, $bild.Hoehe, $motiv.Breite, $motiv.Hoehe)

# ---- IDB_EASYCASH: 26x26 mit Alphakanal ------------------------------------
$knopf = Setze-In-Kachel $motiv 26
Matte-Auf-Weiss $knopf
Schreibe-Bmp32 $knopf (Join-Path $res "easycash.bmp")
Write-Host "  easycash.bmp: 26x26, 32bpp, gerades Alpha"

# ---- IDB_EASYCASH_32: 32x32 opak auf Weiß ----------------------------------
# 1 px Luft ringsum, damit die Kachel in der Listview nicht an den Text stößt
$mitAlpha = Setze-In-Kachel $motiv 32 1
$flach = New-Object Drawing.Bitmap(32, 32, [Drawing.Imaging.PixelFormat]::Format24bppRgb)
$g = [Drawing.Graphics]::FromImage($flach)
$g.Clear([Drawing.Color]::White)
$g.DrawImageUnscaled($mitAlpha, 0, 0)
$g.Dispose()
Schreibe-Bmp24 $flach (Join-Path $res "EasyCash32x32.bmp")
Write-Host "  EasyCash32x32.bmp: 32x32, 24bpp auf Weiß"

$knopf.Dispose(); $mitAlpha.Dispose(); $flach.Dispose()
