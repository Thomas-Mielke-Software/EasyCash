// ErweiterungStoreTests.cs — Tests für den Pipe-Format-Parser

using Xunit;

namespace ECTEngine.Tests
{
    public class ErweiterungStoreTests
    {
        [Fact]
        public void AusPipeFormat_EinfacherFall()
        {
            var store = ErweiterungStore.AusPipeFormat(
                "ECTEOffenePosten|Kundennr=1234|Zahlung1=1000,00||");

            Assert.Equal("1234", store.Hole("ECTEOffenePosten", "Kundennr"));
            Assert.Equal("1000,00", store.Hole("ECTEOffenePosten", "Zahlung1"));
        }

        [Fact]
        public void AusPipeFormat_MehrereNamespaces()
        {
            var store = ErweiterungStore.AusPipeFormat(
                "EasyCash|UrspruenglichesKonto=Bürobedarf|UrspruenglicherNettobetrag=500||" +
                "ECTImport|Quelle=bank.csv||");

            Assert.Equal("Bürobedarf", store.Hole("EasyCash", "UrspruenglichesKonto"));
            Assert.Equal("500", store.Hole("EasyCash", "UrspruenglicherNettobetrag"));
            Assert.Equal("bank.csv", store.Hole("ECTImport", "Quelle"));
        }

        [Fact]
        public void AusPipeFormat_LeerString()
        {
            var store = ErweiterungStore.AusPipeFormat("");
            Assert.Equal(0, store.Count);
        }

        [Fact]
        public void AusPipeFormat_Null()
        {
            var store = ErweiterungStore.AusPipeFormat(null);
            Assert.Equal(0, store.Count);
        }

        [Fact]
        public void AusPipeFormat_SplitBuchungMarker()
        {
            // Realwelt-Testfall aus EasyCashView.cpp
            var store = ErweiterungStore.AusPipeFormat(
                "EasyCash|SplitGegenbuchungOhneVorsteuerabzug=1||");

            Assert.True(store.Hat("EasyCash", "SplitGegenbuchungOhneVorsteuerabzug"));
        }

        [Fact]
        public void ZuPipeFormat_Roundtrip()
        {
            var original = "DLL1|Key1=Val1|Key2=Val2||DLL2|A=B||";
            var store = ErweiterungStore.AusPipeFormat(original);
            var result = store.ZuPipeFormat();

            // Roundtrip-Prüfung: erneut parsen und Werte vergleichen
            var store2 = ErweiterungStore.AusPipeFormat(result);
            Assert.Equal("Val1", store2.Hole("DLL1", "Key1"));
            Assert.Equal("Val2", store2.Hole("DLL1", "Key2"));
            Assert.Equal("B", store2.Hole("DLL2", "A"));
        }

        [Fact]
        public void Setze_UndHole()
        {
            var store = new ErweiterungStore();
            store.Setze("MyPlugin", "Color", "rot");
            Assert.Equal("rot", store.Hole("MyPlugin", "Color"));
        }

        [Fact]
        public void Hole_NichtVorhanden_GibtFallback()
        {
            var store = new ErweiterungStore();
            Assert.Equal("default", store.Hole("X", "Y", "default"));
        }

        [Fact]
        public void Entferne_EntferntKey()
        {
            var store = new ErweiterungStore();
            store.Setze("NS", "Key", "Val");
            Assert.True(store.Hat("NS", "Key"));
            store.Entferne("NS", "Key");
            Assert.False(store.Hat("NS", "Key"));
        }

        [Fact]
        public void Clone_IstUnabhaengig()
        {
            var original = new ErweiterungStore();
            original.Setze("NS", "K", "V");

            var clone = original.Clone();
            clone.Setze("NS", "K", "geaendert");

            Assert.Equal("V", original.Hole("NS", "K"));
            Assert.Equal("geaendert", clone.Hole("NS", "K"));
        }
    }
}
