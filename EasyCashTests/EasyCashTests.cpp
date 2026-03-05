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

// Empfehlung: den TestExplorer links oder rechts docken und mit dem Zahnrad 
//             in "Test Explorer Einstellungen" die Option "Tests nach dem 
//             Buildvorgang ausführen" aktivieren

#include "stdafx.h"
#include "CppUnitTest.h"
#include "../ECTIFace/easycashdoc.h"

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
            buchung.Betrag = 100000; // 1000 Euro in Cent
            buchung.MWSt = 1900; // 19% MWSt
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
            buchung.Betrag = 100000; // 1000 Euro in Cent
            buchung.MWSt = 1900; // 19% MWSt
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
            buchung.Betrag = 100000; // 1000 Euro in Cent
            buchung.MWSt = 1900; // 19% MWSt
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
            buchung.Betrag = 100000; // 1000 Euro in Cent
            buchung.MWSt = 1900; // 19% MWSt
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
            buchung.Betrag = 100000; // 1000 Euro in Cent
            buchung.MWSt = 1900; // 19% MWSt
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
            buchung.Betrag = 100000; // 1000 Euro in Cent
            buchung.MWSt = 1900; // 19% MWSt
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
}
