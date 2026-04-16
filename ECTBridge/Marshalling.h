// Marshalling.h — Konvertierung zwischen nativen MFC-Typen und managed .NET-Typen
//
// Nur für /clr-kompilierte Dateien.

#pragma once

#ifndef __cplusplus_cli
#error "Marshalling.h darf nur in /clr-kompilierten Dateien included werden."
#endif

#include <afx.h>            // CString, CTime
#include <msclr/marshal.h>

namespace ECTBridge
{
    // ─────────────────────────────────────────────────
    // String-Konvertierung
    // ─────────────────────────────────────────────────

    /// <summary>CString → System::String^</summary>
    inline System::String^ ToManaged(const CString& cs)
    {
        return gcnew System::String((LPCTSTR)cs);
    }

    /// <summary>System::String^ → CString</summary>
    inline CString ToNative(System::String^ s)
    {
        if (s == nullptr) return CString("");
        msclr::interop::marshal_context ctx;
        return CString(ctx.marshal_as<const char*>(s));
    }

    // ─────────────────────────────────────────────────
    // Zeit-Konvertierung
    // CTime speichert UTC-Sekunden seit 1970.
    // DateTime hat Tick-Auflösung.
    // ─────────────────────────────────────────────────

    /// <summary>CTime → System::DateTime (lokale Zeit)</summary>
    inline System::DateTime ToManaged(const CTime& ct)
    {
        if (ct.GetTime() == 0)
            return System::DateTime::MinValue;

        return System::DateTime(
            ct.GetYear(), ct.GetMonth(), ct.GetDay(),
            ct.GetHour(), ct.GetMinute(), ct.GetSecond());
    }

    /// <summary>System::DateTime → CTime</summary>
    inline CTime ToNative(System::DateTime dt)
    {
        if (dt == System::DateTime::MinValue)
            return CTime(0);

        return CTime(
            dt.Year, dt.Month, dt.Day,
            dt.Hour, dt.Minute, dt.Second);
    }

    // ─────────────────────────────────────────────────
    // Betrag-Konvertierung
    // CBetrag speichert int Cent + int Promille.
    // ECTEngine.Betrag speichert decimal + decimal.
    // ─────────────────────────────────────────────────

    /// <summary>int Cent + int Promille → ECTEngine.Betrag (Werttyp)</summary>
    inline ECTEngine::Betrag ToManagedBetrag(int nCent, int nMwstPromille)
    {
        return ECTEngine::Betrag::AusCent(nCent, nMwstPromille);
    }
}
