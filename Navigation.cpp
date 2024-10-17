// Navigation.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCash.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "ExtSplitter.h"
#include "mainfrm.h"
#include "ChildFrm.h"
#include "Navigation.h"


// CNavigation

IMPLEMENT_DYNCREATE(CNavigation, CMyListView)

CNavigation::CNavigation()
{
	m_pViewWnd = NULL;
}

CNavigation::~CNavigation()
{
}


BEGIN_MESSAGE_MAP(CNavigation, CMyListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_CLICK, &CNavigation::OnNMClick)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()



// CNavigation message handlers



int CNavigation::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyListView::OnCreate(lpCreateStruct) == -1)
		return -1;

	lpCreateStruct->style |= WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL;

	return 0;
}

void CNavigation::OnSize(UINT nType, int cx, int cy)
{
	CMyListView::OnSize(nType, cx, cy);

	CRect clir;
	GetClientRect(&clir);
	CListCtrl &nav = GetListCtrl();
	nav.SetColumnWidth(0, cx-3);
	nav.EnableGroupView(TRUE);
}

void CNavigation::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// l�uft auf Wine-Emulator, wo es keine ListView-Groups gibt?
	BOOL bWine;
	HKEY hKey;
	bWine = RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Wine", &hKey) == ERROR_SUCCESS;
	
	if (m_pViewWnd)
	{	
		int nGroup;
		if (bWine)
			nGroup = 0;
		else
		{
			LVITEM lvi = { 0 };
			lvi.iItem = pNMItemActivate->iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_GROUPID;
			GetListCtrl().GetItem(&lvi);
			nGroup = lvi.iGroupId;  // bei Journal nach Konto: 0 = Einnahmen, 1 = Ausgaben
		}

		CEasyCashDoc* pDoc = m_pViewWnd->GetDocument();
		if (m_pViewWnd->m_GewaehltesFormular < 0)
		{
			CBuchung *pb;
			int monat;

			// Finde Buchung pb, zu der navigiert werden soll
			switch (m_pViewWnd->m_nAnzeige)
			{
			// Journal nach Datum 
			case 0: 
				if (pNMItemActivate->iItem < 12)
				{
					pb = pDoc->Einnahmen;
					monat = pNMItemActivate->iItem + 1;
				}
				else
				{
					pb = pDoc->Ausgaben;
					monat = pNMItemActivate->iItem - 11;
				}
				// finde Zeile im Journalfenster, in der eine Buchung mit passendem Datum steht, und scrolle da hin
				{
					int nBesteAnn�herungZeile = -1;
					int nBesteAnnaeherungMonate = 13;
					int i;
					for (i = 0; i < MAX_BUCHUNGEN; i++)
					{
						if (m_pViewWnd->ppPosBuchungsliste[i] && (														// keine Leerzeile �.�.
							(pb == pDoc->Einnahmen && pDoc->BuchungIstEinnahme(*(m_pViewWnd->ppPosBuchungsliste[i]))) ||	// und richtiger Buchungstyp (E/A)
							(pb == pDoc->Ausgaben && pDoc->BuchungIstAusgabe(*(m_pViewWnd->ppPosBuchungsliste[i])))))
						{	
							if ((*m_pViewWnd->ppPosBuchungsliste[i])->Datum.GetMonth() == monat)  // eine Buchung mit passendem Monat im Journal gefunden? Optimal!
							{
								break; 
							}
							if (nBesteAnn�herungZeile == -1 || abs((*m_pViewWnd->ppPosBuchungsliste[i])->Datum.GetMonth() - CTime(pDoc->nJahr, monat, 15, 12, 0, 0).GetMonth()) < nBesteAnnaeherungMonate)
							{
								nBesteAnn�herungZeile = i;
								nBesteAnnaeherungMonate = abs((*m_pViewWnd->ppPosBuchungsliste[i])->Datum.GetMonth() - CTime(pDoc->nJahr, monat, 15, 12, 0, 0).GetMonth());
							}
						}
					}
					CString csMsg;
					csMsg.Format("zu Monat %d in den %s gescrollt", monat, (pNMItemActivate->iItem < 12) ? "Einnahmen" : "Ausgaben");
					if (i >= MAX_BUCHUNGEN)			// keinen exakten Monats-Treffer gefunden?
					{
						i = nBesteAnn�herungZeile;  // zu zeitlich n�chster Buchung scrollen
						csMsg.Format("keine %s-Buchung f�r Monat %d gefunden; zur zeitlich n�chsten Buchung gescrollt", (pNMItemActivate->iItem < 12) ? "Einnahmen" : "Ausgaben", monat);
					}
					if (i >= 0 && i < MAX_BUCHUNGEN)
					{
						m_pViewWnd->ScrolleZuBuchung(i);
						((CMainFrame*)AfxGetMainWnd())->SetStatus(csMsg);
						break;
					}
				}
				break;			
			case 1:  // Journal nach Konten
			case 3:  // Anlagenverzeichnis
				{
					CString csKonto = GetListCtrl().GetItemText(pNMItemActivate->iItem, 0);
					BOOL bSucheUnzugewieseneEinnahmenbuchungen = FALSE, bSucheUnzugewieseneAusgabenbuchungen = FALSE;
					if (csKonto == "[noch zu keinem Konto zugewiesene Einnahmen]")
						bSucheUnzugewieseneEinnahmenbuchungen = TRUE;
					if (csKonto == "[noch zu keinem Konto zugewiesene Ausgaben]")
						bSucheUnzugewieseneAusgabenbuchungen = TRUE;
					int i;
					for (i = 0; i < MAX_BUCHUNGEN; i++)
						if (m_pViewWnd->ppPosBuchungsliste[i])
						{
							CString csUrspruenglichesKonto = "";  // AfA-Abgang im Anlagenverzeichnis? Dann nicht das Restbuchwertkonto benutzen, sondern das urspr�ngliche Konto
							if (m_pViewWnd->m_nAnzeige == 3 && (*(m_pViewWnd->ppPosBuchungsliste[i]))->AbschreibungJahre <= 1)
							{
								CString *pcsUrspruenglichesKonto = GetErweiterungKeyCS((*(m_pViewWnd->ppPosBuchungsliste[i]))->Erweiterung, "EasyCash", "UrspruenglichesKonto");
								if (pcsUrspruenglichesKonto->IsEmpty())
								{
									delete pcsUrspruenglichesKonto;
									break;
								}		
								csUrspruenglichesKonto = (*pcsUrspruenglichesKonto).GetBuffer();
								delete pcsUrspruenglichesKonto;
							}
							else
								csUrspruenglichesKonto = (*(m_pViewWnd->ppPosBuchungsliste[i]))->Konto;

							if (((*(m_pViewWnd->ppPosBuchungsliste[i]))->Konto.IsEmpty() && bSucheUnzugewieseneEinnahmenbuchungen && pDoc->BuchungIstEinnahme(*(m_pViewWnd->ppPosBuchungsliste[i])))
							 || ((*(m_pViewWnd->ppPosBuchungsliste[i]))->Konto.IsEmpty() && bSucheUnzugewieseneAusgabenbuchungen && pDoc->BuchungIstAusgabe(*(m_pViewWnd->ppPosBuchungsliste[i])))
							 || csUrspruenglichesKonto == csKonto)
							{
								if (!bWine)  // unter wine l�sst sich nicht der Buchungstyp von der ListView-Gruppe ableiten
								{
									if (nGroup == 1 && pDoc->BuchungIstEinnahme(*(m_pViewWnd->ppPosBuchungsliste[i])))  // Wenn Kontoname sowohl in Einnahmen als auch in Ausgaben existiert sicherstellen,
										continue;																		// dass bei einer Ausgaben-Buchung auch das Ausgaben-Konto gew�hlt wird
									if (nGroup == 0 && pDoc->BuchungIstAusgabe(*(m_pViewWnd->ppPosBuchungsliste[i])))   // und pro forma auch noch mal f�r Einnahmen, obwohl das nicht vorkommen sollte...
										continue;
								}
								m_pViewWnd->ScrolleZuBuchung(i);
								CString csMsg;
								csMsg.Format("zu Konto %s gescrollt", csKonto);
								((CMainFrame*)AfxGetMainWnd())->SetStatus(csMsg);
								break;
							}
						}
				}
				break;
			// Journal nach Bestandskonten
			case 2: 
				CString csGesuchtesBestandskonto;
				int item = pNMItemActivate->iItem;
				if (item < 0 || item / 12 >= m_pViewWnd->m_csaBestandskontenMitBuchungen.GetSize()) break;
				int nBestandskonto = item / 12;
				CString csBestandskonto = m_pViewWnd->m_csaBestandskontenMitBuchungen[nBestandskonto];
				monat = item % 12 + 1;
				// finde Zeile im Journalfenster, in der eine Buchung mit passendem Bestandskonto + Datum steht, und scrolle da hin
				{
					int i;
					for (i = 0; i < MAX_BUCHUNGEN; i++)
						if (m_pViewWnd->ppPosBuchungsliste[i]
							&& (*(m_pViewWnd->ppPosBuchungsliste[i]))->Bestandskonto == csBestandskonto
							&& (*(m_pViewWnd->ppPosBuchungsliste[i]))->Datum.GetMonth() == monat)
						{
							m_pViewWnd->ScrolleZuBuchung(i);
							CString csMsg;
							csMsg.Format("zu Monat %d im Bestandskonto %s gescrollt", monat, (LPCSTR)csBestandskonto);
							((CMainFrame*)AfxGetMainWnd())->SetStatus(csMsg);
							break;
						}
					if (i == MAX_BUCHUNGEN)  // wenn monat nicht gefunden wurde, erstbeste Buchung nehmen
					{
						for (i = 0; i < MAX_BUCHUNGEN; i++)
							if (m_pViewWnd->ppPosBuchungsliste[i]
								&& (*(m_pViewWnd->ppPosBuchungsliste[i]))->Bestandskonto == csBestandskonto)
							{
								m_pViewWnd->ScrolleZuBuchung(i);
								CString csMsg;
								csMsg.Format("zu Monat %d im Bestandskonto %s gescrollt", monat, (LPCSTR)csBestandskonto);
								((CMainFrame*)AfxGetMainWnd())->SetStatus(csMsg);
								break;
							}
					}
				}
				break;
			}
		}
		else  // Formular: zu Seite scrollen
		{
			int item = pNMItemActivate->iItem;
			if (!m_pViewWnd->m_cuiaScrollPos.GetSize())
				m_pViewWnd->ScrolleZuSeite(item + 1);  // keine Abschnitte? dann item index == Seite
			else
				m_pViewWnd->ScrolleZuSeite(m_pViewWnd->m_cuiaScrollPos[item] / 1414 + 1, m_pViewWnd->m_cuiaScrollPos[item] % 1414);
		}
	}

	*pResult = 0;
}

void CNavigation::OnSetFocus(CWnd* pOldWnd)
{
	CMyListView::OnSetFocus(pOldWnd);

	// ListCtrl soll den Focus nicht bekommen, weil dadurch z.B. das Ribbon-Men� f�r das Dokumentfenster disabled w�rde
	STATIC_DOWNCAST(CChildFrame, GetParent()->GetParent())->SetActiveView(m_pViewWnd);
	m_pViewWnd->SetFocus();
}

CEasyCashDoc* CNavigation::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEasyCashDoc)));
	return (CEasyCashDoc*)m_pDocument;
}