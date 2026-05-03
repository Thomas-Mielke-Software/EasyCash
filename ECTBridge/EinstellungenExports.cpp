// EinstellungenExports.cpp — Bridge-Implementierung des Einstellungs-Caches
//
// - Erst-Befuellung: ECT_LadeEinstellungen() enumeriert ueber
//   GetPrivateProfileSectionNames + GetPrivateProfileSection alle
//   key=value-Paare aus der aktiven easyct.ini, baut sie auf das
//   Plugin-API-Schluesselformat um (Finanzamt-Keys bekommen 'f' davor,
//   EinnahmenRechnungsposten 'e', AusgabenRechnungsposten 'a') und
//   uebergibt das Dictionary an Einstellungen.LadeAusBridge().
//
// - Sofortiges Schreiben: Wir abonnieren Einstellungen.WertGeaendert
//   und schreiben jeden geaenderten Wert per WritePrivateProfileString
//   in die ini-Datei. Sektion wird via IniSektion(key) bestimmt
//   (oder explizit aus "[Sektion]Key" geparst).

#include "stdafx.h"
#include "EinstellungenExports.h"
#include <vcclr.h>
#include <msclr/marshal.h>
#include <vector>
#include <string>

#using <ECTEngine.dll>

using namespace System;
using namespace System::Collections::Generic;

// Aus ectifacemisc.cpp
extern "C" AFX_EXT_CLASS LPCSTR IniSektion(LPCSTR id);
extern "C" AFX_EXT_CLASS BOOL   GetIniFileName(char* buffer3, int size);

namespace
{
    // Der WertGeaendert-Handler wird beim ersten Aufruf von
    // ECT_LadeEinstellungen() einmalig registriert.
    static bool s_handlerRegistriert = false;

    // Liefert true, wenn die Sektion ihre Schluessel im Plugin-API-Format
    // mit Praefixbuchstaben fuehrt (Finanzamt='f', EinnahmenRechnungsposten='e',
    // AusgabenRechnungsposten='a').
    bool SektionMitPrefix(const char* sektion, char& prefixOut)
    {
        if (!strcmp(sektion, "Finanzamt"))                { prefixOut = 'f'; return true; }
        if (!strcmp(sektion, "EinnahmenRechnungsposten")) { prefixOut = 'e'; return true; }
        if (!strcmp(sektion, "AusgabenRechnungsposten"))  { prefixOut = 'a'; return true; }
        prefixOut = 0;
        return false;
    }

    // Parst "[Sektion]Key" in (sektion, iniKey).
    // Bei Kurzform-Key: ruft IniSektion(key) und stripped ggf. 'f'/'e'/'a'.
    // Liefert false, wenn key syntaktisch ungueltig ist.
    bool ZerlegeSchluessel(LPCSTR key, std::string& sektion, std::string& iniKey)
    {
        if (!key || !*key) return false;
        if (*key == '[')
        {
            const char* close = strchr(key, ']');
            if (!close) return false;
            sektion.assign(key + 1, close - key - 1);
            iniKey.assign(close + 1);
            return true;
        }
        sektion = IniSektion(key);
        char prefix = 0;
        if (SektionMitPrefix(sektion.c_str(), prefix) && key[0] == prefix)
            iniKey.assign(key + 1);
        else
            iniKey.assign(key);
        return true;
    }

    // Liefert den Plugin-API-Schluessel (Kurzform mit ggf. Praefix-Buchstaben)
    // fuer einen aus der ini gelesenen (sektion, iniKey)-Eintrag.
    std::string KuerzelFuerCache(const char* sektion, const char* iniKey)
    {
        char prefix = 0;
        if (SektionMitPrefix(sektion, prefix))
        {
            std::string s; s += prefix; s += iniKey;
            return s;
        }
        return std::string(iniKey);
    }

    // Wird beim Speichern eines Wertes ueber ECTEngine.Einstellungen.WertGeaendert
    // ausgeloest. Schreibt sofort in die aktive easyct.ini.
    ref class WertGeaendertHandler
    {
    public:
        static void OnWertGeaendert(System::String^ key, System::String^ value)
        {
            if (System::String::IsNullOrEmpty(key)) return;

            char keyBuf[1024], valueBuf[10000], iniBuf[1024];
            msclr::interop::marshal_context ctx;
            const char* keyNative = ctx.marshal_as<const char*>(key);
            const char* valNative = value == nullptr ? "" : ctx.marshal_as<const char*>(value);
            strncpy_s(keyBuf,   keyNative, _TRUNCATE);
            strncpy_s(valueBuf, valNative, _TRUNCATE);

            std::string sektion, iniKey;
            if (!ZerlegeSchluessel(keyBuf, sektion, iniKey)) return;

            if (!GetIniFileName(iniBuf, sizeof(iniBuf))) return;

            ::WritePrivateProfileStringA(sektion.c_str(), iniKey.c_str(),
                                         valueBuf, iniBuf);
        }
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// Public exports
// ─────────────────────────────────────────────────────────────────────────────

void ECT_LadeEinstellungen()
{
    char iniBuf[1024];
    if (!GetIniFileName(iniBuf, sizeof(iniBuf))) return;

    // Alle Sektionen-Namen abrufen (durch \0 getrennt, doppeltes \0 am Ende)
    std::vector<char> sectionsBuffer(32 * 1024);
    DWORD nSec = ::GetPrivateProfileSectionNamesA(
        sectionsBuffer.data(), (DWORD)sectionsBuffer.size(), iniBuf);
    if (nSec == 0) return;

    auto dict = gcnew Dictionary<System::String^, System::String^>(
        System::StringComparer::OrdinalIgnoreCase);

    const char* sectionName = sectionsBuffer.data();
    while (*sectionName)
    {
        // Alle key=value-Paare dieser Sektion
        std::vector<char> sectionData(64 * 1024);
        DWORD nDat = ::GetPrivateProfileSectionA(
            sectionName, sectionData.data(), (DWORD)sectionData.size(), iniBuf);

        if (nDat > 0)
        {
            const char* line = sectionData.data();
            while (*line)
            {
                const char* eq = strchr(line, '=');
                if (eq)
                {
                    std::string iniKey(line, eq - line);
                    std::string val(eq + 1);
                    std::string cacheKey = KuerzelFuerCache(sectionName, iniKey.c_str());
                    dict[gcnew System::String(cacheKey.c_str())] =
                        gcnew System::String(val.c_str());
                }
                line += strlen(line) + 1;
            }
        }
        sectionName += strlen(sectionName) + 1;
    }

    // Handler einmalig registrieren (vor dem Lade-Aufruf, damit kein
    // Race entsteht — LadeAusBridge feuert eh keine Events)
    if (!s_handlerRegistriert)
    {
        ECTEngine::Einstellungen::WertGeaendert +=
            gcnew Action<System::String^, System::String^>(
                &WertGeaendertHandler::OnWertGeaendert);
        s_handlerRegistriert = true;
    }

    ECTEngine::Einstellungen::LadeAusBridge(dict);

    TRACE("ECT_LadeEinstellungen: %d Schluessel aus %s\n", dict->Count, iniBuf);
}

// Rotations-Buffer: ein einzelner statischer Buffer wuerde sich bei mehreren
// ECT_HoleEinstellung-Aufrufen in derselben Expression selbst ueberschreiben
// (z.B. "ECT_HoleEinstellung(a) + ECT_HoleEinstellung(b)"). Mit 8 Buffern
// koennen typische Expressions (bis 8 Aufrufe in Folge) sicher konsumiert
// werden, bevor der erste Buffer recycelt wird.
constexpr int HOLE_BUFFER_COUNT = 8;
constexpr int HOLE_BUFFER_SIZE  = 10000;
static char s_holeBuffers[HOLE_BUFFER_COUNT][HOLE_BUFFER_SIZE];
static int  s_holeBufferIndex = 0;

LPCSTR ECT_HoleEinstellung(LPCSTR key)
{
    char* buf = s_holeBuffers[s_holeBufferIndex];
    s_holeBufferIndex = (s_holeBufferIndex + 1) % HOLE_BUFFER_COUNT;
    if (!key) { buf[0] = 0; return buf; }

    System::String^ result = ECTEngine::Einstellungen::Hole(gcnew System::String(key));
    if (result == nullptr) { buf[0] = 0; return buf; }

    msclr::interop::marshal_context ctx;
    const char* p = ctx.marshal_as<const char*>(result);
    strncpy_s(buf, HOLE_BUFFER_SIZE, p, _TRUNCATE);
    return buf;
}

void ECT_SpeichereEinstellung(LPCSTR key, LPCSTR value)
{
    if (!key) return;
    ECTEngine::Einstellungen::Speichere(
        gcnew System::String(key),
        gcnew System::String(value ? value : ""));
}

int ECT_HoleEinstellungInt(LPCSTR key, int defaultValue)
{
    return ECTEngine::Einstellungen::HoleInt(gcnew System::String(key), defaultValue);
}

BOOL ECT_HoleEinstellungBool(LPCSTR key, BOOL defaultValue)
{
    return ECTEngine::Einstellungen::HoleBool(gcnew System::String(key),
                                              defaultValue ? true : false) ? TRUE : FALSE;
}

void ECT_SpeichereEinstellungInt(LPCSTR key, int value)
{
    if (!key) return;
    ECTEngine::Einstellungen::Speichere(gcnew System::String(key), value);
}

void ECT_SpeichereEinstellungBool(LPCSTR key, BOOL value)
{
    if (!key) return;
    ECTEngine::Einstellungen::Speichere(gcnew System::String(key), value ? true : false);
}
