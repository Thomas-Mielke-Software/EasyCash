# ============================================================================
# check-encoding.ps1 -- Encoding-Waechter fuer die EasyCash-Quellen (PowerShell)
#
# Verhindert die zwei Encoding-Unfaelle mit Windows-1252-Legacy-Dateien:
#   (1) U+FFFD-Korruption: Umlaute werden zu Bytes EF BF BD zerstoert.
#   (2) Encoding-Kippen: eine reine cp1252-Datei wird auf UTF-8 (ohne BOM)
#       umkodiert -> MBCS-MessageBoxen zeigen zur Laufzeit Mojibake.
# UTF-8 *mit* BOM (EF BB BF) ist erlaubt = bewusster Migrations-Zielzustand.
#
# Modi:
#   check-encoding.ps1 -Staged            Prueft die gestagten Blobs (pre-commit)
#   check-encoding.ps1 -Worktree          Prueft ALLE getrackten Quelldateien
#   check-encoding.ps1 -Files a.cpp b.h   Prueft Dateien der Arbeitskopie
#
# Exit 0 = sauber, 1 = Verstoss.
# ============================================================================
param(
  [switch]$Staged,
  [switch]$Worktree,
  [string[]]$Files
)

$textExt = '\.(cpp|cxx|cc|c|h|hpp|hxx|inl|rc|rc2|cs|xaml|def|idl|txt|md)$'

# Liest einen git-Blob in eine Temp-Datei; gibt den Pfad zurueck oder $null.
function Read-BlobToTemp([string]$rev, [string]$path) {
  $tmp = [System.IO.Path]::GetTempFileName()
  cmd /c "git show `"$($rev):$path`" > `"$tmp`" 2> nul" > $null
  if ($LASTEXITCODE -ne 0) {
    Remove-Item $tmp -Force -ErrorAction SilentlyContinue
    return $null
  }
  return $tmp
}

function Measure-Fffd([byte[]]$b) {
  $c = 0
  for ($i = 0; $i -le $b.Length - 3; $i++) {
    if ($b[$i] -eq 0xEF -and $b[$i+1] -eq 0xBF -and $b[$i+2] -eq 0xBD) { $c++ }
  }
  return $c
}
function Test-HighByte([byte[]]$b) {
  foreach ($x in $b) { if ($x -ge 0x80) { return $true } }
  return $false
}
function Test-Utf8Bom([byte[]]$b) {
  return ($b.Length -ge 3 -and $b[0] -eq 0xEF -and $b[1] -eq 0xBB -and $b[2] -eq 0xBF)
}
function Test-Utf8Multibyte([byte[]]$b) {
  for ($i = 0; $i -lt $b.Length; $i++) {
    $c = $b[$i]
    if ($c -ge 0xC2 -and $c -le 0xDF) {
      if ($i+1 -lt $b.Length -and $b[$i+1] -ge 0x80 -and $b[$i+1] -le 0xBF) { return $true }
    }
    elseif ($c -ge 0xE0 -and $c -le 0xEF) {
      if ($i+2 -lt $b.Length -and $b[$i+1] -ge 0x80 -and $b[$i+1] -le 0xBF -and $b[$i+2] -ge 0x80 -and $b[$i+2] -le 0xBF) {
        $istFffd = ($c -eq 0xEF -and $b[$i+1] -eq 0xBF -and $b[$i+2] -eq 0xBD)
        if (-not $istFffd) { return $true }
      }
    }
    elseif ($c -ge 0xF0 -and $c -le 0xF4) {
      if ($i+3 -lt $b.Length -and $b[$i+1] -ge 0x80 -and $b[$i+1] -le 0xBF -and $b[$i+2] -ge 0x80 -and $b[$i+2] -le 0xBF -and $b[$i+3] -ge 0x80 -and $b[$i+3] -le 0xBF) { return $true }
    }
  }
  return $false
}

# ---- Dateiliste bestimmen ----
$list = @()
if ($Staged) {
  $list = @(git diff --cached --name-only --diff-filter=ACM)
} elseif ($Worktree) {
  $list = @(git ls-files)
} elseif ($Files) {
  $list = $Files
}
$list = $list | Where-Object { $_ -and ($_ -match $textExt) -and ($_ -notmatch '^\.claude/worktrees/') }

$violations = 0
foreach ($f in $list) {
  $f = "$f".Trim()
  if (-not $f) { continue }

  # Gestagte bzw. Arbeitskopie-Bytes lesen
  [byte[]]$staged = $null
  if ($Staged) {
    $tmp = Read-BlobToTemp ':0' $f
    if ($null -ne $tmp) { $staged = [System.IO.File]::ReadAllBytes($tmp); Remove-Item $tmp -Force -ErrorAction SilentlyContinue }
  }
  elseif (Test-Path -LiteralPath $f) {
    $staged = [System.IO.File]::ReadAllBytes($f)
  }
  if ($null -eq $staged -or $staged.Length -eq 0) { continue }

  # HEAD-Bytes (Basis-Encoding) lesen -- kann fehlen (neue Datei)
  [byte[]]$head = $null
  $tmph = Read-BlobToTemp 'HEAD' $f
  if ($null -ne $tmph) { $head = [System.IO.File]::ReadAllBytes($tmph); Remove-Item $tmph -Force -ErrorAction SilentlyContinue }

  # (1) U+FFFD -- immer verboten.
  $nf = Measure-Fffd $staged
  if ($nf -gt 0) {
    Write-Host "ENCODING-FEHLER: $f enthaelt $nf x U+FFFD (zerstoerte Umlaute)."
    $violations++
  }

  # (2) cp1252 -> UTF-8 ohne BOM gekippt (BOM = erlaubte Migration).
  if ($null -ne $head) {
    $headWarCp1252 = (Test-HighByte $head) -and (-not (Test-Utf8Multibyte $head)) -and (-not (Test-Utf8Bom $head))
    $stagedIstUtf8 = (Test-Utf8Multibyte $staged) -and (-not (Test-Utf8Bom $staged))
    if ($headWarCp1252 -and $stagedIstUtf8) {
      Write-Host "ENCODING-FEHLER: $f war Windows-1252 und wurde auf UTF-8 (ohne BOM) gekippt -- MBCS-Strings wuerden zur Laufzeit als Mojibake erscheinen."
      $violations++
    }

    # (3) Gegenrichtung: UTF-8 -> Windows-1252 gekippt. Passiert, wenn ein
    #     Werkzeug die Datei als UTF-8 liest (BOM!) und mit cp1252 zurueck-
    #     schreibt -- z.B. [IO.File]::ReadAllText($p, $cp1252), das sein
    #     Encoding-Argument bei vorhandenem BOM stillschweigend ignoriert.
    #     Dabei entsteht KEIN U+FFFD (alle Umlaute sind in cp1252 gueltig),
    #     Regel (1) und (2) greifen also nicht: der Unfall ist unsichtbar,
    #     bis der Compiler die Datei ohne BOM in der Ausfuehrungs-Codepage liest.
    $headWarUtf8 = (Test-Utf8Bom $head) -or (Test-Utf8Multibyte $head)
    $stagedIstCp1252 = (Test-HighByte $staged) -and (-not (Test-Utf8Multibyte $staged)) -and (-not (Test-Utf8Bom $staged))
    if ($headWarUtf8 -and $stagedIstCp1252) {
      Write-Host "ENCODING-FEHLER: $f war UTF-8 und wurde auf Windows-1252 gekippt (BOM weg, Umlaute als Einzelbytes)."
      $violations++
    }
  }
}

if ($violations -gt 0) {
  Write-Host ""
  Write-Host "$violations Encoding-Verstoss/-Verstoesse. Commit abgebrochen."
  Write-Host "Das Encoding einer Legacy-.cpp/.h ist NICHT einheitlich (teils cp1252, teils"
  Write-Host "UTF-8 mit BOM): vor dem Bearbeiten an den Bytes bestimmen und beim Schreiben"
  Write-Host "exakt erhalten. Details: .githooks/check-encoding.ps1."
  Write-Host "Notfall-Bypass (mit Bedacht): git commit --no-verify"
  exit 1
}
exit 0
