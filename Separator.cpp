// Separator.cpp : implementation file
//

#include "stdafx.h"
#include "Separator.h"
#include "VisualStylesXP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeparator

CSeparator::CSeparator()
{
}

CSeparator::~CSeparator()
{
}


BEGIN_MESSAGE_MAP(CSeparator, CWnd)
	//{{AFX_MSG_MAP(CSeparator)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSeparator message handlers

void CSeparator::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    CRect rect;
    GetClientRect(&rect);

    bool bHoriz = true;
    if (rect.Height() > rect.Width())
        bHoriz = false;

    if (g_xpStyle.IsAppThemed())
    {
        int iPartId = bHoriz ? TP_SEPARATORVERT : TP_SEPARATOR;
        HTHEME hTheme = g_xpStyle.OpenThemeData(dc.GetWindow()->GetSafeHwnd(), L"TOOLBAR");
		g_xpStyle.DrawThemeBackground(hTheme, dc.GetSafeHdc(), iPartId, XPS_TB_NORMAL, &rect, NULL);
		g_xpStyle.CloseThemeData(hTheme);
    }
    else
    {
        CPoint pt1, pt2, offset;
        int nMargin = 2;

        if (bHoriz)
        {
            pt1.x = rect.left + nMargin;
            pt1.y = rect.top +(rect.Height() - 1)/2;
            pt2.x = pt1.x;
            pt2.y = pt1.y + 1;
            offset.x = rect.Width() - 2*nMargin;
            offset.y = 0;
        }
        else
        {
            pt1.x = rect.left +(rect.Width() - 1)/2;
            pt1.y = rect.top + nMargin;
            pt2.x = pt1.x + 1;
            pt2.y = pt1.y;
            offset.x = 0;
            offset.y = rect.Height() - 2*nMargin;
        }
        
        COLORREF color = ::GetSysColor(COLOR_3DSHADOW);
        CPen pen(PS_SOLID, 1, color); 
        CPen *oldPen = dc.SelectObject(&pen);
        dc.MoveTo(pt1);
        pt1 += offset;
        dc.LineTo(pt1);

        COLORREF color2 = ::GetSysColor(COLOR_3DHIGHLIGHT);
        CPen pen2(PS_SOLID, 1, color2);
        dc.SelectObject(&pen2);
        dc.MoveTo(pt2);
        pt2 += offset;
        dc.LineTo(pt2);

        dc.SelectObject(oldPen);
    }
}
