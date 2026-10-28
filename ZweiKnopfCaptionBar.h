// ZweiKnopfCaptionBar.h: CaptionBar mit optionalem zweitem Knopf
//
// Diese Datei ist Bestandteil von EasyCash&Tax, der freien EUR-Fibu
//
// Copyleft (GPLv3) 2026  Thomas Mielke
//
// CMFCCaptionBar kennt nur EINEN Knopf. Fuer Meldungen mit zwei Antworten
// (Programm-Update: "Installieren" / "Ueberspringen") zeichnet diese Ableitung
// einen zweiten Knopf rechts neben den ersten. Gezeichnet wird er ueber
// CMFCCaptionBar::OnDrawButton, damit er dieselbe Visual-Manager-Optik hat.
// Tooltip gibt es nur fuer den ersten Knopf.
//
// Ohne SetButton2 verhaelt sich die Leiste exakt wie CMFCCaptionBar.

#pragma once

class CZweiKnopfCaptionBar : public CMFCCaptionBar
{
public:
	CZweiKnopfCaptionBar();

	// Der zweite Knopf erscheint nur, wenn auch der erste (SetButton) gesetzt ist.
	void SetButton2(LPCTSTR lpszLabel, UINT uiCmd);
	void RemoveButton2();

protected:
	virtual void RecalcLayout();
	virtual void OnDrawButton(CDC* pDC, CRect rect, const CString& strButton, BOOL bEnabled);

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	DECLARE_MESSAGE_MAP()

	CString	m_strBtn2Text;
	UINT	m_uiBtn2ID;
	CRect	m_rectButton2;
	BOOL	m_bIsBtn2Pressed;
	BOOL	m_bIsBtn2Highlighted;
};
