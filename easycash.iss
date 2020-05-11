; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=EasyCash&Tax
AppVerName=EasyCash&Tax 2.36 mit Korrektur f�r AfA-Dauer
DiskSpanning=no
AppPublisher=tm
AppPublisherURL=http://www.easyct.de
AppSupportURL=http://www.easyct.de
AppUpdatesURL=http://www.easyct.de
DefaultDirName={pf}\EasyCash&Tax
UsePreviousAppDir=yes
DefaultGroupName=EasyCash
OutputBaseFilename=ECTSetup
OutputDir=.\EasyCash 
MinVersion=5.0
LicenseFile=.\LIZENZ.txt
SignTool=winsdk81sha1   ; dual sign the 
SignTool=winsdk81sha256 ; installer
;Menu -> Tools -> Configure Sign Tool...
;winsdk81sha1: "C:\Program Files (x86)\Windows Kits\8.1\bin\x86\signtool.exe" sign /p mysecretpw /f C:\My\Path\To\cert.p12 /t http://timestamp.comodoca.com $f
;winsdk81sha256:  "C:\Program Files (x86)\Windows Kits\8.1\bin\x86\signtool.exe" sign /p mysecretpw /f C:\My\Path\To\cert.p12 /tr http://timestamp.comodoca.com /fd sha256 /td sha256 /as $f

[Languages]
Name: de; MessagesFile: compiler:Languages\German.isl

[Types]
Name: BRDFormular; Description: EasyCash&Tax mit deutschen Formularen
Name: ATFormular; Description: EasyCash&Tax mit �sterreichischem Formular

[Components]
Name: BRDFormular; Types: BRDFormular; Description: deutsche Formulare
Name: ATFormular; Types: ATFormular; Description: �sterreichisches Formular

[Tasks]
Name: desktopicon; Description: Create a &desktop icon; GroupDescription: Additional icons:; MinVersion: 4,4

[Files]
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcm90.dll"; DestDir: {app};
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcp90.dll"; DestDir: {app};
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcr90.dll"; DestDir: {app};
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.ATL\atl90.dll"; DestDir: {app};
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\mfc90.dll"; DestDir: {app};
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFCLOC\MFC90DEU.dll"; DestDir: {app};
;Source: "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.OPENMP\vcomp90.dll"; DestDir: {app};      
Source: D:\Work\EasyCash\Systemdateien\vcredist_x86.exe; DestDir: {tmp}; Flags: dontcopy;   
;-> [Run] !!

Source: .\Release\EasyCT.exe; DestDir: {app}; Flags: ignoreversion
Source: .\Release\ECTIFace.dll; DestDir: {app}; Flags: ignoreversion  
Source: .\Release\EasyCTXP.dll; DestDir: {app}; Flags: ignoreversion
Source: .\Release\EasyCTX.ocx; DestDir: {app}; Flags: regserver restartreplace  
Source: .\tipps.txt; DestDir: {app}; Flags: ignoreversion             
Source: .\Release\ECTUpdater.exe; DestDir: {app}; Flags: ignoreversion
Source: D:\Work\CrashRpt\bin\CrashSender1402.exe; DestDir: {app}; Flags: ignoreversion
Source: D:\Work\CrashRpt\bin\crashrpt_lang.ini; DestDir: {app}; Flags: ignoreversion
Source: D:\Work\CrashRpt\bin\dbghelp.dll; DestDir: {app}; Flags: ignoreversion 
Source: D:\Work\EasyCash\Doc\PDF-Anleitungen\ECT-Doku_Michael.pdf; DestDir: {app}; Flags: ignoreversion
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019-1.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019-2.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019-3.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019-4.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019-5.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2019\ust-d-2019-6.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020-1.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020-2.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020-3.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020-4.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020-5.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\D-2020\ust-d-2020-6.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-1.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-2.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-01.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-02.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-03.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-04.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-05.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-06.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-07.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-08.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-09.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-10.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-11.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-12.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-q1.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-q2.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-q3.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UVA\D-2020\uva-d-2020-q4.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\EUeR\2019\EUeR-d-2019.ecf; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\EUeR\2019\EUeR-d-2019-1.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\EUeR\2019\EUeR-d-2019-2.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\EUeR\2019\EUeR-d-2019-3.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular  
Source: D:\Work\EasyCash\EUeR\2019\EUeR-d-2019-4.png; DestDir: {app}; Flags: ignoreversion; Components: BRDFormular
Source: D:\Work\EasyCash\UST\AT-2017\U1-at-2017.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2017\U1-at-2017-1.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2017\U1-at-2017-2.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2017\U1-at-2017-3.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular    
Source: D:\Work\EasyCash\UST\AT-2018\U1-at-2018.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2018\U1-at-2018-1.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2018\U1-at-2018-2.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2018\U1-at-2018-3.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2019\U1-at-2019.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2019\U1-at-2019-1.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2019\U1-at-2019-2.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UST\AT-2019\U1-at-2019-3.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-1.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-2.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-3.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-4.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-01.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-02.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-03.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-04.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-05.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-06.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-07.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-08.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-09.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-10.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-11.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-12.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-q1.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-q2.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-q3.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\UVA\AT-2020\U30-AT-2020-q4.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-AT-2019.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-AT-2019-1.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-AT-2019-2.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-AT-2019-3.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-AT-2019-4.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular  
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-K-AT-2019.ecf; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-K-AT-2019-1.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular
Source: D:\Work\EasyCash\EUeR\AT-2019\E1a-K-AT-2019-2.png; DestDir: {app}; Flags: ignoreversion; Components: ATFormular

[Registry]
Root: HKCU; Subkey: Software\Tools; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: Software\Tools\EasyCash; Flags: uninsdeletekey
Root: HKCU; Subkey: Software\Tools\EasyCash; ValueType: string; ValueName: InstallPath; ValueData: {app}     
Root: HKCU; Subkey: "Software\Thomas Mielke Softwareentwicklung"; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Thomas Mielke Softwareentwicklung\EasyCash"; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Thomas Mielke Softwareentwicklung\EasyCash"; ValueType: string; ValueName: InstallPath; ValueData: {app}

[Icons]
Name: {group}\EasyCash&Tax; Filename: {app}\EasyCT.exe
Name: {group}\Dokumentation zu EasyCash&Tax; Filename: {app}\ECT-Doku_Michael.pdf
Name: {userdesktop}\EasyCash&Tax; Filename: {app}\EasyCT.exe; MinVersion: 4,4; Tasks: desktopicon

[Run]
;Filename: {tmp}\vcredist_x86.exe; Parameters: "/q:a /c:""VCREDI~3.EXE /q:a /c:""""msiexec /i vcredist.msi /qn"""" """; Flags: runhidden shellexec waituntilterminated;
Filename: {app}\EasyCT.exe; Description: Launch EasyCash&Tax; Flags: nowait postinstall skipifsilent

[Code]

function CheckProcessRunning( aProcName,
                              aProcDesc: string ): boolean;
var
  ShellResult: boolean;
  ResultCode: integer;
  cmd: string;
  sl: TStringList;
  f: string;
  d: string;
begin
  cmd := 'for /f "delims=," %%i ' + 
         'in (''tasklist /FI "IMAGENAME eq ' + aProcName + '" /FO CSV'') ' + 
         'do if "%%~i"=="' + aProcName + '" exit 1'; 
  f := 'CheckProc.cmd';
  d := AddBackSlash( ExpandConstant( '{tmp}' ));
  sl := TStringList.Create;
  sl.Add( cmd );
  sl.Add( 'exit /0' );
  sl.SaveToFile( d + f );
  sl.Free;
  Result := true;
  while ( Result ) do
  begin
    ResultCode := 1;
    ShellResult := Exec( f,
                         '',
                         d, 
                         SW_HIDE, 
                         ewWaitUntilTerminated, 
                         ResultCode );
    Result := ResultCode > 0;
    if Result and 
       ( MsgBox( aProcDesc + ' ist noch aktiv. Das Programm muss beendet werden, um fortzufahren.'#13#10#13#10 + 
                 'Bitte wechseln Sie zu dem Programm, schlie�en Sie es und dr�cken auf OK. Im Zweifelsfall den Windows Task Manager starten (�ber die Tastaturkombination Strg-Alt-Entf) und dort den Prozess "' + aProcName + '" beenden. ', 
                 mbConfirmation, 
                 MB_OKCANCEL ) <> IDOK ) then
      Break;
  end;
  DeleteFile( d + f );
end;

// Perform some initializations.  Return False to abort setup
function InitializeSetup: Boolean;
begin
  // Do not use any user defined vars in here such as {app}
  Result := not ( CheckProcessRunning( 'EasyCT.exe',      'EasyCash&Tax' ));
end;


function InitializeUninstall: Boolean;
begin
  Result := not ( CheckProcessRunning( 'EasyCT.exe',      'EasyCash&Tax' ));
end;

// Installation der VC Laufzeitumgebung

function IsRuntimeInstalled: Boolean;
begin
  Result := False;
  // TODO: here will be a statement that will check whether the runtime is installed
  // and return True if so; see e.g. http://stackoverflow.com/q/11137424/960757
end;
 
function PrepareToInstall(var NeedsRestart: Boolean): string;
var
  ExitCode: Integer;
begin
  // if the runtime is not already installed
  if not IsRuntimeInstalled then
  begin
    // extract the redist to the temporary folder
    ExtractTemporaryFile('vcredist_x86.exe');
    // run the redist from the temp folder; if that fails, return from this handler the error text
    if not Exec(ExpandConstant('{tmp}\vcredist_x86.exe'), '/q:a /c:"VCREDI~3.EXE /q:a /c:""msiexec /i vcredist.msi /qn"" "', '', SW_SHOW, ewWaitUntilTerminated, ExitCode) then
    begin
      // return the error text
      Result := 'Setup failed to install VC++ runtime. Exit code: ' + IntToStr(ExitCode);
      // exit this function; this makes sense only if there are further prerequisites to install; in this
      // particular example it does nothing because the function exits anyway, so it is pointless here
      Exit;
    end;
  end;
end;