//////////////////////////////////////////////////////////////////
// ECTIFaceMisc - Verschiedene Hilfsfunktionen
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2026  Thomas Mielke
// 
// Dies ist freie Software; Sie dürfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// veröffentlicht, weiterverteilen und/oder modifizieren; entweder gemäß 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie nützlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA.

// Empfehlung: Den TestExplorer links oder rechts docken und mit dem Zahnrad 
//             in "Test Explorer Einstellungen" die Option "Tests nach dem 
//             Buildvorgang ausführen" aktivieren.
//             Zum Debuggen im Test-Explorer Strg-R Strg-T drücken.

#include "stdafx.h"
#include "CppUnitTest.h"
#include "../ECTIFace/easycashdoc.h"
using namespace std;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EasyCashTests
{
    TEST_CLASS(CBuchungTests)
    {
    public:
        TEST_METHOD_INITIALIZE(Setup)
        {
            // MFC-Initialisierung
            AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);
        }

        TEST_METHOD(Test_GetBuchungsjahrNetto_GanzjahresLinearAfaErstesJahr)
        {
            // Arrange
            CBuchung buchung;
            buchung.Betrag = 119000; // 1000 Euro in Cent
            buchung.MWSt = 19000; // 19% MWSt-Satz (*1000, um drei Festkommastellen zu haben)
            buchung.AbschreibungNr = 1;
            buchung.AbschreibungJahre = 5;
            buchung.AbschreibungRestwert = 100000; // Restwert in Cent
            buchung.AbschreibungDegressiv = FALSE;

            // Act
            long result = buchung.GetBuchungsjahrNetto(GANZJAHRES_AFA);

            // Assert
            Assert::AreEqual(20000L, result); // Erwartete jährliche Abschreibung
        }

        TEST_METHOD(Test_GetBuchungsjahrNetto_GanzjahresLinearAfaDrittesJahr)
        {
            // Arrange
            CBuchung buchung;
            buchung.Betrag = 119000; // 1000 Euro in Cent
            buchung.MWSt = 19000; // 19% MWSt
            buchung.AbschreibungNr = 3;
            buchung.AbschreibungJahre = 5;
            buchung.AbschreibungRestwert = 60000; // Restwert in Cent
            buchung.AbschreibungDegressiv = FALSE;

            // Act
            long result = buchung.GetBuchungsjahrNetto(GANZJAHRES_AFA);

            // Assert
            Assert::AreEqual(20000L, result); // Erwartete jährliche Abschreibung
        }

        TEST_METHOD(Test_GetBuchungsjahrNetto_GanzjahresLinearAfaLetztesJahr)
        {
            // Arrange
            CBuchung buchung;
            buchung.Betrag = 119000; // 1000 Euro in Cent
            buchung.MWSt = 19000; // 19% MWSt
            buchung.AbschreibungNr = 5;
            buchung.AbschreibungJahre = 5;
            buchung.AbschreibungRestwert = 20000; // Restwert in Cent
            buchung.AbschreibungDegressiv = FALSE;

            // Act
            long result = buchung.GetBuchungsjahrNetto(GANZJAHRES_AFA);

            // Assert
            Assert::AreEqual(20000L, result); // Erwartete jährliche Abschreibung
        }

        TEST_METHOD(Test_GetBuchungsjahrNetto_GanzjahresDegressiveAfaErstesJahr)
        {
            // Arrange
            CBuchung buchung;
            buchung.Betrag = 119000; // 1000 Euro in Cent
            buchung.MWSt = 19000; // 19% MWSt
            buchung.AbschreibungNr = 1;
            buchung.AbschreibungJahre = 10;
            buchung.AbschreibungRestwert = 100000; // Restwert in Cent
            buchung.AbschreibungDegressiv = TRUE;
            buchung.AbschreibungSatz = 20; // 20% degressive Abschreibung

            // Act
            long result = buchung.GetBuchungsjahrNetto(GANZJAHRES_AFA);

            // Assert
            Assert::AreEqual(20000L, result); // Erwartete jährliche Abschreibung
        }

        TEST_METHOD(Test_GetBuchungsjahrNetto_GanzjahresDegressiveAfaDrittesJahr)
        {
            // Arrange
            CBuchung buchung;
            buchung.Betrag = 119000; // 1000 Euro in Cent
            buchung.MWSt = 19000; // 19% MWSt
            buchung.AbschreibungNr = 2;
            buchung.AbschreibungJahre = 10;
            buchung.AbschreibungRestwert = 80000; // Restwert in Cent
            buchung.AbschreibungDegressiv = TRUE;
            buchung.AbschreibungSatz = 20; // 20% degressive Abschreibung

            // Act
            long result = buchung.GetBuchungsjahrNetto(GANZJAHRES_AFA);

            // Assert
            Assert::AreEqual(16000L, result); // Erwartete jährliche Abschreibung
        }

        TEST_METHOD(Test_GetBuchungsjahrNetto_GanzjahresDegressiveAfaLetztesJahr)
        {
            // Arrange
            CBuchung buchung;
            buchung.Betrag = 119000; // 1000 Euro in Cent
            buchung.MWSt = 19000; // 19% MWSt
            buchung.AbschreibungNr = 10;
            buchung.AbschreibungJahre = 10;
            buchung.AbschreibungRestwert = 16777;  // hypothetischer Restwert nach 9 Jahren: 16777 Cent, da 100000 * 0.8^9 = 16777,216
            buchung.AbschreibungDegressiv = TRUE;  // in der Praxis wäre schon längst auf lineare AfA umgestellt worden.
            buchung.AbschreibungSatz = 20; // 20% degressive Abschreibung
			
            // Act
            long result = buchung.GetBuchungsjahrNetto(GANZJAHRES_AFA);

            // Assert
            Assert::AreEqual(16777L, result); // Erwartet: komplette Abschreibung im letzten Buchungsjahr
        }
    };
    
    TEST_CLASS(CEasyCashDocTests)
    {
    public:
        TEST_METHOD_INITIALIZE(Setup)
        {
            // MFC-Initialisierung
            AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0);
        }
        
        TEST_METHOD(Test_JahreswechselLineareAfa)
        {
            // Arrange
            CBuchung* pBuchung = new CBuchung();
            pBuchung->Datum = CTime(2023, 10, 1, 0, 0, 0);
            pBuchung->Betrag = 357000; // 3000 Euro in Cent + MWSt
            pBuchung->MWSt = 19000; // 19% MWSt
            pBuchung->AbschreibungNr = 1;
            pBuchung->AbschreibungJahre = 3;
            pBuchung->AbschreibungRestwert = 300000; // Restwert in Cent
            pBuchung->AbschreibungDegressiv = FALSE;
            pBuchung->next = NULL;
            
            CEasyCashDoc* pJahr1Doc = new CEasyCashDoc();
            pJahr1Doc->nJahr = 2023;
            pJahr1Doc->AbschreibungGenauigkeit = MONATSGENAUE_AFA;
            pJahr1Doc->Ausgaben = pBuchung;  // Erstellen eines CEasyCashDoc einer einzigen AfA-Ausgabenbuchung
            
            // Jahr 2
            CEasyCashDoc* pJahr2Doc = pJahr1Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr2Doc);
            Assert::IsNotNull(pJahr2Doc->Ausgaben);
            Assert::AreEqual(100000L, (long)pJahr2Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // im 2. Jahr die volle Jahresrate anrechnen
            Assert::AreEqual(275000L, (long)pJahr2Doc->Ausgaben->AbschreibungRestwert);                    // im ersten Jahr sollte nur ein Viertel der üblichen 
                                                                                                           // Jahres-AfA 100000 abgezogen sein, da die Buchung zum 1.10. datiert                                                                                                           
            // Jahr 3
            CEasyCashDoc* pJahr3Doc = pJahr2Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr3Doc);
            Assert::IsNotNull(pJahr3Doc->Ausgaben);
            Assert::AreEqual(100000L, (long)pJahr3Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // auch im 3. Jahr die volle Jahresrate
            Assert::AreEqual(175000L, (long)pJahr3Doc->Ausgaben->AbschreibungRestwert); 

			// Jahr 4 (Rest-AfA wegen unterjähriger Anschaffung bei monatsgenauer AfA-Genauigkeit)
            CEasyCashDoc* pJahr4Doc = pJahr3Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen
            
            // Assert
            Assert::IsNotNull(pJahr4Doc);
            Assert::IsNotNull(pJahr4Doc->Ausgaben);
			Assert::AreEqual(75000L, (long)pJahr4Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // im 4. Jahr soll der Restwert komplett abgeschrieben werden
            Assert::AreEqual(75000L, (long)pJahr4Doc->Ausgaben->AbschreibungRestwert); 

            // Jahr 5
            CEasyCashDoc* pJahr5Doc = pJahr4Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr5Doc);
            Assert::IsNull(pJahr5Doc->Ausgaben);  // Buchung sollte nun nicht mehr auftauchen

            if (pJahr5Doc) delete pJahr5Doc;
            if (pJahr4Doc) delete pJahr4Doc;
            if (pJahr3Doc) delete pJahr3Doc;
            if (pJahr2Doc) delete pJahr2Doc;
            if (pJahr1Doc) delete pJahr1Doc;
        }

        TEST_METHOD(Test_JahreswechselDegressiveAfa)
        {
            // Arrange
            CBuchung* pBuchung = new CBuchung();
            pBuchung->Datum = CTime(2023, 10, 1, 0, 0, 0);
            pBuchung->Betrag = 357000; // 3000 Euro in Cent + MWSt
            pBuchung->MWSt = 19000; // 19% MWSt
            pBuchung->AbschreibungNr = 1;
            pBuchung->AbschreibungJahre = 3;
            pBuchung->AbschreibungRestwert = 300000; // Restwert in Cent
            pBuchung->AbschreibungDegressiv = TRUE;
            pBuchung->AbschreibungSatz = 50; // hypothetische Rate zur Test-Vereinfachung: normal ist ein Satz von 20%
            pBuchung->next = NULL;           // -- bei drei Jahren und 20% würde man nie degressiv abschreiben

            CEasyCashDoc* pJahr1Doc = new CEasyCashDoc();
            pJahr1Doc->nJahr = 2023;
            pJahr1Doc->AbschreibungGenauigkeit = MONATSGENAUE_AFA;
            pJahr1Doc->Ausgaben = pBuchung;  // Erstellen eines CEasyCashDoc einer einzigen AfA-Ausgabenbuchung

            // Jahr 1 Assert
            Assert::AreEqual(37500L, (long)pJahr1Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // im ersten Jahr wird  nur ein Viertel des halben Anschaffungswerts, also 37500, abgezogen, da die Buchung zum 1.10. datiert

            // Jahr 2
            CEasyCashDoc* pJahr2Doc = pJahr1Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr2Doc);
            Assert::IsNotNull(pJahr2Doc->Ausgaben);
            long lErwarteterRestwertJahr2 = 300000L - 37500L;  // = 262500L
			Assert::AreEqual(lErwarteterRestwertJahr2, (long)pJahr2Doc->Ausgaben->AbschreibungRestwert);                   // im ersten Jahr sollte nur ein Viertel des halben Anschaffungswerts, also 37500, abgezogen worden sein, da die Buchung zum 1.10. datiert
            long lErwarteteJahresrate = lErwarteterRestwertJahr2 * pJahr2Doc->Ausgaben->AbschreibungSatz / 100;
            Assert::AreEqual(lErwarteteJahresrate, (long)pJahr2Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // im 2. Jahr die volle Jahresrate, also 50% des Anschaffungswerts, anrechnen
            // Jahres-AfA 100000 abgezogen sein, da die Buchung zum 1.10. datiert                                                                                                           

            // Jahr 3
            CEasyCashDoc* pJahr3Doc = pJahr2Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr3Doc);
            Assert::IsNotNull(pJahr3Doc->Ausgaben);
            long lErwarteterRestwertJahr3 = 131250L;  // == 262500L / 2
            Assert::AreEqual(lErwarteterRestwertJahr3, (long)pJahr3Doc->Ausgaben->AbschreibungRestwert);
			Assert::IsFalse(lErwarteterRestwertJahr3, (long)pJahr3Doc->Ausgaben->AbschreibungDegressiv);  // im 3. Jahr sollte automatisch auf lineare AfA umgestellt worden sein, da die degressive Rate kleiner wäre als die lineare
            long lGesamtlaufzeit = 3 * 12;                   //  von den 32 Monaten Gesamtlaufzeit
            long lRestlaufzeitMonate = lGesamtlaufzeit - 16;  // vier Monate sind bereits im ersten Jahr vergangen, 12 im zweiten, sind noch 20 Monate übrig            
            long lErwarteteJahresrate = lErwarteterRestwertJahr3 * pJahr3Doc->Ausgaben->AbschreibungSatz / 100;
			long lErwarteteJahresrate = lErwarteterRestwertJahr3 * 12 / lRestlaufzeitMonate;  // lineare AfA-Rate für die restlichen 20 Monate (zwölf im Jahr 3, acht im Jahr 4)
            Assert::AreEqual(lErwarteteJahresrate, (long)pJahr3Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // auch im 3. Jahr die volle Jahresrate

            // Jahr 4 (Rest-AfA wegen unterjähriger Anschaffung bei monatsgenauer AfA-Genauigkeit)
            CEasyCashDoc* pJahr4Doc = pJahr3Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr4Doc);
            Assert::IsNotNull(pJahr4Doc->Ausgaben);
			long lErwarteterRestwertJahr4 = 52500L;  // == 131250L * 8 / 20 -- restliche acht Monate im vierten Jahr abschreiben
            Assert::AreEqual(lErwarteterRestwertJahr4, (long)pJahr4Doc->Ausgaben->GetBuchungsjahrNetto(MONATSGENAUE_AFA));  // im 4. Jahr soll der Restwert komplett abgeschrieben werden
            Assert::AreEqual(lErwarteterRestwertJahr4, (long)pJahr4Doc->Ausgaben->AbschreibungRestwert);

            // Jahr 5
            CEasyCashDoc* pJahr5Doc = pJahr4Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr5Doc);
            Assert::IsNull(pJahr5Doc->Ausgaben);  // Buchung sollte nun nicht mehr auftauchen

            if (pJahr5Doc) delete pJahr5Doc;
            if (pJahr4Doc) delete pJahr4Doc;
            if (pJahr3Doc) delete pJahr3Doc;
            if (pJahr2Doc) delete pJahr2Doc;
            if (pJahr1Doc) delete pJahr1Doc;
        }
    };
}
