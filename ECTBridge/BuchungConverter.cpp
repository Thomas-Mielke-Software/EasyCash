// BuchungConverter.cpp — Implementierung der Buchungs-Konverter
//
// Kompilierflag: /clr (Common Language Runtime Support)

#include "stdafx.h"
#include "BuchungConverter.h"

using namespace System;

namespace ECTBridge
{
    // ─────────────────────────────────────────────────
    // CBuchung → ECTEngine.Buchung
    // ─────────────────────────────────────────────────

    ECTEngine::Buchung^ ConvertToManaged(
        const CBuchung& n, ECTEngine::Buchungsart art)
    {
        auto m = gcnew ECTEngine::Buchung();

        m->BruttoBetrag = ECTEngine::Betrag::AusCent(n.Betrag, n.MWSt);
        m->Art          = art;
        m->Datum        = ToManaged(n.Datum);
        m->Beschreibung = ToManaged(n.Beschreibung);
        m->Konto        = ToManaged(n.Konto);
        m->Belegnummer  = ToManaged(n.Belegnummer);

        m->AfaNr            = n.AbschreibungNr;
        m->AfaJahre         = n.AbschreibungJahre;
        m->AfaRestwertCent  = n.AbschreibungRestwert;
        m->AfaDegressiv     = n.AbschreibungDegressiv != FALSE;
        m->AfaSatz          = n.AbschreibungSatz;
        m->AfaGenauigkeit   = (ECTEngine::AfaGenauigkeit)n.AbschreibungGenauigkeit;

        m->Bestandskonto = ToManaged(n.Bestandskonto);
        m->Betrieb       = ToManaged(n.Betrieb);

        // Pipe-Format parsen
        m->Erweiterungen = ECTEngine::ErweiterungStore::AusPipeFormat(
            ToManaged(n.Erweiterung));

        return m;
    }

    // ─────────────────────────────────────────────────
    // ECTEngine.Buchung → CBuchung (ohne next)
    // ─────────────────────────────────────────────────

    void FillFromManaged(CBuchung& n, ECTEngine::Buchung^ m)
    {
        n.Betrag                = m->BruttoBetrag.InCent;
        n.MWSt                  = m->BruttoBetrag.MwstPromille;
        n.Datum                 = ToNative(m->Datum);
        n.Beschreibung          = ToNative(m->Beschreibung);
        n.Konto                 = ToNative(m->Konto);
        n.Belegnummer           = ToNative(m->Belegnummer);

        n.AbschreibungNr           = m->AfaNr;
        n.AbschreibungJahre        = m->AfaJahre;
        n.AbschreibungRestwert     = m->AfaRestwertCent;
        n.AbschreibungDegressiv    = m->AfaDegressiv ? TRUE : FALSE;
        n.AbschreibungSatz         = m->AfaSatz;
        n.AbschreibungGenauigkeit  = (int)m->AfaGenauigkeit;

        n.Bestandskonto = ToNative(m->Bestandskonto);
        n.Betrieb       = ToNative(m->Betrieb);

        // Erweiterungen zurück ins Pipe-Format
        n.Erweiterung = ToNative(m->Erweiterungen->ZuPipeFormat());

        // next wird vom Aufrufer verwaltet
    }

    CBuchung* CreateNative(ECTEngine::Buchung^ m)
    {
        CBuchung* n = new CBuchung();
        FillFromManaged(*n, m);
        n->next = nullptr;
        return n;
    }

    // ─────────────────────────────────────────────────
    // CDauerbuchung → ECTEngine.Dauerbuchung
    // ─────────────────────────────────────────────────

    ECTEngine::Dauerbuchung^ ConvertToManaged(const CDauerbuchung& n)
    {
        auto m = gcnew ECTEngine::Dauerbuchung();

        m->BruttoBetrag = ECTEngine::Betrag::AusCent(n.Betrag, n.MWSt);
        m->Beschreibung = ToManaged(n.Beschreibung);
        m->Art          = (ECTEngine::Buchungsart)n.Buchungstyp;
        m->Intervall    = (ECTEngine::Intervall)n.Intervall;
        m->VonDatum     = ToManaged(n.VonDatum);
        m->BisDatum     = ToManaged(n.BisDatum);
        m->Buchungstag  = n.Buchungstag;
        m->AusgefuehrtBis = ToManaged(n.AktualisiertBisDatum);
        m->Konto        = ToManaged(n.Konto);
        m->Belegnummer  = ToManaged(n.Belegnummer);
        m->Bestandskonto = ToManaged(n.Bestandskonto);
        m->Betrieb      = ToManaged(n.Betrieb);

        m->Erweiterungen = ECTEngine::ErweiterungStore::AusPipeFormat(
            ToManaged(n.Erweiterung));

        return m;
    }

    // ─────────────────────────────────────────────────
    // ECTEngine.Dauerbuchung → CDauerbuchung
    // ─────────────────────────────────────────────────

    void FillFromManaged(CDauerbuchung& n, ECTEngine::Dauerbuchung^ m)
    {
        n.Betrag              = m->BruttoBetrag.InCent;
        n.MWSt                = m->BruttoBetrag.MwstPromille;
        n.Beschreibung        = ToNative(m->Beschreibung);
        n.Buchungstyp         = (int)m->Art;
        n.Intervall           = (int)m->Intervall;
        n.VonDatum            = ToNative(m->VonDatum);
        n.BisDatum            = ToNative(m->BisDatum);
        n.Buchungstag         = m->Buchungstag;
        n.AktualisiertBisDatum = ToNative(m->AusgefuehrtBis);
        n.Konto               = ToNative(m->Konto);
        n.Belegnummer         = ToNative(m->Belegnummer);
        n.Bestandskonto       = ToNative(m->Bestandskonto);
        n.Betrieb             = ToNative(m->Betrieb);
        n.Erweiterung         = ToNative(m->Erweiterungen->ZuPipeFormat());
    }

    CDauerbuchung* CreateNative(ECTEngine::Dauerbuchung^ m)
    {
        CDauerbuchung* n = new CDauerbuchung();
        FillFromManaged(*n, m);
        n->next = nullptr;
        return n;
    }
}
