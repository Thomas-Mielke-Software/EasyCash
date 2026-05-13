<#
.SYNOPSIS
    Erhöht die SemVer-Versionsnummer in Directory.Build.props und aktualisiert
    abhängige Dateien (Inno-Setup-Script, Registrierungs-Textdateien).

.PARAMETER Part
    Welcher Versionsteil erhöht werden soll: major, minor oder patch.

.PARAMETER Suffix
    Neuer VersionSuffix (z.B. "alpha", "rc.1", "beta").
    - Nicht angegeben  -> bestehender Suffix bleibt erhalten
    - Leerstring ""    -> Suffix wird entfernt (Release)

.PARAMETER DryRun
    Zeigt nur an, was geändert würde, ohne Dateien zu schreiben.

.EXAMPLE
    .\bump-version.ps1 -Part minor
    .\bump-version.ps1 -Part patch -Suffix rc.1
    .\bump-version.ps1 -Part major -Suffix ""
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [ValidateSet('major','minor','patch')]
    [string]$Part,

    [string]$Suffix,

    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$repoRoot     = $PSScriptRoot
$propsPath    = Join-Path $repoRoot 'Directory.Build.props'
$issPath      = Join-Path $repoRoot 'easycash.iss'
$docsDir      = Join-Path $env:USERPROFILE 'Documents\EasyCash'
$regFiles     = @(
    Join-Path $docsDir 'TestReg.txt'
    Join-Path $docsDir 'VollReg.txt'
)

# --- 1. Aktuelle Version lesen --------------------------------------------------
if (-not (Test-Path $propsPath)) { throw "Nicht gefunden: $propsPath" }
$propsText = Get-Content $propsPath -Raw -Encoding UTF8

if ($propsText -notmatch '<VersionPrefix>(\d+)\.(\d+)\.(\d+)</VersionPrefix>') {
    throw "VersionPrefix in Directory.Build.props nicht gefunden oder ungültig."
}
$curMajor = [int]$Matches[1]
$curMinor = [int]$Matches[2]
$curPatch = [int]$Matches[3]

if ($propsText -match '<VersionSuffix>([^<]*)</VersionSuffix>') {
    $curSuffix = $Matches[1]
} else {
    $curSuffix = ''
}

# --- 2. Neue Version berechnen --------------------------------------------------
switch ($Part) {
    'major' { $newMajor = $curMajor + 1; $newMinor = 0;             $newPatch = 0 }
    'minor' { $newMajor = $curMajor;     $newMinor = $curMinor + 1; $newPatch = 0 }
    'patch' { $newMajor = $curMajor;     $newMinor = $curMinor;     $newPatch = $curPatch + 1 }
}
$newVersion    = "$newMajor.$newMinor.$newPatch"
$newMajorMinor = "$newMajor.$newMinor"

# Suffix-Logik: nicht angegeben = behalten; sonst übernehmen (auch "" = leeren)
if ($PSBoundParameters.ContainsKey('Suffix')) {
    $newSuffix = $Suffix
} else {
    $newSuffix = $curSuffix
}

$oldDisplay = "$curMajor.$curMinor.$curPatch" + $(if ($curSuffix) { "-$curSuffix" })
$newDisplay = "$newVersion"                   + $(if ($newSuffix) { "-$newSuffix" })

Write-Host ""
Write-Host "Version: $oldDisplay  ->  $newDisplay" -ForegroundColor Cyan
Write-Host ""

# --- 3. Helper: Datei in Windows-1252 lesen/schreiben ---------------------------
$enc1252 = [System.Text.Encoding]::GetEncoding(1252)

function Update-File1252 {
    param(
        [string]$Path,
        [string]$Pattern,
        [string]$Replacement,
        [switch]$Required
    )
    if (-not (Test-Path $Path)) {
        if ($Required) { throw "Nicht gefunden: $Path" }
        Write-Host "  uebersprungen (fehlt): $Path" -ForegroundColor Yellow
        return
    }
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    $text  = $enc1252.GetString($bytes)

    if ($text -notmatch $Pattern) {
        Write-Host "  WARN: Pattern nicht gefunden in $Path" -ForegroundColor Yellow
        return
    }
    $newText = [regex]::Replace($text, $Pattern, $Replacement)
    if ($newText -eq $text) {
        Write-Host "  unverändert: $Path"
        return
    }
    Write-Host "  -> $Path" -ForegroundColor Green
    if (-not $DryRun) {
        [System.IO.File]::WriteAllBytes($Path, $enc1252.GetBytes($newText))
    }
}

# --- 4. Directory.Build.props aktualisieren (UTF-8) -----------------------------
Write-Host "Directory.Build.props:"
$newPropsText = $propsText `
    -replace '<VersionPrefix>\d+\.\d+\.\d+</VersionPrefix>', "<VersionPrefix>$newVersion</VersionPrefix>" `
    -replace '<VersionSuffix>[^<]*</VersionSuffix>',         "<VersionSuffix>$newSuffix</VersionSuffix>"

Write-Host "  -> $propsPath" -ForegroundColor Green
if (-not $DryRun) {
    # UTF-8 ohne BOM, um vorhandene Encoding-Konvention beizubehalten
    [System.IO.File]::WriteAllText($propsPath, $newPropsText, (New-Object System.Text.UTF8Encoding $false))
}

# --- 5. easycash.iss aktualisieren (Win-1252) -----------------------------------
Write-Host "easycash.iss:"
$issVersion = $newVersion
if ($newSuffix) { $issVersion = "$newVersion ($newSuffix)" }
Update-File1252 -Path $issPath `
    -Pattern '(?m)^(AppVerName=EasyCash&Tax )[^\r\n]+' `
    -Replacement "`${1}$issVersion" `
    -Required

# --- 6. TestReg.txt / VollReg.txt aktualisieren (Win-1252, nur Major.Minor) -----
#Write-Host "Registrierungs-Dateien:"
#foreach ($f in $regFiles) {
#    Update-File1252 -Path $f `
#        -Pattern 'Die aktuelle Version ist \d+\.\d+\.' `
#        -Replacement "Die aktuelle Version ist $newMajorMinor."
#}
# XXXXXX nach 4er Preview-Phase wieder aktivieren !!! XXXXXX

# --- 7. EasyCash.rc aktualisieren (Win-1252, Major,Minor,Patch,GitCommitCount) --
Write-Host "EasyCash.rc:"
$rcPath = Join-Path $repoRoot 'EasyCash.rc'
$gitCount = (& git -C $repoRoot rev-list --count HEAD 2>$null)
if ($LASTEXITCODE -ne 0 -or -not $gitCount) {
    Write-Host "  WARN: git rev-list fehlgeschlagen, Revision=0" -ForegroundColor Yellow
    $gitCount = '0'
}
$gitCount = $gitCount.Trim()
Write-Host "  Revision (Git-Commit-Count): $gitCount"
$rcComma   = "$newMajor,$newMinor,$newPatch,$gitCount"
$rcDotted  = "$newMajor.$newMinor.$newPatch.$gitCount"
Update-File1252 -Path $rcPath `
    -Pattern '(?m)^(\s*FILEVERSION\s+)\d+,\d+,\d+,\d+' `
    -Replacement "`${1}$rcComma" `
    -Required
Update-File1252 -Path $rcPath `
    -Pattern '(?m)^(\s*PRODUCTVERSION\s+)\d+,\d+,\d+,\d+' `
    -Replacement "`${1}$rcComma" `
    -Required
Update-File1252 -Path $rcPath `
    -Pattern '(VALUE\s+"FileVersion",\s*")\d+\.\d+\.\d+\.\d+(")' `
    -Replacement "`${1}$rcDotted`${2}" `
    -Required
Update-File1252 -Path $rcPath `
    -Pattern '(VALUE\s+"ProductVersion",\s*")\d+\.\d+\.\d+\.\d+(")' `
    -Replacement "`${1}$rcDotted`${2}" `
    -Required

Write-Host ""
if ($DryRun) {
    Write-Host "DryRun: keine Dateien geschrieben, kein Build, kein ISCC." -ForegroundColor Yellow
    return
} else {
    Write-Host "Fertig." -ForegroundColor Green
}

# --- 8. Git-Commit mit neuer Version  -------------------------------------------
& git -C $repoRoot commit -a -m "$newVersion Release" 2>$null

# --- 9. EasyCash-Hauptprojekt bauen (Release/Win32) -----------------------------
Write-Host ""
Write-Host "Baue EasyCash (Release/Win32)..." -ForegroundColor Cyan
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe nicht gefunden: $vswhere"
}
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' |
    Select-Object -First 1
if (-not $msbuild -or -not (Test-Path $msbuild)) {
    throw "MSBuild.exe nicht über vswhere gefunden."
}
$slnPath = Join-Path $repoRoot 'easycashVS2022.sln'
& $msbuild $slnPath /t:EasyCash /p:Configuration=Release /p:Platform=Win32 /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) {
    throw "Build fehlgeschlagen (ExitCode $LASTEXITCODE)."
}

# --- 10. Inno-Setup kompilieren -------------------------------------------------
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "easycash.iss"

# --- 11. Filezilla öffnen -------------------------------------------------------
$CurrentDir = Get-Location
$RelativePath = "Setup"
$AbsolutePath = Join-Path -Path $CurrentDir -ChildPath $RelativePath
& "C:\Program Files\FileZilla FTP Client\filezilla" "--local=$AbsolutePath"

# --- 12. im Setup-Verzeichnis mit Versionsnummer archivieren --------------------
$FileName = "ECTSetup"
$FileNamePreview = "ECTSetup4"
$Extension = ".exe"
$OldName = "$FileNamePreview$Extension"
# XXXXXX ^ Version 4 nur in der Preview-Phase benutzen !!! XXXXXX
$NewName = "$FileName$newVersion$Extension"
Write-Host "Umbenennen von $OldName in $NewName"
Copy-Item -Path ".\Setup\$OldName" -Destination ".\Setup\$NewName"