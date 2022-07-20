// drawtool.cpp - implementation for drawing tools
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
#include "drawtool.h"

/////////////////////////////////////////////////////////////////////////////
// CDrawTool implementation

CPtrList CDrawTool::c_tools;

static CSelectTool selectTool; // 선택 
static CRectTool lineTool(line); // 선그리기 
static CRectTool rectTool(rect); // 사각형 
static CRectTool roundRectTool(roundRect); //끝이 둥근 사각형  
static CRectTool ellipseTool(ellipse); // 타원 
static CPolyTool polyTool; // 폴리곤 

CPoint CDrawTool::c_down;
UINT CDrawTool::c_nDownFlags;
CPoint CDrawTool::c_last;
DrawShape CDrawTool::c_drawShape = selection;

CDrawTool::CDrawTool(DrawShape drawShape) // enum 인자 받음 
{
	m_drawShape = drawShape; //drawShape에 대한 정보 
	c_tools.AddTail(this); // 맨마지막 꼬리의 정보를 얻음 
}

CDrawTool* CDrawTool::FindTool(DrawShape drawShape)
{
	POSITION pos = c_tools.GetHeadPosition(); // 목록의 처음 지점 위치를 가져옴 
	while (pos != NULL) // 포지션이 NULL 이 아닐때 
	{
		CDrawTool* pTool = (CDrawTool*)c_tools.GetNext(pos);  // 목록 처음(head)  위치 부터 다음->다음 탐색 
		if (pTool->m_drawShape == drawShape) // 그리려고 하는 도형이 뭔지 
			return pTool; 
	}

	return NULL;
}

void CDrawTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	// deactivate any in-place active item on this view!
	COleClientItem* pActiveItem = pView->GetDocument()->GetInPlaceActiveItem(pView); // pView의 GetDocument()의 GetInPlaceActiveItem(pView)함수 객체  
	if (pActiveItem != NULL) // 해당 값이 null 이 아니라면 
	{
		pActiveItem->Close(); // 프로그램 종료 
		ASSERT(pView->GetDocument()->GetInPlaceActiveItem(pView) == NULL); // 디버깅 모드에서 조건에 대한 검증 조건에 맞다면 넘어가고 없다면 프로그램 종료 
	}

	pView->SetCapture(); // 모든  마우스 입력을 CWnd로 보냄
	c_nDownFlags = nFlags; // 
	c_down = point; // 
	c_last = point; // 
}

void CDrawTool::OnLButtonDblClk(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& /*point*/)
{
}

void CDrawTool::OnLButtonUp(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& point)
{
	ReleaseCapture(); // 캡처 종료 

	if (point == c_down) 
		c_drawShape = selection; //drawshape  선택 
}

void CDrawTool::OnMouseMove(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& point)
{
	c_last = point;
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW)); // 커서 설정 ,  null 값이면 화면에서 없어짐 
}

void CDrawTool::OnEditProperties(CDrawView* /*pView*/)
{
}

void CDrawTool::OnCancel()
{
	c_drawShape = selection; //취소 시, selection 으로 가기 
}

////////////////////////////////////////////////////////////////////////////
// CResizeTool

enum SelectMode   
{
	none,  // 아무것도 없을 떄
	netSelect, // 그물의 형태로 영역 선택  
	move, // 이동 
	size // 사이즈 조절 
};

SelectMode selectMode = none;
int nDragHandle; 

CPoint lastPoint;

CSelectTool::CSelectTool()
	: CDrawTool(selection)
{
}

void CSelectTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	CPoint local = point; 
	pView->ClientToDoc(local);

	CDrawObj* pObj;
	selectMode = none;

	// Check for resizing (only allowed on single selections)
	if (pView->m_selection.GetCount() == 1) // 1 번이면 
	{
		pObj = pView->m_selection.GetHead(); //헤드 요소(또는 헤드 요소에 대한 참조)를 가져옵니다.
		nDragHandle = pObj->HitTest(local, pView, TRUE); // 클라이언트 영역을 기준으로 지정된 지점의 위치를 확인
		if (nDragHandle != 0)
			selectMode = size; // size조절 
	}

	// See if the click was on an object, select and start move if so
	if (selectMode == none)
	{
		pObj = pView->GetDocument()->ObjectAt(local); //지정된 인덱스 경로에 있는 개체를 반환합니다.

		if (pObj != NULL)
		{
			selectMode = move;

			if (!pView->IsSelected(pObj)) // 현재 속성이 선택됐는지 여부 반환 
				pView->Select(pObj, (nFlags & MK_SHIFT) != 0);

			// Ctrl+Click clones the selection...
			if ((nFlags & MK_CONTROL) != 0)
				pView->CloneSelection();// view 에 있는 함수로 다음 으로 이동 하면서 현재 객체와 같은 인스턴스를 생성 한다. 
		}
	}

	// Click on background, start a net-selection
	if (selectMode == none) 
	{
		if ((nFlags & MK_SHIFT) == 0)
			pView->Select(NULL);

		selectMode = netSelect;

		CClientDC dc(pView);
		CRect rect(point.x, point.y, point.x, point.y); 
		rect.NormalizeRect(); // 사각형 정규화-> 정규화된 사각형만 사용한다.   
		dc.DrawFocusRect(rect);  // 포커스를 나타내는 데 사용되는 스타일로 사각형을 그립니다.
	}

	lastPoint = local;
	CDrawTool::OnLButtonDown(pView, nFlags, point);
}

void CSelectTool::OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	if ((nFlags & MK_SHIFT) != 0)
	{
		// Shift+DblClk deselects object...
		CPoint local = point;
		pView->ClientToDoc(local);
		CDrawObj* pObj = pView->GetDocument()->ObjectAt(local);
		if (pObj != NULL)
			pView->Deselect(pObj); //지정된 차트의 선택을 취소합니다.


	}
	else
	{
		// "Normal" DblClk opens properties, or OLE server...
		if (pView->m_selection.GetCount() == 1)
			pView->m_selection.GetHead()->OnOpen(pView); //통신 개체가 Opening 상태로 전환한 후 해당 개체에 대해 지정된 기간 내에 완료되어야 하는 처리를 삽입합니다.
	}

	CDrawTool::OnLButtonDblClk(pView, nFlags, point);
}

void CSelectTool::OnEditProperties(CDrawView* pView)
{
	if (pView->m_selection.GetCount() == 1)
		pView->m_selection.GetHead()->OnEditProperties();
}

void CSelectTool::OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	if (pView->GetCapture() == pView)
	{
		if (selectMode == netSelect) 
		{
			CClientDC dc(pView);//클라이언트 영역에서 그림을 그릴 때 
			CRect rect(c_down.x, c_down.y, c_last.x, c_last.y); 
			rect.NormalizeRect();
			dc.DrawFocusRect(rect); //사각형에 포커스가 있음을 나타내는 데 사용되는 스타일로 사각형을 그립니다.


			pView->SelectWithinRect(rect, TRUE);
		}
		else if (selectMode != none)
		{
			pView->GetDocument()->UpdateAllViews(pView); // 클라이언트의 영역을 무효화 시키는 UpdateAllViews() 함수 
			/*
			WM_PAINT메세지가 View 클래스에 전해지고 OnDraw() 시켜주는 역할
			document에서도 사용 가능하다. 
			
			*/
		}
	}

	CDrawTool::OnLButtonUp(pView, nFlags, point);
}

void CSelectTool::OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	if (pView->GetCapture() != pView)
	{
		if (c_drawShape == selection && pView->m_selection.GetCount() == 1)
		{
			CDrawObj* pObj = pView->m_selection.GetHead();
			CPoint local = point; 
			pView->ClientToDoc(local);
			int nHandle = pObj->HitTest(local, pView, TRUE); //트리 뷰 컨트롤의 클라이언트 영역을 기준으로 지정된 지점의 위치를 확인하려면 이 함수를 호출합니다.
			if (nHandle != 0)
			{
				SetCursor(pObj->GetHandleCursor(nHandle)); //
				return; // bypass CDrawTool
			}
		}
		if (c_drawShape == selection) 
			CDrawTool::OnMouseMove(pView, nFlags, point); //마우스 커서가 이동할 때 호출됩니다.
		return;
	}

	if (selectMode == netSelect)
	{
		CClientDC dc(pView);
		CRect rect(c_down.x, c_down.y, c_last.x, c_last.y);
		rect.NormalizeRect(); //CRect높이와 너비가 모두 양수가 되도록 정규화 합니다.
		dc.DrawFocusRect(rect); //사각형에 포커스가 있음을 나타내는 데 사용되는 스타일로 사각형을 그립니다.
		rect.SetRect(c_down.x, c_down.y, point.x, point.y); //의 크기를 CRect 지정된 좌표로 설정합니다.
		rect.NormalizeRect();
		dc.DrawFocusRect(rect);

		CDrawTool::OnMouseMove(pView, nFlags, point);
		return;
	}

	CPoint local = point;
	pView->ClientToDoc(local);
	CPoint delta = (CPoint)(local - lastPoint);

	POSITION pos = pView->m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = pView->m_selection.GetNext(pos);
		CRect position = pObj->m_position;

		if (selectMode == move)
		{
			position += delta;
			pObj->MoveTo(position, pView);
		}
		else if (nDragHandle != 0)
		{
			pObj->MoveHandleTo(nDragHandle, local, pView);
		}
	}

	lastPoint = local;

	if (selectMode == size && c_drawShape == selection)
	{
		c_last = point;
		SetCursor(pView->m_selection.GetHead()->GetHandleCursor(nDragHandle));
		return; // bypass CDrawTool
	}

	c_last = point;

	if (c_drawShape == selection)
		CDrawTool::OnMouseMove(pView, nFlags, point);
}

////////////////////////////////////////////////////////////////////////////
// CRectTool (does rectangles, round-rectangles, and ellipses)

CRectTool::CRectTool(DrawShape drawShape)
	: CDrawTool(drawShape)
{
}

void CRectTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	CDrawTool::OnLButtonDown(pView, nFlags, point);

	CPoint local = point;
	pView->ClientToDoc(local);

	CDrawRect* pObj = new CDrawRect(CRect(local, CSize(0, 0)));
	switch (m_drawShape)
	{
	default:
		ASSERT(FALSE); // unsuported shape! 일때는 프로그램 종료 

	case rect: // 사각
		pObj->m_nShape = CDrawRect::rectangle;  //CDrawRect enum  구조체 
		break;

	case roundRect: // 둥근 사각
		pObj->m_nShape = CDrawRect::roundRectangle;
		break;

	case ellipse: // 타원 
		pObj->m_nShape = CDrawRect::ellipse;
		break;

	case line: // 선
		pObj->m_nShape = CDrawRect::line;
		break;
	}
	pView->GetDocument()->Add(pObj);
	pView->Select(pObj);

	selectMode = size;
	nDragHandle = 1;
	lastPoint = local;
}

void CRectTool::OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	CDrawTool::OnLButtonDblClk(pView, nFlags, point);
}

void CRectTool::OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	if (point == c_down)
	{
		// Don't create empty objects...
		CDrawObj *pObj = pView->m_selection.GetTail();
		pView->GetDocument()->Remove(pObj);
		pObj->Remove();
		selectTool.OnLButtonDown(pView, nFlags, point); // try a select!
	}

	selectTool.OnLButtonUp(pView, nFlags, point);
}

void CRectTool::OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
	selectTool.OnMouseMove(pView, nFlags, point);
}


////////////////////////////////////////////////////////////////////////////
// CPolyTool

CPolyTool::CPolyTool()
	: CDrawTool(poly)
{
	m_pDrawObj = NULL;
}

void CPolyTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	CDrawTool::OnLButtonDown(pView, nFlags, point);

	CPoint local = point;
	pView->ClientToDoc(local);

	if (m_pDrawObj == NULL)
	{
		pView->SetCapture();

		m_pDrawObj = new CDrawPoly(CRect(local, CSize(0, 0)));
		pView->GetDocument()->Add(m_pDrawObj);
		pView->Select(m_pDrawObj);
		m_pDrawObj->AddPoint(local, pView);
	}
	else if (local == m_pDrawObj->m_points[0])
	{
		// Stop when the first point is repeated...
		ReleaseCapture();
		m_pDrawObj->m_nPoints -= 1;
		if (m_pDrawObj->m_nPoints < 2)
		{
			m_pDrawObj->Remove();
		}
		else
		{
			pView->InvalObj(m_pDrawObj);
		}
		m_pDrawObj = NULL;
		c_drawShape = selection;
		return;
	}

	local.x += 1; // adjacent points can't be the same!
	m_pDrawObj->AddPoint(local, pView); // CPoint를 동적으로 생성하여 Document의 vmPt라는 CPoint를 관리하는 vector에 push_back으로 생성된 CPoint를 저장해 줍니다.

	selectMode = size;
	nDragHandle = m_pDrawObj->GetHandleCount(); //프로세스의 활성 핸들 수를 가져옵니다.
	lastPoint = local;
}

void CPolyTool::OnLButtonUp(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& /*point*/)
{
	// Don't release capture yet!
}

void CPolyTool::OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	if (m_pDrawObj != NULL && (nFlags & MK_LBUTTON) != 0)
	{
		CPoint local = point;
		pView->ClientToDoc(local);
		m_pDrawObj->AddPoint(local);
		nDragHandle = m_pDrawObj->GetHandleCount();//프로세스의 활성 핸들 수를 가져옵니다.
		lastPoint = local;
		c_last = point;
		SetCursor(AfxGetApp()->LoadCursor(IDC_PENCIL));
	}
	else
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS)); //lpszCursorName에서 지정하는 미리 정의된 Windows 커서 리소스를 로드합니다.
		selectTool.OnMouseMove(pView, nFlags, point);
	}
}

void CPolyTool::OnLButtonDblClk(CDrawView* pView, UINT , const CPoint& )
{
	ReleaseCapture(); //캡쳐 해제 

	int nPoints = m_pDrawObj->m_nPoints;
	if (nPoints > 2 &&
		(m_pDrawObj->m_points[nPoints - 1] == m_pDrawObj->m_points[nPoints - 2] ||
		m_pDrawObj->m_points[nPoints - 1].x - 1 == m_pDrawObj->m_points[nPoints - 2].x &&
		m_pDrawObj->m_points[nPoints - 1].y == m_pDrawObj->m_points[nPoints - 2].y))

	{
		// Nuke the last point if it's the same as the next to last...
		m_pDrawObj->m_nPoints -= 1;
		pView->InvalObj(m_pDrawObj);
	}

	m_pDrawObj = NULL; // m_pDrawObj null 값 
	c_drawShape = selection; 
}

void CPolyTool::OnCancel()
{
	CDrawTool::OnCancel();

	m_pDrawObj = NULL;
}

/////////////////////////////////////////////////////////////////////////////
