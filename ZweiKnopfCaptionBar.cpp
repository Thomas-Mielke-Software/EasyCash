// ZweiKnopfCaptionBar.cpp: CaptionBar mit optionalem zweitem Knopf, siehe ZweiKnopfCaptionBar.h
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EUR-Fibu
//
// Copyleft (GPLv3) 2026  Thomas Mielke
//
// Dies ist freie Software; Sie duerfen sie unter den Bedingungen der
// GNU General Public License, wie von der Free Software Foundation
// veroeffentlicht, weiterverteilen und/oder modifizieren; entweder gemaess
// Version 3 der Lizenz oder (nach Ihrer Option) jeder spaeteren Version.
//
// Die Layout-Konstanten spiegeln afxcaptionbar.cpp (MFC 14.x):
// nButtonHorzMargin = 10, nMessageBarMargin = 4.

#include "stdafx.h"
#include "ZweiKnopfCaptionBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nButtonHorzMarginMfc = 10;
static const int nMessageBarMarginMfc = 4;

BEGIN_MESSAGE_MAP(CZweiKnopfCaptionBar, CMFCCaptionBar)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

CZweiKnopfCaptionBar::CZweiKnopfCaptionBar()
{
	m_uiBtn2ID = 0;
	m_rectButton2.SetRectEmpty();
	m_bIsBtn2Pressed = FALSE;
	m_bIsBtn2Highlighted = FALSE;
}

void CZweiKnopfCaptionBar::SetButton2(LPCTSTR lpszLabel, UINT uiCmd)
{
	m_strBtn2Text = lpszLabel ? lpszLabel : "";
	m_uiBtn2ID = uiCmd;
	m_bIsBtn2Pressed = m_bIsBtn2Highlighted = FALSE;
	if (GetSafeHwnd())
	{
		RecalcLayout();
		RedrawWindow();
	}
}

void CZweiKnopfCaptionBar::RemoveButton2()
{
	if (m_strBtn2Text.IsEmpty() && m_uiBtn2ID == 0) return;
	SetButton2(NULL, 0);
}

// ----------------------------------------------------------
// Layout: Die Basis legt Text und ersten Knopf an (im Message-Bar-Modus mit
// linksbuendigem Text steht der Knopf direkt hinter dem Text). Knopf 2 kommt
// rechts daneben; reicht der Platz nicht, ruecken beide Knoepfe nach links und
// der Text wird entsprechend gekuerzt (Ellipse + Tooltip macht die Basis).

void CZweiKnopfCaptionBar::RecalcLayout()
{
	CMFCCaptionBar::RecalcLayout();

	m_rectButton2.SetRectEmpty();
	if (m_strBtn2Text.IsEmpty() || m_uiBtn2ID == 0 || m_strBtnText.IsEmpty() || m_rectButton.IsRectEmpty())
		return;

	CRect rectClient;
	GetClientRect(rectClient);
	if (rectClient.IsRectEmpty())
		return;

	int nBreite;
	{
		CClientDC dc(NULL);
		CFont* pOldFont = dc.SelectObject(m_hFont == NULL ? &(GetGlobalData()->fontRegular) : CFont::FromHandle(m_hFont));
		nBreite = dc.GetTextExtent(m_strBtn2Text).cx + 2 * m_nHorzElementOffset;
		if (m_bIsMessageBarMode)
			nBreite += 2 * nButtonHorzMarginMfc;
		dc.SelectObject(pOldFont);
	}

	m_rectButton2 = m_rectButton;
	m_rectButton2.left = m_rectButton.right + m_nHorzElementOffset;
	m_rectButton2.right = m_rectButton2.left + nBreite;

	// rechte Grenze wie in der Basis: Rand und ggf. Schliessen-Knopf abziehen
	int nRechts = rectClient.right - m_nMargin;
	if (m_bIsMessageBarMode)
		nRechts -= nMessageBarMarginMfc + m_rectClose.Width();

	if (m_rectButton2.right > nRechts)
	{
		int nVersatz = m_rectButton2.right - nRechts;
		m_rectButton.OffsetRect(-nVersatz, 0);
		m_rectButton2.OffsetRect(-nVersatz, 0);
	}

	if (m_rectButton.left < rectClient.left + m_nMargin)
	{	// zu schmal fuer beide Knoepfe: zweiten weglassen statt ueberlappen
		m_rectButton2.SetRectEmpty();
		return;
	}

	int nTextEnde = m_rectButton.left - m_nHorzElementOffset;
	if (!m_rectDrawText.IsRectEmpty() && m_rectDrawText.right > nTextEnde)
	{
		m_rectDrawText.right = max(m_rectDrawText.left, nTextEnde);
		m_bTextIsTruncated = TRUE;
		UpdateTooltips();
	}
}

// ----------------------------------------------------------
// Zeichnen: Die Basis-Implementierung liest den Zustand (gedrueckt/hervor-
// gehoben/Dropdown) aus den Membern des ersten Knopfs. Fuer Knopf 2 werden
// diese Member kurz umgeschaltet und danach wiederhergestellt.

void CZweiKnopfCaptionBar::OnDrawButton(CDC* pDC, CRect rect, const CString& strButton, BOOL bEnabled)
{
	CMFCCaptionBar::OnDrawButton(pDC, rect, strButton, bEnabled);

	if (m_rectButton2.IsRectEmpty())
		return;

	BOOL bPressed	= m_bIsBtnPressed;
	BOOL bHighlight	= m_bIsBtnHighlighted;
	BOOL bForce		= m_bIsBtnForcePressed;
	BOOL bDropDown	= m_bBtnHasDropDownArrow;
	BOOL bEnabledAlt = m_bBtnEnabled;
	UINT uiID		= m_uiBtnID;

	m_bIsBtnPressed			= m_bIsBtn2Pressed;
	m_bIsBtnHighlighted		= m_bIsBtn2Highlighted;
	m_bIsBtnForcePressed	= FALSE;
	m_bBtnHasDropDownArrow	= FALSE;
	m_bBtnEnabled			= TRUE;
	m_uiBtnID				= m_uiBtn2ID;

	CMFCCaptionBar::OnDrawButton(pDC, m_rectButton2, m_strBtn2Text, TRUE);

	m_bIsBtnPressed			= bPressed;
	m_bIsBtnHighlighted		= bHighlight;
	m_bIsBtnForcePressed	= bForce;
	m_bBtnHasDropDownArrow	= bDropDown;
	m_bBtnEnabled			= bEnabledAlt;
	m_uiBtnID				= uiID;
}

// ----------------------------------------------------------
// Maus: Knopf 1 und Schliessen-Knopf behandelt die Basis, hier nur Knopf 2.

void CZweiKnopfCaptionBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CMFCCaptionBar::OnMouseMove(nFlags, point);	// startet auch das TrackMouseEvent fuer OnMouseLeave

	if (m_rectButton2.IsRectEmpty())
		return;

	BOOL bHighlight = m_rectButton2.PtInRect(point);
	if (bHighlight != m_bIsBtn2Highlighted)
	{
		m_bIsBtn2Highlighted = bHighlight;
		m_bIsBtn2Pressed = (nFlags & MK_LBUTTON) && bHighlight;
		InvalidateRect(m_rectButton2);
		UpdateWindow();
	}
}

void CZweiKnopfCaptionBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMFCCaptionBar::OnLButtonDown(nFlags, point);

	if (m_bIsBtn2Highlighted && !m_rectButton2.IsRectEmpty())
	{
		m_bIsBtn2Pressed = TRUE;
		InvalidateRect(m_rectButton2);
		UpdateWindow();
	}
}

void CZweiKnopfCaptionBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL bKnopf2 = m_bIsBtn2Pressed;
	UINT uiID = m_uiBtn2ID;
	if (bKnopf2)
	{
		m_bIsBtn2Pressed = FALSE;
		InvalidateRect(m_rectButton2);
		UpdateWindow();
	}

	CMFCCaptionBar::OnLButtonUp(nFlags, point);

	// Kommando als letztes senden: der Handler darf die Leiste verstecken oder umbauen
	if (bKnopf2 && uiID != 0)
		GetOwner()->SendMessage(WM_COMMAND, uiID);
}

void CZweiKnopfCaptionBar::OnMouseLeave()
{
	CMFCCaptionBar::OnMouseLeave();

	if (m_bIsBtn2Pressed || m_bIsBtn2Highlighted)
	{
		m_bIsBtn2Pressed = FALSE;
		m_bIsBtn2Highlighted = FALSE;
		InvalidateRect(m_rectButton2);
		UpdateWindow();
	}
}
