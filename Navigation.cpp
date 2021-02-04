// Navigation.cpp : implementation file
//

#include "stdafx.h"
#include "EasyCash.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
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
	nav.SetColumnWidth(0, clir.Width());
	nav.EnableGroupView(TRUE);
}

void CNavigation::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (m_pViewWnd)
	{		
		CEasyCashDoc* pDoc = m_pViewWnd->GetDocument();
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
			while (pb)
			{
				if (pb->Datum.GetMonth() == monat || !pb->next)
					break;
				pb = pb->next;
			}
			// finde Zeile im Journalfenster, in der eine Buchung mit passendem Datum steht, und scrolle da hin
			if (pb)
			{
				int i;
				for (i = 0; i < MAX_BUCHUNGEN; i++)
					if (m_pViewWnd->ppPosBuchungsliste[i] && pb == *(m_pViewWnd->ppPosBuchungsliste[i]))
					{
						m_pViewWnd->ScrolleZuBuchung(i);
						CString csMsg;
						csMsg.Format("zu Monat %d in den %s gescrollt", monat, (pNMItemActivate->iItem < 12) ? "Einnahmen" : "Ausgaben");
						((CMainFrame*)AfxGetMainWnd())->SetStatus(csMsg);
						break;
					}
			}
			break;
		// Journal nach Konten
		case 1:
			{
				CString csKonto = GetListCtrl().GetItemText(pNMItemActivate->iItem, 0);
				int i;
				for (i = 0; i < MAX_BUCHUNGEN; i++)
					if (m_pViewWnd->ppPosBuchungsliste[i] && (*(m_pViewWnd->ppPosBuchungsliste[i]))->Konto == csKonto)
					{
						m_pViewWnd->ScrolleZuBuchung(i);
						CString csMsg;
						csMsg.Format("zu Konto %s gescrollt", csKonto);
						((CMainFrame*)AfxGetMainWnd())->SetStatus(csMsg);
						break;
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
	
	m_pViewWnd->SetFocus();

	*pResult = 0;
}

void CNavigation::OnSetFocus(CWnd* pOldWnd)
{
	CMyListView::OnSetFocus(pOldWnd);

	// ListCtrl soll den Focus nicht bekommen, weil dadurch z.B. das Ribbon-Menü für das Dokumentfenster disabled würde
	STATIC_DOWNCAST(CChildFrame, GetParent()->GetParent())->SetActiveView(m_pViewWnd);
}
