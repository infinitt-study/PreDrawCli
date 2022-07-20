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

static CSelectTool selectTool; // ���� 
static CRectTool lineTool(line); // ���׸��� 
static CRectTool rectTool(rect); // �簢�� 
static CRectTool roundRectTool(roundRect); //���� �ձ� �簢��  
static CRectTool ellipseTool(ellipse); // Ÿ�� 
static CPolyTool polyTool; // ������ 

CPoint CDrawTool::c_down;
UINT CDrawTool::c_nDownFlags;
CPoint CDrawTool::c_last;
DrawShape CDrawTool::c_drawShape = selection;

CDrawTool::CDrawTool(DrawShape drawShape) // enum ���� ���� 
{
	m_drawShape = drawShape; //drawShape�� ���� ���� 
	c_tools.AddTail(this); // �Ǹ����� ������ ������ ���� 
}

CDrawTool* CDrawTool::FindTool(DrawShape drawShape)
{
	POSITION pos = c_tools.GetHeadPosition(); // ����� ó�� ���� ��ġ�� ������ 
	while (pos != NULL) // �������� NULL �� �ƴҶ� 
	{
		CDrawTool* pTool = (CDrawTool*)c_tools.GetNext(pos);  // ��� ó��(head)  ��ġ ���� ����->���� Ž�� 
		if (pTool->m_drawShape == drawShape) // �׸����� �ϴ� ������ ���� 
			return pTool; 
	}

	return NULL;
}

void CDrawTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
	// deactivate any in-place active item on this view!
	COleClientItem* pActiveItem = pView->GetDocument()->GetInPlaceActiveItem(pView); // pView�� GetDocument()�� GetInPlaceActiveItem(pView)�Լ� ��ü  
	if (pActiveItem != NULL) // �ش� ���� null �� �ƴ϶�� 
	{
		pActiveItem->Close(); // ���α׷� ���� 
		ASSERT(pView->GetDocument()->GetInPlaceActiveItem(pView) == NULL); // ����� ��忡�� ���ǿ� ���� ���� ���ǿ� �´ٸ� �Ѿ�� ���ٸ� ���α׷� ���� 
	}

	pView->SetCapture(); // ���  ���콺 �Է��� CWnd�� ����
	c_nDownFlags = nFlags; // 
	c_down = point; // 
	c_last = point; // 
}

void CDrawTool::OnLButtonDblClk(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& /*point*/)
{
}

void CDrawTool::OnLButtonUp(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& point)
{
	ReleaseCapture(); // ĸó ���� 

	if (point == c_down) 
		c_drawShape = selection; //drawshape  ���� 
}

void CDrawTool::OnMouseMove(CDrawView* /*pView*/, UINT /*nFlags*/, const CPoint& point)
{
	c_last = point;
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW)); // Ŀ�� ���� ,  null ���̸� ȭ�鿡�� ������ 
}

void CDrawTool::OnEditProperties(CDrawView* /*pView*/)
{
}

void CDrawTool::OnCancel()
{
	c_drawShape = selection; //��� ��, selection ���� ���� 
}

////////////////////////////////////////////////////////////////////////////
// CResizeTool

enum SelectMode   
{
	none,  // �ƹ��͵� ���� ��
	netSelect, // �׹��� ���·� ���� ����  
	move, // �̵� 
	size // ������ ���� 
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
	if (pView->m_selection.GetCount() == 1) // 1 ���̸� 
	{
		pObj = pView->m_selection.GetHead(); //��� ���(�Ǵ� ��� ��ҿ� ���� ����)�� �����ɴϴ�.
		nDragHandle = pObj->HitTest(local, pView, TRUE); // Ŭ���̾�Ʈ ������ �������� ������ ������ ��ġ�� Ȯ��
		if (nDragHandle != 0)
			selectMode = size; // size���� 
	}

	// See if the click was on an object, select and start move if so
	if (selectMode == none)
	{
		pObj = pView->GetDocument()->ObjectAt(local); //������ �ε��� ��ο� �ִ� ��ü�� ��ȯ�մϴ�.

		if (pObj != NULL)
		{
			selectMode = move;

			if (!pView->IsSelected(pObj)) // ���� �Ӽ��� ���õƴ��� ���� ��ȯ 
				pView->Select(pObj, (nFlags & MK_SHIFT) != 0);

			// Ctrl+Click clones the selection...
			if ((nFlags & MK_CONTROL) != 0)
				pView->CloneSelection();// view �� �ִ� �Լ��� ���� ���� �̵� �ϸ鼭 ���� ��ü�� ���� �ν��Ͻ��� ���� �Ѵ�. 
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
		rect.NormalizeRect(); // �簢�� ����ȭ-> ����ȭ�� �簢���� ����Ѵ�.   
		dc.DrawFocusRect(rect);  // ��Ŀ���� ��Ÿ���� �� ���Ǵ� ��Ÿ�Ϸ� �簢���� �׸��ϴ�.
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
			pView->Deselect(pObj); //������ ��Ʈ�� ������ ����մϴ�.


	}
	else
	{
		// "Normal" DblClk opens properties, or OLE server...
		if (pView->m_selection.GetCount() == 1)
			pView->m_selection.GetHead()->OnOpen(pView); //��� ��ü�� Opening ���·� ��ȯ�� �� �ش� ��ü�� ���� ������ �Ⱓ ���� �Ϸ�Ǿ�� �ϴ� ó���� �����մϴ�.
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
			CClientDC dc(pView);//Ŭ���̾�Ʈ �������� �׸��� �׸� �� 
			CRect rect(c_down.x, c_down.y, c_last.x, c_last.y); 
			rect.NormalizeRect();
			dc.DrawFocusRect(rect); //�簢���� ��Ŀ���� ������ ��Ÿ���� �� ���Ǵ� ��Ÿ�Ϸ� �簢���� �׸��ϴ�.


			pView->SelectWithinRect(rect, TRUE);
		}
		else if (selectMode != none)
		{
			pView->GetDocument()->UpdateAllViews(pView); // Ŭ���̾�Ʈ�� ������ ��ȿȭ ��Ű�� UpdateAllViews() �Լ� 
			/*
			WM_PAINT�޼����� View Ŭ������ �������� OnDraw() �����ִ� ����
			document������ ��� �����ϴ�. 
			
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
			int nHandle = pObj->HitTest(local, pView, TRUE); //Ʈ�� �� ��Ʈ���� Ŭ���̾�Ʈ ������ �������� ������ ������ ��ġ�� Ȯ���Ϸ��� �� �Լ��� ȣ���մϴ�.
			if (nHandle != 0)
			{
				SetCursor(pObj->GetHandleCursor(nHandle)); //
				return; // bypass CDrawTool
			}
		}
		if (c_drawShape == selection) 
			CDrawTool::OnMouseMove(pView, nFlags, point); //���콺 Ŀ���� �̵��� �� ȣ��˴ϴ�.
		return;
	}

	if (selectMode == netSelect)
	{
		CClientDC dc(pView);
		CRect rect(c_down.x, c_down.y, c_last.x, c_last.y);
		rect.NormalizeRect(); //CRect���̿� �ʺ� ��� ����� �ǵ��� ����ȭ �մϴ�.
		dc.DrawFocusRect(rect); //�簢���� ��Ŀ���� ������ ��Ÿ���� �� ���Ǵ� ��Ÿ�Ϸ� �簢���� �׸��ϴ�.
		rect.SetRect(c_down.x, c_down.y, point.x, point.y); //�� ũ�⸦ CRect ������ ��ǥ�� �����մϴ�.
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
		ASSERT(FALSE); // unsuported shape! �϶��� ���α׷� ���� 

	case rect: // �簢
		pObj->m_nShape = CDrawRect::rectangle;  //CDrawRect enum  ����ü 
		break;

	case roundRect: // �ձ� �簢
		pObj->m_nShape = CDrawRect::roundRectangle;
		break;

	case ellipse: // Ÿ�� 
		pObj->m_nShape = CDrawRect::ellipse;
		break;

	case line: // ��
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
	m_pDrawObj->AddPoint(local, pView); // CPoint�� �������� �����Ͽ� Document�� vmPt��� CPoint�� �����ϴ� vector�� push_back���� ������ CPoint�� ������ �ݴϴ�.

	selectMode = size;
	nDragHandle = m_pDrawObj->GetHandleCount(); //���μ����� Ȱ�� �ڵ� ���� �����ɴϴ�.
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
		nDragHandle = m_pDrawObj->GetHandleCount();//���μ����� Ȱ�� �ڵ� ���� �����ɴϴ�.
		lastPoint = local;
		c_last = point;
		SetCursor(AfxGetApp()->LoadCursor(IDC_PENCIL));
	}
	else
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS)); //lpszCursorName���� �����ϴ� �̸� ���ǵ� Windows Ŀ�� ���ҽ��� �ε��մϴ�.
		selectTool.OnMouseMove(pView, nFlags, point);
	}
}

void CPolyTool::OnLButtonDblClk(CDrawView* pView, UINT , const CPoint& )
{
	ReleaseCapture(); //ĸ�� ���� 

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

	m_pDrawObj = NULL; // m_pDrawObj null �� 
	c_drawShape = selection; 
}

void CPolyTool::OnCancel()
{
	CDrawTool::OnCancel();

	m_pDrawObj = NULL;
}

/////////////////////////////////////////////////////////////////////////////
