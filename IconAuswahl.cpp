// IconAuswahl.cpp : implementation file
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EÜR-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
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
#include "IconAuswahl.h"
#include "XFolderDialog.h"
#include "MandantName.h"
#include "WinVer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIconAuswahl dialog


CIconAuswahl::CIconAuswahl(UINT id, CEasyCashView* pParent /*=NULL*/)
	: CDialog(id, pParent)
{
	m_pParent = pParent;
	m_nModus = 0;
	//{{AFX_DATA_INIT(CIconAuswahl)
	//}}AFX_DATA_INIT
}


void CIconAuswahl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIconAuswahl)
	DDX_Control(pDX, IDC_ICONLISTE, m_liste);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIconAuswahl, CDialog)
	//{{AFX_MSG_MAP(CIconAuswahl)
	ON_NOTIFY(NM_DBLCLK, IDC_ICONLISTE, OnDblclkIconliste)
	ON_BN_CLICKED(IDC_Neu, OnNeu)
	ON_BN_CLICKED(IDC_LOESCHEN, OnLoeschen)
	ON_BN_CLICKED(IDC_UMBENENNEN, OnUmbenennen)
	ON_BN_CLICKED(IDC_ICON_AENDERN, OnIconAendern)
	ON_BN_CLICKED(IDC_PROPERTY, OnProperty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIconAuswahl message handlers

BOOL CIconAuswahl::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Imagelist füllen
	{
		CBitmap generische_bitmap;
		m_imgList.Create(32, 32, ILC_COLOR24, 0, 100);
		generische_bitmap.LoadBitmap(GetIconBitmaps());
		m_imgList.Add(&generische_bitmap, RGB(255,0,255));
		generische_bitmap.Detach();	
	}
	m_liste.SetImageList(&m_imgList, LVSIL_NORMAL);

	InitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIconAuswahl::InitDialog()
{
	m_liste.DeleteAllItems();
	
	LVITEM Item;
	memset(&Item, 0x00, sizeof(Item));
	int i;
	switch (m_nModus)
	{
	case 1:	// Auswahl-Modus mit der Option zu ändern
		{
			int nItem = 0;
			for (i = 0; i < 100; i++)
			{
				CString csKey;
				csKey.Format("%s%-02.2dName", GetKey(), i);
				CString csName = GetProfileString(GetSection(), csKey.GetBuffer(0), "");
				csName.ReleaseBuffer();
				csKey.Format("%s%-02.2d%s", GetKey(), i, GetProperty());
				CString csProperty = GetProfileString(GetSection(), csKey.GetBuffer(0), "");
				csProperty.ReleaseBuffer();
				csKey.Format("%s%-02.2dIcon", GetKey(), i);
				CString csIcon = GetProfileString(GetSection(), csKey.GetBuffer(0), "0");
				if (!csProperty.IsEmpty() || !csName.IsEmpty())
				{
					Item.iItem = nItem;
					Item.iSubItem = 0;
					Item.mask = LVIF_IMAGE|LVIF_TEXT;
					Item.iImage = atoi(csIcon.GetBuffer(0));
					Item.pszText = csName.GetBuffer(0);
					Item.cchTextMax = strlen(Item.pszText)+1;
					m_liste.InsertItem(&Item);
					m_liste.SetItemData(nItem, nItem);
/*					{	// Tooltip setzen mit Property                 ----- geht nicht mit MFC 4.2 .....
						LVSETINFOTIP lvInfoTip;
						memset(lvInfoTip, 0x00, sizeof(lvInfoTip));
						pszText = "Hallo"; <-- UNICODE!
						lvInfoTip.cbSize = sizeof(lvInfoTip);
						lvInfoTip.iItem = i;
						m_liste.BOOL SetInfoTip(&lvInfoTip);
					}
*/
					nItem++;
				}
			}

			CString csWindowText;
			csWindowText.Format("%s auswählen", GetKey());
			SetWindowText(csWindowText);
			GetDlgItem(IDC_BITTE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_Neu)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LOESCHEN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UMBENENNEN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ICON_AENDERN)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_PROPERTY)->ShowWindow(SW_SHOW);
		}

		break;

	default: // reine Icon-Auswahl
		for (i = 0; i < GetIconCount(); i++)
		{
			Item.iItem = i;
			Item.iSubItem = 0;
			Item.mask = LVIF_IMAGE|LVIF_TEXT;
			Item.iImage = i;
			Item.pszText = GetIcons()[i].Text;
			Item.cchTextMax = strlen(Item.pszText)+1;
			m_liste.InsertItem(&Item);
		}

	}
}


void CIconAuswahl::OnOK() 
{
	POSITION p = m_liste.GetFirstSelectedItemPosition();	
	if (p)
	{
		m_nSelected = m_liste.GetNextSelectedItem(p);
		switch (m_nModus)
		{
		case 1:
			m_nSelected = (int)m_liste.GetItemData(m_nSelected);	// machen wir's mal ganz korrekt!
			break;
		}
	}
	else
		return;

	CDialog::OnOK();
}

void CIconAuswahl::OnDblclkIconliste(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOK();
	
	*pResult = 0;
}

void CIconAuswahl::OnCancel() 
{
	m_nSelected = -1;
	
	CDialog::OnCancel();
}

void CIconAuswahl::OnNeu()
{
	int nSelected = IconAuswahl();

	CString csSelected;
	if (nSelected >= 0)
	{
		// Datenverzeichnis wählen
		CString csProperty;
		if (ChooseProperty(csProperty))
		{
			int i;
			for (i = 0; i < 100; i++)
			{
				CString csKey;
				csKey.Format("%s%-02.2dName", GetKey(), i);
				if (GetProfileString(GetSection(), csKey.GetBuffer(0), "") == "")
					break;
			}

			if (i >= 100)
				AfxMessageBox("Oh, sorry: Kann nur 100 Einträge anlegen!");
			else
			{
				csSelected.Format("%d", nSelected);
				CString csKey;
				csKey.Format("%s%-02.2dName", GetKey(), i);
				WriteProfileString(GetSection(), csKey.GetBuffer(0), GetIconText(i, nSelected).GetBuffer(0));
				csKey.Format("%s%-02.2d%s", GetKey(), i, GetProperty());
				WriteProfileString(GetSection(), csKey.GetBuffer(0), csProperty.GetBuffer(0));
				csKey.Format("%s%-02.2dIcon", GetKey(), i);
				WriteProfileString(GetSection(), csKey.GetBuffer(0), csSelected.GetBuffer(0));

				CString csMsg = "'" + (CString)GetIconText(i, nSelected) + "' wurde" + (*GetProperty() ? (CString)" mit dem " + (CString)GetProperty() + " '" + csProperty + "'" : "") + " angelegt. (Hinweis: Der Name kann nachträglich im Auswahldialog mit dem Knopf 'Umbenennen' geändert werden.)";
				AfxMessageBox(csMsg);
			}
		}
	}			

	Neu();

	InitDialog();
}

void CIconAuswahl::OnLoeschen() 
{
	POSITION p = m_liste.GetFirstSelectedItemPosition();	
	if (p)
	{
		int nZuLoeschendesElement = m_liste.GetNextSelectedItem(p);
		switch (m_nModus)
		{
		case 1:
			{
				int nZuLoeschendesElementNachIndex = (int)m_liste.GetItemData(nZuLoeschendesElement);

				// letztes Element in der Liste suchen
				int i;
				for (i = 0; i < 100; i++)
				{
					CString csKey;
					csKey.Format("%s%-02.2dName", GetKey(), i);
					if (GetProfileString(GetSection(), csKey.GetBuffer(0), "") == "")
						break;
				}

				CString csKey;
				if (i > 0)
				{
					i--;

					// Daten des letzten Eintrags merken, wenn es nicht nur noch einen gibt
					CString csName;
					CString csProperty;
					CString csIcon;
					if (i > 1)
					{
						CString csKey;
						csKey.Format("%s%-02.2dName", GetKey(), i);
						csName = GetProfileString(GetSection(), csKey.GetBuffer(0), "");
						csKey.Format("%s%-02.2dDatenverzeichnis", GetKey(), i);
						csProperty = GetProfileString(GetSection(), csKey.GetBuffer(0), "");
						csKey.Format("%s%-02.2dIcon", GetKey(), i);
						csIcon = GetProfileString(GetSection(), csKey.GetBuffer(0), "0");
					}

					// letzten Eintrag an zu löschende Position übertragen
					csKey.Format("%s%-02.2dName", GetKey(), nZuLoeschendesElementNachIndex);
					WriteProfileString(GetSection(), csKey.GetBuffer(0), csName.GetBuffer(0));
					csKey.Format("%s%-02.2d%s", GetKey(), nZuLoeschendesElementNachIndex, GetProperty());
					WriteProfileString(GetSection(), csKey.GetBuffer(0), csProperty.GetBuffer(0));
					csKey.Format("%s%-02.2dIcon", GetKey(), nZuLoeschendesElementNachIndex);
					WriteProfileString(GetSection(), csKey.GetBuffer(0), csIcon.GetBuffer(0));
				}

				// letzte Position physisch löschen
				csKey.Format("%s%-02.2dName", GetKey(), i);
				WriteProfileString(GetSection(), csKey.GetBuffer(0), "");
				csKey.Format("%s%-02.2dDatenverzeichnis", GetKey(), i);
				WriteProfileString(GetSection(), csKey.GetBuffer(0), "");
				csKey.Format("%s%-02.2dIcon", GetKey(), i);
				WriteProfileString(GetSection(), csKey.GetBuffer(0), "");

				Loeschen(nZuLoeschendesElementNachIndex);

			}
			break;
		}

		InitDialog();
	}
	else
		AfxMessageBox("Bitte erst einmal ein Element auswählen");
}

void CIconAuswahl::OnUmbenennen() 
{
	POSITION p = m_liste.GetFirstSelectedItemPosition();	
	if (p)
	{
		int nZuUmbenennendesElement = m_liste.GetNextSelectedItem(p);
		switch (m_nModus)
		{
		case 1:
			{
				int nZuUmbenennendesElementNachIndex = (int)m_liste.GetItemData(nZuUmbenennendesElement);

				// letztes Element in der Liste suchen
				CString csKey;
				csKey.Format("%s%-02.2dName", GetKey(), nZuUmbenennendesElementNachIndex);
				CString csName = GetProfileString(GetSection(), csKey.GetBuffer(0), "");	
				
				CMandantName dlg;
				dlg.m_csName = csName;
				dlg.DoModal();

				csKey.Format("%s%-02.2dName", GetKey(), nZuUmbenennendesElementNachIndex);
				WriteProfileString(GetSection(), csKey.GetBuffer(0), dlg.m_csName.GetBuffer(0));				
				
				Umbenennen(nZuUmbenennendesElementNachIndex);

				InitDialog();
			}
			break;
		}
	}
	else
		AfxMessageBox("Bitte erst einmal ein Element auswählen");
}

void CIconAuswahl::OnIconAendern()
{
	POSITION p = m_liste.GetFirstSelectedItemPosition();
	if (p)
	{
		int nZuUmbenennendesElement = m_liste.GetNextSelectedItem(p);
		switch (m_nModus)
		{
		case 1:
		{
			int i = (int)m_liste.GetItemData(nZuUmbenennendesElement);

			int gewaehltesIcon = IconAuswahl();

			CString csGewaehltesIcon;
			csGewaehltesIcon.Format("%d", gewaehltesIcon);

			CString csKey;
			csKey.Format("%s%-02.2dIcon", GetKey(), i);
			WriteProfileString(GetSection(), csKey.GetBuffer(0), csGewaehltesIcon.GetBuffer(0));

			InitDialog();
		}
		break;
		}
	}
	else
		AfxMessageBox("Bitte erst einmal ein Element auswählen");

}

void CIconAuswahl::OnProperty() 
{
	POSITION p = m_liste.GetFirstSelectedItemPosition();	
	if (p)
	{
		int nZuUmbenennendesElement = m_liste.GetNextSelectedItem(p);
		switch (m_nModus)
		{
		case 1:
			{
				int nZuUmbenennendesElementNachIndex = (int)m_liste.GetItemData(nZuUmbenennendesElement);

				// letztes Element in der Liste suchen
				CString csKey;
				csKey.Format("%s%-02.2d%s", GetKey(), nZuUmbenennendesElementNachIndex, GetProperty());
				CString csProperty = GetProfileString(GetSection(), csKey.GetBuffer(0), "");	

				if (ChooseProperty(csProperty))
					WriteProfileString(GetSection(), csKey.GetBuffer(0), csProperty.GetBuffer(0));				

				InitDialog();
			}
			break;
		}
	}
	else
		AfxMessageBox("Bitte erst einmal ein Element auswählen");
}