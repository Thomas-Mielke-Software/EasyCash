// UStAbgleichDlg.cpp : Bedienoberfläche zum Abgleich der Kontenzuweisungen von USt-Voranmeldung und Umsatzsteuererklärung
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

#include "stdafx.h"
#include "easycash.h"
#include "Einstellungen1.h"
#include "ECTIFace\EasyCashDoc.h"
#include "UStAbgleichDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Menü-IDs des Vorschlagsmenüs (nur für TrackPopupMenu mit TPM_RETURNCMD, keine Kommandos)
#define MENUE_KANDIDAT_BASIS	1		// + Index in der Kandidatenliste
#define MENUE_UST_ENTFERNEN		500
#define MENUE_UVA_SETZEN		501
#define MENUE_UVA_ENTFERNEN		502
#define MENUE_BELASSEN			503
#define MENUE_NICHTS			504

// Menütexte: '&' wäre sonst das Zeichen für die Zugriffstaste
static CString MenueText(CString csText)
{
	csText.Replace("&", "&&");
	return csText;
}

static CPoint MenuePosition()
{
	CPoint punkt;
	GetCursorPos(&punkt);
	return punkt;
}

int UStAbgleichWaehleFeld(CWnd *pWnd, const CUStAbgleich &abgleich, BOOL bUSt, const CUIntArray &kandidaten, LPCTSTR kopfzeile)
{
	CMenu menue;
	if (!menue.CreatePopupMenu()) return 0;
	menue.AppendMenu(MF_STRING | MF_GRAYED, 0, MenueText(kopfzeile));
	menue.AppendMenu(MF_SEPARATOR);
	for (int i = 0; i < kandidaten.GetSize(); i++)
		menue.AppendMenu(MF_STRING, MENUE_KANDIDAT_BASIS + i, MenueText(abgleich.FeldText(bUSt, kandidaten[i])));
	menue.AppendMenu(MF_SEPARATOR);
	menue.AppendMenu(MF_STRING, MENUE_NICHTS, "Jetzt nicht zuordnen");

	CPoint punkt = MenuePosition();
	int nGewaehlt = menue.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, punkt.x, punkt.y, pWnd);
	if (nGewaehlt >= MENUE_KANDIDAT_BASIS && nGewaehlt < MENUE_KANDIDAT_BASIS + kandidaten.GetSize())
		return kandidaten[nGewaehlt - MENUE_KANDIDAT_BASIS];
	return 0;
}

void UStAbgleichNachZuweisung(CWnd *pWnd, CString &csFeldzuweisung, BOOL bEinnahmen, LPCTSTR konto, LPCTSTR formularname, int nJahr, int nAlt)
{
	BOOL bUStGeaendert = !strcmp(formularname, USTABGLEICH_FORMULAR_UST);
	if (!bUStGeaendert && strcmp(formularname, USTABGLEICH_FORMULAR_UVA)) return;	// anderes Formular
	CUStAbgleich &abgleich = GetUStAbgleich();
	if (!abgleich.Lade(nJahr)) return;

	CUStAbgleich::Mitfuehrung m;
	abgleich.NachZuweisung(csFeldzuweisung, bEinnahmen, bUStGeaendert, nAlt, m);

	LPCTSTR partner = m.bPartnerIstUSt ? USTABGLEICH_FORMULAR_UST : USTABGLEICH_FORMULAR_UVA;
	LPCTSTR geaendert = m.bPartnerIstUSt ? USTABGLEICH_FORMULAR_UVA : USTABGLEICH_FORMULAR_UST;
	const char *soLassen = "Nein: so lassen (gilt dann als gewollt und wird nicht mehr gemeldet)";
	CString csMeldung;

	if (m.art == CUStAbgleich::Mitfuehrung::WIDERSPRUCH && m.nPartnerNeu)
	{
		csMeldung.Format("Das Konto '%s' ist für die %s diesem Feld zugeordnet:\n\n%s\n\nZur neuen Zuordnung für die %s gehört dort aber:\n\n%s\n\n"
			"Soll die Zuordnung für die %s darauf umgestellt werden?\n\nJa: umstellen\n%s",
			konto, partner, (LPCTSTR)abgleich.FeldText(m.bPartnerIstUSt, m.nPartnerAlt), geaendert,
			(LPCTSTR)abgleich.FeldText(m.bPartnerIstUSt, m.nPartnerNeu), partner, soLassen);
		if (AfxMessageBox(csMeldung, MB_YESNO | MB_ICONQUESTION) == IDYES)
			CUStAbgleich::SchreibeZuweisung(csFeldzuweisung, partner, m.nPartnerNeu);
		else
			CUStAbgleich::SetzeBelassen(csFeldzuweisung, TRUE);
	}
	else if (m.art == CUStAbgleich::Mitfuehrung::WIDERSPRUCH && m.kandidaten.GetSize() > 1)
	{
		csMeldung.Format("Das Konto '%s' ist für die %s diesem Feld zugeordnet:\n\n%s\n\nZur neuen Zuordnung für die %s passt das nicht; "
			"dort kommen mehrere andere Felder in Frage. Soll jetzt eines davon gewählt werden?\n\nJa: Feld wählen\n%s",
			konto, partner, (LPCTSTR)abgleich.FeldText(m.bPartnerIstUSt, m.nPartnerAlt), geaendert, soLassen);
		if (AfxMessageBox(csMeldung, MB_YESNO | MB_ICONQUESTION) == IDYES)
			m.art = CUStAbgleich::Mitfuehrung::PARTNER_WAEHLEN;
		else
			CUStAbgleich::SetzeBelassen(csFeldzuweisung, TRUE);
	}
	else if (m.art == CUStAbgleich::Mitfuehrung::WIDERSPRUCH || m.art == CUStAbgleich::Mitfuehrung::PARTNER_ENTFERNEN)
	{
		csMeldung.Format("Das Konto '%s' ist für die %s diesem Feld zugeordnet:\n\n%s\n\nZur geänderten Zuordnung für die %s passt das nicht mehr. "
			"Soll die Zuordnung für die %s entfernt werden?\n\nJa: entfernen\n%s",
			konto, partner, (LPCTSTR)abgleich.FeldText(m.bPartnerIstUSt, m.nPartnerAlt), geaendert, partner, soLassen);
		if (AfxMessageBox(csMeldung, MB_YESNO | MB_ICONQUESTION) == IDYES)
			CUStAbgleich::SchreibeZuweisung(csFeldzuweisung, partner, 0);
		else
			CUStAbgleich::SetzeBelassen(csFeldzuweisung, TRUE);
	}

	if (m.art == CUStAbgleich::Mitfuehrung::PARTNER_WAEHLEN)
	{
		CString csKopf;
		csKopf.Format("Konto '%s': Feld %d der %s verteilt sich in der %s auf mehrere Felder. Welches gilt?", konto, m.nGeaendertNeu, geaendert, partner);
		int nFeld = UStAbgleichWaehleFeld(pWnd, abgleich, m.bPartnerIstUSt, m.kandidaten, csKopf);
		if (nFeld) CUStAbgleich::SchreibeZuweisung(csFeldzuweisung, partner, nFeld);
	}
}

CString UStAbgleichBefundText(const CUStAbgleich &abgleich, CUStAbgleich::Befund befund, int nUvaFeld, int nUStFeld, BOOL bEinnahmen)
{
	CString csText;
	switch (befund)
	{
	case CUStAbgleich::UST_FEHLT:
		{
			CUIntArray kandidaten;
			abgleich.UStKandidaten(nUvaFeld, bEinnahmen, kandidaten);
			csText.Format("Für die " USTABGLEICH_FORMULAR_UST " fehlt die Zuordnung. Zum Feld %d der Voranmeldung gehört dort %s.",
				nUvaFeld, kandidaten.GetSize() ? (LPCTSTR)abgleich.FeldText(TRUE, kandidaten[0]) : "");
		}
		break;
	case CUStAbgleich::UST_MEHRDEUTIG:
		csText.Format("Für die " USTABGLEICH_FORMULAR_UST " fehlt die Zuordnung. Das Feld %d der Voranmeldung verteilt sich dort auf mehrere Felder; eines davon ist zu wählen.", nUvaFeld);
		break;
	case CUStAbgleich::UVA_FEHLT:
		csText.Format("Für die " USTABGLEICH_FORMULAR_UVA " fehlt die Zuordnung. Zum Feld %d der Erklärung gehört dort %s.",
			nUStFeld, (LPCTSTR)abgleich.FeldText(FALSE, abgleich.UvaFeldZu(nUStFeld)));
		break;
	case CUStAbgleich::WIDERSPRUCH:
		csText.Format("Die Zuordnungen passen nicht zusammen. Voranmeldung: %s. Erklärung: %s.",
			(LPCTSTR)abgleich.FeldText(FALSE, nUvaFeld), (LPCTSTR)abgleich.FeldText(TRUE, nUStFeld));
		break;
	}
	return csText;
}

// Buchungen eines Kontos im Buchungsjahr des Dokuments
static int ZaehleBuchungen(CEasyCashDoc *pDoc, BOOL bEinnahmen, const CString &csKonto)
{
	int nAnzahl = 0;
	if (!pDoc) return 0;
	for (CBuchung *p = bEinnahmen ? pDoc->Einnahmen : pDoc->Ausgaben; p; p = p->next)
		if (p->Datum.GetYear() == pDoc->nJahr && p->Konto == csKonto)
			nAnzahl++;
	return nAnzahl;
}

int UStAbgleichZaehleBefunde(const CUStAbgleich &abgleich, CEinstellungen1 *pEinstellungen1, CEasyCashDoc *pDoc, int nFuerFormular, int *pnMitBuchungen)
{
	int nAnzahl = 0;
	if (pnMitBuchungen) *pnMitBuchungen = 0;
	if (!abgleich.IstAktiv() || !pEinstellungen1) return 0;

	for (int nArt = 0; nArt < 2; nArt++)
	{
		BOOL bEinnahmen = nArt == 0;
		CString *pKonten = bEinnahmen ? pEinstellungen1->EinnahmenRechnungsposten : pEinstellungen1->AusgabenRechnungsposten;
		CString *pFeldzuweisungen = bEinnahmen ? pEinstellungen1->EinnahmenFeldzuweisungen : pEinstellungen1->AusgabenFeldzuweisungen;
		for (int i = 0; i < 100 && !pKonten[i].IsEmpty(); i++)
		{
			CUStAbgleich::Befund befund = abgleich.Pruefe(pFeldzuweisungen[i], bEinnahmen);
			BOOL bZaehlt;
			if (nFuerFormular == 0)			// verfälscht die Voranmeldung
				bZaehlt = befund == CUStAbgleich::UVA_FEHLT || befund == CUStAbgleich::WIDERSPRUCH;
			else if (nFuerFormular == 1)	// verfälscht die Erklärung
				bZaehlt = befund == CUStAbgleich::UST_FEHLT || befund == CUStAbgleich::UST_MEHRDEUTIG || befund == CUStAbgleich::WIDERSPRUCH;
			else
				bZaehlt = befund != CUStAbgleich::STIMMIG;
			if (!bZaehlt) continue;
			nAnzahl++;
			if (pnMitBuchungen && ZaehleBuchungen(pDoc, bEinnahmen, pKonten[i]) > 0) (*pnMitBuchungen)++;
		}
	}
	return nAnzahl;
}

/////////////////////////////////////////////////////////////////////////////
// CUStAbgleichListe

#define TOOLTIP_SPALTEN_JE_ZEILE	16	// Tool-ID = Zeile * 16 + Spalte + 1

BEGIN_MESSAGE_MAP(CUStAbgleichListe, CListCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

void CUStAbgleichListe::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();
	EnableToolTips(TRUE);
}

// Jede Zelle ist ein eigenes "Tool": MFC fragt hier, was unter dem Mauszeiger liegt
INT_PTR CUStAbgleichListe::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	LVHITTESTINFO treffer;
	memset(&treffer, 0, sizeof(treffer));
	treffer.pt = point;
	if (::SendMessage(m_hWnd, LVM_SUBITEMHITTEST, 0, (LPARAM)&treffer) < 0 || treffer.iItem < 0
		|| treffer.iSubItem < 0 || treffer.iSubItem >= TOOLTIP_SPALTEN_JE_ZEILE)
		return -1;

	CRect rect;
	if (!ListView_GetSubItemRect(m_hWnd, treffer.iItem, treffer.iSubItem, treffer.iSubItem ? LVIR_BOUNDS : LVIR_LABEL, &rect))
		return -1;

	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT_PTR)(treffer.iItem * TOOLTIP_SPALTEN_JE_ZEILE + treffer.iSubItem + 1);
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	pTI->rect = rect;
	return (INT_PTR)pTI->uId;
}

BOOL CUStAbgleichListe::OnToolTipText(UINT /*id*/, NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	if (pNMHDR->idFrom == 0) return FALSE;	// Anfrage des listeneigenen Tooltips, nicht unsere
	int nZelle = (int)pNMHDR->idFrom - 1;
	int nItem = nZelle / TOOLTIP_SPALTEN_JE_ZEILE, nSubItem = nZelle % TOOLTIP_SPALTEN_JE_ZEILE;
	if (nItem < 0 || nItem >= GetItemCount()) return FALSE;

	// nur, wenn der Text nicht in die Spalte passt (in der ersten Spalte steht davor noch das Ankreuzkästchen)
	CString csText = GetItemText(nItem, nSubItem);
	if (csText.IsEmpty() || GetStringWidth(csText) + (nSubItem == 0 ? 36 : 14) <= GetColumnWidth(nSubItem))
		return FALSE;

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 500);			// lange Feldnamen umbrechen
	::SendMessage(pNMHDR->hwndFrom, TTM_SETDELAYTIME, TTDT_AUTOPOP, 20000);	// und lange genug stehen lassen
	if (pNMHDR->code == TTN_NEEDTEXTA)
	{
		m_csTooltip = csText;
		((TOOLTIPTEXTA *)pNMHDR)->lpszText = (LPSTR)(LPCSTR)m_csTooltip;
	}
	else
	{
		m_cswTooltip = csText;
		((TOOLTIPTEXTW *)pNMHDR)->lpszText = (LPWSTR)(LPCWSTR)m_cswTooltip;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CUStAbgleichDlg

enum { SPALTE_KONTO = 0, SPALTE_ART, SPALTE_UVA, SPALTE_UST, SPALTE_BUCHUNGEN, SPALTE_VORSCHLAG };

CUStAbgleichDlg::CUStAbgleichDlg(CEinstellungen1 *pEinstellungen1, CEasyCashDoc *pDoc, int nJahr, CWnd* pParent /*=NULL*/)
	: CDialog(CUStAbgleichDlg::IDD, pParent), m_abgleich(GetUStAbgleich())
{
	m_pEinstellungen1 = pEinstellungen1;
	m_pDoc = pDoc;
	m_nJahr = nJahr;
	m_bGeaendert = FALSE;
	m_bListeWirdGefuellt = FALSE;
	m_nListenRand = -1;
}

// Verteilt die Breite der Liste auf die Spalten, sodass alle ohne waagerechtes Rollen sichtbar sind.
// "Art" und "Buchungen" bekommen, was ihr Inhalt braucht; den Rest teilen sich die Textspalten,
// der Vorschlag bekommt davon am meisten. Abgeschnittenes zeigt der Tooltip der Liste.
void CUStAbgleichDlg::SpaltenbreitenAnpassen()
{
	if (m_nListenRand < 0 || !m_liste.GetSafeHwnd() || !m_liste.GetHeaderCtrl() || m_liste.GetHeaderCtrl()->GetItemCount() <= SPALTE_VORSCHLAG) return;

	// Die Breite wird aus der des Dialogs berechnet: Beim Vergrößern hat die Liste ihre neue Größe evtl. noch nicht.
	CRect dialog;
	GetClientRect(&dialog);
	int nBreite = dialog.Width() - m_nListenRand - GetSystemMetrics(SM_CXVSCROLL) - 2 * GetSystemMetrics(SM_CXEDGE) - 2;

	CString csBuchungen;
	csBuchungen.Format("Buchungen %04d", m_pDoc ? m_pDoc->nJahr : m_nJahr);
	int nArt = m_liste.GetStringWidth("Einnahmen") + 24;
	int nBuchungen = m_liste.GetStringWidth(csBuchungen) + 24;
	int nRest = nBreite - nArt - nBuchungen;
	if (nRest < 400) nRest = 400;	// sehr schmaler Dialog: lieber rollen als unlesbar schmale Spalten

	m_liste.SetColumnWidth(SPALTE_KONTO, nRest * 18 / 100);
	m_liste.SetColumnWidth(SPALTE_ART, nArt);
	m_liste.SetColumnWidth(SPALTE_UVA, nRest * 21 / 100);
	m_liste.SetColumnWidth(SPALTE_UST, nRest * 21 / 100);
	m_liste.SetColumnWidth(SPALTE_BUCHUNGEN, nBuchungen);
	m_liste.SetColumnWidth(SPALTE_VORSCHLAG, nRest * 40 / 100);
}

void CUStAbgleichDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	SpaltenbreitenAnpassen();
}

void CUStAbgleichDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USTABGLEICH_LISTE, m_liste);
}

BEGIN_MESSAGE_MAP(CUStAbgleichDlg, CDialog)
	ON_BN_CLICKED(IDC_USTABGLEICH_AENDERN, OnAendern)
	ON_NOTIFY(NM_DBLCLK, IDC_USTABGLEICH_LISTE, OnDblclkListe)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_USTABGLEICH_LISTE, OnItemchangedListe)
	ON_WM_SIZE()
END_MESSAGE_MAP()

CString &CUStAbgleichDlg::Feldzuweisung(const Zeile &zeile)
{
	return zeile.bEinnahmen ? m_pEinstellungen1->EinnahmenFeldzuweisungen[zeile.nKonto] : m_pEinstellungen1->AusgabenFeldzuweisungen[zeile.nKonto];
}

CString CUStAbgleichDlg::Kontoname(const Zeile &zeile)
{
	return zeile.bEinnahmen ? m_pEinstellungen1->EinnahmenRechnungsposten[zeile.nKonto] : m_pEinstellungen1->AusgabenRechnungsposten[zeile.nKonto];
}

CString CUStAbgleichDlg::VorschlagText(const Zeile &zeile)
{
	CString csText;
	switch (zeile.aktion)
	{
	case UST_SETZEN:
		if (zeile.nZiel)
			csText.Format("Erklärung: %s", (LPCTSTR)m_abgleich.FeldText(TRUE, zeile.nZiel));
		else
			csText = "Erklärung: Zuordnung entfernen";
		break;
	case UVA_SETZEN:
		if (zeile.nZiel)
			csText.Format("Voranmeldung: %s", (LPCTSTR)m_abgleich.FeldText(FALSE, zeile.nZiel));
		else
			csText = "Voranmeldung: Zuordnung entfernen";
		break;
	case BELASSEN:
		csText = "so lassen und nicht mehr melden";
		break;
	default:
		csText = zeile.befund == CUStAbgleich::UST_MEHRDEUTIG ? "mehrere Felder möglich - bitte wählen (Doppelklick)" : "bitte entscheiden (Doppelklick)";
	}
	return csText;
}

void CUStAbgleichDlg::ZeileAnzeigen(int nItem)
{
	const Zeile &zeile = m_zeilen[nItem];
	CString csBuchungen;
	if (zeile.nBuchungen) csBuchungen.Format("%d", zeile.nBuchungen);
	m_liste.SetItemText(nItem, SPALTE_KONTO, Kontoname(zeile));
	m_liste.SetItemText(nItem, SPALTE_ART, zeile.bEinnahmen ? "Einnahmen" : "Ausgaben");
	m_liste.SetItemText(nItem, SPALTE_UVA, m_abgleich.FeldText(FALSE, zeile.nUvaFeld));
	m_liste.SetItemText(nItem, SPALTE_UST, m_abgleich.FeldText(TRUE, zeile.nUStFeld));
	m_liste.SetItemText(nItem, SPALTE_BUCHUNGEN, csBuchungen);
	m_liste.SetItemText(nItem, SPALTE_VORSCHLAG, VorschlagText(zeile));
}

BOOL CUStAbgleichDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_liste.SetExtendedStyle(m_liste.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);	// Tooltips: siehe CUStAbgleichListe
	CString csBuchungen;
	csBuchungen.Format("Buchungen %04d", m_pDoc ? m_pDoc->nJahr : m_nJahr);
	m_liste.InsertColumn(SPALTE_KONTO, "E/Ü-Konto", LVCFMT_LEFT, 170);
	m_liste.InsertColumn(SPALTE_ART, "Art", LVCFMT_LEFT, 70);
	m_liste.InsertColumn(SPALTE_UVA, USTABGLEICH_FORMULAR_UVA, LVCFMT_LEFT, 210);
	m_liste.InsertColumn(SPALTE_UST, USTABGLEICH_FORMULAR_UST, LVCFMT_LEFT, 210);
	m_liste.InsertColumn(SPALTE_BUCHUNGEN, csBuchungen, LVCFMT_RIGHT, 95);
	m_liste.InsertColumn(SPALTE_VORSCHLAG, "Vorschlag", LVCFMT_LEFT, 330);

	// Sechs Spalten brauchen Platz: Der Dialog öffnet sich deutlich breiter als in der Ressource angelegt,
	// aber nie breiter als der Bildschirm. Die Spalten richten sich dann nach der Breite der Liste.
	CRect fenster, dialog, liste, arbeitsbereich;
	GetWindowRect(&fenster);
	GetClientRect(&dialog);
	m_liste.GetWindowRect(&liste);
	m_nListenRand = dialog.Width() - liste.Width();
	MONITORINFO monitor;
	monitor.cbSize = sizeof(monitor);
	if (GetMonitorInfo(MonitorFromWindow(GetParent() ? GetParent()->GetSafeHwnd() : m_hWnd, MONITOR_DEFAULTTONEAREST), &monitor))
		arbeitsbereich = monitor.rcWork;
	else
		SystemParametersInfo(SPI_GETWORKAREA, 0, &arbeitsbereich, 0);
	int nBreite = min(fenster.Width() * 7 / 4, arbeitsbereich.Width() - 40);
	if (nBreite > fenster.Width())
	{
		SetWindowPos(NULL, 0, 0, nBreite, fenster.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		CenterWindow();
	}
	SpaltenbreitenAnpassen();
	SetDlgItemText(IDC_USTABGLEICH_HINWEIS,
		"Die Liste zeigt die E/Ü-Konten, deren Zuordnungen für die " USTABGLEICH_FORMULAR_UVA " und die " USTABGLEICH_FORMULAR_UST
		" nicht zusammenpassen. Konten mit Buchungen im Buchungsjahr stehen oben: Nur sie verfälschen die Formulare dieses Jahres. "
		"'Übernehmen' trägt die angehakten Vorschläge ein. Ein Doppelklick auf eine Zeile bietet andere Möglichkeiten an, "
		"auch 'so lassen', wenn die Abweichung gewollt ist.");

	if (!m_abgleich.Lade(m_nJahr) || !m_pEinstellungen1)
	{
		SetDlgItemText(IDC_USTABGLEICH_HINWEIS, "Ein Abgleich ist nicht möglich: Es ist kein Formular '" USTABGLEICH_FORMULAR_UST "' installiert, das die Entsprechungen zur Voranmeldung enthält.");
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_USTABGLEICH_AENDERN)->EnableWindow(FALSE);
		return TRUE;
	}

	// Konten mit Buchungen im Buchungsjahr zuerst: nur sie verfälschen die Formulare dieses Jahres tatsächlich
	for (int nDurchgang = 0; nDurchgang < 2; nDurchgang++)
		for (int nArt = 0; nArt < 2; nArt++)
		{
			BOOL bEinnahmen = nArt == 0;
			CString *pKonten = bEinnahmen ? m_pEinstellungen1->EinnahmenRechnungsposten : m_pEinstellungen1->AusgabenRechnungsposten;
			CString *pFeldzuweisungen = bEinnahmen ? m_pEinstellungen1->EinnahmenFeldzuweisungen : m_pEinstellungen1->AusgabenFeldzuweisungen;
			for (int i = 0; i < 100 && !pKonten[i].IsEmpty(); i++)
			{
				Zeile zeile;
				zeile.befund = m_abgleich.Pruefe(pFeldzuweisungen[i], bEinnahmen);
				if (zeile.befund == CUStAbgleich::STIMMIG) continue;
				zeile.nBuchungen = ZaehleBuchungen(m_pDoc, bEinnahmen, pKonten[i]);
				if ((nDurchgang == 0) != (zeile.nBuchungen > 0)) continue;
				zeile.bEinnahmen = bEinnahmen;
				zeile.nKonto = i;
				zeile.nUvaFeld = CUStAbgleich::LiesZuweisung(pFeldzuweisungen[i], USTABGLEICH_FORMULAR_UVA);
				zeile.nUStFeld = CUStAbgleich::LiesZuweisung(pFeldzuweisungen[i], USTABGLEICH_FORMULAR_UST);
				zeile.aktion = KEINE;
				zeile.nZiel = 0;

				CUIntArray kandidaten;
				m_abgleich.UStKandidaten(zeile.nUvaFeld, bEinnahmen, kandidaten);
				if (zeile.befund == CUStAbgleich::UVA_FEHLT)
				{
					zeile.aktion = UVA_SETZEN;
					zeile.nZiel = m_abgleich.UvaFeldZu(zeile.nUStFeld);
				}
				else if (kandidaten.GetSize() == 1)	// UST_FEHLT, oder WIDERSPRUCH mit eindeutigem Gegenstück zur Voranmeldung
				{
					zeile.aktion = UST_SETZEN;
					zeile.nZiel = kandidaten[0];
				}
				m_zeilen.Add(zeile);
			}
		}

	m_bListeWirdGefuellt = TRUE;
	for (int nItem = 0; nItem < m_zeilen.GetSize(); nItem++)
	{
		m_liste.InsertItem(nItem, "");
		ZeileAnzeigen(nItem);
		// Fehlendes ist unstrittig und wird vorab angehakt; bei einem Widerspruch soll der Benutzer selbst entscheiden
		m_liste.SetCheck(nItem, m_zeilen[nItem].befund == CUStAbgleich::UST_FEHLT || m_zeilen[nItem].befund == CUStAbgleich::UVA_FEHLT);
	}
	m_bListeWirdGefuellt = FALSE;

	if (m_zeilen.GetSize() == 0)
	{
		SetDlgItemText(IDC_USTABGLEICH_HINWEIS, "Die Zuordnungen der E/Ü-Konten für Voranmeldung und Erklärung passen zusammen. Es ist nichts zu tun.");
		GetDlgItem(IDC_USTABGLEICH_AENDERN)->EnableWindow(FALSE);
	}
	else
		m_liste.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	return TRUE;
}

void CUStAbgleichDlg::VorschlagAendern(int nItem)
{
	if (nItem < 0 || nItem >= m_zeilen.GetSize()) return;
	Zeile &zeile = m_zeilen[nItem];
	BOOL bWiderspruch = zeile.befund == CUStAbgleich::WIDERSPRUCH;

	CUIntArray kandidaten;
	m_abgleich.UStKandidaten(zeile.nUvaFeld, zeile.bEinnahmen, kandidaten);
	int nUvaZurErklaerung = m_abgleich.UvaFeldZu(zeile.nUStFeld);
	if (nUvaZurErklaerung == zeile.nUvaFeld || !m_abgleich.UvaFeldZuweisbar(nUvaZurErklaerung, zeile.bEinnahmen)) nUvaZurErklaerung = 0;

	CMenu menue;
	if (!menue.CreatePopupMenu()) return;
	CString csKopf;
	csKopf.Format("Konto '%s':", (LPCTSTR)Kontoname(zeile));
	menue.AppendMenu(MF_STRING | MF_GRAYED, 0, MenueText(csKopf));
	menue.AppendMenu(MF_SEPARATOR);
	int i;
	for (i = 0; i < kandidaten.GetSize(); i++)
		if ((int)kandidaten[i] != zeile.nUStFeld)
			menue.AppendMenu(MF_STRING | (zeile.aktion == UST_SETZEN && zeile.nZiel == (int)kandidaten[i] ? MF_CHECKED : 0),
				MENUE_KANDIDAT_BASIS + i, MenueText("Erklärung: " + m_abgleich.FeldText(TRUE, kandidaten[i])));
	if (bWiderspruch)
		menue.AppendMenu(MF_STRING | (zeile.aktion == UST_SETZEN && !zeile.nZiel ? MF_CHECKED : 0), MENUE_UST_ENTFERNEN, "Erklärung: Zuordnung entfernen");
	if (nUvaZurErklaerung)
		menue.AppendMenu(MF_STRING | (zeile.aktion == UVA_SETZEN && zeile.nZiel == nUvaZurErklaerung ? MF_CHECKED : 0),
			MENUE_UVA_SETZEN, MenueText("Voranmeldung: " + m_abgleich.FeldText(FALSE, nUvaZurErklaerung)));
	if (bWiderspruch)
		menue.AppendMenu(MF_STRING | (zeile.aktion == UVA_SETZEN && !zeile.nZiel ? MF_CHECKED : 0), MENUE_UVA_ENTFERNEN, "Voranmeldung: Zuordnung entfernen");
	menue.AppendMenu(MF_SEPARATOR);
	menue.AppendMenu(MF_STRING | (zeile.aktion == BELASSEN ? MF_CHECKED : 0), MENUE_BELASSEN, "So lassen und nicht mehr melden");
	menue.AppendMenu(MF_STRING, MENUE_NICHTS, "Vorerst nichts ändern");

	CPoint punkt = MenuePosition();
	int nGewaehlt = menue.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, punkt.x, punkt.y, this);
	if (!nGewaehlt) return;	// Menü ohne Auswahl geschlossen

	BOOL bAnhaken = TRUE;
	if (nGewaehlt >= MENUE_KANDIDAT_BASIS && nGewaehlt < MENUE_KANDIDAT_BASIS + kandidaten.GetSize())
	{
		zeile.aktion = UST_SETZEN;
		zeile.nZiel = kandidaten[nGewaehlt - MENUE_KANDIDAT_BASIS];
	}
	else if (nGewaehlt == MENUE_UST_ENTFERNEN)
	{
		zeile.aktion = UST_SETZEN;
		zeile.nZiel = 0;
	}
	else if (nGewaehlt == MENUE_UVA_SETZEN)
	{
		zeile.aktion = UVA_SETZEN;
		zeile.nZiel = nUvaZurErklaerung;
	}
	else if (nGewaehlt == MENUE_UVA_ENTFERNEN)
	{
		zeile.aktion = UVA_SETZEN;
		zeile.nZiel = 0;
	}
	else if (nGewaehlt == MENUE_BELASSEN)
		zeile.aktion = BELASSEN;
	else
		bAnhaken = FALSE;	// "vorerst nichts ändern": Vorschlag bleibt stehen, wird aber nicht übernommen

	m_bListeWirdGefuellt = TRUE;
	ZeileAnzeigen(nItem);
	m_liste.SetCheck(nItem, bAnhaken && zeile.aktion != KEINE);
	m_bListeWirdGefuellt = FALSE;
}

void CUStAbgleichDlg::OnAendern()
{
	POSITION pos = m_liste.GetFirstSelectedItemPosition();
	if (!pos)
	{
		AfxMessageBox("Bitte erst eine Zeile auswählen.");
		return;
	}
	VorschlagAendern(m_liste.GetNextSelectedItem(pos));
}

void CUStAbgleichDlg::OnDblclkListe(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMITEMACTIVATE *pAktiviert = (NMITEMACTIVATE *)pNMHDR;
	if (pAktiviert->iItem >= 0) VorschlagAendern(pAktiviert->iItem);
	*pResult = 0;
}

// Eine Zeile ohne Vorschlag lässt sich nicht einfach anhaken: dann muss erst gewählt werden.
void CUStAbgleichDlg::OnItemchangedListe(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLISTVIEW *pGeaendert = (NMLISTVIEW *)pNMHDR;
	*pResult = 0;
	if (m_bListeWirdGefuellt || !(pGeaendert->uChanged & LVIF_STATE)) return;
	if (!((pGeaendert->uOldState ^ pGeaendert->uNewState) & LVIS_STATEIMAGEMASK)) return;	// kein Wechsel des Häkchens
	int nItem = pGeaendert->iItem;
	if (nItem < 0 || nItem >= m_zeilen.GetSize()) return;
	if (m_liste.GetCheck(nItem) && m_zeilen[nItem].aktion == KEINE)
	{
		m_bListeWirdGefuellt = TRUE;
		m_liste.SetCheck(nItem, FALSE);
		m_liste.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_bListeWirdGefuellt = FALSE;
		PostMessage(WM_COMMAND, MAKEWPARAM(IDC_USTABGLEICH_AENDERN, BN_CLICKED), (LPARAM)GetDlgItem(IDC_USTABGLEICH_AENDERN)->GetSafeHwnd());
	}
}

void CUStAbgleichDlg::OnOK()
{
	for (int nItem = 0; nItem < m_zeilen.GetSize(); nItem++)
	{
		const Zeile &zeile = m_zeilen[nItem];
		if (!m_liste.GetCheck(nItem) || zeile.aktion == KEINE) continue;
		CString &csFeldzuweisung = Feldzuweisung(zeile);
		if (zeile.aktion == BELASSEN)
			CUStAbgleich::SetzeBelassen(csFeldzuweisung, TRUE);
		else
		{
			CUStAbgleich::SetzeBelassen(csFeldzuweisung, FALSE);
			CUStAbgleich::SchreibeZuweisung(csFeldzuweisung, zeile.aktion == UST_SETZEN ? USTABGLEICH_FORMULAR_UST : USTABGLEICH_FORMULAR_UVA, zeile.nZiel);
		}
		m_bGeaendert = TRUE;
	}

	CDialog::OnOK();
}
