// MainFrm.cpp : implementation of the CMainFrame class
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
#include "EasyCash.h"
#include "ECTIFace\EasyCashDoc.h"
#include "EasyCashView.h"
#include "..\GrafLib\cimage\cimage.h"
#include "MainFrm.h"
#include "PluginManager.h"
#include "IconAuswahlMandant.h"
#include "XFolderDialog.h"
#include "RegistrierenDlg.h"
#include <memory>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CPluginElement

CPluginElement::CPluginElement()
{
	bitmap = NULL;
	icon = NULL;
	next = NULL;
}

CPluginElement::~CPluginElement()
{
	if (bitmap) { delete bitmap;		bitmap	= NULL; }
	if (icon)   { ::DestroyIcon(icon);	icon	= NULL; }
	if (next)   { delete next;			next	= NULL; }
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonButtonEx

CMFCRibbonButtonEx::CMFCRibbonButtonEx() : CMFCRibbonButton() { }
CMFCRibbonButtonEx::CMFCRibbonButtonEx(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, BOOL bAlwaysShowDescription)
	: CMFCRibbonButton(nID, lpszText, nSmallImageIndex, nLargeImageIndex, bAlwaysShowDescription) { }
CMFCRibbonButtonEx::CMFCRibbonButtonEx(UINT nID, LPCTSTR lpszText, HICON hIcon, BOOL bAlwaysShowDescription, HICON hIconSmall, BOOL bAutoDestroyIcon, BOOL bAlphaBlendIcon)
	: CMFCRibbonButton(nID, lpszText, hIcon, bAlwaysShowDescription , hIconSmall, bAutoDestroyIcon, bAlphaBlendIcon) { }
void CMFCRibbonButtonEx::SetCheck(BOOL bCheck)
{
	m_bChecked = bCheck;
}
void CMFCRibbonButtonEx::DrawImage(CDC* pDC, RibbonImageType type, CRect rectImage)
{
	CMFCRibbonButton::DrawImage(pDC, type, rectImage);
	if (type == RibbonImageLarge && m_bChecked)	
		DrawCheckmark(pDC, IDB_BIG_ICON_CHECKMARK, &rectImage);
}
void CMFCRibbonButtonEx::DrawCheckmark(CDC* pDC, int CheckmarkResourceBitmapID, RECT *r)
{
	HDC  hdc;
	CDC  *dc;
	CDC dcMem;
	CBitmap cbm;

	VERIFY(hdc = pDC->m_hDC);
	VERIFY(dc = pDC);

	dcMem.CreateCompatibleDC(dc);

	cbm.LoadBitmap(CheckmarkResourceBitmapID);
	PremultiplyBitmapAlpha(dcMem.m_hDC, cbm);
	SelectObject(dcMem.m_hDC, cbm.m_hObject);

    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;
    ::AlphaBlend(hdc, r->left, r->top, r->right-r->left, r->bottom-r->top, dcMem, 0, 0, 32, 32, bf);

	VERIFY(dcMem.DeleteDC());
}
void CMFCRibbonButtonEx::PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp)
{
   BITMAP bm = { 0 };
   GetObject(hBmp, sizeof(bm), &bm);
   BITMAPINFO* bmi = (BITMAPINFO*) _alloca(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
   ::ZeroMemory(bmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
   bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   BOOL bRes = ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS);
   if( !bRes || bmi->bmiHeader.biBitCount != 32 ) return;
   LPBYTE pBitData = (LPBYTE) ::LocalAlloc(LPTR, bm.bmWidth * bm.bmHeight * sizeof(DWORD));
   if( pBitData == NULL ) return;
   LPBYTE pData = pBitData;
   ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, pData, bmi, DIB_RGB_COLORS);
   for( int y = 0; y < bm.bmHeight; y++ ) {
      for( int x = 0; x < bm.bmWidth; x++ ) {
         pData[0] = (BYTE)((DWORD)pData[0] * pData[3] / 255);
         pData[1] = (BYTE)((DWORD)pData[1] * pData[3] / 255);
         pData[2] = (BYTE)((DWORD)pData[2] * pData[3] / 255);
         pData += 4;
      }
   }
   ::SetDIBits(hDC, hBmp, 0, bm.bmHeight, pBitData, bmi, DIB_RGB_COLORS);
   ::LocalFree(pBitData);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_INITMENU()
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_PLUGINMANAGER, OnFilePluginmanager)
	ON_COMMAND(ID_FILE_OEFFNE_DATENVERZEICHNIS, OnFileOeffneDatenverzeichnis)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_2007, ID_VIEW_APPLOOK_2005, &CMainFrame::OnAppLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_2007, ID_VIEW_APPLOOK_2005, &CMainFrame::OnUpdateAppLook)
	//ON_COMMAND(ID_VIEW_CAPTION_BAR, &CMainFrame::OnViewCaptionBar)
	//ON_UPDATE_COMMAND_UI(ID_VIEW_CAPTION_BAR, &CMainFrame::OnUpdateViewCaptionBar)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_FILE_MANDANTEN, OnFileMandanten)
	ON_COMMAND(ID_FILE_WAEHLE_DATENVERZEICHNIS, OnFileWaehleDatenverzeichnis)
	ON_COMMAND(ID_FILE_REGISTRIERUNGSINFORMATIONEN_WIEDERHERSTELLEN, OnFileRegistrierungsinformationenWiederherstellen)
	ON_COMMAND(ID_FILE_REGISTRIERUNGSINFORMATIONEN_SICHERN, OnFileRegistrierungsinformationenSichern)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_WM_COPYDATA()
	ON_COMMAND(ID_EINSTELLUNGEN_SPENDE_BANK, OnSpendeBank)   
	ON_COMMAND(ID_EINSTELLUNGEN_SPENDE_PAYPAL, OnSpendePayPal)
	ON_COMMAND(ID_EINSTELLUNGEN_SPENDE_BITCOIN, OnSpendeBitcoin)
	ON_COMMAND(ID_EINSTELLUNGEN_SPENDE_FLATTR, OnSpendeFlattr)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SALDO,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_nPluginCount = 0;
	m_nAppLook = theApp.GetInt (_T("ApplicationLook"), ID_VIEW_APPLOOK_2005);
	m_pPlugins = NULL;
	m_pInfoButton = NULL;
	m_pFilterKontoButton = NULL;
	m_pFilterBetriebButton = NULL;
	m_pFilterBestandskontoButton = NULL;
	m_pAnsichtFormulareButton = NULL;
	m_pAnsichtPluginsButton = NULL;
	m_bInformationModeElapsed = TRUE;
	m_pSucheCombobox = NULL;
}

CMainFrame::~CMainFrame()
{
	if (m_pPlugins) delete m_pPlugins;
}

// these constants represent the dimensions and number of buttons in
// the default MFC-generated toolbar. If you need something different,
// feel free to change them. For extra credit, you can load the
// toolbar's existing image list at runtime and copy the parameters from
// there.
static const int	kImageWidth (32);
static const int	kImageHeight (32);
static const int	kNumImages (13);

static const UINT	kToolBarBitDepth (ILC_COLOR24);

// this color will be treated as transparent in the loaded bitmaps --
// in other words, any pixel of this color will be set at runtime to
// the user's button color. The Visual Studio toolbar editor defaults
// to 192, 192, 192 (light gray).
static const RGBTRIPLE	kBackgroundColor = {192, 192, 192};

// find every pixel of the default background color in the specified
// bitmap and set each one to the user's button color.
static void	ReplaceBackgroundColor (CBitmap& ioBM)
{
	// figure out how many pixels there are in the bitmap
	BITMAP		bmInfo;

	VERIFY (ioBM.GetBitmap (&bmInfo));

	// add support for additional bit depths if you choose
	VERIFY (bmInfo.bmBitsPixel == 24);
	VERIFY (bmInfo.bmWidthBytes == (bmInfo.bmWidth * 3));

	const UINT		numPixels (bmInfo.bmHeight * bmInfo.bmWidth);

	// get a pointer to the pixels
	DIBSECTION  ds;

	VERIFY (ioBM.GetObject (sizeof (DIBSECTION), &ds) == sizeof (DIBSECTION));

	RGBTRIPLE*		pixels = reinterpret_cast<RGBTRIPLE*>(ds.dsBm.bmBits);
	VERIFY (pixels != NULL);

	// get the user's preferred button color from the system
	const COLORREF		buttonColor (::GetSysColor (COLOR_BTNFACE));
	const RGBTRIPLE		userBackgroundColor = {
	GetBValue (buttonColor), GetGValue (buttonColor), GetRValue (buttonColor)};

	// search through the pixels, substituting the user's button
	// color for any pixel that has the magic background color
	for (UINT i = 0; i < numPixels; ++i)
	{
		if (pixels [i].rgbtBlue == kBackgroundColor.rgbtBlue
		&& pixels [i].rgbtGreen == kBackgroundColor.rgbtGreen
		&& pixels [i].rgbtRed == kBackgroundColor.rgbtRed)
		{
			pixels [i] = userBackgroundColor;
		}
	}
}


// create an image list for the specified BMP resource
static void	MakeToolbarImageList (UINT inBitmapID,
                                  CImageList&	outImageList)
{
	CBitmap		bm;

	// if we use CBitmap::LoadBitmap() to load the bitmap, the colors
	// will be reduced to the bit depth of the main screen and we won't
	// be able to access the pixels directly. To avoid those problems,
	// we'll load the bitmap as a DIBSection instead and attach the
	// DIBSection to the CBitmap.
	VERIFY (bm.Attach (::LoadImage (::AfxFindResourceHandle(
	MAKEINTRESOURCE (inBitmapID), RT_BITMAP),
	MAKEINTRESOURCE (inBitmapID), IMAGE_BITMAP, 0, 0,
	(LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));

	// replace the specified color in the bitmap with the user's
	// button color
	::ReplaceBackgroundColor (bm);

	// create a 24 bit image list with the same dimensions and number
	// of buttons as the toolbar
	int nNumImages = kNumImages;
	//nNumImages = bm.GetBitmapDimension().cx / kImageWidth;
	VERIFY (outImageList.Create (
	kImageWidth, kImageHeight, kToolBarBitDepth, nNumImages, 0));

	// attach the bitmap to the image list
	VERIFY (outImageList.Add (&bm, RGB (0, 0, 0)) != -1);
}


// load the high color toolbar images and attach them to m_wndToolBar
void CMainFrame::AttachToolbarImages (UINT inNormalImageID,
                                      UINT inDisabledImageID,
                                      UINT inHotImageID)
{
  // make high-color image lists for each of the bitmaps
 ::MakeToolbarImageList (inNormalImageID, m_ToolbarImages);
 ::MakeToolbarImageList (inDisabledImageID, m_ToolbarImagesDisabled);
 ::MakeToolbarImageList (inHotImageID, m_ToolbarImagesHot);

  // get the toolbar control associated with the CToolbar object
 CToolBarCtrl&	barCtrl = m_wndToolBar.GetToolBarCtrl();

  // attach the image lists to the toolbar control
 barCtrl.SetImageList (&m_ToolbarImages);
 barCtrl.SetDisabledImageList (&m_ToolbarImagesDisabled);
 barCtrl.SetHotImageList (&m_ToolbarImagesHot);
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	OnAppLook(m_nAppLook);

/* Tabs nötig? stürzt ab! toxischer Code...
	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdiTabParams);
*/
	if (!CreateRibbonBar())
	{
		TRACE0("Konnte den Ribbon Bar nicht erzeugen\n");
		return -1;      // fail to create
	}

	// create standard toolbar
/*	EnableDocking(CBRS_ALIGN_ANY);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
*/

/*
	// attach the hicolor bitmaps to the toolbar
	AttachToolbarImages (IDB_HICOLOR_TOOLBAR, IDB_HICOLOR_TOOLBAR_DISABLED, IDB_HICOLOR_TOOLBAR_HOT);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// create find toolbar
	if (!m_wndFindToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | CBRS_ALIGN_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||			WS_VISIBLE | 
		!m_wndFindToolBar.LoadToolBar(IDR_FIND))
	{
		TRACE0("Failed to create find toolbar\n");
		return -1;      // fail to create
	}	
	m_wndFindToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndFindToolBar, AFX_IDW_DOCKBAR_BOTTOM);
	m_wndFindToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	// add ComboBox to Toolbar
	if (!m_wndFindToolBar.CreateComboBox(m_wndFindToolBar.m_comboBox1, 0, ID_FINDCOMBO, 200, 150))
	{
		TRACE0("Failed to create toolbar's combo box\n");
		return -1;      // fail to create
	}

	// create plugin toolbar
	if (!m_wndPluginToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_VISIBLE | WS_CHILD | CBRS_ALIGN_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndPluginToolBar.LoadToolBar(IDR_PLUGIN))
	{
		TRACE0("Failed to create plugin toolbar\n");
		return -1;      // fail to create
	}	
	ShowPlugins("");
	m_wndPluginToolBar.EnableDocking(CBRS_ALIGN_ANY);
	DockControlBarLeftOf(&m_wndPluginToolBar, &m_wndToolBar);
	m_wndPluginToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
*/
	// create status bar
	if (!m_wndStatusBar.Create(this)/* ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT))*/)
	{
		TRACE0("Fehler beim Erstellen der Statusleiste.\n");
		return -1;      // fail to create
	}
	//m_wndStatusBar.SetPaneInfo(1, ID_SALDO, SBPS_NORMAL, 300);
	//m_wndStatusBar.SetPaneStyle(SBPS_STRETCH | SBPS_NOBORDERS);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUS, "Bereit", TRUE), "Status");
	SetTimer(1, 1, NULL);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_SALDO, ""), "Saldo");
		
	return 0;
}


BOOL CMainFrame::CreateRibbonBar()
{
	if (!m_wndRibbonBar.Create(this))
	{
		return FALSE;
	}
	//return m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	// alter programmatischer Aufbau des Ribbons aus VS 2008:

	// Load panel images:
	m_PanelImages16.SetImageSize(CSize (16, 16));
	m_PanelImages16.Load(IDB_TOOLBAR_16);
	m_PanelImages32.SetImageSize(CSize (32, 32));
	m_PanelImages32.Load(IDB_TOOLBAR_32);

	// Add main panel:
	Add_MainPanel ();

	Add_Category1();
	Add_Category2();

	// Add quick access toolbar commands:
	Add_QAT ();

	// Add "Style" button to the right of tabs:
	CMFCRibbonButton* pStyleButton = new CMFCRibbonButton ((UINT)-1, _T("Style\ns"), -1, -1);
	pStyleButton->SetMenu (IDR_THEME_MENU, TRUE);  // Right align
	m_wndRibbonBar.AddToTabs (pStyleButton);

	// Add "About" button to the right of tabs:
	m_wndRibbonBar.AddToTabs (new CMFCRibbonButton (ID_APP_ABOUT, _T("Info\ni"), m_PanelImages32.ExtractIcon(13), 0, m_PanelImages16.ExtractIcon(13)));

	// m_wndRibbonBar.SaveToXMLFile("res\\ribbon.mfcribbon-ms");

	return TRUE;

}

void CMainFrame::Add_MainPanel()
{
	m_MainButton.SetImage (IDB_EASYCASH);
	m_MainButton.SetToolTipText (_T("Datei"));
	m_MainButton.SetText (_T("\nd"));

	m_wndRibbonBar.SetApplicationButton (&m_MainButton, CSize (45, 45));

	CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory (_T("Datei"), IDB_TOOLBAR_16, IDB_TOOLBAR_32);
	
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_NEW, "&Neu\nStrg+N", 0, 0));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_OPEN, "Ö&ffnen...\nStrg+O", 1, 1));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_SAVE, "&Speichern\nStrg+S", 2, 2));

	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_SAVE_AS, "Speichern &unter\nStrg+U", 2, 2));

	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_EXPORT, "&Export...", 26, 26));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_JAHRESWECHSEL, "&Jahreswechsel", 27, 27));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_BACKUP_SUBFOLDER, "Datensicherung in &Unterverzeichnis", 3, 3));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_BACKUP_EXTERNAL, "Datensicherung auf externes &Laufwerk", 3, 3));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_BACKUP, "Datens&icherung auf CD/DVD", 3, 3));

	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_MANDANTEN, "&Mandanten...", 4, 4));	
	
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_REGISTRIERUNGSINFORMATIONEN_WIEDERHERSTELLEN, "Registrierungsinformationen &wiederherstellen...", 28, 28));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_REGISTRIERUNGSINFORMATIONEN_SICHERN, "&Registrierungsinformationen sichern...", 29, 29));

	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_OEFFNE_DATENVERZEICHNIS, "Öffne &Datenverzeichnis mit Explorer", 30, 30));
	pMainPanel->Add (new CMFCRibbonButton (ID_FILE_WAEHLE_DATENVERZEICHNIS, "&Wähle neues Datenverzeichnis aus", 31, 31));

	
	//pMainPanel->Add (new CMFCRibbonButton (ID_EDIT_EINNAHME_BUCHEN, "&Einnahme buchen\nStrg + E", 5, 5));
	//pMainPanel->Add (new CMFCRibbonButton (ID_EDIT_AUSGABE_BUCHEN, "&Ausgabe buchen\nStrg + A", 6, 6));
	//pMainPanel->Add (new CMFCRibbonButton (ID_EDIT_DAUERBUCHUNGEN_EINGEBEN, "&Dauerbuchungen eingeben", 7, 7));
	//pMainPanel->Add (new CMFCRibbonButton (ID_EDIT_DAUERBUCHUNGEN_AUSFUEHREN, "Dauerbuchungen a&usführen\nStrg + D", 8, 8));
	//pMainPanel->Add (new CMFCRibbonButton (ID_FILE_PRINT, "&Drucken...\nStrg+P", 9, 9));
	//pMainPanel->Add (new CMFCRibbonButton (ID_FILE_PRINT_PREVIEW, "Sei&tenansicht", 10, 10));
	//pMainPanel->Add (new CMFCRibbonButton (ID_VIEW_JOURNAL_SWITCH, "Durch &Journalansicht rotieren", 11, 11));
	//pMainPanel->Add (new CMFCRibbonButton (ID_VIEW_OPTIONS, "&Einstellungen", 12, 12));

	m_pInfoButton = new CMFCRibbonButton(ID_APP_ABOUT, "Inf&o über EasyCash...", 13, 13);
	pMainPanel->Add(m_pInfoButton);

/*
	std::auto_ptr<CMFCRibbonButton> apBtnPrint(new CMFCRibbonButton (ID_FILE_PRINT, _T("&Print"), 4, 4));
	apBtnPrint->AddSubItem (new CMFCRibbonLabel (_T("Preview and print the document")));
	apBtnPrint->AddSubItem (new CMFCRibbonButton (ID_FILE_PRINT, _T("&Print"), 4, 4, TRUE));
	apBtnPrint->AddSubItem (new CMFCRibbonButton (ID_FILE_PRINT_DIRECT, _T("&Quick Print"), 6, 6, TRUE));
	apBtnPrint->AddSubItem (new CMFCRibbonButton (ID_FILE_PRINT_PREVIEW, _T("Print Pre&view"), 7, 7, TRUE));
	apBtnPrint->SetKeys (_T("p"), _T("w"));
	pMainPanel->Add (apBtnPrint.release());
*/
	pMainPanel->AddRecentFilesList (_T("Zuletzt bearbeitete Buchungsjahre"));

	pMainPanel->AddToBottom (new CMFCRibbonMainPanelButton (ID_APP_EXIT, "Be&enden", 32));

}

void CMainFrame::Add_QAT ()
{
	CMFCRibbonQuickAccessToolBarDefaultState qatState;
	qatState.AddCommand(ID_FILE_NEW, FALSE);
	qatState.AddCommand(ID_FILE_OPEN, FALSE);
	qatState.AddCommand(ID_FILE_SAVE);
	qatState.AddCommand(ID_FILE_SAVE_AS, FALSE);
	qatState.AddCommand(ID_FILE_EXPORT, FALSE);
	qatState.AddCommand(ID_FILE_BACKUP, FALSE);
	qatState.AddCommand(ID_FILE_JAHRESWECHSEL, FALSE);
	qatState.AddCommand(ID_FILE_MANDANTEN, FALSE);
	qatState.AddCommand(ID_FILE_REGISTRIERUNGSINFORMATIONEN_WIEDERHERSTELLEN, FALSE);
	qatState.AddCommand(ID_FILE_REGISTRIERUNGSINFORMATIONEN_SICHERN, FALSE);
	qatState.AddCommand(ID_FILE_OEFFNE_DATENVERZEICHNIS, FALSE);
	qatState.AddCommand(ID_FILE_WAEHLE_DATENVERZEICHNIS, FALSE);
	qatState.AddCommand(ID_APP_ABOUT, FALSE);
	qatState.AddCommand(ID_EDIT_EINNAHME_BUCHEN, FALSE);
	qatState.AddCommand(ID_EDIT_AUSGABE_BUCHEN, FALSE);
	qatState.AddCommand(ID_EDIT_DAUERBUCHUNGEN_EINGEBEN, FALSE);
	qatState.AddCommand(ID_EDIT_DAUERBUCHUNGEN_AUSFUEHREN, FALSE);
	qatState.AddCommand(ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_SWITCH, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_DATUM, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_KONTEN, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_BESTKONTEN, FALSE);
	qatState.AddCommand(ID_VIEW_OPTIONS);
	qatState.AddCommand(ID_ANSICHT_FORMULARE, FALSE);
	qatState.AddCommand(ID_FORMULAR_INFO, FALSE);
	qatState.AddCommand(ID_FORMULAR_NEU, FALSE);
	qatState.AddCommand(ID_FILE_PLUGINMANAGER, FALSE);
	qatState.AddCommand(ID_NEXT, FALSE);
	qatState.AddCommand(ID_PREV, FALSE);
	qatState.AddCommand(ID_FILE_PRINT);
	qatState.AddCommand(ID_FILE_PRINT_PREVIEW, FALSE);
	qatState.AddCommand(ID_FILE_PRINT_SETUP, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_MONAT, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_KONTO, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_BETRIEB, FALSE);
	qatState.AddCommand(ID_VIEW_JOURNAL_BESTANDSKONTO, FALSE);
	qatState.AddCommand(ID_VIEW_OPTIONS, FALSE);
	m_wndRibbonBar.SetQuickAccessDefaultState (qatState);
}

void CMainFrame::Add_Category1()
{
	CMFCRibbonCategory* pCategory = m_wndRibbonBar.AddCategory(_T("&Start"), IDB_TOOLBAR_16, IDB_TOOLBAR_32);

	CMFCRibbonPanel* pPanel1 = pCategory->AddPanel(_T("Buchen\nb"));

	std::auto_ptr<CMFCRibbonButton> apBtn1(new CMFCRibbonButton(ID_EDIT_EINNAHME_BUCHEN, "Einnahme\ne", 5, 5));
	apBtn1->SetAlwaysLargeImage();
	pPanel1->Add(apBtn1.release());

	std::auto_ptr<CMFCRibbonButton> apBtn2(new CMFCRibbonButton(ID_EDIT_AUSGABE_BUCHEN, "Ausgabe\na", 6, 6));
	apBtn2->SetAlwaysLargeImage();
	pPanel1->Add(apBtn2.release());

	std::auto_ptr<CMFCRibbonButton> apBtn3(new CMFCRibbonButton(ID_EDIT_DAUERBUCHUNGEN_EINGEBEN, "Dauerbuch. eingeben\ng", 7, 7));
	apBtn3->SetAlwaysLargeImage();
	pPanel1->Add(apBtn3.release());

	std::auto_ptr<CMFCRibbonButton> apBtn4(new CMFCRibbonButton(ID_EDIT_DAUERBUCHUNGEN_AUSFUEHREN, "Dauerbuch. ausführen\nn", 8, 8));
	apBtn4->SetMenu(IDR_DAUERBUCHUNGEN_AUSFUEHREN_MONATE, TRUE);
	apBtn4->SetAlwaysLargeImage();
	pPanel1->Add(apBtn4.release());

	std::auto_ptr<CMFCRibbonButton> apBtn5(new CMFCRibbonButton(ID_EDIT_UMSATZSTEUERVORAUSZAHLUNGEN, "USt-Voraus- zahlungen\ns", 22, 22));
	apBtn5->SetAlwaysLargeImage();
	pPanel1->Add(apBtn5.release());

	CMFCRibbonPanel* pPanel2 = pCategory->AddPanel(_T("Ansicht\nA"));

	std::auto_ptr<CMFCRibbonButton> apBtn20(new CMFCRibbonButton(ID_VIEW_JOURNAL_SWITCH, "Journal\nj", 11, 11));
	apBtn20->SetMenu(IDR_JOURNALANSICHT, TRUE);
	apBtn20->SetAlwaysLargeImage();
	pPanel2->Add(apBtn20.release());
	
	m_pAnsichtFormulareButton = new CMFCRibbonButton(ID_ANSICHT_FORMULARE, "Formular\nf", 21, 21);
	m_pAnsichtFormulareButton->SetMenu(IDR_ANSICHT_FORMULARE, TRUE);
	m_pAnsichtFormulareButton->RemoveAllSubItems();
	m_pAnsichtFormulareButton->SetAlwaysLargeImage();
	pPanel2->Add(m_pAnsichtFormulareButton);

	std::auto_ptr<CMFCRibbonButton> apBtn22(new CMFCRibbonButton(ID_VIEW_ZOOM_SWITCH, "Zoom\nz", 34, 34));
	apBtn22->SetMenu(IDR_ZOOMFAKTOR, TRUE);
	apBtn22->SetAlwaysLargeImage();
	pPanel2->Add(apBtn22.release());

	m_pAnsichtPluginsButton = new CMFCRibbonButton(ID_FILE_PLUGINMANAGER, "Plugin\np", 25, 25);
	m_pAnsichtPluginsButton->SetMenu(IDR_ANSICHT_PLUGINS, TRUE);
	m_pAnsichtPluginsButton->RemoveAllSubItems();
	m_pAnsichtPluginsButton->SetAlwaysLargeImage();
	pPanel2->Add(m_pAnsichtPluginsButton);
	ShowPluginsRibbonMenu(m_pAnsichtPluginsButton);

	std::auto_ptr<CMFCRibbonButton> apBtn21(new CMFCRibbonButton(ID_FILE_PRINT2, "Druck\nd", 9, 9));
	apBtn21->SetMenu(IDR_JOURNALDRUCK, TRUE);
	apBtn21->SetAlwaysLargeImage();
	pPanel2->Add(apBtn21.release());

	CMFCRibbonPanel* pPanel3 = pCategory->AddPanel(_T("Filter\nf"));

	std::auto_ptr<CMFCRibbonButton> apBtn7(new CMFCRibbonButton(ID_VIEW_JOURNAL_MONAT, "Monat\nm", 15, 15));
	apBtn7->SetMenu(IDR_FILTER_MONAT, TRUE);
	apBtn7->SetAlwaysLargeImage();
	pPanel3->Add(apBtn7.release());

	m_pFilterKontoButton = new CMFCRibbonButton(ID_VIEW_JOURNAL_KONTO, "Konto\nk", 16, 16);
	m_pFilterKontoButton->SetMenu(IDR_FILTER_KONTO, TRUE);
	m_pFilterKontoButton->RemoveAllSubItems();
	m_pFilterKontoButton->SetAlwaysLargeImage();
	pPanel3->Add(m_pFilterKontoButton);

	m_pFilterBetriebButton = new CMFCRibbonButton(ID_VIEW_JOURNAL_BETRIEB, "Betrieb\nt", 18, 18);
	m_pFilterBetriebButton->SetMenu(IDR_FILTER_KONTO, TRUE);
	m_pFilterBetriebButton->RemoveAllSubItems();
	m_pFilterBetriebButton->SetAlwaysLargeImage();
	pPanel3->Add(m_pFilterBetriebButton);

	m_pFilterBestandskontoButton = new CMFCRibbonButton(ID_VIEW_JOURNAL_BESTANDSKONTO, "Bestands- konto\no", 17, 17);
	m_pFilterBestandskontoButton->SetMenu(IDR_FILTER_KONTO, TRUE);
	m_pFilterBestandskontoButton->RemoveAllSubItems();
	m_pFilterBestandskontoButton->SetAlwaysLargeImage();
	pPanel3->Add(m_pFilterBestandskontoButton);

	CMFCRibbonPanel* pPanel4 = pCategory->AddPanel(_T("Suche\ns"));

	m_pSucheCombobox = new CMFCRibbonComboBox(ID_VIEW_FINDTOOLBAR, TRUE, 75);
	pPanel4->Add(m_pSucheCombobox);
	std::auto_ptr<CMFCRibbonButton> apBtn40(new CMFCRibbonButton(ID_NEXT, "vorwärts\nv", 19));
	apBtn40->SetDefaultCommand();
	pPanel4->Add(apBtn40.release());
	pPanel4->Add(new CMFCRibbonButton(ID_PREV, "rückwärts\nr", 20));
}

void CMainFrame::Add_Category2()
{
	CMFCRibbonCategory* pCategory = m_wndRibbonBar.AddCategory(_T("&Optionen"), IDB_TOOLBAR_EINSTELLUNGEN_16, IDB_TOOLBAR_EINSTELLUNGEN_32);

	CMFCRibbonPanel* pPanel1 = pCategory->AddPanel(_T("Programm\ne"));

	std::auto_ptr<CMFCRibbonButton> apBtn1(new CMFCRibbonButton(ID_VIEW_OPTIONS, "Einstellungen\np", 0, 0));
	apBtn1->SetAlwaysLargeImage();
	pPanel1->Add(apBtn1.release());

	std::auto_ptr<CMFCRibbonButton> apBtn2(new CMFCRibbonButton(ID_APP_ABOUT, "Info\nI", 5, 5));
	apBtn2->SetAlwaysLargeImage();
	pPanel1->Add(apBtn2.release());

	CMFCRibbonPanel* pPanel2 = pCategory->AddPanel(_T("Oberflächen-Style\nO"));

	pPanel2->Add(new CMFCRibbonCheckBox(ID_VIEW_APPLOOK_2007_1, _T("Onyx")));
	pPanel2->Add(new CMFCRibbonCheckBox(ID_VIEW_APPLOOK_2007_2, _T("Silber")));
	pPanel2->Add(new CMFCRibbonCheckBox(ID_VIEW_APPLOOK_2007_3, _T("Aqua")));
	pPanel2->Add(new CMFCRibbonCheckBox(ID_VIEW_APPLOOK_2007, _T("Luna")));
	pPanel2->Add(new CMFCRibbonCheckBox(ID_VIEW_APPLOOK_2003, _T("Blau")));
	pPanel2->Add(new CMFCRibbonCheckBox(ID_VIEW_APPLOOK_2005, _T("Grau")));

	CMFCRibbonPanel* pPanel3 = pCategory->AddPanel(_T("Tipp des Tages\nT"));

	pPanel3->Add(new CMFCRibbonButton(ID_TIPP_DES_TAGES_JETZT, _T("Tipp jetzt anzeigen"), 1));
	pPanel3->Add(new CMFCRibbonCheckBox(ID_TIPP_DES_TAGES_PROGRAMMSTART, _T("bei Programmstart")));
	pPanel3->Add(new CMFCRibbonLinkCtrl(ID_TIPP_DES_TAGES_EMAIL, _T("Tipp vorschlagen"), _T("mailto:thomas@mielke.software?subject=TIPP%20DES%20TAGES%20VORSCHLAG")));

	CMFCRibbonPanel* pPanel4 = pCategory->AddPanel(_T("Hilfe\nH"));

	pPanel4->Add(new CMFCRibbonLinkCtrl(ID_EINSTELLUNGEN_DOKU, _T("Online-Dokumentation"), _T("https://www.easyct.de/articles.php?cat_id=2")));
	pPanel4->Add(new CMFCRibbonLinkCtrl(ID_EINSTELLUNGEN_FAQ, _T("Häufig gestellte Fragen"), _T("https://www.easyct.de/faq.php")));
	pPanel4->Add(new CMFCRibbonLinkCtrl(ID_EINSTELLUNGEN_BUGTRACKER, _T("Fehler/Anregung melden"), _T("https://www.easyct.de/tracker/login_page.php")));

	CMFCRibbonPanel* pPanel5 = pCategory->AddPanel(_T("Kontakt\nK"));

	pPanel5->Add(new CMFCRibbonLinkCtrl(ID_EINSTELLUNGEN_EMAIL, _T("E-Mail an den Autor"), _T("mailto:thomas@mielke.software?subject=ECT-Kontakt")));
	pPanel5->Add(new CMFCRibbonLinkCtrl(ID_EINSTELLUNGEN_PGP, _T("PGP-Schlüssel holen"), _T("https://mielke.software/D8AE2CE41CB1D1A61087165B95DC1917252AD305.php")));
	pPanel5->Add(new CMFCRibbonLinkCtrl(ID_EINSTELLUNGEN_KONTAKT, _T("Kontaktformular"), _T("https://www.easyct.de/contact.php")));
	
	CMFCRibbonPanel* pPanel6 = pCategory->AddPanel(_T("Spende via\nS"));
	pPanel6->Add(new CMFCRibbonButton(ID_EINSTELLUNGEN_SPENDE_BANK, _T("Banküber- weisung"), 2, 2));
	pPanel6->Add(new CMFCRibbonButton(ID_EINSTELLUNGEN_SPENDE_PAYPAL, _T("PayPal"), 3, 3));
	pPanel6->Add(new CMFCRibbonButton(ID_EINSTELLUNGEN_SPENDE_BITCOIN, _T("Bitcoin"), 4, 4));
	pPanel6->Add(new CMFCRibbonButton(ID_EINSTELLUNGEN_SPENDE_FLATTR, _T("Flattr"), 6, 6));
}

void CMainFrame::DockControlBarLeftOf(CToolBar* Bar, CToolBar* LeftOf)
{
	CRect rect;
	DWORD dw;
	UINT n;
	
	// get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout(TRUE);
	
	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1,0);
	dw=LeftOf->GetBarStyle();
	n = 0;
	n = (dw&CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw&CBRS_ALIGN_BOTTOM && n==0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw&CBRS_ALIGN_LEFT && n==0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw&CBRS_ALIGN_RIGHT && n==0) ? AFX_IDW_DOCKBAR_RIGHT : n;
	
	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line. By calculating a rectangle, we
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar,n,&rect);
}

// ---- PLUGINS ----


void CMainFrame::ShowPlugins(char *szName)
{
/*	// kein Name angegeben?
	if (!*szName)
	{	// alle keys enumerieren

		// Separator nach Pluginmanager-Button:
		TBBUTTON tbb;
		tbb.iBitmap = NULL;
		tbb.idCommand = 0;
		tbb.fsState = 0;
		tbb.fsStyle = TBSTYLE_SEP;
		m_wndPluginToolBar.GetToolBarCtrl().AddButtons(1, &tbb);	

		HKEY hKey;
		char subkey[1000];

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Tools\\EasyCash\\Plugins", 0L, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
		{
			long lCb;	
			int i;
			for (i = 0; RegEnumKey(hKey, i, subkey, lCb = sizeof(subkey)) == ERROR_SUCCESS; i++)
			{
				if (*subkey) ShowPlugins(subkey);	// rekursiver Aufruf für alle Subkeys			
			}

			RegCloseKey(hKey);
		}
		
		CPluginElement *pElement;
		int i, n;
		for (n = 0, pElement = m_pPlugins; pElement; pElement = pElement->next, n++)	// wie viele Elemente sind in der Plugin-Liste?
			;

		n++;	// Platz für Plugin Manager:
		n++;	// Platz für Separator
		UINT *plugintoolbar_ids = new UINT[n];		// entsprechend großes Array anlegen	
		
		plugintoolbar_ids[0] = ID_FILE_PLUGINMANAGER;
		plugintoolbar_ids[1] = ID_SEPARATOR;

		for (i = 2, pElement = m_pPlugins; i < n && pElement; pElement = pElement->next, i++)	// IDs aus Plugin-Liste ins Array kopieren
			plugintoolbar_ids[i] = pElement->id;
		m_wndPluginToolBar.SetButtons(plugintoolbar_ids, n);	// Toolbar updaten
		delete plugintoolbar_ids;
	}
	else
	{	// sonst das spezifizierte plugin anzeigen
		HKEY hKey;
		CString csName, szBeschreibung, csAufruf, csBitmapDateiname, csTyp;

		CString csPlugin = (CString)"Software\\Tools\\EasyCash\\Plugins\\" + (CString)szName;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, csPlugin, &hKey) == ERROR_SUCCESS)
		{
			long lRetCode, lDummy;
			long lType = REG_SZ;
			long lCb = 1000;
			lDummy = 0L;
			
			lRetCode = RegQueryValueEx(hKey, TEXT("Name"), NULL, (ULONG *)&lType, (LPBYTE)csName.GetBuffer(1000), (ULONG *)&lCb);
			if (lRetCode == ERROR_SUCCESS)
			{
				lCb = 1000;
				lRetCode = RegQueryValueEx(hKey, TEXT("Beschreibung"), NULL, (ULONG *)&lType, (LPBYTE)szBeschreibung.GetBuffer(1000), (ULONG *)&lCb);
				if (lRetCode == ERROR_SUCCESS) 
				{
					lCb = 1000;
					lRetCode = RegQueryValueEx(hKey, TEXT("Aufruf"), NULL, (ULONG *)&lType, (LPBYTE)csAufruf.GetBuffer(1000), (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) 
					{
						lCb = 1000;
						lRetCode = RegQueryValueEx(hKey, TEXT("Bitmap"), NULL, (ULONG *)&lType, (LPBYTE)csBitmapDateiname.GetBuffer(1000), (ULONG *)&lCb);
						if (lRetCode == ERROR_SUCCESS) 
						{
							lCb = 1000;
							lRetCode = RegQueryValueEx(hKey, TEXT("Typ"), NULL, (ULONG *)&lType, (LPBYTE)csTyp.GetBuffer(1000), (ULONG *)&lCb);
							if (lRetCode != ERROR_SUCCESS) 
							{
								csTyp = "standard";
							}
							csName.ReleaseBuffer(); 
							szBeschreibung.ReleaseBuffer();
							csAufruf.ReleaseBuffer(); 
							csBitmapDateiname.ReleaseBuffer();
							csTyp.ReleaseBuffer();

							// neues Plugin speichern
							int id_incrementor = ID_CMD_PLUGIN_BASE;
							CPluginElement **ppTemp = &m_pPlugins;
							while (*ppTemp)	// Ende der Kette und des ID-Bereichs suchen
							{	
								if ((*ppTemp)->id >= id_incrementor) id_incrementor++;	// muss immer um 1 größer sein als alle bestehenden IDs
								ppTemp = &((*ppTemp)->next);
							}
							*ppTemp = new CPluginElement;
							(*ppTemp)->name = (CString)szName;
							(*ppTemp)->beschreibung = (CString)szBeschreibung;
							(*ppTemp)->aufruf = csAufruf;
							LoadBitmap(&(*ppTemp)->bitmap, csBitmapDateiname.GetBuffer(0));
							(*ppTemp)->id = id_incrementor;
							(*ppTemp)->typ = csTyp;
							(*ppTemp)->typ.MakeLower();
							(*ppTemp)->next = NULL;
							
							int nBitmapPos = m_wndPluginToolBar.GetToolBarCtrl().AddBitmap(1, (*ppTemp)->bitmap);
							if (nBitmapPos >= 0 )
							{
								TBBUTTON tbb;
								tbb.iBitmap = nBitmapPos;
								tbb.idCommand = (*ppTemp)->id;
								tbb.fsState = TBSTATE_ENABLED|TBSTYLE_AUTOSIZE ;
								tbb.fsStyle = TBSTYLE_GROUP;
//								m_wndPluginToolBar.GetToolBarCtrl().AddButtons(1, &tbb);
								m_wndPluginToolBar.GetToolBarCtrl().AddStrings((*ppTemp)->beschreibung);
							}

							m_nPluginCount++;
						}
					}
				}
			}

			RegCloseKey(hKey);
		}
	}	
*/}


void CMainFrame::ShowPluginsRibbonMenu(CMFCRibbonButton *pAnsichtPluginsButton, char *szName, int nIndex)
{
	if (!pAnsichtPluginsButton) return;

	// kein Name angegeben?
	if (!szName)
	{	// alle keys enumerieren
		pAnsichtPluginsButton->RemoveAllSubItems();	// Dummy von Ribbon-Button Menü entfernen

		HKEY hKey;
		char subkey[1000];

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Tools\\EasyCash\\Plugins", 0L, KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
		{
			long lCb;	
			int i;
			for (i = 0; RegEnumKey(hKey, i, subkey, lCb = sizeof(subkey)) == ERROR_SUCCESS; i++)
			{
				if (*subkey) ShowPluginsRibbonMenu(pAnsichtPluginsButton, subkey, i);	// rekursiver Aufruf für alle Subkeys			
			}

			RegCloseKey(hKey);
		}
	}
	else
	{	// sonst das spezifizierte plugin anzeigen
		HKEY hKey;
		CString csName, szBeschreibung, csAufruf, csBitmapDateiname, csTyp;

		CString csPlugin = (CString)"Software\\Tools\\EasyCash\\Plugins\\" + (CString)szName;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, csPlugin, &hKey) == ERROR_SUCCESS)
		{
			long lRetCode, lDummy;
			long lType = REG_SZ;
			long lCb = 1000;
			lDummy = 0L;
			
			lRetCode = RegQueryValueEx(hKey, TEXT("Name"), NULL, (ULONG *)&lType, (LPBYTE)csName.GetBuffer(1000), (ULONG *)&lCb);
			if (lRetCode == ERROR_SUCCESS)
			{
				lCb = 1000;
				lRetCode = RegQueryValueEx(hKey, TEXT("Beschreibung"), NULL, (ULONG *)&lType, (LPBYTE)szBeschreibung.GetBuffer(1000), (ULONG *)&lCb);
				if (lRetCode == ERROR_SUCCESS) 
				{
					lCb = 1000;
					lRetCode = RegQueryValueEx(hKey, TEXT("Aufruf"), NULL, (ULONG *)&lType, (LPBYTE)csAufruf.GetBuffer(1000), (ULONG *)&lCb);
					if (lRetCode == ERROR_SUCCESS) 
					{
						lCb = 1000;
						lRetCode = RegQueryValueEx(hKey, TEXT("Bitmap"), NULL, (ULONG *)&lType, (LPBYTE)csBitmapDateiname.GetBuffer(1000), (ULONG *)&lCb);
						if (lRetCode == ERROR_SUCCESS) 
						{
							lCb = 1000;
							lRetCode = RegQueryValueEx(hKey, TEXT("Typ"), NULL, (ULONG *)&lType, (LPBYTE)csTyp.GetBuffer(1000), (ULONG *)&lCb);
							if (lRetCode != ERROR_SUCCESS) 
							{
								csTyp = "standard";
							}
							else
								csTyp.ReleaseBuffer();
							csName.ReleaseBuffer(); 
							szBeschreibung.ReleaseBuffer();
							csAufruf.ReleaseBuffer(); 
							csBitmapDateiname.ReleaseBuffer();

							// neues Plugin speichern
							int id_incrementor = ID_CMD_PLUGIN_BASE;
							CPluginElement **ppTemp = &m_pPlugins;
							while (*ppTemp)	// Ende der Kette und des ID-Bereichs suchen
							{	
								if ((*ppTemp)->id >= id_incrementor) id_incrementor++;	// muss immer um 1 größer sein als alle bestehenden IDs
								ppTemp = &((*ppTemp)->next);
							}
							*ppTemp = new CPluginElement;
							(*ppTemp)->name = (CString)szName;
							(*ppTemp)->beschreibung = (CString)szBeschreibung;
							(*ppTemp)->aufruf = csAufruf;
							LoadBitmap(&(*ppTemp)->bitmap, csBitmapDateiname.GetBuffer(0));
							if ((*ppTemp)->bitmap)
								(*ppTemp)->icon = HICONFromCBitmap(*(*ppTemp)->bitmap);
							else
								(*ppTemp)->icon = m_PanelImages32.ExtractIcon(25);
							(*ppTemp)->id = id_incrementor;
							(*ppTemp)->typ = csTyp;
							(*ppTemp)->typ.MakeLower();
							(*ppTemp)->next = NULL;
							
							std::auto_ptr<CMFCRibbonButton> apBtn(new CMFCRibbonButton((*ppTemp)->id, (LPCTSTR)(*ppTemp)->name, (*ppTemp)->icon));
							apBtn->SetToolTipText((*ppTemp)->beschreibung);
							pAnsichtPluginsButton->AddSubItem(apBtn.release());

							m_nPluginCount++;
						}
					}
				}
			}

			RegCloseKey(hKey);
		}
	}
}

void CMainFrame::LoadBitmap(CBitmap **ppcbm, char* filename)
{
	extern int search_formats(char *);
	int format = search_formats(filename+strlen(filename)-3);

	::CImage *image = new ::CImage(filename, format);
	if (image) 
		*ppcbm = image->MakeBitmap();
	else 
		*ppcbm = NULL;
	delete image;
}

// ---- WEITERES ----

void CMainFrame::SetStatus(LPCTSTR status)
{
	if (!strcmp(status, "Bereit")) return; // "Bereit" unterdrücken
	if (m_wndStatusBar) m_wndStatusBar.SetInformation(status);	// bei WM_COMMAND m_wndStatusBar.SetInformation(NULL) !!!
	//m_bInformationModeElapsed = FALSE;
	//this->SetTimer(0, 10000, NULL); 
}


void CMainFrame::UpdateSaldo()
{
	CMDIChildWndEx *pChild = (CMDIChildWndEx *)GetActiveFrame();
	CEasyCashView *pecv = (CEasyCashView*)pChild->GetActiveView();
	UpdateSaldo(pecv);
}

void CMainFrame::UpdateSaldo(CEasyCashView *pecv)
{
	CMDIChildWndEx *pChild = (CMDIChildWndEx *)GetActiveFrame();
	BOOL nMonatsFilterDisplay = FALSE;
	int saldo = 0, nettosaldo = 0;
	char buffer[100];
	char buffer2[100];
	CString cs;
	int nJahr = 0;
	if (pecv && (pecv->IsKindOf( RUNTIME_CLASS( CEasyCashView ) ) || pecv->IsKindOf( RUNTIME_CLASS( CScrollView ) )))
	{
		CEasyCashDoc* pDoc = pecv->GetDocument();

		nMonatsFilterDisplay = pecv->m_MonatsFilterDisplay;
		nJahr = pDoc->nJahr;

		// Saldo ist die Differenz aus Ausgaben und Einnahmen
		if (pecv->einstellungen4)
		{
			if (pecv->einstellungen4->m_finanzamtserstattungen != "" || pecv->einstellungen4->m_finanzamtszahlungen != "")
			{
				saldo = pDoc->EinnahmenSumme(pecv->m_MonatsFilterDisplay, pecv->einstellungen4->m_finanzamtserstattungen) 
						- pDoc->AusgabenSumme(pecv->m_MonatsFilterDisplay, pecv->einstellungen4->m_finanzamtszahlungen);
			
				nettosaldo = pDoc->EinnahmenSummeNetto(pecv->m_MonatsFilterDisplay, pecv->einstellungen4->m_finanzamtserstattungen) 
							 - pDoc->AusgabenSummeNetto(pecv->m_MonatsFilterDisplay, pecv->einstellungen4->m_finanzamtszahlungen);
			}
			else
				nettosaldo = saldo = pDoc->EinnahmenSumme(pecv->m_MonatsFilterDisplay) - pDoc->AusgabenSumme(pecv->m_MonatsFilterDisplay);
		}							 
		else
			nettosaldo = saldo = 0;
	}

	// Saldo in Dialogbox darstellen
	int_to_currency_tausenderpunkt(saldo, 4, buffer);
	int_to_currency_tausenderpunkt(nettosaldo, 4, buffer2);
	if (nettosaldo == saldo)
	{
		if (nMonatsFilterDisplay)
			cs.Format("Saldo für %d%s/%04d: %s", nMonatsFilterDisplay <= 12 ? nMonatsFilterDisplay : (nMonatsFilterDisplay-13)/3+1, nMonatsFilterDisplay <= 12 ? "" : "Q", nJahr, buffer);
		else
			cs.Format("Saldo für %04d: %s", nJahr, buffer);
	}
	else
	{
		if (nMonatsFilterDisplay)
			cs.Format("Saldo für %d%s/%04d (ohne Finanzamts-Zahlungen): %s (Netto) / %s (Brutto)", nMonatsFilterDisplay <= 12 ? nMonatsFilterDisplay : nMonatsFilterDisplay-12, nMonatsFilterDisplay <= 12 ? "" : "Q", nJahr, buffer2, buffer);
		else
			cs.Format("Saldo für %04d (ohne Finanzamts-Zahlungen): %s (Netto) / %s (Brutto)", nJahr, buffer2, buffer);
	}

	//pCmdUI->SetText((LPCSTR)cs);
	//((CMFCRibbonStatusBarPane*)m_wndStatusBar.FindByID(ID_SALDO))->SetText(cs);
	
	if (m_wndStatusBar)
	{
		m_wndStatusBar.GetExElement(0)->SetText(cs);
		m_wndStatusBar.Invalidate(true);
		m_wndStatusBar.RecalcLayout();
	}
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	RECT rScreen;
	GetDesktopWindow()->GetWindowRect(&rScreen);
	
	cs.x = theApp.GetProfileInt("Fenster", "FrameWndPosX", 0);
	cs.y = theApp.GetProfileInt("Fenster", "FrameWndPosY", 0);
	cs.cx = theApp.GetProfileInt("Fenster", "FrameWndSizeX", 0);
	cs.cy = theApp.GetProfileInt("Fenster", "FrameWndSizeY", 0);

	if (cs.x < rScreen.left) cs.x = rScreen.left;
	if (cs.x > rScreen.right-100) cs.x = rScreen.right-100;
	if (cs.y < rScreen.top) cs.y = rScreen.top;
	if (cs.y > rScreen.bottom-100) cs.y = rScreen.bottom-100;
	if (cs.cx > rScreen.right-rScreen.left || cs.cx <= 0) cs.cx = rScreen.right-rScreen.left;
	if (cs.cy > rScreen.bottom-rScreen.top || cs.cy <= 0) cs.cy = rScreen.bottom-rScreen.top;

	return CMDIFrameWndEx::PreCreateWindow(cs);
}

BOOL CMainFrame::CreateCaptionBar()
{//ID_VIEW_CAPTION_BAR
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_JOURNAL_SWITCH, -1, TRUE))
	{
		TRACE0("Failed to create caption bar\n");
		return FALSE;
	}

	BOOL bNameValid;

	CString strTemp, strTemp2;

	//bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
	//ASSERT(bNameValid);
	//m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);

	m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
	bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TIP);
	ASSERT(bNameValid);
	m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnAppLook(UINT id)
{
	CDockingManager::SetDockingMode(DT_SMART);

	m_nAppLook = id;

	CTabbedPane::m_StyleTabWnd = CMFCTabCtrl::STYLE_3D;

	switch (m_nAppLook)
	{
	case ID_VIEW_APPLOOK_2007:
	case ID_VIEW_APPLOOK_2007_1:
	case ID_VIEW_APPLOOK_2007_2:
	case ID_VIEW_APPLOOK_2007_3:
		// enable Office 2007 look:

		switch (m_nAppLook)
		{
		case ID_VIEW_APPLOOK_2007:
			CMFCVisualManagerOffice2007::SetStyle (CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_2007_1:
			CMFCVisualManagerOffice2007::SetStyle (CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_2007_2:
			CMFCVisualManagerOffice2007::SetStyle (CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_2007_3:
			CMFCVisualManagerOffice2007::SetStyle (CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

#if (_MSC_VER >= 1600)
		m_wndRibbonBar.SetWindows7Look(FALSE);
#endif
		CMFCVisualManager::SetDefaultManager (RUNTIME_CLASS (CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode (DT_SMART);
		break;


	case ID_VIEW_APPLOOK_2003:
#if (_MSC_VER >= 1600)
		m_wndRibbonBar.SetWindows7Look(FALSE);
#endif
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode (DT_SMART);
		break;

	case ID_VIEW_APPLOOK_2005:  // Windows7
#if (_MSC_VER >= 1600)
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
#endif
		CDockingManager::SetDockingMode (DT_SMART);
#if (_MSC_VER >= 1601)
		m_wndRibbonBar.SetWindows7Look(TRUE);
#endif
		break;

	default:
		ASSERT (FALSE);
	}

/*	if (m_pInfoButton)
	{
		m_pInfoButton->SetImageIndex(m_nAppLook == ID_VIEW_APPLOOK_2007_1 ? 1 : 13, FALSE);
		m_pInfoButton->SetImageIndex(m_nAppLook == ID_VIEW_APPLOOK_2007_1 ? 1 : 13, TRUE);
		m_pInfoButton->Redraw();   geht nicht :(
	}
*/
	CDockingManager* pDockManager = GetDockingManager ();
	if (pDockManager != NULL)
	{
		ASSERT_VALID (pDockManager);
		pDockManager->AdjustPaneFrames ();
	}

	CTabbedPane::ResetTabs ();

	RecalcLayout ();
	RedrawWindow (NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

	theApp.WriteInt (_T("ApplicationLook"), m_nAppLook);
}

void CMainFrame::OnUpdateAppLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(m_nAppLook == pCmdUI->m_nID);
}


void CMainFrame::OnFileMandanten() 
{
	CString Mandant0Existiert = theApp.GetProfileString("Mandanten", "Mandant00Datenverzeichnis", "");
	if (Mandant0Existiert.IsEmpty())
	{
		// Mandanten erzeugen, wenn nicht vorhanden
		if (AfxMessageBox("Mit dieser Funktion können Mandanten angelegt werden, d.h. Datenverzeichnisse für verschiedene Steuerpflichtige, die so z.B. auch verschiedene Kontenrahmen nutzen können. Wenn es darum geht, verschiedene Geschäftsbereiche durch separate Einnahme-Überschuss-Rechnungen abzubilden bitte den Menüpunkt 'Journal nur für Betrieb' im Ansicht-Menü wählen (wird enthalten sein ab v1.5X). Zunächst bitte ein Symbol auswählen, das den bisherigen Datenbestand als 'Mandant 1' repräsentiert...", MB_OKCANCEL) == IDCANCEL) return;

		// Icon wählen
		CIconAuswahlMandant dlg(NULL);
		dlg.DoModal();		
		CString csSelected;
		if (dlg.m_nSelected >= 0)
		{
			csSelected.Format("%d", dlg.m_nSelected);
			theApp.WriteProfileString("Mandanten", "Mandant00Name", "Mandant 1");
			theApp.WriteProfileString("Mandanten", "Mandant00Datenverzeichnis", theApp.GetProfileString("Allgemein", "Datenverzeichnis", ""));
			theApp.WriteProfileString("Mandanten", "Mandant00Icon", csSelected);

			SetMandant(0);
			if (AfxMessageBox("Sehr gut! Das bisherige Datenverzeichnis wurde mit dem neu erstellten Mandanten 'Mandant 1' verknüpft. Als nächstes im Mandanten-Auswahl Dialog bitte mit dem Knopf 'Neuer Mandant' einen oder mehrere weitere Mandanten anlegen und zum Schluss den Mandanten, mit dem Sie aktuell arbeiten wollen mit Doppelklick auf das Icon auswählen. Hinweis: Bitte benutzen Sie ein anderes Verzeichnis als das des ersten Mandanten. Jeder Mandant benötigt ein eigenes Verzeichnis.", MB_OKCANCEL) == IDCANCEL) return;
		}
		else 
			return;
	}

	{
		// Mandanten auswählen, wenn vorhanden
		CIconAuswahlMandant dlgIcon(NULL);
		{
			dlgIcon.m_nModus = 1;
			dlgIcon.DoModal();
			if (dlgIcon.m_nSelected == -1) return;
		}
		SetMandant(dlgIcon.m_nSelected);
		CString csKey, csDatenverzeichnis;
		csKey.Format("Mandant%-02.2dDatenverzeichnis", dlgIcon.m_nSelected);
		csDatenverzeichnis = theApp.GetProfileString("Mandanten", csKey, "");
		if (!csDatenverzeichnis.IsEmpty())
				theApp.WriteProfileString("Allgemein", "Datenverzeichnis", csDatenverzeichnis);
		
		char szIniFileName[500];
		strcpy(szIniFileName, csDatenverzeichnis.GetBuffer(0));
		strcat(szIniFileName, "\\easyct.ini");
		SetIniFileName(szIniFileName);	

		char last_file[500];
		GetPrivateProfileString("Allgemein", "LetzteDatei", "", last_file, sizeof(last_file), szIniFileName);
		if (*last_file != '\0')
			AfxGetApp()->OpenDocumentFile(last_file);
		else
			AfxMessageBox("Hinweis: Unter Menü->Datei->Neu kann jetzt eine neue Buchungsdatei angelegt werden.");
	}
}


BOOL CMainFrame::DestroyWindow() 
{
	RECT r;
	GetWindowRect(&r);


	theApp.WriteProfileInt("Fenster", "FrameWndPosX", r.left);
	theApp.WriteProfileInt("Fenster", "FrameWndPosY", r.top);
	theApp.WriteProfileInt("Fenster", "FrameWndSizeX", r.right - r.left);
	theApp.WriteProfileInt("Fenster", "FrameWndSizeY", r.bottom - r.top);
	
	return CMDIFrameWndEx::DestroyWindow();
}

void CMainFrame::OnInitMenu(CMenu* pMenu)
{
	CMDIFrameWndEx::OnInitMenu(pMenu);

	// CG: This block added by 'Tip of the Day' component.
	{
		// TODO: This code adds the "Tip of the Day" menu item
		// on the fly.  It may be removed after adding the menu
		// item to all applicable menu items using the resource
		// editor.

		// Add Tip of the Day menu item on the fly!
		static CMenu* pSubMenu = NULL;

		CString strHelp; strHelp.LoadString(CG_IDS_TIPOFTHEDAYHELP);
		CString strMenu;
		int nMenuCount = pMenu->GetMenuItemCount();
		BOOL bFound = FALSE;
		int i;
		for (i = 0; i < nMenuCount; i++) 
		{
			pMenu->GetMenuString(i, strMenu, MF_BYPOSITION);
			if (strMenu == strHelp)
			{ 
				pSubMenu = pMenu->GetSubMenu(i);
				bFound = TRUE;
				ASSERT(pSubMenu != NULL);
			}
		}

		CString strTipMenu;
		strTipMenu.LoadString(CG_IDS_TIPOFTHEDAYMENU);
		if (!bFound)
		{
			// Help menu is not available. Please add it!
			if (pSubMenu == NULL) 
			{
				// The same pop-up menu is shared between mainfrm and frame 
				// with the doc.
				static CMenu popUpMenu;
				pSubMenu = &popUpMenu;
				pSubMenu->CreatePopupMenu();
				pSubMenu->InsertMenu(0, MF_STRING|MF_BYPOSITION, 
					CG_IDS_TIPOFTHEDAY, strTipMenu);
			} 
			pMenu->AppendMenu(MF_STRING|MF_BYPOSITION|MF_ENABLED|MF_POPUP, 
				(UINT)pSubMenu->m_hMenu, strHelp);
			DrawMenuBar();
		} 
		else
		{      
			// Check to see if the Tip of the Day menu has already been added.
			pSubMenu->GetMenuString(0, strMenu, MF_BYPOSITION);

			if (strMenu != strTipMenu) 
			{
				// Tip of the Day submenu has not been added to the 
				// first position, so add it.
				pSubMenu->InsertMenu(0, MF_BYPOSITION);  // Separator
				pSubMenu->InsertMenu(0, MF_STRING|MF_BYPOSITION, 
					CG_IDS_TIPOFTHEDAY, strTipMenu);
			}
		}
	}

	// im Mandanten-Modus.alle .eca-Dateien im Mandantenverzeichnis im MRU Bereich aulisten
	{
		// on the fly ...
		CMenu* pSubMenu;
		int nMainMenuCount = pMenu->GetMenuItemCount();
		int iMainMenu;
		for (iMainMenu = 0; iMainMenu < nMainMenuCount; iMainMenu++)
		{
			char sMenuText[100];
			pMenu->GetMenuString(iMainMenu, sMenuText, sizeof(sMenuText), MF_BYPOSITION);
			if (!strcmp(sMenuText, "&Datei"))
			{
				pSubMenu = pMenu->GetSubMenu(iMainMenu);	// Datei-Menü
				if (pSubMenu)
				{
					int nSubMenuCount = pSubMenu->GetMenuItemCount();
					int i;
					for (i = 0; i < nSubMenuCount; i++) 
					{
						int xx = pSubMenu->GetMenuItemID(i);
						TRACE1("Menu ID: %d\r\n", xx);
						if (xx  >= ID_CMD_CUSTOM_MRU && xx < ID_CMD_CUSTOM_MRU + 1000 || xx == ID_FILE_OEFFNE_DATENVERZEICHNIS)	// MRU file 1 ob existiert
						{
							BOOL bAllesgut = TRUE;
							while (((xx=pSubMenu->GetMenuItemID(i))) >= ID_CMD_CUSTOM_MRU && xx < ID_CMD_CUSTOM_MRU + 1000)
							{
								if (!pSubMenu->RemoveMenu(i, MF_BYPOSITION))			// altes MRU/Verzeichnislisting entfernen
									bAllesgut = FALSE;
							}
							if (bAllesgut)
							{
								theApp.m_MandantenverzeichnisMRUDateinamen.RemoveAll();
								int base = i;
								char filenamebuffer[500];
								if (GetIniFileName(filenamebuffer, sizeof(filenamebuffer))) 
								{ 
									char *cp;
									if (cp = strrchr(filenamebuffer, '\\'))
									{
										strcpy(cp+1, "*.eca");
										CFileFind ff;
										if (ff.FindFile(filenamebuffer))
										{
											int bGoOn;
											do
											{
												bGoOn = ff.FindNextFile();
												pSubMenu->InsertMenu(base/*i wenn in aufsteigender Reihenfolge sortiert werden soll*/, MF_BYPOSITION, ID_CMD_CUSTOM_MRU+i-base, (LPCSTR)ff.GetFileName());
												theApp.m_MandantenverzeichnisMRUDateinamen.Add(ff.GetFilePath());
												i++;
											}	
											while (bGoOn);
										}
									}
								}
							}
							break;
						}
					}
				}
				break;
			}
		}
	}
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	//	m_bInformationModeElapsed = TRUE;  Information Mode als Standard, nicht mehr nach n Sekunden ausblenden!
	if (nIDEvent == 1) // 'Initialisiere ...' löschen
	{
		SetStatus("Bereit");
		KillTimer(1);
	}

	CMDIFrameWndEx::OnTimer(nIDEvent);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nCode == CN_COMMAND && m_bInformationModeElapsed && nID != 57670)
	{
		//m_wndStatusBar.SetInformation(NULL);	// Bei WM_COMMAND Information-Mode für Status-Bar ausschalten
		//TRACE("nID: %d\r\n", nID);
	}

	if ((nID >= ID_EINSTELLUNGEN_DOKU && nID <= ID_EINSTELLUNGEN_KONTAKT) || nID == ID_TIPP_DES_TAGES_EMAIL)
	{
		if (nCode == CN_COMMAND)
		{
			OnLink(nID);
			return TRUE;
		}
		else if (nCode == CN_UPDATE_COMMAND_UI) 
		{
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	else if (nID == ID_VIEW_PLUGINTOOLBAR)
	{
		if (nCode == CN_COMMAND)
		{
			if (m_wndPluginToolBar.IsWindowVisible())
			{
				m_wndPluginToolBar.ShowWindow(SW_HIDE);
			}
			else
			{
				m_wndPluginToolBar.ShowWindow(SW_SHOW);
			}
			return TRUE;
		}
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->SetCheck(m_wndPluginToolBar.IsWindowVisible());
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	else if (nID == ID_VIEW_FINDTOOLBAR)
	{
		if (nCode == CN_COMMAND)
		{
			if (m_wndFindToolBar.m_hWnd)	// VS9
			{
				if (m_wndFindToolBar.IsWindowVisible())
				{
					m_wndFindToolBar.ShowWindow(SW_HIDE);
				}
				else
				{
					m_wndFindToolBar.ShowWindow(SW_SHOW);
				}
			}
			return TRUE;
		}
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			if (m_wndFindToolBar.m_hWnd)	// VS9
			{
				((CCmdUI*)pExtra)->SetCheck(m_wndFindToolBar.IsWindowVisible());
				((CCmdUI*)pExtra)->Enable(TRUE);
			}
			return TRUE;
		}
	}
	else if (nID >= ID_CMD_CUSTOM_MRU && nID <= ID_CMD_CUSTOM_MRU + theApp.m_MandantenverzeichnisMRUDateinamen.GetSize())
	{
		if (nCode == CN_COMMAND)
		{
			return TRUE;
		}
	}
	else if (nID == ID_TIPP_DES_TAGES_JETZT)
	{
		if (nCode == CN_COMMAND)
		{
			theApp.ShowTipOfTheDay();
			return TRUE;
		}
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->Enable(TRUE);
			return TRUE;
		}
	}
	else if (nID == ID_TIPP_DES_TAGES_PROGRAMMSTART)
	{			
		if (nCode == CN_COMMAND)
		{
			int Startup = theApp.GetProfileIntA("Tip", "StartUp", 0);
			theApp.WriteProfileInt("Tip", "StartUp", Startup ? 0 : 1);
			return TRUE;
		}
		else if (nCode == CN_UPDATE_COMMAND_UI)
		{
			((CCmdUI*)pExtra)->Enable(TRUE);
			int Startup = theApp.GetProfileIntA("Tip", "StartUp", 0);
			((CCmdUI*)pExtra)->SetCheck(Startup == 0);
			return TRUE;
		}
	}

	return CMDIFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnLink(UINT nID)
{
	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> ar;
	m_wndRibbonBar.GetElementsByID (nID, ar);

	if (ar.GetSize () >= 1)
	{
		CMFCRibbonLinkCtrl* pLink = DYNAMIC_DOWNCAST (CMFCRibbonLinkCtrl, ar[0]);
		if (pLink != NULL)
		{
			if (!pLink->OpenLink ())
			{
				AfxMessageBox (_T("Link clicked."));
			}
		}
	}
}

void CMainFrame::OnFilePluginmanager() 
{
	if (DSAMessageBox(IDS_ZEIGE_PLUGIN_MANAGER, MB_YESNO) == IDYES)
	{
		CPluginManager dlg(this);
		dlg.DoModal();
	}
}

void CMainFrame::OnFileOeffneDatenverzeichnis() 
{
	char filenamebuffer[500];
	if (GetIniFileName(filenamebuffer, sizeof(filenamebuffer))) 
	{ 
		char *cp;
		if (cp = strrchr(filenamebuffer, '\\'))
		{
			*cp = '\0';

			ShellExecute(m_hWnd, "open", filenamebuffer, NULL, ".", SW_SHOWNORMAL);
		}
	}		
}

void CMainFrame::OnFileWaehleDatenverzeichnis() 
{
	// je nach Modus Datenverzeichnis voreinstellen
	CString csDatenverzeichnis;
	int nMandant;
	if ((nMandant = GetMandant()) >= 0)
	{
		CString csKey;
		csKey.Format("Mandant%-02.2dDatenverzeichnis", nMandant);
		csDatenverzeichnis = theApp.GetProfileString("Mandanten", csKey, "");
		if (csDatenverzeichnis.IsEmpty()) 
		{
			if (!SHGetSpecialFolderPath(AfxGetMainWnd()->GetSafeHwnd(), csDatenverzeichnis.GetBuffer(MAX_PATH), CSIDL_PERSONAL, FALSE))
				csDatenverzeichnis = "C:\\";
		}			
	}
	else
		csDatenverzeichnis = theApp.GetProfileString("Allgemein", "Datenverzeichnis");	// keine Mandanten? Dann aus der Allgemein Sektion des Windows-Verzeichnisses nehmen!
	CString csAltesDatenverzeichnis = csDatenverzeichnis;

	// Datenverzeichnis wählen
	if (SelectFolder(csDatenverzeichnis.GetBuffer(MAX_PATH), "Datenverzeichnis auswählen") && csAltesDatenverzeichnis != csDatenverzeichnis) 
	{
		csDatenverzeichnis.ReleaseBuffer();

		if (csDatenverzeichnis.GetLength() && csDatenverzeichnis[csDatenverzeichnis.GetLength()-1] == '\\')
			csDatenverzeichnis = csDatenverzeichnis.Left(csDatenverzeichnis.GetLength()-1);
	
		if ((nMandant = GetMandant()) >= 0)
		{
			CString csKey;
			csKey.Format("Mandant%-02.2dDatenverzeichnis", nMandant);
			theApp.WriteProfileString("Mandanten", csKey, csDatenverzeichnis);
		}
		else
			theApp.WriteProfileString("Allgemein", "Datenverzeichnis", csDatenverzeichnis);	

		SetIniFileName(((CString)(csDatenverzeichnis + "\\easyct.ini")).GetBuffer(0));

		if (csDatenverzeichnis != csAltesDatenverzeichnis)
			AfxMessageBox("Bitte kopieren Sie die easyct.ini und alle benötigten Buchungsdateien (Jahr????.eca) ggf. in das neue Datenverzeichnis.");
	}
}

void CMainFrame::OnFileRegistrierungsinformationenWiederherstellen() 
{
	extern char reg_name[100];

	char Dateiname[1000];
	*Dateiname = '\0';
	if (!GetIniFileName(Dateiname, sizeof(Dateiname))) { AfxMessageBox("Konnte Konfigurationsdatei EasyCT.ini nicht öffnen"); return; }
	char *cp;
	if (cp = strrchr(Dateiname, '\\'))
	{
		if (*reg_name)
			strcpy(cp+1, reg_name);
		else
			strcpy(cp+1, "EC&T-Registrierungsinformationen");
	}
	else
		strcpy(Dateiname, "EC&T-Registrierungsinformationen");
	while (strlen(Dateiname))
		if (Dateiname[strlen(Dateiname)-1] == ' ')
			Dateiname[strlen(Dateiname)-1] = '\0';
		else 
			break;
	strcat(Dateiname, ".ectreg");

	CFileDialog cf(TRUE, "*.ectreg", Dateiname, 0, "EasyCash&Tax Registrierungsinformationen (*.ectreg)|*.ectreg||",this);
	if (cf.DoModal() != IDOK)
		return;
	strcpy(Dateiname, cf.GetPathName().GetBuffer(0));

	// ___ Registry-Daten aus Datei lesen ___
	int nCount = 0;
	CString csDarunter;

	// Allgemein
	CString Datenverzeichnis;
	GetPrivateProfileString("Allgemein", "Datenverzeichnis", "", Datenverzeichnis.GetBuffer(1000), 1000, Dateiname);
	Datenverzeichnis.ReleaseBuffer();
	if (!Datenverzeichnis.IsEmpty()) { nCount++; if (csDarunter.IsEmpty()) csDarunter = ", darunter "; csDarunter += "'Datenverzeichnis=" + Datenverzeichnis + "'"; }
	CString RegKey;
	GetPrivateProfileString("Allgemein", "RegKey", "", RegKey.GetBuffer(1000), 1000, Dateiname);
	RegKey.ReleaseBuffer();
	if (!RegKey.IsEmpty())  { nCount++; if (csDarunter.IsEmpty()) csDarunter = ", darunter "; else csDarunter += " und "; csDarunter += "'RegKey=" + RegKey + "'"; }

	// Tip
	int		Startup    = GetPrivateProfileInt("Tip", "StartUp", 0, Dateiname);
	if (Startup) nCount++;
	int		FilePos    = GetPrivateProfileInt("Tip", "FilePos", 0, Dateiname);
	if (FilePos) nCount++;
	CString TimeStamp;
	GetPrivateProfileString("Tip", "TimeStamp", NULL, TimeStamp.GetBuffer(1000), 1000, Dateiname);
	TimeStamp.ReleaseBuffer();
	if (!TimeStamp.IsEmpty()) nCount++;
	// FolderHistory 
	CString Folder0000;
	GetPrivateProfileString("FolderHistory", "Folder0000", "", Folder0000.GetBuffer(1000), 1000, Dateiname);
	Folder0000.ReleaseBuffer();
	if (!Folder0000.IsEmpty()) nCount++;
	CString Folder;
	GetPrivateProfileString("FolderHistory", "Folder", "", Folder.GetBuffer(1000), 1000, Dateiname);
	Folder.ReleaseBuffer();
	if (!Folder.IsEmpty()) nCount++;

	// XFolderDialog
	int		ViewMode   = GetPrivateProfileInt("XFolderDialog", "ViewMode", 0, Dateiname);
	if (ViewMode) nCount++;

	// Mandanten
	CString csNameKey[100], csName[100], csPropertyKey[100], csProperty[100], csIconKey[100], csIcon[100];
	int i, nMandanten = 0;
	for (i = 0; i < 100; i++)
	{
		csNameKey[i].Format("Mandant%-02.2dName", i);
		GetPrivateProfileString("Mandanten", csNameKey[i].GetBuffer(0), "", csName[i].GetBuffer(1000), 1000, Dateiname);
		csName[i].ReleaseBuffer();
		if (!csName[i].IsEmpty()) nCount++;
		csPropertyKey[i].Format("Mandant%-02.2dDatenverzeichnis", i);
		GetPrivateProfileString("Mandanten", csPropertyKey[i].GetBuffer(0), "", csProperty[i].GetBuffer(1000), 1000, Dateiname);
		csProperty[i].ReleaseBuffer();
		if (!csProperty[i].IsEmpty()) nCount++;
		csIconKey[i].Format("Mandant%-02.2dIcon", i);
		 GetPrivateProfileString("Mandanten", csIconKey[i].GetBuffer(0), "0", csIcon[i].GetBuffer(1000), 1000, Dateiname);
		csIcon[i].ReleaseBuffer();		
		if (csIcon[i] != "0") nCount++;

		if (!csName[i].IsEmpty() || !csPropertyKey[i].IsEmpty()) nMandanten++;
	}

	// Fenster
	int BuchenPosX, BuchenPosY;
	int FrameWndPosX, FrameWndPosY;
	int FrameWndSizeX, FrameWndSizeY;
	BuchenPosX = GetPrivateProfileInt("Fenster", "BuchenPosX", 0, Dateiname);
	if (BuchenPosX) nCount++;
	BuchenPosY = GetPrivateProfileInt("Fenster", "BuchenPosY", 0, Dateiname);
	if (BuchenPosY) nCount++;
	FrameWndPosX = GetPrivateProfileInt("Fenster", "FrameWndPosX", 0, Dateiname);
	if (FrameWndPosX) nCount++;
	FrameWndPosY = GetPrivateProfileInt("Fenster", "FrameWndPosY", 0, Dateiname);
	if (FrameWndPosY) nCount++;
	FrameWndSizeX = GetPrivateProfileInt("Fenster", "FrameWndSizeX", 0, Dateiname);
	if (FrameWndSizeX) nCount++;
	FrameWndSizeY = GetPrivateProfileInt("Fenster", "FrameWndSizeY", 0, Dateiname);
	if (FrameWndSizeY) nCount++;

	// DSA
	CString DSA61447;
	GetPrivateProfileString("DSA", "DSA61447", "", DSA61447.GetBuffer(1000), 1000, Dateiname);
	DSA61447.ReleaseBuffer();
	if (!DSA61447.IsEmpty()) nCount++;

	// ___ Sicherheitsabfrage ___
	CString csMsg;
	csMsg.Format("Es wurden %d Registrierungseinträge gefunden%s. Sollen die Daten eingelesen werden?", nCount, csDarunter.GetBuffer(0));
	if (AfxMessageBox(csMsg, MB_YESNO) != IDYES)
		return;
	
	// ___ in Registry schreiben ___
	if (!Datenverzeichnis.IsEmpty()) 
	{
		theApp.WriteProfileString("Allgemein", "Datenverzeichnis", Datenverzeichnis);
		//SetIniFileName(((CString)(Datenverzeichnis + "\\easyct.ini")).GetBuffer(0));
	}
	if (!RegKey.IsEmpty()) theApp.WriteProfileString("Allgemein", "RegKey", RegKey);
	// Tip
	if (Startup) theApp.WriteProfileInt("Tip", "StartUp", Startup);
	if (FilePos) theApp.WriteProfileInt("Tip", "FilePos", FilePos);
	if (!TimeStamp.IsEmpty()) theApp.WriteProfileString("Tip", "TimeStamp", TimeStamp);
	// FolderHistory 
	if (!Folder0000.IsEmpty()) theApp.WriteProfileString("FolderHistory", "Folder0000", Folder0000);
	if (!Folder.IsEmpty()) theApp.WriteProfileString("FolderHistory", "Folder", Folder);
	// XFolderDialog
	if (ViewMode) theApp.WriteProfileInt("XFolderDialog", "ViewMode", ViewMode);
	// Fenster
	if (BuchenPosX) theApp.WriteProfileInt("Fenster", "BuchenPosX", BuchenPosX);
	if (BuchenPosY) theApp.WriteProfileInt("Fenster", "BuchenPosY", BuchenPosY);
	if (FrameWndPosX) theApp.WriteProfileInt("Fenster", "FrameWndPosX", FrameWndPosX);
	if (FrameWndPosY) theApp.WriteProfileInt("Fenster", "FrameWndPosY", FrameWndPosY);
	if (FrameWndSizeX) theApp.WriteProfileInt("Fenster", "FrameWndSizeX", FrameWndSizeX);
	if (FrameWndSizeY) theApp.WriteProfileInt("Fenster", "FrameWndSizeY", FrameWndSizeY);
	// DSA
	if (DSA61447) theApp.WriteProfileString("DSA", "DSA61447", DSA61447);
	// Mandanten
	if (nMandanten)
		for (i = 0; i < 100; i++)
		{
			theApp.WriteProfileString("Mandanten", csNameKey[i].GetBuffer(0), csName[i]);
			theApp.WriteProfileString("Mandanten", csPropertyKey[i].GetBuffer(0), csProperty[i]);
			theApp.WriteProfileString("Mandanten", csIconKey[i].GetBuffer(0), csIcon[i]);
		}

}

void CMainFrame::OnFileRegistrierungsinformationenSichern() 
{
	theApp.RegistrierungsinformationenSichern(NULL);
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	AfxTrace("CMainFrame::OnKeyDown: %0x (%c) %0x %d\r\n", (int)nChar, (int)nChar, nFlags, nRepCnt);
	
	CMDIFrameWndEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_SETSTATUS && wParam == 0x4712)
	{
		SetStatus((LPCTSTR)lParam);
		return TRUE;

	}
/*	else if (wParam == WM_USER+0xAC)
	{
		if (lParam == 0L)
		{
			char pluginname[1000];
			*pluginname = '\0';

			OpenClipboard();
			HGLOBAL hmem = GetClipboardData(CF_TEXT);
			if (hmem)
			{
				char *cp = (char *)GlobalLock(hmem);
				if (cp)
				{
					strncpy(pluginname, cp, sizeof(pluginname)-1);
					pluginname[sizeof(pluginname)-1] = '\0';
				}
				else
					*pluginname = '\0';
				GlobalUnlock(hmem);
			}
			CloseClipboard();	
			
			// zu aktivierendes Plugin suchen
			int id_incrementor = ID_CMD_PLUGIN_BASE;
			CPluginElement *pTemp = m_pPlugins;
			while (pTemp)	// Ende der Kette und des ID-Bereichs suchen
			{	
				if (!strcmp(pTemp->name, pluginname))
				{
					SendMessage(WM_COMMAND, pTemp->id, 0L);
					AfxGetMainWnd()->SetFocus();
					return TRUE;
				}
				pTemp = pTemp->next;
			}

			return FALSE;
		}
	}
*/	else if (message == WM_COMMAND)
	{
		if (wParam >= ID_CMD_CUSTOM_MRU && wParam <= ID_CMD_CUSTOM_MRU + theApp.m_MandantenverzeichnisMRUDateinamen.GetSize())
		{
			theApp.OpenDocumentFile(theApp.m_MandantenverzeichnisMRUDateinamen[wParam-ID_CMD_CUSTOM_MRU]);
			return TRUE;
		}
	}

	return CMDIFrameWndEx::WindowProc(message, wParam, lParam);
}

void CMainFrame::OnSpendeBank()
{
	HGLOBAL hResult = GlobalAlloc(GMEM_MOVEABLE, strlen(GLOBAL_IBAN)+1);	// In die Zwischenablage mit der IBAN
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hResult); 
	memcpy(lptstrCopy, GLOBAL_IBAN, strlen(GLOBAL_IBAN)+1); 
	GlobalUnlock(hResult);
	OpenClipboard(); 
	EmptyClipboard();
	HGLOBAL hmem = SetClipboardData(CF_TEXT, hResult);
	CloseClipboard();	

	CString csBodytext;
	extern char reg[100];
	if (!*reg)
		csBodytext = "mailto:thomas@mielke.software?subject=EasyCash-Registrierung-Spende%20(Bank)&body=Betrag:%20__%20EUR%0a%0aRegistriercode%20senden?%20(J/N)?%20J%20%0a%0aRechnungsadresse:%0a%0a%0a";
	else
		csBodytext = (CString)"mailto:thomas@mielke.software?subject=EasyCash-Registrierung-Spende%20(Bank)&body=Betrag:%20__%20EUR%0a%0aRegistriercode:%20" + reg + "%0a%0aRechnungsadresse:%0a%0a%0a";

	int n = (int)ShellExecute(m_hWnd, "open", csBodytext, NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Herzlichen Dank! Es gab aber ein kleines Problem mit dem E-Mail Programm auf diesem Rechner. Bitte das Mail-Programm manuell starten und Eine E-Mail an MielkeT@gmx.de mit Betreff 'EasyCash-Registrierung-Spende (Bank)' und Betrag sowie Rechnungsadresse im Mailtext angeben. Nach dem Absenden sollte der Code in wenigen Tagen eintreffen. Die IBAN wurde aber in die Zwischenablage kopiert und kann ggf. in Ihr Online-Banking eingefügt werden. Der Kontoinhaber lautet 'Thomas Mielke' und die BIC, für den Fall einer Auslandsüberweisung, ist " + (CString)GLOBAL_BIC + ".", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Herzlichen Dank! Die Spenden-Registrierung sollte jetzt im E-Mail-Programm geöffnet worden sein. (Wenn nicht, bitte das Mail-Programm manuell starten und Eine E-Mail an MielkeT@gmx.de mit Betreff 'EasyCash-Registrierung-Spende (Bank)' und Betrag sowie Rechnungsadresse im Mailtext angeben. Nach dem Absenden sollte der Code in wenigen Tagen eintreffen. Die IBAN wurde übrigens in die Zwischenablage kopiert und kann ggf. in Ihr Online-Banking eingefügt werden. Der Kontoinhaber lautet 'Thomas Mielke' und die BIC, für den Fall einer Auslandsüberweisung, ist " + (CString)GLOBAL_BIC + ".", MB_ICONINFORMATION);
	}
}

void CMainFrame::OnSpendePayPal()
{
	CString csBodytext;
	extern char reg[100];
	if (!*reg)
		csBodytext = "mailto:thomas@mielke.software?subject=EasyCash-Registrierung-Spende%20(PayPal)&body=Betrag:%20__%20EUR%0a%0aHabe%20schon%20einen%20Registriercode%20und%20brauche%20nur%20die%20Rechnung%20(J/N)?%20N%20%0a%0a";
	else
		csBodytext = (CString)"mailto:thomas@mielke.software?subject=EasyCash-Registrierung-Spende%20(PayPal)&body=Betrag:%20__%20EUR%0a%0aRegistriercode:%20" + reg + "%0a%0a";

	int n = (int)ShellExecute(m_hWnd, "open", "https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=MPGX6R863RLP8", NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Herzlichen Dank! Es gab aber ein kleines Problem mit dem Web-Browser auf diesem Rechner. Bitte starten Sie den Web-Browser manuell und gehen auf www.easyct.de. Dort gleich im Willkommenstext befindet sich ein PayPal-Spenden-Knopf. Als Nächstes wird die Registrierungsmail im E-Mail-Programm geöffnet.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Herzlichen Dank! Der Web-Browser sollte jetzt mit der PayPal-Spendenseite geöffnet worden sein. (Wenn nicht, starten Sie den Web-Browser manuell und gehen auf www.easyct.de. Dort gleich im Willkommenstext befindet sich ein PayPal-Spenden-Knopf.) Als Nächstes wird die Registrierungsmail im E-Mail-Programm geöffnet.", MB_ICONINFORMATION);
	}
	
	n = (int)ShellExecute(m_hWnd, "open", csBodytext, NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Es gab ein kleines Problem mit dem E-Mail Programm auf diesem Rechner. Bitte das Mail-Programm manuell starten und Eine E-Mail an MielkeT@gmx.de mit Betreff 'EasyCash-Registrierung-Spende (PayPal)' und Betrag sowie Rechnungsadresse im Mailtext angeben.) Nach dem Absenden sollte der Code in wenigen Tagen eintreffen.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Die Spenden-Registrierung sollte jetzt im E-Mail-Programm geöffnet worden sein. (Wenn nicht, bitte das Mail-Programm manuell starten und Eine E-Mail an MielkeT@gmx.de mit Betreff 'EasyCash-Registrierung-Spende (PayPal)' und Betrag sowie Rechnungsadresse im Mailtext angeben.) Nach dem Absenden sollte der Code in wenigen Tagen eintreffen.", MB_ICONINFORMATION);
	}
}

void CMainFrame::OnSpendeBitcoin()
{
	char *btcadrs[] = {
		"bitcoin:17u1T7gwsoWTkmdY6t1NTKWEAZcrxmcvkW?label=EC%26T-Spende",
		"bitcoin:14Wy8uMcBTUFYjj5kfRfCoMxa3mTxvzBBY?label=EC%26T-Spende",
		"bitcoin:1MamjqTmu5XF3sZCYdkac1TqjKoXf62tcT?label=EC%26T-Spende",
		"bitcoin:14bLPzpsVbQ6Qohd4KiacoNokR5Z3wUvyq?label=EC%26T-Spende",
		"bitcoin:1CDSYQagELCgwQQd1sLer8QAr552Cxwsr3?label=EC%26T-Spende",
		"bitcoin:1FbcJaV6CB2LLWhyiNg4P5H4f2uSeicFBt?label=EC%26T-Spende",
		"bitcoin:1PLRUaxpSdcA8X9hUtRheRs1kf2LwdPnqj?label=EC%26T-Spende",
		"bitcoin:1D14UmQqSNkPLCL2PiD9TQvdRgx26HU17U?label=EC%26T-Spende",
		"bitcoin:1Kgtx6R7zzXV4MHJ8ozAMxocwuWSQRxav1?label=EC%26T-Spende",
		"bitcoin:138DJRdyYM2ZGLDGNT7nzGWDiLjLDLDNCa?label=EC%26T-Spende"
	};
	char adrbuf[35];

	CTime now = CTime::GetCurrentTime();	// Letzte Ziffer des Monatstages ist unser Index
	int index = now.GetDay() % 10;
	strncpy(adrbuf, btcadrs[index]+8, 34);
	adrbuf[34] = '\0';

	HGLOBAL hResult = GlobalAlloc(GMEM_MOVEABLE, 35);	// In die Zwischenablage mit der Adresse
	LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hResult); 
	memcpy(lptstrCopy, adrbuf, 35); 
	GlobalUnlock(hResult); 
	OpenClipboard();
	EmptyClipboard();
	HGLOBAL hmem = SetClipboardData(CF_TEXT, hResult);
	CloseClipboard();	

	int n = (int)ShellExecute(m_hWnd, "open", btcadrs[index], NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Herzlichen Dank! Es gab aber ein kleines Problem mit dem Bitcoin-Client auf diesem Rechner. (Ist tatsächlich einer installiert?) Die Bitcoin-Adresse wurde jedenfalls in die Zwischenablage kopiert. Bitte Starten Sie den Bitcoin-Client manuell und fügen Sie die Adresse aus der Zwischenablage ein. Als Nächstes wird die Registrierungsmail im E-Mail-Programm geöffnet.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Herzlichen Dank! Der Bitcoin-Client sollte jetzt mit der richtige Bitcoin-Adresse geöffnet worden sein. (Wenn nicht, bitte den Bitcoin-Client manuell starten und die Bitcoin-Adresse " + (CString)adrbuf + " aus der Zwischenablage einfügen.) Als Nächstes wird die Registrierungsmail im E-Mail-Programm geöffnet.", MB_ICONINFORMATION);
	}
	
	CString csBodytext;
	extern char reg[100];
	if (!*reg)
		csBodytext = "mailto:thomas@mielke.software?subject=EasyCash-Registrierung-Spende%20(BTC)&body=Betrag:%20__%20mBTC%0a%0aHabe%20schon%20einen%20Registriercode%20und%20brauche%20nur%20die%20Rechnung%20(J/N)?%20N%20%0a%0aRechnungsadresse:%0a%0a%0a";
	else
		csBodytext = (CString)"mailto:thomas@mielke.software?subject=EasyCash-Registrierung-Spende%20(BTC)&body=Betrag:%20__%20mBTC%0a%0aRegistriercode:%20" + reg + "%0a%0aRechnungsadresse:%0a%0a%0a";

	n = (int)ShellExecute(m_hWnd, "open", csBodytext, NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Es gab ein kleines Problem mit dem E-Mail Programm auf diesem Rechner. Bitte das Mail-Programm manuell starten und Eine E-Mail an MielkeT@gmx.de mit Betreff 'EasyCash-Registrierung-Spende' und BTC-Betrag sowie Rechnungsadresse im Mailtext angeben.) Nach dem Absenden sollte der Code in wenigen Tagen eintreffen.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Die Spenden-Registrierung sollte jetzt im E-Mail-Programm geöffnet worden sein. (Wenn nicht, bitte das Mail-Programm manuell starten und Eine E-Mail an MielkeT@gmx.de mit Betreff 'EasyCash-Registrierung-Spende' und BTC-Betrag sowie Rechnungsadresse im Mailtext angeben.) Nach dem Absenden sollte der Code in wenigen Tagen eintreffen.", MB_ICONINFORMATION);
	}
}

void CMainFrame::OnSpendeFlattr()
{
	int n = (int)ShellExecute(m_hWnd, "open", "https://flattr.com/submit/auto?fid=7rnje0&url=http%3A%2F%2Fwww.easyct.de", NULL, ".", SW_SHOWNORMAL);
	if (n <= 32)
	{
		AfxMessageBox("Herzlichen Dank! Es gab aber ein kleines Problem mit dem Web-Browser auf diesem Rechner. Bitte starten Sie den Web-Browser manuell und gehen auf www.easyct.de. Dort gleich im Willkommenstext befindet sich ein Flattr-Spenden-Knopf.", MB_ICONSTOP);
	}
	else
	{
		AfxMessageBox("Herzlichen Dank! Der Web-Browser sollte jetzt mit der Flattr-Spendenseite geöffnet worden sein. (Wenn nicht, starten Sie den Web-Browser manuell und gehen auf www.easyct.de. Dort gleich im Willkommenstext befindet sich ein Flattr-Spenden-Knopf.)", MB_ICONINFORMATION);
	}
}
