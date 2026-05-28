// EinstellungenExports.cpp -- Bridge-Implementierung des Einstellungs-Caches

#include "stdafx.h"
#include "EinstellungenExports.h"
#include <vcclr.h>
#include <msclr/marshal.h>
#include <vector>
#include <string>

#using <ECTEngine.dll>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Globalization;

// Aus ectifacemisc.cpp
extern "C" AFX_EXT_CLASS LPCSTR IniSektion(LPCSTR id);
extern "C" AFX_EXT_CLASS BOOL   GetIniFileName(char* buffer3, int size);

namespace
{
    static bool s_handlerRegistriert = false;

    bool SektionMitPrefix(const char* sektion, char& prefixOut)
    {
        if (!strcmp(sektion, "Finanzamt"))                { prefixOut = 'f'; return true; }
        if (!strcmp(sektion, "EinnahmenRechnungsposten")) { prefixOut = 'e'; return true; }
        if (!strcmp(sektion, "AusgabenRechnungsposten"))  { prefixOut = 'a'; return true; }
        prefixOut = 0;
        return false;
    }

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

    std::string KuerzelFuerCache(const char* sektion, const char* iniKey)
    {
        char prefix = 0;
        if (SektionMitPrefix(sektion, prefix))
        {
            // Kurzform: 'f' + Name -> "fname" (so wie Plugin-Interface)
            std::string s; s += prefix; s += iniKey;
            return s;
        }
        // Unbekannte Sektionen: Bracket-Form "[Sektion]Key" -- so wie
        // Plugins (ECTImport, ...) sie ueber ECT_HoleEinstellung abfragen.
        // Frueher gab es hier nur iniKey (ohne Sektion), das war kompatibel
        // mit der alten OCX-Implementierung (vor commit dc70f89), die per
        // GetPrivateProfileString direkt abfragte. Seit der Cache aktiv ist,
        // muss der Speicher-Key zum Lookup-Key passen, sonst findet
        // "ECT_HoleEinstellung('[CSVIMPORT.0]Name')" nichts und z.B. der
        // CSV-Plugin-Dialog bleibt leer.
        std::string s;
        s += '['; s += sektion; s += ']'; s += iniKey;
        return s;
    }

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
            ::WritePrivateProfileStringA(sektion.c_str(), iniKey.c_str(), valueBuf, iniBuf);
        }
    };
}

// -----------------------------------------------------------------------------
// Rotations-Buffer (8 Slots) für LPCSTR-Rückgaben
// -----------------------------------------------------------------------------
constexpr int HOLE_BUFFER_COUNT = 8;
constexpr int HOLE_BUFFER_SIZE  = 10000;
static char s_holeBuffers[HOLE_BUFFER_COUNT][HOLE_BUFFER_SIZE];
static int  s_holeBufferIndex = 0;

static char* NaechsterBuffer()
{
    char* buf = s_holeBuffers[s_holeBufferIndex];
    s_holeBufferIndex = (s_holeBufferIndex + 1) % HOLE_BUFFER_COUNT;
    return buf;
}

static LPCSTR ManagedStringZuBuffer(System::String^ s)
{
    char* buf = NaechsterBuffer();
    if (s == nullptr) { buf[0] = 0; return buf; }
    msclr::interop::marshal_context ctx;
    strncpy_s(buf, HOLE_BUFFER_SIZE, ctx.marshal_as<const char*>(s), _TRUNCATE);
    return buf;
}

// Hilfsfunktion: Speichere-Aufruf mit std::string-Key und native char*-Value
static void SpeichereKV(const std::string& key, const char* val)
{
    ECTEngine::Einstellungen::Speichere(
        gcnew System::String(key.c_str()),
        gcnew System::String(val ? val : ""));
}

static void SpeichereKVInt(const std::string& key, int val)
{
    char buf[32]; sprintf_s(buf, "%d", val);
    SpeichereKV(key, buf);
}

// -----------------------------------------------------------------------------
// Cache-Lifecycle
// -----------------------------------------------------------------------------

void ECT_LadeEinstellungen()
{
    char iniBuf[1024];
    if (!GetIniFileName(iniBuf, sizeof(iniBuf))) return;

    std::vector<char> sectionsBuffer(32 * 1024);
    DWORD nSec = ::GetPrivateProfileSectionNamesA(
        sectionsBuffer.data(), (DWORD)sectionsBuffer.size(), iniBuf);
    if (nSec == 0) return;

    auto dict = gcnew Dictionary<System::String^, System::String^>(
        System::StringComparer::OrdinalIgnoreCase);

    const char* sectionName = sectionsBuffer.data();
    while (*sectionName)
    {
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

    if (!s_handlerRegistriert)
    {
        ECTEngine::Einstellungen::WertGeaendert +=
            gcnew Action<System::String^, System::String^>(
                &WertGeaendertHandler::OnWertGeaendert);
        s_handlerRegistriert = true;
    }

    ECTEngine::Einstellungen::LadeAusBridge(dict);
    TRACE("ECT_LadeEinstellungen: %d Schlüssel aus %s\n", dict->Count, iniBuf);
}

// -----------------------------------------------------------------------------
// Einfache Key-Value-Einstellungen
// -----------------------------------------------------------------------------

LPCSTR ECT_HoleEinstellung(LPCSTR key)
{
    if (!key) { char* b = NaechsterBuffer(); b[0] = 0; return b; }
    return ManagedStringZuBuffer(ECTEngine::Einstellungen::Hole(gcnew System::String(key)));
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
    return ECTEngine::Einstellungen::HoleBool(
        gcnew System::String(key), defaultValue ? true : false) ? TRUE : FALSE;
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

// -----------------------------------------------------------------------------
// EinnahmenKonten
// -----------------------------------------------------------------------------

int ECT_AnzahlEinnahmenKonten()
{
    return ECTEngine::Einstellungen::EinnahmenKonten->Count;
}

LPCSTR ECT_HoleEinnahmenKonto(int index)
{
    auto list = ECTEngine::Einstellungen::EinnahmenKonten;
    if (index < 0 || index >= list->Count) { char* b = NaechsterBuffer(); b[0] = 0; return b; }
    return ManagedStringZuBuffer(list[index]);
}

void ECT_SpeichereEinnahmenKonto(int index, LPCSTR name)
{
    if (!name || index < 0 || index > 99) return;
    char keyBuf[8]; sprintf_s(keyBuf, "e%02d", index);
    SpeichereKV(keyBuf, name);
}

// -----------------------------------------------------------------------------
// AusgabenKonten
// -----------------------------------------------------------------------------

int ECT_AnzahlAusgabenKonten()
{
    return ECTEngine::Einstellungen::AusgabenKonten->Count;
}

LPCSTR ECT_HoleAusgabenKonto(int index)
{
    auto list = ECTEngine::Einstellungen::AusgabenKonten;
    if (index < 0 || index >= list->Count) { char* b = NaechsterBuffer(); b[0] = 0; return b; }
    return ManagedStringZuBuffer(list[index]);
}

void ECT_SpeichereAusgabenKonto(int index, LPCSTR name)
{
    if (!name || index < 0 || index > 99) return;
    char keyBuf[8]; sprintf_s(keyBuf, "a%02d", index);
    SpeichereKV(keyBuf, name);
}

// -----------------------------------------------------------------------------
// Presets (Buchungsposten)
// -----------------------------------------------------------------------------

BOOL ECT_HolePreset(int index, ECT_Preset* outPreset)
{
    if (!outPreset) return FALSE;
    ZeroMemory(outPreset, sizeof(*outPreset));
    auto list = ECTEngine::Einstellungen::Presets;
    if (index < 0 || index >= list->Count) return FALSE;
    ECTEngine::Preset^ p = list[index];
    if (p == nullptr) return FALSE;
    msclr::interop::marshal_context ctx;
    if (p->Text  != nullptr) strncpy_s(outPreset->text,  ctx.marshal_as<const char*>(p->Text),  _TRUNCATE);
    if (p->Konto != nullptr) strncpy_s(outPreset->konto, ctx.marshal_as<const char*>(p->Konto), _TRUNCATE);
    outPreset->ausgabe = p->Ausgabe ? TRUE : FALSE;
    outPreset->mwst    = p->Mwst;
    outPreset->afaj    = p->AfaJ;
    return TRUE;
}

void ECT_SpeicherePreset(int index, const ECT_Preset* p)
{
    if (!p || index < 0 || index > 99) return;
    char pfxBuf[4]; sprintf_s(pfxBuf, "%02d", index);
    std::string pfx(pfxBuf);
    SpeichereKV(pfx + "Text", p->text);
    SpeichereKV(pfx + "Ausg", p->ausgabe ? "1" : "0");
    SpeichereKVInt(pfx + "MWSt", p->mwst);
    SpeichereKVInt(pfx + "AfAJ", p->afaj);
    SpeichereKV(pfx + "Rech", p->konto);
}

// -----------------------------------------------------------------------------
// Betriebe
// -----------------------------------------------------------------------------

int ECT_AnzahlBetriebe()
{
    return ECTEngine::Einstellungen::Betriebe->Count;
}

BOOL ECT_HoleBetrieb(int index, ECT_Betrieb* outBetrieb)
{
    if (!outBetrieb) return FALSE;
    ZeroMemory(outBetrieb, sizeof(*outBetrieb));
    auto list = ECTEngine::Einstellungen::Betriebe;
    if (index < 0 || index >= list->Count) return FALSE;
    ECTEngine::Betrieb^ b = list[index];
    if (b == nullptr) return FALSE;
    msclr::interop::marshal_context ctx;
    if (b->Name            != nullptr) strncpy_s(outBetrieb->name,             ctx.marshal_as<const char*>(b->Name),            _TRUNCATE);
    if (b->Unternehmensart != nullptr) strncpy_s(outBetrieb->unternehmensart,  ctx.marshal_as<const char*>(b->Unternehmensart), _TRUNCATE);
    outBetrieb->icon = b->Icon;
    return TRUE;
}

void ECT_SpeichereBetrieb(int index, const ECT_Betrieb* b)
{
    if (!b || index < 0 || index > 99) return;
    char pfxBuf[4]; sprintf_s(pfxBuf, "%02d", index);
    std::string pfx("Betrieb"); pfx += pfxBuf;
    SpeichereKV(pfx + "Name",            b->name);
    SpeichereKV(pfx + "Unternehmensart", b->unternehmensart);
    SpeichereKVInt(pfx + "Icon",         b->icon);
}

// -----------------------------------------------------------------------------
// Bestandskonten
// -----------------------------------------------------------------------------

int ECT_AnzahlBestandskonten()
{
    return ECTEngine::Einstellungen::Bestandskonten->Count;
}

BOOL ECT_HoleBestandskonto(int index, ECT_BestandskontoInfo* outInfo)
{
    if (!outInfo) return FALSE;
    ZeroMemory(outInfo, sizeof(*outInfo));
    auto list = ECTEngine::Einstellungen::Bestandskonten;
    if (index < 0 || index >= list->Count) return FALSE;
    ECTEngine::Bestandskonto^ bk = list[index];
    if (bk == nullptr) return FALSE;
    msclr::interop::marshal_context ctx;
    if (bk->Name != nullptr) strncpy_s(outInfo->name, ctx.marshal_as<const char*>(bk->Name), _TRUNCATE);
    outInfo->icon = bk->Icon;
    return TRUE;
}

void ECT_SpeichereBestandskonto(int index, const ECT_BestandskontoInfo* info)
{
    if (!info || index < 0 || index > 99) return;
    char pfxBuf[4]; sprintf_s(pfxBuf, "%02d", index);
    std::string pfx("Bestandskonto"); pfx += pfxBuf;
    SpeichereKV(pfx + "Name", info->name);
    SpeichereKVInt(pfx + "Icon", info->icon);
}

BOOL ECT_HoleBestandskontoSaldo(int index, int jahr, int* centOut)
{
    if (!centOut) return FALSE;
    *centOut = 0;
    auto list = ECTEngine::Einstellungen::Bestandskonten;
    if (index < 0 || index >= list->Count) return FALSE;
    ECTEngine::Bestandskonto^ bk = list[index];
    if (bk == nullptr) return FALSE;
    auto saldo = bk->Saldo;
    if (!saldo->ContainsKey(jahr)) return FALSE;
    System::Decimal d = saldo[jahr];
    *centOut = (int)((double)d * 100.0);
    return TRUE;
}

void ECT_SpeichereBestandskontoSaldo(int index, int jahr, int cent)
{
    if (index < 0 || index > 99) return;
    char keyBuf[64]; sprintf_s(keyBuf, "Bestandskonto%02dSaldo%04d", index, jahr);
    System::Decimal d = System::Decimal(cent) / System::Decimal(100);
    auto ci = gcnew CultureInfo("de-DE");
    System::String^ valStr = d.ToString("0.00", ci);
    ECTEngine::Einstellungen::Speichere(gcnew System::String(keyBuf), valStr);
}