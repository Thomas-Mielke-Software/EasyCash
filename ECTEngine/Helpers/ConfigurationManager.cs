using System;
using System.Collections.Generic;
using System.IO;

namespace ECTEngine.Helpers
{
    /// <summary>
    /// Verwaltet INI-Datei-Konfiguration und Standardeinstellungen
    /// </summary>
    public class ConfigurationManager
    {
        private readonly Dictionary<string, Dictionary<string, string>> _configuration = new();
        private readonly string _configFilePath;

        public ConfigurationManager(string configFilePath = null)
        {
            _configFilePath = configFilePath ?? GetDefaultConfigPath();
            LoadConfiguration();
        }

        /// <summary>
        /// Ermittelt den Standard-Konfigurationspfad
        /// </summary>
        private string GetDefaultConfigPath()
        {
            string appDataPath = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
            string companyPath = Path.Combine(appDataPath, "EasyCash&Tax");
            
            if (!Directory.Exists(companyPath))
                Directory.CreateDirectory(companyPath);

            return Path.Combine(companyPath, "easyct.ini");
        }

        /// <summary>
        /// Lädt die Konfiguration aus der INI-Datei
        /// </summary>
        private void LoadConfiguration()
        {
            if (!File.Exists(_configFilePath))
                return;

            try
            {
                var lines = File.ReadAllLines(_configFilePath);
                string currentSection = "";

                foreach (var line in lines)
                {
                    string trimmedLine = line.Trim();
                    
                    if (string.IsNullOrEmpty(trimmedLine) || trimmedLine.StartsWith(";"))
                        continue;

                    if (trimmedLine.StartsWith("[") && trimmedLine.EndsWith("]"))
                    {
                        currentSection = trimmedLine.Substring(1, trimmedLine.Length - 2);
                        if (!_configuration.ContainsKey(currentSection))
                            _configuration[currentSection] = new Dictionary<string, string>();
                    }
                    else if (trimmedLine.Contains("="))
                    {
                        var parts = trimmedLine.Split('=', 2);
                        if (parts.Length == 2 && !string.IsNullOrEmpty(currentSection))
                        {
                            _configuration[currentSection][parts[0].Trim()] = parts[1].Trim();
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Fehler beim Laden der Konfiguration: {ex.Message}");
            }
        }

        /// <summary>
        /// Holt einen Konfigurationswert
        /// </summary>
        public string GetValue(string section, string key, string defaultValue = "")
        {
            if (_configuration.ContainsKey(section) && _configuration[section].ContainsKey(key))
                return _configuration[section][key];
            return defaultValue;
        }

        /// <summary>
        /// Holt einen numerischen Konfigurationswert
        /// </summary>
        public int GetIntValue(string section, string key, int defaultValue = 0)
        {
            string value = GetValue(section, key);
            return int.TryParse(value, out int result) ? result : defaultValue;
        }

        /// <summary>
        /// Holt einen booleschen Konfigurationswert
        /// </summary>
        public bool GetBoolValue(string section, string key, bool defaultValue = false)
        {
            string value = GetValue(section, key).ToLower();
            return value == "true" || value == "1" ? true : 
                   value == "false" || value == "0" ? false : defaultValue;
        }

        /// <summary>
        /// Setzt einen Konfigurationswert
        /// </summary>
        public void SetValue(string section, string key, string value)
        {
            if (!_configuration.ContainsKey(section))
                _configuration[section] = new Dictionary<string, string>();

            _configuration[section][key] = value;
            SaveConfiguration();
        }

        /// <summary>
        /// Speichert die Konfiguration in die INI-Datei
        /// </summary>
        private void SaveConfiguration()
        {
            try
            {
                var lines = new List<string>();

                foreach (var section in _configuration)
                {
                    lines.Add($"[{section.Key}]");
                    foreach (var item in section.Value)
                    {
                        lines.Add($"{item.Key}={item.Value}");
                    }
                    lines.Add("");
                }

                File.WriteAllLines(_configFilePath, lines);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Fehler beim Speichern der Konfiguration: {ex.Message}");
            }
        }
    }
}