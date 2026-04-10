using System;

namespace ECTEngine.Calculations
{
    /// <summary>
    /// Verwaltet Backup-Anfragen und automatische Wiederherstellungsdateien
    /// </summary>
    public class BackupManager
    {
        public enum BackupAction
        {
            /// <summary>
            /// Keine Aktion
            /// </summary>
            None = 5,

            /// <summary>
            /// Backup in Unterordner erstellen
            /// </summary>
            BackupToSubfolder = 1,

            /// <summary>
            /// Backup auf externem Medium erstellen
            /// </summary>
            BackupToExternal = 2,

            /// <summary>
            /// Vollständiges Backup erstellen
            /// </summary>
            FullBackup = 3
        }

        private DateTime _nextBackupPrompt;
        private int _backupInterval = 7; // Tage

        public BackupManager()
        {
            _nextBackupPrompt = DateTime.Now.AddDays(_backupInterval);
        }

        /// <summary>
        /// Prüft ob eine Backup-Anfrage angezeigt werden sollte
        /// </summary>
        public bool ShouldPromptForBackup()
        {
            return DateTime.Now >= _nextBackupPrompt;
        }

        /// <summary>
        /// Setzt den Zeitpunkt der nächsten Backup-Anfrage
        /// </summary>
        public void ScheduleNextBackupPrompt(int intervalDays = 7)
        {
            _backupInterval = intervalDays;
            _nextBackupPrompt = DateTime.Now.AddDays(intervalDays);
        }

        /// <summary>
        /// Erzeugt den Pfad für eine Wiederherstellungsdatei
        /// </summary>
        public string GetRecoveryFilePath(string documentPath)
        {
            if (!documentPath.EndsWith(".eca", StringComparison.OrdinalIgnoreCase))
                return null;

            return documentPath.Substring(0, documentPath.Length - 4) + ".~eca";
        }

        /// <summary>
        /// Prüft ob eine Wiederherstellungsdatei existiert und neuer als das Original ist
        /// </summary>
        public bool HasNewerRecoveryFile(string documentPath)
        {
            string recoveryPath = GetRecoveryFilePath(documentPath);
            if (string.IsNullOrEmpty(recoveryPath))
                return false;

            try
            {
                var docInfo = new System.IO.FileInfo(documentPath);
                var recoveryInfo = new System.IO.FileInfo(recoveryPath);

                if (!recoveryInfo.Exists)
                    return false;

                return recoveryInfo.LastWriteTime > docInfo.LastWriteTime;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Erstellt automatische Wiederherstellungsdatei
        /// </summary>
        public void CreateRecoveryFile(string documentPath)
        {
            string recoveryPath = GetRecoveryFilePath(documentPath);
            if (string.IsNullOrEmpty(recoveryPath))
                return;

            try
            {
                // In echter Implementierung würde hier die Datei gespeichert
                // Dies ist nur ein Stub für die Struktur
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Fehler beim Erstellen der Wiederherstellungsdatei: {ex.Message}");
            }
        }

        /// <summary>
        /// Löscht die Wiederherstellungsdatei
        /// </summary>
        public void DeleteRecoveryFile(string documentPath)
        {
            string recoveryPath = GetRecoveryFilePath(documentPath);
            if (string.IsNullOrEmpty(recoveryPath))
                return;

            try
            {
                if (System.IO.File.Exists(recoveryPath))
                    System.IO.File.Delete(recoveryPath);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Fehler beim Löschen der Wiederherstellungsdatei: {ex.Message}");
            }
        }
    }
}