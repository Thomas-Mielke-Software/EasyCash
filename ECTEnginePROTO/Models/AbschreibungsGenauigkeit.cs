namespace ECTEngine.Models
{
    /// <summary>
    /// Abschreibungsgenauigkeit (AfA-Genauigkeit)
    /// </summary>
    public enum AbschreibungsGenauigkeit
    {
        /// <summary>
        /// Entsprechend den Einstellungen im Dokument
        /// </summary>
        EntsprechendEinstellungen = 3,

        /// <summary>
        /// Ganzjährige Abschreibung (vollständiges Jahr)
        /// </summary>
        GanzjahresAfa = 0,

        /// <summary>
        /// Halbjahresregel (bis 30. Juni = halb Jahr, ab 1. Juli = ganzes Jahr)
        /// </summary>
        HalbjahresAfa = 1,

        /// <summary>
        /// Monatsgenaue Abschreibung (ab Anschaffungsmonat)
        /// </summary>
        MonatsgenauAfa = 2
    }
}