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
        
        TEST_METHOD(Test_Jahreswechsel)
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
			Assert::AreEqual(275000L, (long)pJahr2Doc->Ausgaben->AbschreibungRestwert); // sollte nur die Hälfte der üblichen Jahresrate von 20000 sein, da die Buchung zum 1.7. datiert

            // Jahr 3
            CEasyCashDoc* pJahr3Doc = pJahr2Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen

            // Assert
            Assert::IsNotNull(pJahr3Doc);
            Assert::IsNotNull(pJahr3Doc->Ausgaben);
            Assert::AreEqual(175000L, (long)pJahr3Doc->Ausgaben->AbschreibungRestwert); // sollte nur die Hälfte der üblichen Jahresrate von 20000 sein, da die Buchung zum 1.7. datiert

			// Jahr 4 (Rest-AfA wegen unterjähriger Anschaffung bei monatsgenauer AfA-Genauigkeit)
            CEasyCashDoc* pJahr4Doc = pJahr3Doc->Jahreswechsel(0); // Jahreswechsel durchführen, um die AfA-Buchung ins nächste Jahr zu übertragen
            
            // Assert
            Assert::IsNotNull(pJahr4Doc);
            Assert::IsNotNull(pJahr4Doc->Ausgaben);
            Assert::AreEqual(75000L, (long)pJahr4Doc->Ausgaben->AbschreibungRestwert); // sollte nur die Hälfte der üblichen Jahresrate von 20000 sein, da die Buchung zum 1.7. datiert

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
