#ifndef _VISUALSTYLE_XP_H_
#define _VISUALSTYLE_XP_H_

// VisualStylesXP.h : header file
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien E�R-Fibu
//
// Copyleft (GPLv3) 2020  Thomas Mielke
// 
// Dies ist freie Software; Sie d�rfen sie unter den Bedingungen der 
// GNU General Public License, wie von der Free Software Foundation 
// ver�ffentlicht, weiterverteilen und/oder modifizieren; entweder gem�� 
// Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren Version.
//
// Diese Software wird in der Hoffnung weiterverbreitet, dass sie n�tzlich 
// sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die implizierte 
// Garantie der MARKTREIFE oder der VERWENDBARKEIT F�R EINEN BESTIMMTEN ZWECK.
// Mehr Details finden Sie in der GNU Lesser General Public License.
//
// Sie sollten eine Kopie der GNU General Public License Version 3 zusammen mit 
// dieser Software erhalten haben; falls nicht, schreiben Sie an die Free 
// Software Foundation, Inc., 51 Franklin St, 5th Floor, Boston, MA 02110, USA. 

#pragma once

#include "uxtheme.h"
#include "tmschema.h"

enum EXP_State {
    XPS_NORMAL  =1,
    XPS_HOT     =2,
    XPS_PRESSED =3,
    XPS_DISABLED=4
};

enum EXP_ToolBarState {
    XPS_TB_NORMAL      =1,
    XPS_TB_HOT         =2,
    XPS_TB_PRESSED     =3,
    XPS_TB_DISABLED    =4,
    XPS_TB_CHECKED     =5,
    XPS_TB_HOTCHECKED  =6
};

enum EXP_SliderThumbState {
    XPS_ST_NORMAL  =1,
    XPS_ST_HOT     =2,
    XPS_ST_PRESSED =3,
    XPS_ST_FOCUS   =4,
    XPS_ST_DISABLED=5
};

enum EXP_CheckBoxIndex {
    XPS_CB_Unchecked    =0,
    XPS_CB_Checked      =4,
    XPS_CB_Mixed        =8
};

enum EXP_SBArrowsIndex {
    XPS_SBArrows_Up     =0,
    XPS_SBArrows_Down   =4,
    XPS_SBArrows_Left   =8,
    XPS_SBArrows_Right  =12
};

class CVisualStylesXP
{
private:
	HMODULE m_hThemeDll;
	void* GetProc(LPCSTR szProc, void* pfnFail);

	typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
	static HTHEME OpenThemeDataFail(HWND , LPCWSTR )
	{return NULL;}

	typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
	static HRESULT CloseThemeDataFail(HTHEME)
	{return E_FAIL;}

	typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect,  const RECT *pClipRect);
	static HRESULT DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, const RECT *)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
		DWORD dwTextFlags2, const RECT *pRect);
	static HRESULT DrawThemeTextFail(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme,  HDC hdc, 
		int iPartId, int iStateId,  const RECT *pBoundingRect, 
		RECT *pContentRect);
	static HRESULT GetThemeBackgroundContentRectFail(HTHEME hTheme,  HDC hdc, 
		int iPartId, int iStateId,  const RECT *pBoundingRect, 
		RECT *pContentRect)
	{return E_FAIL;}
	typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDEXTENT)(HTHEME hTheme,  HDC hdc,
		int iPartId, int iStateId, const RECT *pContentRect, 
		RECT *pExtentRect);
	static HRESULT GetThemeBackgroundExtentFail(HTHEME hTheme,  HDC hdc,
		int iPartId, int iStateId, const RECT *pContentRect, 
		RECT *pExtentRect)
	{return E_FAIL;}

	typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize,  SIZE *psz);
	static HRESULT GetThemePartSizeFail(HTHEME, HDC, int, int, RECT *, enum THEMESIZE, SIZE *)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMETEXTEXTENT)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
		DWORD dwTextFlags,  const RECT *pBoundingRect, 
		RECT *pExtentRect);
	static HRESULT GetThemeTextExtentFail(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
		DWORD dwTextFlags,  const RECT *pBoundingRect, 
		RECT *pExtentRect)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMETEXTMETRICS)(HTHEME hTheme,  HDC hdc, 
		int iPartId, int iStateId,  TEXTMETRIC* ptm);
	static HRESULT GetThemeTextMetricsFail(HTHEME hTheme,  HDC hdc, 
		int iPartId, int iStateId,  TEXTMETRIC* ptm)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDREGION)(HTHEME hTheme,  HDC hdc,  
		int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion);
	static HRESULT GetThemeBackgroundRegionFail(HTHEME hTheme,  HDC hdc,  
		int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNHITTESTTHEMEBACKGROUND)(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
		POINT ptTest,  WORD *pwHitTestCode);
	static HRESULT HitTestThemeBackgroundFail(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
		POINT ptTest,  WORD *pwHitTestCode)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMEEDGE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
		const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect);
	static HRESULT DrawThemeEdgeFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
		const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMEICON)(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
	static HRESULT DrawThemeIconFail(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex)
	{return E_FAIL;}

	typedef BOOL (__stdcall *PFNISTHEMEPARTDEFINED)(HTHEME hTheme, int iPartId, 
		int iStateId);
	static BOOL IsThemePartDefinedFail(HTHEME hTheme, int iPartId, 
		int iStateId)
	{return FALSE;}

	typedef BOOL (__stdcall *PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(HTHEME hTheme, 
		int iPartId, int iStateId);
	static BOOL IsThemeBackgroundPartiallyTransparentFail(HTHEME hTheme, 
		int iPartId, int iStateId)
	{return FALSE;}

	typedef HRESULT (__stdcall *PFNGETTHEMECOLOR)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  COLORREF *pColor);
	static HRESULT GetThemeColorFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  COLORREF *pColor)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEMETRIC)(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  int *piVal);
	static HRESULT GetThemeMetricFail(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  int *piVal)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMESTRING)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars);
	static HRESULT GetThemeStringFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEBOOL)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  BOOL *pfVal);
	static HRESULT GetThemeBoolFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  BOOL *pfVal)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEINT)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  int *piVal);
	static HRESULT GetThemeIntFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  int *piVal)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEENUMVALUE)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  int *piVal);
	static HRESULT GetThemeEnumValueFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  int *piVal)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEPOSITION)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  POINT *pPoint);
	static HRESULT GetThemePositionFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  POINT *pPoint)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEFONT)(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  LOGFONT *pFont);
	static HRESULT GetThemeFontFail(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  LOGFONT *pFont)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMERECT)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  RECT *pRect);
	static HRESULT GetThemeRectFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  RECT *pRect)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEMARGINS)(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins);
	static HRESULT GetThemeMarginsFail(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEINTLIST)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  INTLIST *pIntList);
	static HRESULT GetThemeIntListFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  INTLIST *pIntList)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEPROPERTYORIGIN)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin);
	static HRESULT GetThemePropertyOriginFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, 
		LPCWSTR pszSubIdList);
	static HRESULT SetWindowThemeFail(HWND hwnd, LPCWSTR pszSubAppName, 
		LPCWSTR pszSubIdList)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEFILENAME)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars);
	static HRESULT GetThemeFilenameFail(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars)
	{return E_FAIL;}

	typedef COLORREF (__stdcall *PFNGETTHEMESYSCOLOR)(HTHEME hTheme, int iColorId);
	static COLORREF GetThemeSysColorFail(HTHEME hTheme, int iColorId)
	{return RGB(255,255,255);}

	typedef HBRUSH (__stdcall *PFNGETTHEMESYSCOLORBRUSH)(HTHEME hTheme, int iColorId);
	static HBRUSH GetThemeSysColorBrushFail(HTHEME hTheme, int iColorId)
	{return NULL;}

	typedef BOOL (__stdcall *PFNGETTHEMESYSBOOL)(HTHEME hTheme, int iBoolId);
	static BOOL GetThemeSysBoolFail(HTHEME hTheme, int iBoolId)
	{return FALSE;}

	typedef int (__stdcall *PFNGETTHEMESYSSIZE)(HTHEME hTheme, int iSizeId);
	static int GetThemeSysSizeFail(HTHEME hTheme, int iSizeId)
	{return 0;}

	typedef HRESULT (__stdcall *PFNGETTHEMESYSFONT)(HTHEME hTheme, int iFontId,  LOGFONT *plf);
	static HRESULT GetThemeSysFontFail(HTHEME hTheme, int iFontId,  LOGFONT *plf)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMESYSSTRING)(HTHEME hTheme, int iStringId, 
		LPWSTR pszStringBuff, int cchMaxStringChars);
	static HRESULT GetThemeSysStringFail(HTHEME hTheme, int iStringId, 
		LPWSTR pszStringBuff, int cchMaxStringChars)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMESYSINT)(HTHEME hTheme, int iIntId, int *piValue);
	static HRESULT GetThemeSysIntFail(HTHEME hTheme, int iIntId, int *piValue)
	{return E_FAIL;}

	typedef BOOL (__stdcall *PFNISTHEMEACTIVE)();
	static BOOL IsThemeActiveFail()
	{return FALSE;}

	typedef BOOL(__stdcall *PFNISAPPTHEMED)();
	static BOOL IsAppThemedFail()
	{return FALSE;}

	typedef HTHEME (__stdcall *PFNGETWINDOWTHEME)(HWND hwnd);
	static HTHEME GetWindowThemeFail(HWND hwnd)
	{return NULL;}

	typedef HRESULT (__stdcall *PFNENABLETHEMEDIALOGTEXTURE)(HWND hwnd, DWORD dwFlags);
	static HRESULT EnableThemeDialogTextureFail(HWND hwnd, DWORD dwFlags)
	{return E_FAIL;}

	typedef BOOL (__stdcall *PFNISTHEMEDIALOGTEXTUREENABLED)(HWND hwnd);
	static BOOL IsThemeDialogTextureEnabledFail(HWND hwnd)
	{return FALSE;}

	typedef DWORD (__stdcall *PFNGETTHEMEAPPPROPERTIES)();
	static DWORD GetThemeAppPropertiesFail()
	{return 0;}

	typedef void (__stdcall *PFNSETTHEMEAPPPROPERTIES)(DWORD dwFlags);
	static void SetThemeAppPropertiesFail(DWORD dwFlags)
	{return;}

	typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(
		LPWSTR pszThemeFileName, int cchMaxNameChars, 
		LPWSTR pszColorBuff, int cchMaxColorChars,
		LPWSTR pszSizeBuff, int cchMaxSizeChars);
	static HRESULT GetCurrentThemeNameFail(
		LPWSTR pszThemeFileName, int cchMaxNameChars, 
		LPWSTR pszColorBuff, int cchMaxColorChars,
		LPWSTR pszSizeBuff, int cchMaxSizeChars)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMEDOCUMENTATIONPROPERTY)(LPCWSTR pszThemeName,
		LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars);
	static HRESULT GetThemeDocumentationPropertyFail(LPCWSTR pszThemeName,
		LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, HDC hdc,  RECT* prc);
	static HRESULT DrawThemeParentBackgroundFail(HWND hwnd, HDC hdc,  RECT* prc)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNENABLETHEMING)(BOOL fEnable);
	static HRESULT EnableThemingFail(BOOL fEnable)
	{return E_FAIL;}
public:
	HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT CloseThemeData(HTHEME hTheme);
	HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
	HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, 
		DWORD dwTextFlags2, const RECT *pRect);
	HRESULT GetThemeBackgroundContentRect(HTHEME hTheme,  HDC hdc, 
		int iPartId, int iStateId,  const RECT *pBoundingRect, 
		RECT *pContentRect);
	HRESULT GetThemeBackgroundExtent(HTHEME hTheme,  HDC hdc,
		int iPartId, int iStateId, const RECT *pContentRect, 
		RECT *pExtentRect);
	HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz);
	HRESULT GetThemeTextExtent(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
		DWORD dwTextFlags,  const RECT *pBoundingRect, 
		RECT *pExtentRect);
	HRESULT GetThemeTextMetrics(HTHEME hTheme,  HDC hdc, 
		int iPartId, int iStateId,  TEXTMETRIC* ptm);
	HRESULT GetThemeBackgroundRegion(HTHEME hTheme,  HDC hdc,  
		int iPartId, int iStateId, const RECT *pRect,  HRGN *pRegion);
	HRESULT HitTestThemeBackground(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, DWORD dwOptions, const RECT *pRect,  HRGN hrgn, 
		POINT ptTest,  WORD *pwHitTestCode);
	HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, 
		const RECT *pDestRect, UINT uEdge, UINT uFlags,   RECT *pContentRect);
	HRESULT DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, 
		int iStateId, const RECT *pRect, HIMAGELIST himl, int iImageIndex);
	BOOL IsThemePartDefined(HTHEME hTheme, int iPartId, 
		int iStateId);
	BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, 
		int iPartId, int iStateId);
	HRESULT GetThemeColor(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  COLORREF *pColor);
	HRESULT GetThemeMetric(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  int *piVal);
	HRESULT GetThemeString(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  LPWSTR pszBuff, int cchMaxBuffChars);
	HRESULT GetThemeBool(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  BOOL *pfVal);
	HRESULT GetThemeInt(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  int *piVal);
	HRESULT GetThemeEnumValue(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  int *piVal);
	HRESULT GetThemePosition(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  POINT *pPoint);
	HRESULT GetThemeFont(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  LOGFONT *pFont);
	HRESULT GetThemeRect(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  RECT *pRect);
	HRESULT GetThemeMargins(HTHEME hTheme,  HDC hdc, int iPartId, 
		int iStateId, int iPropId,  RECT *prc,  MARGINS *pMargins);
	HRESULT GetThemeIntList(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  INTLIST *pIntList);
	HRESULT GetThemePropertyOrigin(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  enum PROPERTYORIGIN *pOrigin);
	HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
		LPCWSTR pszSubIdList);
	HRESULT GetThemeFilename(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  LPWSTR pszThemeFileName, int cchMaxBuffChars);
	COLORREF GetThemeSysColor(HTHEME hTheme, int iColorId);
	HBRUSH GetThemeSysColorBrush(HTHEME hTheme, int iColorId);
	BOOL GetThemeSysBool(HTHEME hTheme, int iBoolId);
	int GetThemeSysSize(HTHEME hTheme, int iSizeId);
	HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId,  LOGFONT *plf);
	HRESULT GetThemeSysString(HTHEME hTheme, int iStringId, 
		LPWSTR pszStringBuff, int cchMaxStringChars);
	HRESULT GetThemeSysInt(HTHEME hTheme, int iIntId, int *piValue);
	BOOL IsThemeActive();
	BOOL IsAppThemed();	
	HTHEME GetWindowTheme(HWND hwnd);
	HRESULT EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags);
	BOOL IsThemeDialogTextureEnabled(HWND hwnd);
	DWORD GetThemeAppProperties();
	void SetThemeAppProperties(DWORD dwFlags);
	HRESULT GetCurrentThemeName(
		LPWSTR pszThemeFileName, int cchMaxNameChars, 
		LPWSTR pszColorBuff, int cchMaxColorChars,
		LPWSTR pszSizeBuff, int cchMaxSizeChars);
	HRESULT GetThemeDocumentationProperty(LPCWSTR pszThemeName,
		LPCWSTR pszPropertyName,  LPWSTR pszValueBuff, int cchMaxValChars);
	HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc,  RECT* prc);
	HRESULT EnableTheming(BOOL fEnable);
public:
	CVisualStylesXP(void);
	~CVisualStylesXP(void);
};

extern CVisualStylesXP g_xpStyle;

#endif