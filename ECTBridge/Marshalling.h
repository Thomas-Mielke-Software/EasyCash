// Marshalling.h — Typ-Konvertierung zwischen nativ und managed
//
// Enthält inline-Helfer für die häufigsten Konvertierungen:
//   CString  ↔  System::String^
//   CTime    ↔  System::DateTime
//
// Diese Datei wird NUR aus /clr-kompilierten Dateien inkludiert
// (BuchungConverter.cpp, EasyCashDocBridge.cpp, Exports.cpp).

#pragma once
#pragma managed(push, on)

#using "mscorlib.dll"

#include <msclr/marshal_cppstd.h>

namespace ECTBridge
{
    // ──────────────────────────────────────────────
    // CString ↔ System::String^
    // ──────────────────────────────────────────────

    /// <summary>CString → managed String^</summary>
    inline System::String^ ToManaged(const CString& cs)
    {
        return gcnew System::String((LPCTSTR)cs);
    }

    /// <summary>managed String^ → CString</summary>
    inline CString ToNative(System::String^ s)
    {
        if (s == nullptr) return CString("");
        using namespace System::Runtime::InteropServices;
        System::IntPtr ptr = Marshal::StringToHGlobalAnsi(s);
        CString result((const char*)ptr.ToPointer());
        Marshal::FreeHGlobal(ptr);
        return result;
    }

    // ──────────────────────────────────────────────
    // CTime ↔ System::DateTime
    // ──────────────────────────────────────────────

    /// <summary>CTime → managed DateTime</summary>
    inline System::DateTime ToManagedDateTime(const CTime& ct)
    {
        // CTime kann ungültig sein (Jahr 0, etc.)
        if (ct.GetYear() < 1 || ct.GetYear() > 9999)
            return System::DateTime(2000, 1, 1);

        return System::DateTime(
            ct.GetYear(), ct.GetMonth(), ct.GetDay(),
            ct.GetHour(), ct.GetMinute(), ct.GetSecond());
    }

    /// <summary>managed DateTime → CTime</summary>
    inline CTime ToNativeTime(System::DateTime dt)
    {
        return CTime(dt.Year, dt.Month, dt.Day,
                     dt.Hour, dt.Minute, dt.Second);
    }
}

#pragma managed(pop)
