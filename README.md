# Finanzbuchhaltung EasyCash&Tax

Eine Mini-Finanzbuchhaltung für Windows, Wine und ähnliche win32-basierten Systeme. Architektur: C++ mit Microsoft Foundation Classes (MFC).

Homepage: https://www.easyct.de

Forks sind willkommen, merge requests umso mehr!

## Warnhinweis

Der Quellcode könnte ästhetische Gefühle verletzen und einiges an Leidensfähigkeit beanspruchen.
Die Codebasis ist ein Vierteljahrhundert alt und Konzepte von modernem C++, wie etwa smart pointer, sind selten genutzt worden.
Tatsächlich wurde vieles häufig noch mit dem Griff in die C-Mottenkiste umgesetzt.

Wer nicht ganz so masochistisch drauf ist, möge sich erst mal der [Plugin-Schnittstelle](https://www.easyct.de/articles.php?cat_id=5) bedienen.

## Lizenz

Meistens GNU General Public License Version 3 -- bitte weiteres in LIZENZ.txt nachschauen.

## Voraussetzungen
Folgendes in dieser Reihenfolge installieren:
- VS2008 Standard Edition
- VS2008 Feature Pack
- VS2008 SP1
- KB976656 (um .pch-Bug zu vermeiden)

Weiteres:
- Grafikbibliotheken (in der Projektdatei untergebracht im Verzeichnis [GrafLib](https://github.com/Thomas-Mielke-Software/GrafLib) auf derselben Ebene wie das EasyCash-Repoverzeichnis):
    - cimage von Alejandro Aguilar Sierra (Jahrgang 1995)
    - libpng von Guy Eric Schalnat, Andreas Dilger und Glenn Randers-Pehrson (release 1.0.1)
    - libjpeg von der Independent JPEG Group (release 5a of 7-Dec-94)
    - zlib von Jean-loup Gailly und Mark Adler (release 1.0.4)
- CrashRpt [SourceForge](siehe http://crashrpt.sourceforge.net/) oder [den Fork hier auf GitHib, für VS 2022](https://github.com/Thomas-Mielke-Software/CrashRpt/)

## Bauen:
- in VisualStudio 2008 rechte Maustaste auf das Subprojekt 'EasyCash', als Startprojekt auswählen und dann F7 drücken

## Verpacken:
- InnoSetup herunterladen ( http://www.jrsoftware.org/isdl.php )
- InnoSetup starten
- easycash.iss öffnen
- ggf. sigenes Software-Signaturzertifikat bei *SignTool=* anpassen
- ggf. Pfade und *AppVerName=* anpassen

## Credits:
- XMLite: leichtgewichtige Klassen für XML-Parsing von Kyung Min Cho
- QuickList: CListCtrl replacement von PEK
- XHistoryCombo: Speicherung von History in ComboBoxen von Paul S. Vickery
- XFolderDialog: Ordner-öffnen-Dialog von Hans Dietrich
- sha1: Hashing-Algorithmus-Implementation von Brian Gladman 
- Dank an Killy für die österreichischen Formulare
- Dank an alle, die Plugins entwickelt haben
- Dank an die Nutzer, die mit Feedback, Spende oder einfach einer 'Hallo-ich-bin-auch-dabei'-Registrierung zur Software beigetragen haben
- Dank an alle die ich hier vergessen habe

