// 2DArray.h: interface for the C2DArray class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_2DARRAY_H__86045FE5_1A92_4495_AAD7_C276F3E7758E__INCLUDED_)
#define AFX_2DARRAY_H__86045FE5_1A92_4495_AAD7_C276F3E7758E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class C2DArray  
{
public:

	typedef void *W_PTR;

	C2DArray();
	
	void Init(int rows, int cols);

	virtual ~C2DArray();

	W_PTR& operator()(int row,int col)
	{
		ASSERT(m_pData);
		ASSERT(row*m_cols + col < m_rows*m_cols);
		
		return *(m_pData+row*m_cols + col);
	}

protected:
	W_PTR	*m_pData;
	int		m_rows;
	int		m_cols;
};

#endif // !defined(AFX_2DARRAY_H__86045FE5_1A92_4495_AAD7_C276F3E7758E__INCLUDED_)
