# ============================================================================
# baue-programmicon.ps1 -- erzeugt res\EasyCash.ico (IDR_MAINFRAME)
# aus dem Master-PNG res\EasyCash_v4_512x512.png.
#
#     powershell -ExecutionPolicy Bypass -File RES\werkzeuge\baue-programmicon.ps1
# ============================================================================
param(
    [string]$Quelle = "EasyCash_v4_512x512.png",
    [string]$Ziel   = "easycash.ico"
)

. "$PSScriptRoot\bildwerkzeug.ps1"
$res = Split-Path -Parent $PSScriptRoot

$bild = Lies-Pixel (Join-Path $res $Quelle)
Write-Host ("Quelle: {0} ({1}x{2})" -f $Quelle, $bild.Breite, $bild.Hoehe)

# Kein Freistellen: das Programm-Icon darf ringsum Luft behalten, sonst
# wirkt es in der Taskleiste größer als die Nachbar-Icons.
Schreibe-Ico $bild (Join-Path $res $Ziel)
