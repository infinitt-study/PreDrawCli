// drawobj.cpp - implementation for drawing objects
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#include "stdafx.h"
#include "drawcli.h"

#include "drawdoc.h"
#include "drawvw.h"
#include "drawobj.h"

#include "cntritem.h"
#include "rectdlg.h"

IMPLEMENT_SERIAL(CDrawObj, CObject, 0)
//IMPLEMENT_SERIAL(): 계층 구조 내에서 클래스 이름 및 위치에 대한 런타임 액세스 권한이 있는 동적 CObject 파생 클래스에 필요한 C++ 코드를 생성합니다.

CDrawObj::CDrawObj()
{
}

CDrawObj::~CDrawObj()
{
}

CDrawObj::CDrawObj(const CRect& position)
{
	m_position = position;
	m_pDocument = NULL;

	m_bPen = TRUE;
	m_logpen.lopnStyle = PS_INSIDEFRAME;
	//PS_INSIDEFRAME: 이 펜을 경계 사각형을 사용하는 GDI 그리기 함수에서 사용하면 펜의 너비를 고려하여 경계 사각형에 완전히 맞도록 그림의 치수가 축소됩니다. 이는 기하학적 펜에만 적용됩니다.
	m_logpen.lopnWidth.x = 1;
	m_logpen.lopnWidth.y = 1;
	m_logpen.lopnColor = RGB(0, 0, 0);

	m_bBrush = TRUE;
	m_logbrush.lbStyle = BS_SOLID;
	m_logbrush.lbColor = RGB(192, 192, 192);
	m_logbrush.lbHatch = HS_HORIZONTAL;
	//HS_HORIZONTAL: 가로 빗살 무늬???
}

void CDrawObj::Serialize(CArchive& ar)
//Serialize(): 이 함수는 루트를 성공적으로 직렬화해야 하는 인스턴스 목록을 컴파일합니다.
{
	CObject::Serialize(ar);
	//Serialize(): 이 함수는 루트를 성공적으로 직렬화해야 하는 인스턴스 목록을 컴파일합니다.
	if (ar.IsStoring())
		//IsStoring(): 보관에서 데이터를 저장하는지 여부를 확인
	{
		ar << m_position;
		ar << (WORD)m_bPen;  //word : 단어 차이를 계산...?? //찾으니까 이렇게 설명이 되어있는데 이게 맞는 설명인지 모르겠네요..
		ar.Write(&m_logpen, sizeof(LOGPEN));
		//Write():지정된 바이트 수를 보관 파일에 씁니다.
		ar << (WORD)m_bBrush;
		ar.Write(&m_logbrush, sizeof(LOGBRUSH));
		//Write():지정된 바이트 수를 보관 파일에 씁니다.

	}
	else
	{
		// get the document back pointer from the archive
		m_pDocument = (CDrawDoc*)ar.m_pDocument;
		ASSERT_VALID(m_pDocument);
		//ASSERT_VALID(): 객체의 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용
		ASSERT_KINDOF(CDrawDoc, m_pDocument);
		//ASSERT_KINDOF(CDrawDoc:파생 클래스의 이름입니다. ,  m_pDocument: 클래스 객체에 대한 포인터)
		//이 매크로는 가리키는 객체가 지정된 클래스의 객체이거나 지정된 클래스에서 파생된 클래스의 객체임을 어설션합니다.

		WORD wTemp;
		ar >> m_position;
		ar >> wTemp; m_bPen = (BOOL)wTemp;
		ar.Read(&m_logpen,sizeof(LOGPEN));
		//Read(): 보관 파일에서 지정된 바이트 수를 읽습니다.
		ar >> wTemp; m_bBrush = (BOOL)wTemp;
		ar.Read(&m_logbrush, sizeof(LOGBRUSH));
		//Read(): 보관 파일에서 지정된 바이트 수를 읽습니다.

	}
}

void CDrawObj::Remove() // 지정한 CDrawObj를 모두 제거
{
	delete this;
}

void CDrawObj::Draw(CDC*) 
{
}

void CDrawObj::DrawTracker(CDC* pDC, TrackerState state) //개체의 테두리 그릴 때 호출
 {
	ASSERT_VALID(this);
	//ASSERT_VALID(): 객체의 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용

	switch (state)
	{
	case normal:
		break;

	case selected:
	case active:
		{
			int nHandleCount = GetHandleCount();
			//GetHandleCount(): 프로세스의 활성 핸들 수를 가져옵니다.
			for (int nHandle = 1; nHandle <= nHandleCount; nHandle += 1)
			{
				CPoint handle = GetHandle(nHandle);
				//GetHandle(): 요소에 대 한 HWND를 반환 합니다.
				pDC->PatBlt(handle.x - 3, handle.y - 3, 7, 7, DSTINVERT);
				//PatBlt (): 해당 DC의 비트맵 영역을 패턴형태로 초기화하는데 사용
				//DSTINVERT: 대상 사각형을 반전시킵니다.
			}
		}
		break;
	}
}

// position is in logical //위치 표시
void CDrawObj::MoveTo(const CRect& position, CDrawView* pView)
// MoveTO (): 현재 포지션을 특정한 좌표로 이동
{
	ASSERT_VALID(this);
	//ASSERT_VALID(): 객체의 메모리 영역과 멤버변수에 대한 유효성을 검사하는데 사용되는 매크로 함수 

	if (position == m_position)
		return;

	if (pView == NULL)
	{
		Invalidate();
		//Invalidate(): CWnd의 전체 클라이언트 영역을 무효화 //화면 재표시
		m_position = position;
		Invalidate();
		//Invalidate(): CWnd의 전체 클라이언트 영역을 무효화 //화면 재표시

	}
	else
	{    
		//drawview 의 InvalObj
		pView->InvalObj(this);
		m_position = position;
		pView->InvalObj(this);
	}
	m_pDocument->SetModifiedFlag();
	//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.
}

// Note: if bSelected, hit-codes start at one for the top-left
// and increment clockwise, 0 means no hit.
// If !bSelected, 0 = no hit, 1 = hit (anywhere)

// point is in logical coordinates  // 논리적 좌표 포지션
int CDrawObj::HitTest(CPoint point, CDrawView* pView, BOOL bSelected)
//대부분 끌어서 놓기 동작(Drag And Drop)을 수행할 때에 끌기(Drag) 대상 항목을
//현재 위치에 사용할 수 있는지를 결정하는 용도로 가장 많이 사용됩니다.
{
	ASSERT_VALID(this);
	//ASSERT_VALID(): 객체의 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용합니다.
	ASSERT(pView != NULL);
	//ASSERT(): 인수를 평가합니다.

	if (bSelected)
	{
		int nHandleCount = GetHandleCount();
		//GetHandleCount(): 프로세스의 활성 핸들 수를 가져옵니다.

		for (int nHandle = 1; nHandle <= nHandleCount; nHandle += 1)
		{
			// GetHandleRect returns in logical coords //GetHandleRect(): 논리적 좌표를 리턴해줌
			CRect rc = GetHandleRect(nHandle,pView);
			if (point.x >= rc.left && point.x < rc.right &&
				point.y <= rc.top && point.y > rc.bottom)
				return nHandle;
		}
	}
	else
	{
		if (point.x >= m_position.left && point.x < m_position.right &&
			point.y <= m_position.top && point.y > m_position.bottom)
			return 1;
	}
	return 0;
}

// rect must be in logical coordinates
BOOL CDrawObj::Intersects(const CRect& rect)
//Intersects(): 첫 번째 범위가 두 번째 범위와 교차 하는지 여부를 확인 합니다.
{
	ASSERT_VALID(this);
	//ASSERT_VALID(): 객체의 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용합니다.

	CRect fixed = m_position;
	//CRect: CRect 개체를 생성
	fixed.NormalizeRect();
	//NormalizeRect():CRect높이와 너비가 모두 양수가 되도록 정규화 합니다.
	CRect rectT = rect;
	rectT.NormalizeRect();
	//NormalizeRect():CRect높이와 너비가 모두 양수가 되도록 정규화 합니다.
	return !(rectT & fixed).IsRectEmpty();
	//IsRectEmpty(): 비어 있는지 여부를 CRect 확인
}

int CDrawObj::GetHandleCount()
//GetHandleCount(): 프로세스의 활성 핸들 수를 가져옵니다.
{
	ASSERT_VALID(this);
	//ASSERT_VALID(): 객체의 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용
	return 8;
}

// returns logical coords of center of handle
CPoint CDrawObj::GetHandle(int nHandle)
{
	ASSERT_VALID(this);
	int x, y, xCenter, yCenter;

	// this gets the center regardless of left/right and top/bottom ordering
	xCenter = m_position.left + m_position.Width() / 2;
	yCenter = m_position.top + m_position.Height() / 2;

	switch (nHandle)
	{
	default:
		ASSERT(FALSE);

	case 1:
		x = m_position.left;
		y = m_position.top;
		break;

	case 2:
		x = xCenter;
		y = m_position.top;
		break;

	case 3:
		x = m_position.right;
		y = m_position.top;
		break;

	case 4:
		x = m_position.right;
		y = yCenter;
		break;

	case 5:
		x = m_position.right;
		y = m_position.bottom;
		break;

	case 6:
		x = xCenter;
		y = m_position.bottom;
		break;

	case 7:
		x = m_position.left;
		y = m_position.bottom;
		break;

	case 8:
		x = m_position.left;
		y = yCenter;
		break;
	}

	return CPoint(x, y);
	//CPoint(): 인수가 지정되지 않으면 x 및 y 멤버가 0으로 설정됩니다.
}

// return rectange of handle in logical coords
CRect CDrawObj::GetHandleRect(int nHandleID, CDrawView* pView)
{
	ASSERT_VALID(this);
	ENSURE(pView != NULL);
	//ENSURE(): 데이터 정확성의 유효성을 검사하는 데 사용합니다.

	CRect rect;
	// get the center of the handle in logical coords
	CPoint point = GetHandle(nHandleID);
	//GetHandle(): 요소에 대 한 HWND를 반환 합니다.
	// convert to client/device coords
	pView->DocToClient(point);
	// return CRect of handle in device coords
	rect.SetRect(point.x-3, point.y-3, point.x+3, point.y+3);
	// SetRect(): 크기를 CRect 지정된 좌표로 설정
	pView->ClientToDoc(rect);

	return rect;
}

HCURSOR CDrawObj::GetHandleCursor(int nHandle)
{
	ASSERT_VALID(this);

	LPCTSTR id;
	switch (nHandle)
	{
	default:
		ASSERT(FALSE);

	case 1:
	case 5:
		id = IDC_SIZENWSE;//IDC_SIZENWSE(): 북동쪽과 남서쪽을 가리키는 이중 뾰족한 화살표 커서.
		break;

	case 2:
	case 6:
		id = IDC_SIZENS; //IDC_SIZENS(): 북쪽과 남쪽을 가리키는 이중 뾰족한 화살표 커서.
		break;

	case 3:
	case 7:
		id = IDC_SIZENESW;//IDC_SIZENWSE(): 북동쪽과 남서쪽을 가리키는 이중 뾰족한 화살표 커서.
		break;

	case 4:
	case 8:
		id = IDC_SIZEWE;//IDC_SIZENS(): 북쪽과 남쪽을 가리키는 이중 뾰족한 화살표 커서.
		break;
	}

	return AfxGetApp()->LoadStandardCursor(id);
	//AfxGetApp():	이 함수에서 반환된 포인터를 사용하여 기본 메시지 디스패치 코드 또는 맨 위 창과 같은 응용 프로그램 정보에 액세스할 수 있습니다.
	//LoadStandardCursor(id): id에서 지정하는 미리 정의된 Windows 커서 리소스를 로드합니다.
}

// point must be in logical
void CDrawObj::MoveHandleTo(int nHandle, CPoint point, CDrawView* pView)
{
	ASSERT_VALID(this);

	CRect position = m_position;
	switch (nHandle)
	{
	default:
		ASSERT(FALSE);

	case 1:
		position.left = point.x;
		position.top = point.y;
		break;

	case 2:
		position.top = point.y;
		break;

	case 3:
		position.right = point.x;
		position.top = point.y;
		break;

	case 4:
		position.right = point.x;
		break;

	case 5:
		position.right = point.x;
		position.bottom = point.y;
		break;

	case 6:
		position.bottom = point.y;
		break;

	case 7:
		position.left = point.x;
		position.bottom = point.y;
		break;

	case 8:
		position.left = point.x;
		break;
	}

	MoveTo(position, pView);
	//MoveTo(): 현재 포지션을 특정한 좌표로 이동
}

void CDrawObj::Invalidate()
{
	ASSERT_VALID(this);
	m_pDocument->UpdateAllViews(NULL, HINT_UPDATE_DRAWOBJ, this);
	//UpdateAllViews(): 문서가 수정된 후 이 함수를 호출
}

CDrawObj* CDrawObj::Clone(CDrawDoc* pDoc)
//clone(): 현재와 동일한 클래스의 인스턴스를 만듦.
{
	ASSERT_VALID(this);

	CDrawObj* pClone = new CDrawObj(m_position);
	pClone->m_bPen = m_bPen;
	pClone->m_logpen = m_logpen;
	pClone->m_bBrush = m_bBrush;
	pClone->m_logbrush = m_logbrush;
	ASSERT_VALID(pClone);

	if (pDoc != NULL)
		pDoc->Add(pClone);

	return pClone;
}

void CDrawObj::OnEditProperties()
{
	ASSERT_VALID(this);

	CPropertySheet sheet( _T("Shape Properties") );
	//CPropertySheet: 속성 시트를 나타냄
	CRectDlg dlg;
	dlg.m_bNoFill = !m_bBrush;
	dlg.m_penSize = m_bPen ? m_logpen.lopnWidth.x : 0;
	sheet.AddPage( &dlg );
	//AddPage(): 제공된 페이지를 속성 시트의 맨 오른쪽 탭에 추가합니다.

	if (sheet.DoModal() != IDOK)
		//DoModal(): 이 멤버 함수를 호출하여 모달 대화 상자를 호출하고 완료되면 대화 상자 결과를 반환합니다.

		return;

	m_bBrush = !dlg.m_bNoFill;
	m_bPen = dlg.m_penSize > 0;
	if (m_bPen)
	{
		m_logpen.lopnWidth.x = dlg.m_penSize;
		m_logpen.lopnWidth.y = dlg.m_penSize;
	}

	Invalidate();
	m_pDocument->SetModifiedFlag();
	//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.

}

void CDrawObj::OnOpen(CDrawView* /*pView*/ )
//OnOpen(): 통신 개체가 Opening 상태로 전환한 후 해당 개체에 대해 지정된 기간 내에 완료되어야 하는 처리를 삽입합니다.
{
	OnEditProperties();
	//이 컨트롤을 재정의하여 페이지의 적절한 컨트롤로 포커스를 설정할 수 있습
}

void CDrawObj::SetLineColor(COLORREF color)
//SetLineColor(): 트리 뷰 컨트롤의 현재 선 색을 설정하려면 이 멤버 함수를 호출합니다
{
	ASSERT_VALID(this);

	m_logpen.lopnColor = color;
	Invalidate(); //무효화 시키는 함수
	m_pDocument->SetModifiedFlag();
	//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.
}

void CDrawObj::SetFillColor(COLORREF color)
{
	ASSERT_VALID(this);

	m_logbrush.lbColor = color;
	Invalidate();
	m_pDocument->SetModifiedFlag();
	//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.

}

#ifdef _DEBUG
void CDrawObj::AssertValid()
//AssertValid(): 이 개체의이 무결성을 확인합니다.
{
	ASSERT(m_position.left <= m_position.right);
	ASSERT(m_position.bottom <= m_position.top);
}
#endif

////////////////////////////////////////////////////////////////////////////
// CDrawRect

IMPLEMENT_SERIAL(CDrawRect, CDrawObj, 0)

CDrawRect::CDrawRect()
{
}

CDrawRect::CDrawRect(const CRect& position)
	: CDrawObj(position)
{
	ASSERT_VALID(this);

	m_nShape = rectangle;
	m_roundness.x = 16;
	m_roundness.y = 16;
}

void CDrawRect::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	CDrawObj::Serialize(ar);
	if (ar.IsStoring())
		//IsStoring(): 보관에서 데이터를 저장하는지 여부를 확인

	{
		ar << (WORD) m_nShape;
		ar << m_roundness;
	}
	else
	{
		WORD wTemp;
		ar >> wTemp; m_nShape = (Shape)wTemp;
		ar >> m_roundness;
	}
}

void CDrawRect::Draw(CDC* pDC)
{
	ASSERT_VALID(this);

	CBrush brush;
	if (!brush.CreateBrushIndirect(&m_logbrush))
		//CreateBrushIndirect(): 구조체에 지정된 스타일, 색 및 패턴을 사용하여 브러시를  초기화합니다.
		return;
	CPen pen;
	if (!pen.CreatePenIndirect(&m_logpen))
		//CreatePenIndirect(): 가리키는 구조체에 지정된 스타일, 너비 및 색이 있는 펜을 초기화
		return;

	CBrush* pOldBrush;
	CPen* pOldPen;

	if (m_bBrush)
		pOldBrush = pDC->SelectObject(&brush);
	// SelectObject(): 펜과 같은 GDI 그리기 개체를 선택
	else
		pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
	//SelectStockObject(): Windows 제공된 미리 정의된 스톡 펜, 브러시 또는 글꼴 중 하나를 선택

	if (m_bPen)
		pOldPen = pDC->SelectObject(&pen);
	// SelectObject(): 펜과 같은 GDI 그리기 개체를 선택
	else
		pOldPen = (CPen*)pDC->SelectStockObject(NULL_PEN);
	//SelectStockObject(): Windows 제공된 미리 정의된 스톡 펜, 브러시 또는 글꼴 중 하나를 선택
	CRect rect = m_position;
	switch (m_nShape)
	{
	case rectangle:
		pDC->Rectangle(rect);
		//Rectangle(): 현재 펜을 사용하여 사각형을 그리고 현재 브러시를 사용하여 채웁니다.
		break;

	case roundRectangle:
		pDC->RoundRect(rect, m_roundness);
		//RoundRect():	현재 펜을 사용하여 모서리가 둥근 사각형을 그리고 현재 브러시를 사용하여 채운 사각형을 그립니다.
		break;

	case ellipse:
		pDC->Ellipse(rect);
		//Ellipse(): 타원을 그립니다.
		break;

	case line:
		if (rect.top > rect.bottom)
		{
			rect.top -= m_logpen.lopnWidth.y / 2;
			rect.bottom += (m_logpen.lopnWidth.y + 1) / 2;
		}
		else
		{
			rect.top += (m_logpen.lopnWidth.y + 1) / 2;
			rect.bottom -= m_logpen.lopnWidth.y / 2;
		}

		if (rect.left > rect.right)
		{
			rect.left -= m_logpen.lopnWidth.x / 2;
			rect.right += (m_logpen.lopnWidth.x + 1) / 2;
		}
		else
		{
			rect.left += (m_logpen.lopnWidth.x + 1) / 2;
			rect.right -= m_logpen.lopnWidth.x / 2;
		}

		pDC->MoveTo(rect.TopLeft());
		//TopLeft(): 사각형의 왼쪽 위 모퉁이의 좌표를 지정
		pDC->LineTo(rect.BottomRight());
		// BottomRight(): 사각형의 오른쪽 아래 모퉁이의 좌표를 지정
		break;
	}

	pDC->SelectObject(pOldBrush); //SelectObject(): 펜과 같은 GDI 그리기 개체를 선택합니다
	pDC->SelectObject(pOldPen);//SelectObject(): 펜과 같은 GDI 그리기 개체를 선택합니다
}


int CDrawRect::GetHandleCount()
{
	ASSERT_VALID(this);

	return m_nShape == line ? 2 :
		CDrawObj::GetHandleCount() + (m_nShape == roundRectangle);
	//GetHandleCount(): 프로세스의 활성 핸들 수를 가져옵니다.
}

// returns center of handle in logical coordinates
CPoint CDrawRect::GetHandle(int nHandle)
{
	ASSERT_VALID(this);

	if (m_nShape == line && nHandle == 2)
		nHandle = 5;
	else if (m_nShape == roundRectangle && nHandle == 9)
	{
		CRect rect = m_position;
		rect.NormalizeRect();
		//NormalizeRect(): CRect높이와 너비가 모두 양수가 되도록 정규화 
		CPoint point = rect.BottomRight();
		//BottomRight(): 사각형의 오른쪽 아래 모퉁이의 좌표를 지정 합니다.
		point.x -= m_roundness.x / 2;
		point.y -= m_roundness.y / 2;
		return point;
	}

	return CDrawObj::GetHandle(nHandle);
}

HCURSOR CDrawRect::GetHandleCursor(int nHandle)
{
	ASSERT_VALID(this);

	if (m_nShape == line && nHandle == 2)
		nHandle = 5;
	else if (m_nShape == roundRectangle && nHandle == 9)
		return AfxGetApp()->LoadStandardCursor(IDC_SIZEALL);
	//AfxGetApp():	이 함수에서 반환된 포인터를 사용하여 기본 메시지 디스패치 코드 또는 맨 위 창과 같은 응용 프로그램 정보에 액세스할 수 있습니다.
	//LoadStandardCursor(): 성공한 경우 커서에 대한 핸들입니다. 그렇지 않으면 NULL 
	//IDC_SIZEALL():  네 개의 뾰족한 화살표입니다. 창 크기를 조정하는 데 사용할 커서입니다.

	return CDrawObj::GetHandleCursor(nHandle);
}

// point is in logical coordinates
void CDrawRect::MoveHandleTo(int nHandle, CPoint point, CDrawView* pView)
{
	ASSERT_VALID(this);
	//ASSERT_VALID(): 객체의 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용
	if (m_nShape == line && nHandle == 2)
		nHandle = 5;
	else if (m_nShape == roundRectangle && nHandle == 9)
	{
		CRect rect = m_position;
		rect.NormalizeRect();
		//NormalizeRect(): 높이와 너비가 모두 양수가 되도록 정규화
		if (point.x > rect.right - 1)
			point.x = rect.right - 1;
		else if (point.x < rect.left + rect.Width() / 2)
			point.x = rect.left + rect.Width() / 2;
		if (point.y > rect.bottom - 1)
			point.y = rect.bottom - 1;
		else if (point.y < rect.top + rect.Height() / 2)
			point.y = rect.top + rect.Height() / 2;
		m_roundness.x = 2 * (rect.right - point.x);
		m_roundness.y = 2 * (rect.bottom - point.y);
		m_pDocument->SetModifiedFlag();
		//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.


		if (pView == NULL)
			Invalidate();
		else
			pView->InvalObj(this);
		return;
	}

	CDrawObj::MoveHandleTo(nHandle, point, pView);
}

// rect must be in logical coordinates
BOOL CDrawRect::Intersects(const CRect& rect)
//Intersects(): 첫 번째 범위가 두 번째 범위와 교차 하는지 여부를 확인 
{
	ASSERT_VALID(this);

	CRect rectT = rect;
	rectT.NormalizeRect();
	//NormalizeRect(): CRect높이와 너비가 모두 양수가 되도록 정규화 합니

	CRect fixed = m_position;
	fixed.NormalizeRect();
	if ((rectT & fixed).IsRectEmpty())
		//IsRectEmpty(): 비어 있는지 여부를 확인
		return FALSE;

	CRgn rgn;
	switch (m_nShape)
	{
	case rectangle:
		return TRUE;

	case roundRectangle:
		rgn.CreateRoundRectRgn(fixed.left, fixed.top, fixed.right, fixed.bottom,
			//CreateRoundRectRgn(): 개체에 저장된 둥근 모서리가 있는 사각형 영역을 만듭니다
			m_roundness.x, m_roundness.y);
		break;

	case ellipse:
		rgn.CreateEllipticRgnIndirect(fixed);
		//CreateEllipticRgnIndirect(): 타원형 영역을 만듦
		break;

	case line:
		{
			int x = (m_logpen.lopnWidth.x + 5) / 2;
			int y = (m_logpen.lopnWidth.y + 5) / 2;
			POINT points[4];
			points[0].x = fixed.left;
			points[0].y = fixed.top;
			points[1].x = fixed.left;
			points[1].y = fixed.top;
			points[2].x = fixed.right;
			points[2].y = fixed.bottom;
			points[3].x = fixed.right;
			points[3].y = fixed.bottom;

			if (fixed.left < fixed.right)
			{
				points[0].x -= x;
				points[1].x += x;
				points[2].x += x;
				points[3].x -= x;
			}
			else
			{
				points[0].x += x;
				points[1].x -= x;
				points[2].x -= x;
				points[3].x += x;
			}

			if (fixed.top < fixed.bottom)
			{
				points[0].y -= y;
				points[1].y += y;
				points[2].y += y;
				points[3].y -= y;
			}
			else
			{
				points[0].y += y;
				points[1].y -= y;
				points[2].y -= y;
				points[3].y += y;
			}
			rgn.CreatePolygonRgn(points, 4, ALTERNATE);
			//ALTERNATE: 대체 옵션을 나타냅니다.
			//CreatePolygonRgn(): 다각형 영역을 만듦
		}
		break;
	}
	return rgn.RectInRegion(fixed);
}	//RectInRegion(): fixed로 지정된 사각형의 일부가 개체에 저장된 rgn 영역의 경계 내에 있는지 여부를 확인합니다

CDrawObj* CDrawRect::Clone(CDrawDoc* pDoc)
{
	ASSERT_VALID(this);

	CDrawRect* pClone = new CDrawRect(m_position);
	pClone->m_bPen = m_bPen;
	pClone->m_logpen = m_logpen;
	pClone->m_bBrush = m_bBrush;
	pClone->m_logbrush = m_logbrush;
	pClone->m_nShape = m_nShape;
	pClone->m_roundness = m_roundness;
	ASSERT_VALID(pClone);

	if (pDoc != NULL)
		pDoc->Add(pClone);

	ASSERT_VALID(pClone);
	return pClone;
}

////////////////////////////////////////////////////////////////////////////
// CDrawPoly

IMPLEMENT_SERIAL(CDrawPoly, CDrawObj, 0)
//IMPLEMENT_SERIAL(): 계층 구조 내에서 클래스 이름 및 위치에 대한 런타임 액세스 권한이 있는 동적 CObject 파생 클래스에 필요한 C++ 코드를 생성

CDrawPoly::CDrawPoly()
{
	m_points = NULL;
	m_nPoints = 0;
	m_nAllocPoints = 0;
}

CDrawPoly::CDrawPoly(const CRect& position)
	: CDrawObj(position)
{
	m_points = NULL;
	m_nPoints = 0;
	m_nAllocPoints = 0;
	m_bPen = TRUE;
	m_bBrush = FALSE;
}

CDrawPoly::~CDrawPoly()
{
	if (m_points != NULL)
		delete[] m_points;
}

void CDrawPoly::Serialize( CArchive& ar )
{
	int i;
	CDrawObj::Serialize( ar );
	if( ar.IsStoring() )
		//IsStoring(): 보관에서 데이터를 저장하는지 여부를 확인

	{
		ar << (WORD) m_nPoints;
		ar << (WORD) m_nAllocPoints;
		for (i = 0;i< m_nPoints; i++)
			ar << m_points[i];
	}
	else
	{
		WORD wTemp;
		ar >> wTemp; m_nPoints = wTemp;
		ar >> wTemp; m_nAllocPoints = wTemp;
		m_points = new CPoint[m_nAllocPoints];
		for (i = 0;i < m_nPoints; i++)
			ar >> m_points[i];
	}
}

void CDrawPoly::Draw(CDC* pDC)
{
	ASSERT_VALID(this);

	CBrush brush;
	if (!brush.CreateBrushIndirect(&m_logbrush))
		//CreateBrushIndirect(): 구조체에 지정된 스타일, 색 및 패턴을 사용하여 브러시를 LOGBRUSH 초기화합니다.
		return;
	CPen pen;
	if (!pen.CreatePenIndirect(&m_logpen))
		//CreatePenIndirect(): 가리키는 구조체에 지정된 스타일, 너비 및 색이 있는 펜을 초기화
		return;

	CBrush* pOldBrush;
	CPen* pOldPen;

	if (m_bBrush)
		pOldBrush = pDC->SelectObject(&brush);
	//SelectObject(): 디바이스 컨텍스트로 개체를 선택
	else
		pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	if (m_bPen)
		pOldPen = pDC->SelectObject(&pen);
	//SelectObject(): 디바이스 컨텍스트로 개체를 선택
	else
		pOldPen = (CPen*)pDC->SelectStockObject(NULL_PEN);
	//SelectStockObject(): 미리 정의된 스톡 펜, 브러시 또는 글꼴 중 하나에 해당하는 개체를 선택

	pDC->Polygon(m_points, m_nPoints);
	//Polygon(): 현재 펜을 사용하여 선으로 연결된 둘 이상의 점(꼭짓점)으로 구성된 다각형을 그립니다.

	pDC->SelectObject(pOldBrush);
	//SelectObject(): 디바이스 컨텍스트로 개체를 선택
	pDC->SelectObject(pOldPen);
	//SelectObject(): 디바이스 컨텍스트로 개체를 선택

}

// position must be in logical coordinates
void CDrawPoly::MoveTo(const CRect& position, CDrawView* pView)
{
	ASSERT_VALID(this);
	if (position == m_position)
		return;

	if (pView == NULL)
		Invalidate();
	else
		pView->InvalObj(this);

	for (int i = 0; i < m_nPoints; i += 1)
	{
		m_points[i].x += position.left - m_position.left;
		m_points[i].y += position.top - m_position.top;
	}

	m_position = position;

	if (pView == NULL)
		Invalidate();
	else
		pView->InvalObj(this);
	m_pDocument->SetModifiedFlag();
	//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.
}

int CDrawPoly::GetHandleCount()
{
	return m_nPoints;
}

CPoint CDrawPoly::GetHandle(int nHandle)
{
	ASSERT_VALID(this);

	ASSERT(nHandle >= 1 && nHandle <= m_nPoints);
	return m_points[nHandle - 1];
}

HCURSOR CDrawPoly::GetHandleCursor(int )
{
	return AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	//IDC_ARROW(): 표준 화살표 커서.
}

// point is in logical coordinates
void CDrawPoly::MoveHandleTo(int nHandle, CPoint point, CDrawView* pView)
{
	ASSERT_VALID(this);
	ASSERT(nHandle >= 1 && nHandle <= m_nPoints);
	//ASSERT: 인수를 평가
	if (m_points[nHandle - 1] == point)
		return;

	m_points[nHandle - 1] = point;
	RecalcBounds(pView);

	if (pView == NULL)
		Invalidate();
	else
		pView->InvalObj(this);
	m_pDocument->SetModifiedFlag();
	//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.

}

// rect must be in logical coordinates
BOOL CDrawPoly::Intersects(const CRect& rect)
{
	ASSERT_VALID(this);
	CRgn rgn;
	rgn.CreatePolygonRgn(m_points, m_nPoints, ALTERNATE);
	return rgn.RectInRegion(rect);
	//RectInRegion(): rect으로 지정된 사각형의 일부가 개체에 저장된 영역의 경계 내에 있는지 여부 확인
}

CDrawObj* CDrawPoly::Clone(CDrawDoc* pDoc)
{
	ASSERT_VALID(this);

#pragma warning(suppress:6211)
	CDrawPoly* pClone = new CDrawPoly(m_position);
	pClone->m_bPen = m_bPen;
	pClone->m_logpen = m_logpen;
	pClone->m_bBrush = m_bBrush;
	pClone->m_logbrush = m_logbrush;
	pClone->m_points = new CPoint[m_nAllocPoints];
	
	memcpy_s(pClone->m_points, sizeof(CPoint) * m_nAllocPoints, m_points, sizeof(CPoint) * m_nPoints);
	//memcpy_s(): 버퍼 간에 바이트를 복사
	pClone->m_nAllocPoints = m_nAllocPoints;
	pClone->m_nPoints = m_nPoints;
	ASSERT_VALID(pClone);

	if (pDoc != NULL)
		pDoc->Add(pClone);

	ASSERT_VALID(pClone);
	return pClone;
}

// point is in logical coordinates
void CDrawPoly::AddPoint(const CPoint& point, CDrawView* pView)
{
	ASSERT_VALID(this);
	if (m_nPoints == m_nAllocPoints)
	{
		CPoint* newPoints = new CPoint[m_nAllocPoints + 10];
		if (m_points != NULL)
		{
			memcpy_s(newPoints, sizeof(CPoint) * (m_nAllocPoints + 10), m_points, sizeof(CPoint) * m_nAllocPoints);
			//memcpy_s(): 버퍼 간에 바이트를 복사
			delete[] m_points;
		}
		m_points = newPoints;
		m_nAllocPoints += 10;
	}

	if (m_nPoints == 0 || m_points[m_nPoints - 1] != point)
	{
		m_points[m_nPoints++] = point;
		if (!RecalcBounds(pView))
		{
			if (pView == NULL)
				Invalidate();
			else
				pView->InvalObj(this);
		}
		m_pDocument->SetModifiedFlag();
		//SetModifiedFlag(): 문서를 수정한 후 이 함수를 호출합니다.

	}
}

BOOL CDrawPoly::RecalcBounds(CDrawView* pView)
{
	ASSERT_VALID(this);

	if (m_nPoints == 0)
		return FALSE;

	CRect bounds(m_points[0], CSize(0, 0));
	for (int i = 1; i < m_nPoints; ++i)
	{
		if (m_points[i].x < bounds.left)
			bounds.left = m_points[i].x;
		if (m_points[i].x > bounds.right)
			bounds.right = m_points[i].x;
		if (m_points[i].y < bounds.top)
			bounds.top = m_points[i].y;
		if (m_points[i].y > bounds.bottom)
			bounds.bottom = m_points[i].y;
	}

	if (bounds == m_position)
		return FALSE;

	if (pView == NULL)
		Invalidate();
	else
		pView->InvalObj(this);

	m_position = bounds;

	if (pView == NULL)
		Invalidate();
	else
		pView->InvalObj(this);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CDrawOleObj, CDrawObj, 0)
//IMPLEMENT_SERIAL():
//계층 구조 내에서 클래스 이름 및 위치에 대한 런타임 액세스 권한이 있는 
//동적 CObject 파생 클래스에 필요한 C++ 코드를 생성합니다.

BOOL CDrawOleObj::c_bShowItems = TRUE;

CDrawOleObj::CDrawOleObj() : m_extent(0,0)
{
	m_pClientItem = NULL;
}

CDrawOleObj::CDrawOleObj(const CRect& position)
	: CDrawObj(position), m_extent(0, 0)
{
	m_pClientItem = NULL;
}

CDrawOleObj::~CDrawOleObj()
{
	if (m_pClientItem != NULL)
	{
		m_pClientItem->Release();
		//Release(): 이 함수를 호출하여 OLE 항목에서 사용하는 리소스를 정리
		m_pClientItem = NULL;
	}
}

void CDrawOleObj::Remove()
{
	if (m_pClientItem != NULL)
	{
		m_pClientItem->Delete();
		//Delete(): 컨테이너 문서에서 OLE 항목을 삭제하려면 이 함수를 호출
		m_pClientItem = NULL;
	}
	CDrawObj::Remove();
}

void CDrawOleObj::Serialize( CArchive& ar )
{
	ASSERT_VALID(this);

	CDrawObj::Serialize(ar);

	if (ar.IsStoring())
		//IsStoring(): 보관에서 데이터를 저장하는지 여부를 확인
	{
		ar << m_extent;
		ar << m_pClientItem;
	}
	else
	{
		ar >> m_extent;
		ar >> m_pClientItem;
		m_pClientItem->m_pDrawObj = this;
	}
}

CDrawObj* CDrawOleObj::Clone(CDrawDoc* pDoc)
{
	ASSERT_VALID(this);

	AfxGetApp()->BeginWaitCursor();
	//BeginWaitCursor(): 명령을 실행할 때 눈에 띄는 시간 간격이 소요되는 경우 커서를 모래 시계로 표시하려면 이 함수를 호출합니다.

	CDrawOleObj* pClone = NULL;
	CDrawItem* pItem = NULL;
	TRY
	{
		// perform a "deep copy" -- need to copy CDrawOleObj and the CDrawItem
		//  that it points to.
		pClone = new CDrawOleObj(m_position);
#pragma warning(suppress:6014)
		pItem = new CDrawItem(m_pDocument, pClone);
		if (!pItem->CreateCloneFrom(m_pClientItem))
			//CreateCloneFrom(): 이 함수를 호출하여 지정된 OLE 항목의 복사본을 만듭니다
		{
			AfxThrowMemoryException();
			//AfxThrowMemoryException(): 메모리 예외를 throw합니다.
		}

		pClone->m_pClientItem = pItem;
		pClone->m_bPen = m_bPen;
		pClone->m_logpen = m_logpen;
		pClone->m_bBrush = m_bBrush;
		pClone->m_logbrush = m_logbrush;
		ASSERT_VALID(pClone);

		if (pDoc != NULL)
			pDoc->Add(pClone);
	}
	CATCH_ALL(e)
	{
		if(pItem)
			pItem->Delete();

		if(pClone)
		{
			pClone->m_pClientItem = NULL;
			pClone->Remove();
		}
		AfxGetApp()->EndWaitCursor();
		//EndWaitCursor(): 모래 시계 커서에서 이전 커서로 반환하도록 멤버 함수를 호출한 후 이 함수를 호출 BeginWaitCursor 합니다. 

		THROW_LAST();
		//THROW_LAST(): 예외를 다음 외부 CATCH 블록으로 다시 throw합니다.
	}
	END_CATCH_ALL//마지막 CATCH_ALL 또는 블록의 끝AND_CATCH_ALL 표시합니다.

	AfxGetApp()->EndWaitCursor();
	//EndWaitCursor(): 모래 시계 커서에서 이전 커서로 반환하도록 멤버 함수를 호출한 후 이 함수를 호출 BeginWaitCursor 합니다. 
	return pClone;
}

void CDrawOleObj::Draw(CDC* pDC)
{
	ASSERT_VALID(this);

	CDrawItem* pItem = m_pClientItem;
	if (pItem != NULL)
	{
		// draw the OLE item itself
		pItem->Draw(pDC, m_position);

		// don't draw tracker in print preview or on printer
		if (!pDC->IsPrinting())
			//IsPrinting():디바이스 컨텍스트가 인쇄에 사용되는지 여부를 확인
		{
			// use a CRectTracker to draw the standard effects
			CRectTracker tracker;
			tracker.m_rect = m_position;
			pDC->LPtoDP(tracker.m_rect);
			//LPtoDP():논리 단위를 디바이스 단위로 변환

			if (c_bShowItems)
			{
				// put correct border depending on item type
				if (pItem->GetType() == OT_LINK)
					//GetType(): 이 함수를 호출하여 OLE 항목이 포함 또는 연결되었는지 또는 정적 항목인지 확인합니다.
					tracker.m_nStyle |= CRectTracker::dottedLine;
				else
					tracker.m_nStyle |= CRectTracker::solidLine;
			}

			// put hatching over the item if it is currently open
			if (pItem->GetItemState() == COleClientItem::openState ||
				pItem->GetItemState() == COleClientItem::activeUIState)
				//GetItemState():  함수를 호출하여 OLE 항목의 현재 상태를 가져옵니다.
			{
				tracker.m_nStyle |= CRectTracker::hatchInside;
			}
			tracker.Draw(pDC);
		}
	}
}

void CDrawOleObj::OnOpen(CDrawView* pView)
{
	AfxGetApp()->BeginWaitCursor();
	//BeginWaitCursor(): 명령을 실행할 때 눈에 띄는 시간 간격이 소요되는 경우 커서를 모래 시계로 표시하려면 이 함수를 호출합니다.
	//DoVerb(): 지정된 동사를 실행하기 위한 호출
	//OLEIVERB_OPEN: 별도의 창에서 항목 편집
	//OLEIVERB_PRIMARY: 기본 동사
	//VK_CONTROL: control 키
	//GetKeyState():지정된 가상 키의 상태를 검색합니다. 상태는 키가 위, 아래 또는 토글(켜짐, 꺼짐, 키를 누를 때마다 번갈아 가며)인지 여부를 지정
	m_pClientItem->DoVerb(
		GetKeyState(VK_CONTROL) < 0 ? OLEIVERB_OPEN : OLEIVERB_PRIMARY,
		pView);
	AfxGetApp()->EndWaitCursor();
	//EndWaitCursor(): 모래 시계 커서에서 이전 커서로 반환하도록 멤버 함수를 호출한 후 이 함수를 호출 BeginWaitCursor 합니다.
}

void CDrawOleObj::OnEditProperties()
{
	// using COlePropertiesDialog directly means no scaling
	COlePropertiesDialog dlg(m_pClientItem, 100, 100, NULL);

	dlg.DoModal();
	//DoModal(): 이 멤버 함수를 호출하여 모달 대화 상자를 호출하고 완료되면 대화 상자 결과를 반환합니다.
}

// position is in logical
void CDrawOleObj::MoveTo(const CRect& position, CDrawView* pView)
{
	ASSERT_VALID(this);

	if (position == m_position)
		return;

	// call base class to update position
	CDrawObj::MoveTo(position, pView);

	// update position of in-place editing session on position change
	if (m_pClientItem->IsInPlaceActive())
		//IsInPlaceActive(): 이 함수를 호출하여 OLE 항목이 현재 위치 활성 상태인지 확인합
		m_pClientItem->SetItemRects();
	//SetItemRects():이 함수를 호출하여 경계 사각형 또는 OLE 항목의 표시되는 사각형을 설정합니다.
}

/////////////////////////////////////////////////////////////////////////////
