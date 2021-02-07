// 2DArray.cpp: implementation of the C2DArray class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Msp.h"
#include "2DArray.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
C2DArray::C2DArray():
	m_rows(0), m_cols(0), m_pData(0)
{
}

void C2DArray::Init(int rows, int cols)
{
	m_rows=rows;
	m_cols=cols;
	m_pData =  new W_PTR[m_rows * m_cols];
	memset(m_pData, 0, m_rows * m_cols);
}

C2DArray::~C2DArray()
{
	if(m_pData)
		delete[] m_pData;
}
