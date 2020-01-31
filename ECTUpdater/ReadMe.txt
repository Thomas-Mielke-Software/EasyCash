================================================================================
    MICROSOFT FOUNDATION CLASS-BIBLIOTHEK: 
    ECTUpdater-Projektübersicht
===============================================================================

Der Anwendungs-Assistent hat diese ECTUpdater-Anwendung für Sie 
erstellt. Diese Anwendung zeigt nicht nur die Grundlagen der Verwendung von 
Microsoft Foundation Classes, sondern dient auch als Ausgangspunkt für das 
Schreiben Ihrer Anwendung.

Diese Datei bietet eine Übersicht über den Inhalt der einzelnen Dateien, aus
denen Ihre ECTUpdater-Anwendung besteht.

ECTUpdater.vcproj
    Dies ist die Hauptprojektdatei für VC++-Projekte, die mithilfe eines 
    Anwendungs-Assistenten erstellt werden.
    Sie enthält Informationen zur Visual C++-Version, mit der die Datei 
    generiert wurde, sowie Informationen zu Plattformen, Konfigurationen und 
    Projektfeatures, die mit dem Anwendungs-Assistenten ausgewählt wurden.

ECTUpdater.h
    Dies ist die Hauptheaderdatei für die Anwendung. Sie enthält weitere
    projektspezifische Header (einschließlich "Resource.h") und deklariert
    die CECTUpdaterApp-Anwendungsklasse.

ECTUpdater.cpp
    Dies ist die Quelldatei der Hauptanwendung, die die CECTUpdaterApp-
    Anwendungsklasse enthält.

ECTUpdater.rc
    Dies ist eine Auflistung aller vom Programm verwendeten 
    Microsoft Windows-Ressourcen. Sie enthält die Symbole, Bitmaps und Cursor, 
    die im Unterverzeichnis "RES" gespeichert werden. Diese Datei kann direkt 
    in Microsoft Visual C++ bearbeitet werden. Ihre Projektressourcen befinden 
    sich in 1031.

res\ECTUpdater.ico
    Dies ist eine Symboldatei, die als Symbol der Anwendung verwendet wird. 
    Dieses Symbol ist in der Hauptressourcendatei "ECTUpdater.rc" 
    enthalten.

res\ECTUpdater.rc2
    Diese Datei enthält Ressourcen, die nicht von Microsoft Visual C++ 
    bearbeitet werden. Sie sollten alle Ressourcen, die nicht mit dem
    Ressourcen-Editor bearbeitet werden können, in dieser Datei platzieren.


/////////////////////////////////////////////////////////////////////////////

Der Anwendungs-Assistent erstellt eine Dialogfeldklasse:

ECTUpdaterDlg.h, ECTUpdaterDlg.cpp – das Dialogfeld
    Diese Dateien enthalten Ihre CECTUpdaterDlg-Klasse. Diese Klasse 
    bestimmt das Verhalten des Hauptdialogfelds Ihrer Anwendung. Die Vorlage 
    des Dialogfelds befindet sich in der Datei "ECTUpdater.rc", die 
    in Microsoft Visual C++ bearbeitet werden kann.


/////////////////////////////////////////////////////////////////////////////

Andere Standarddateien:

StdAfx.h, StdAfx.cpp
    Mit diesen Dateien werden eine vorkompilierte Headerdatei (PCH)
    mit dem Namen "ECTUpdater.pch" und eine 
    vorkompilierte Typendatei mit dem Namen "StdAfx.obj" erstellt.

Resource.h
    Dies ist die Standardheaderdatei, die neue Ressourcen-IDs definiert.
    Die Datei wird mit Microsoft Visual C++ gelesen und aktualisiert.

/////////////////////////////////////////////////////////////////////////////

Weitere Hinweise:

Der Anwendungs-Assistent verwendet "TODO:", um auf Teile des Quellcodes
hinzuweisen, die Sie ergänzen oder anpassen sollten.

Wenn Ihre Anwendung MFC in einer gemeinsam genutzten DLL verwendet, 
müssen Sie die MFC-DLLs verteilen. Wenn die Anwendung eine andere Sprache 
als die des Gebietsschemas des Betriebssystems verwendet, müssen Sie außerdem 
die entsprechenden lokalisierten Ressourcen in "MFC90XXX.DLL" verteilen.
Weitere Informationen zu diesen beiden Themen finden Sie im Abschnitt 
zum Verteilen von Visual C++-Anwendungen in der Dokumentation zu MSDN.

/////////////////////////////////////////////////////////////////////////////
