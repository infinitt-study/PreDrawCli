// drawvw.cpp : implementation of the CDrawView class
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
#include <afxpriv.h>

#include "drawcli.h"

#include "drawdoc.h"
#include "drawobj.h"
#include "cntritem.h"
#include "drawvw.h"

#include "drawobj.h"
#include "drawtool.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// private clipboard format (list of Draw objects)
CLIPFORMAT CDrawView::m_cfDraw = (CLIPFORMAT)
	::RegisterClipboardFormat(_T("MFC Draw Sample"));
CLIPFORMAT CDrawView::m_cfObjectDescriptor = NULL;

/////////////////////////////////////////////////////////////////////////////
// CDrawView

IMPLEMENT_DYNCREATE(CDrawView, CScrollView)

BEGIN_MESSAGE_MAP(CDrawView, CScrollView)
	//{{AFX_MSG_MAP(CDrawView)
	ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
	ON_COMMAND(ID_CANCEL_EDIT, OnCancelEdit)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_DRAW_SELECT, OnDrawSelect)
	ON_COMMAND(ID_DRAW_ROUNDRECT, OnDrawRoundRect)
	ON_COMMAND(ID_DRAW_RECT, OnDrawRect)
	ON_COMMAND(ID_DRAW_LINE, OnDrawLine)
	ON_COMMAND(ID_DRAW_ELLIPSE, OnDrawEllipse)
	ON_UPDATE_COMMAND_UI(ID_DRAW_ELLIPSE, OnUpdateDrawEllipse)
	ON_UPDATE_COMMAND_UI(ID_DRAW_LINE, OnUpdateDrawLine)
	ON_UPDATE_COMMAND_UI(ID_DRAW_RECT, OnUpdateDrawRect)
	ON_UPDATE_COMMAND_UI(ID_DRAW_ROUNDRECT, OnUpdateDrawRoundRect)
	ON_UPDATE_COMMAND_UI(ID_DRAW_SELECT, OnUpdateDrawSelect)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVEBACK, OnUpdateSingleSelect)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateAnySelect)
	ON_COMMAND(ID_DRAW_POLYGON, OnDrawPolygon)
	ON_UPDATE_COMMAND_UI(ID_DRAW_POLYGON, OnUpdateDrawPolygon)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GRID, OnUpdateViewGrid)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_OBJECT_FILLCOLOR, OnObjectFillColor)
	ON_COMMAND(ID_OBJECT_LINECOLOR, OnObjectLineColor)
	ON_COMMAND(ID_OBJECT_MOVEBACK, OnObjectMoveBack)
	ON_COMMAND(ID_OBJECT_MOVEFORWARD, OnObjectMoveForward)
	ON_COMMAND(ID_OBJECT_MOVETOBACK, OnObjectMoveToBack)
	ON_COMMAND(ID_OBJECT_MOVETOFRONT, OnObjectMoveToFront)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_SHOWOBJECTS, OnViewShowObjects)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWOBJECTS, OnUpdateViewShowObjects)
	ON_COMMAND(ID_EDIT_PROPERTIES, OnEditProperties)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateEditProperties)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVEFORWARD, OnUpdateSingleSelect)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVETOBACK, OnUpdateSingleSelect)
	ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVETOFRONT, OnUpdateSingleSelect)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawView construction/destruction

CDrawView::CDrawView()
{
	m_bGrid = TRUE; // 보조선 
	m_gridColor = RGB(0, 0, 128); // 색초기화 
	m_bActive = FALSE; //활성화  
// new
	if( m_cfObjectDescriptor == NULL )
		m_cfObjectDescriptor = (CLIPFORMAT)::RegisterClipboardFormat(_T("Object Descriptor") );
	m_prevDropEffect = DROPEFFECT_NONE;
// end new
}

CDrawView::~CDrawView()
{
}

BOOL CDrawView::PreCreateWindow(CREATESTRUCT& cs)
{
	ASSERT(cs.style & WS_CHILD);
	if (cs.lpszClass == NULL)
		cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS);
	return TRUE;
}

void CDrawView::OnActivateView(BOOL bActivate, CView* pActiveView,
	CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActiveView, pDeactiveView);

	// invalidate selections when active status changes
	if (m_bActive != bActivate)
	{
		if (bActivate)  // if becoming active update as if active
			m_bActive = bActivate;
		if (!m_selection.IsEmpty())
			OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
		m_bActive = bActivate;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView drawing

void CDrawView::InvalObj(CDrawObj* pObj)
{
	CRect rect = pObj->m_position; //pobj의 m_position 을 rect에 대입 
	DocToClient(rect);
	if (m_bActive && IsSelected(pObj)) //VIEW가 ACTIVE , 선택된 pObj가 있다면  
	{
		rect.left -= 4;
		rect.top -= 5;
		rect.right += 5;
		rect.bottom += 4;
	}
	rect.InflateRect(1, 1); // handles CDrawOleObj objects //InflateRect 는 CRect 측면의 중심에서 멀리 이동하여 확장합니다.

	InvalidateRect(rect, FALSE); //InvalidateRect 함수:  지정된 창의 업데이트 영역에 사각형을 추가합니다. 
}

void CDrawView::OnUpdate(CView* , LPARAM lHint, CObject* pHint)
{
	switch (lHint)
	{
	case HINT_UPDATE_WINDOW:    // redraw entire window
		Invalidate(FALSE);// 그림을 그릴 떄 마다 다시 그리기 : 깜빡임 현상 일으킬 수 있음 
		break;

	case HINT_UPDATE_DRAWOBJ:   // a single object has changed
		InvalObj((CDrawObj*)pHint);
		break;

	case HINT_UPDATE_SELECTION: // an entire selection has changed
		{
			CDrawObjList* pList = pHint != NULL ?
				(CDrawObjList*)pHint : &m_selection;
			POSITION pos = pList->GetHeadPosition();
			while (pos != NULL)
				InvalObj(pList->GetNext(pos));
		}
		break;

	case HINT_DELETE_SELECTION: // an entire selection has been removed 지워진것들 
		if (pHint != &m_selection)
		{
			CDrawObjList* pList = (CDrawObjList*)pHint;
			POSITION pos = pList->GetHeadPosition();
			while (pos != NULL)
			{
				CDrawObj* pObj = pList->GetNext(pos);
				InvalObj(pObj);
				Remove(pObj);   // remove it from this view's selection
			}
		}
		break;

	case HINT_UPDATE_OLE_ITEMS:
		{
			CDrawDoc* pDoc = GetDocument(); //이 함수를 호출하여 보기의 문서에 대한 포인터를 가져옵니다.
			POSITION pos = pDoc->GetObjects()->GetHeadPosition();
			while (pos != NULL)
			{
				CDrawObj* pObj = pDoc->GetObjects()->GetNext(pos); //	조사된 정보의 크기를 리턴해 준다.
				if (pObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj))) // CDrawOleObj 런타임 클래스 구조를 가져옵니다.
					InvalObj(pObj);
			}
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
}

void CDrawView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	CScrollView::OnPrepareDC(pDC, pInfo); // 생성자 함수 

	// mapping mode is MM_ANISOTROPIC
	// these extents setup a mode similar to MM_LOENGLISH
	// MM_LOENGLISH is in .01 physical inches
	// these extents provide .01 logical inches

	pDC->SetMapMode(MM_ANISOTROPIC); //매핑 모드를 설정합니다. ->MM_ANISOTROPIC 논리 단위는 임의로 배율 조정된 축이 있는 임의의 단위로 변환됩니다.  
	pDC->SetViewportExt(pDC->GetDeviceCaps(LOGPIXELSX), // 지정된 장치에 대한 장치별 정보를 검색합니다.
		pDC->GetDeviceCaps(LOGPIXELSY)); //디스플레이 디바이스에 대한 다양한 디바이스 관련 정보를 검색합니다.
	pDC->SetWindowExt(100, -100); // 논리적인 화면 dc 의 크기를  100 , -100 으로 고정 한다. 

	// set the origin of the coordinate system to the center of the page
	CPoint ptOrg; 
	ptOrg.x = GetDocument()->GetSize().cx / 2; //이 배열에 있는 요소의 수를 가져옵니다
	ptOrg.y = GetDocument()->GetSize().cy / 2;

	// ptOrg is in logical coordinates
	pDC->OffsetWindowOrg(-ptOrg.x,ptOrg.y); //현재 창 원점의 좌표를 기준으로 창 원점의 좌표를 수정합니다.
}

BOOL CDrawView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	// do the scroll
	if (!CScrollView::OnScrollBy(sizeScroll, bDoScroll)) //사용자가 보기의 현재 테두리에 대해 OLE 항목을 끌거나 세로 또는 가로 스크롤 막대를 조작하여 문서의 현재 보기 너머 영역을 볼 때 프레임워크에서 호출
		return FALSE; //스크롤 함수가 없다면 false 리턴

	// update the position of any in-place active item
	if (bDoScroll)
	{
		UpdateActiveItem();
		UpdateWindow();
	}
	return TRUE;
}

void CDrawView::OnDraw(CDC* pDC)
{
	CDrawDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc); //개체 내부 상태의 유효성에 대한 가정을 테스트

	CDC dc; //CDC
	CDC* pDrawDC = pDC; 
	CBitmap bitmap; // bitmap 구조체 변수 선언 
	CBitmap* pOldBitmap = 0;

	// only paint the rect that needs repainting
	CRect client;
	pDC->GetClipBox(client); //사각형 차원을 RECT 받을 구조체 또는 CRect 개체를 가리킵니다. 
	CRect rect = client;
	DocToClient(rect);

	if (!pDC->IsPrinting())
	{
		// draw to offscreen bitmap for fast looking repaints
		if (dc.CreateCompatibleDC(pDC))
		{
			if (bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()))
			{
				OnPrepareDC(&dc, NULL);
				pDrawDC = &dc;

				// offset origin more because bitmap is just piece of the whole drawing
				dc.OffsetViewportOrg(-rect.left, -rect.top);
				pOldBitmap = dc.SelectObject(&bitmap);
				dc.SetBrushOrg(rect.left % 8, rect.top % 8);

				// might as well clip to the same rectangle
				dc.IntersectClipRect(client);
			}
		}
	}

	// paint background
	CBrush brush;
	if (!brush.CreateSolidBrush(pDoc->GetPaperColor()))
		return;

	brush.UnrealizeObject();
	pDrawDC->FillRect(client, &brush);

	if (!pDC->IsPrinting() && m_bGrid)
		DrawGrid(pDrawDC);

	pDoc->Draw(pDrawDC, this);

	if (pDrawDC != pDC)
	{
		pDC->SetViewportOrg(0, 0); //현재 사용중인 DC의 Viewport의 기준점을 이동시킵니다. 
		pDC->SetWindowOrg(0,0); //이 함수는 현재 윈도우의 출력 기준좌표를 사용자가 지정한 좌표로 변경하는 함수
		pDC->SetMapMode(MM_TEXT); //  우리가 프로그램에서 사용한 논리적인 출력단위를 실제 출력장치(예를들면 그래픽카드와 같은 장치)에서
		dc.SetViewportOrg(0, 0);
		dc.SetWindowOrg(0,0);
		dc.SetMapMode(MM_TEXT);
		pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(),
			&dc, 0, 0, SRCCOPY);
		dc.SelectObject(pOldBitmap);
	}
}

void CDrawView::Remove(CDrawObj* pObj)
{
	POSITION pos = m_selection.Find(pObj);
	if (pos != NULL)
		m_selection.RemoveAt(pos); // 해당 위치의 요소를 지워 준다 .
}

void CDrawView::PasteNative(COleDataObject& dataObject)
{
	// get file refering to clipboard data
	CFile* pFile = dataObject.GetFileData(m_cfDraw); //이 함수를 호출하여 파생 개체를 CFile만들고 CFile 지정된 형식의 데이터를 포인터로 CFile 검색합니다.
	if (pFile == NULL)
		return;

	// connect the file to the archive
	CArchive ar(pFile, CArchive::load);
	TRY
	{
		ar.m_pDocument = GetDocument(); // set back-pointer in archive 보기의 문서에 대한 포인터를 가져온다. 

		// read the selection
		m_selection.Serialize(ar); // 저장과 로드를 한번에 하는 직렬화  
	}
	CATCH_ALL(e)
	{
		ar.Close();
		delete pFile;
		THROW_LAST(); //예외를 다음 외부 CATCH 블록으로 다시 throw합니다.
	}
	END_CATCH_ALL

	ar.Close(); //버퍼에 남아 있는 모든 데이터를 플러시하고, 보관 파일을 닫고, 파일에서 보관의 연결을 끊습니다.
	delete pFile;
}

void CDrawView::PasteEmbedded(COleDataObject& dataObject, CPoint point )
{
	BeginWaitCursor(); //사용자가 장기 작업을 수행하는 동안 대기 커서를 표시하는 한 가지 방법(일반적으로 모래시계로 표시됨)을 제공합니다.

	// paste embedded
	CDrawOleObj* pObj = new CDrawOleObj(GetInitialPosition());
	ASSERT_VALID(pObj); //개체 내부 상태의 유효성에 대한 가정을 테스트하는 데 사용합니다.


#pragma warning(suppress:6014)
	CDrawItem* pItem = new CDrawItem(GetDocument(), pObj);
	ASSERT_VALID(pItem);
	pObj->m_pClientItem = pItem;

	TRY
	{
		if (!pItem->CreateFromData(&dataObject) && //지정된 지역 및 변환 데이터에서 지역을 만듭니다.
			!pItem->CreateStaticFromData(&dataObject))// 개체에서 정적 항목을 만들려면 이 함수를 COleDataObject 호출합니다.

		{
			AfxThrowMemoryException();      // any exception will do
		}

		// add the object to the document
		GetDocument()->Add(pObj);
		m_selection.AddTail(pObj);
		ClientToDoc( point );
		pObj->MoveTo( CRect( point, pObj->m_extent ), this );

		// try to get initial presentation data
		pItem->UpdateLink(); // 연결된 항목의 경우 함수는 링크 원본을 찾아 OLE 항목에 대한 새 프레젠테이션을 가져옵니다. 
		pItem->UpdateExtent(); 
	}
	CATCH_ALL(e)
	{
		// clean up item
		pItem->Delete();
		pObj->m_pClientItem = NULL;
		GetDocument()->Remove(pObj);
		pObj->Remove();

		AfxMessageBox(IDP_FAILED_TO_CREATE);
	}
	END_CATCH_ALL

	EndWaitCursor();
}

void CDrawView::DrawGrid(CDC* pDC) // grid그리기 
{
	CDrawDoc* pDoc = GetDocument(); 

	COLORREF oldBkColor = pDC->SetBkColor(pDoc->GetPaperColor()); // 배경색을 변경한다.
	//SetBkColor : 현재 배경색을 지정된 색으로 설정합니다.
	CRect rect; // rect 점 정의  
	rect.left = -pDoc->GetSize().cx / 2;
	rect.top = -pDoc->GetSize().cy / 2; // x, y 의 -1/2 값을  left top 
	rect.right = rect.left + pDoc->GetSize().cx; //위의 구한 값의 + 2배 를 더해 준 값이 오른쪽 ,위  끝좌표 
	rect.bottom = rect.top + pDoc->GetSize().cy;

	// Center lines
	CPen penDash; 
	penDash.CreatePen(PS_DASH, 1, m_gridColor); // 지정한 스타일, 너비 및 브러시 특성을 사용하여 논리적 화장품 또는 기하학적 펜을 만들고 개체에 CPen 연결합니다.
	CPen* pOldPen = pDC->SelectObject(&penDash); 

	pDC->MoveTo(0, rect.top); //현재 위치를 지정한 지점(y또는 기준)으로 xpoint이동합니다.
	pDC->LineTo(0, rect.bottom); //현재 위치에서 지정 xy 한 점 및(또는 point)까지 선을 그립니다.
	pDC->MoveTo(rect.left, 0);
	pDC->LineTo(rect.right, 0);

	// Major unit lines
	CPen penDot;
	penDot.CreatePen(PS_DOT, 1, m_gridColor); //지정한 스타일, 너비 및 브러시 특성을 사용하여 논리적 화장품 또는 기하학적 펜을 만들고 개체에 CPen 연결합니다.
	pDC->SelectObject(&penDot); //디바이스 컨텍스트로 개체를 선택합니다.
	// 세로선 
	for (int x = rect.left / 100 * 100; x < rect.right; x += 100)
	{
		if (x != 0)
		{
			pDC->MoveTo(x, rect.top); //현재 위치를 지정한 지점(y또는 기준)으로 xpoint이동합니다.
			pDC->LineTo(x, rect.bottom); //현재 위치에서 지정 xy 한 점 및(또는 point)까지 선을 그립니다.
		}
	}
	//가로선 
	for (int y = rect.top / 100 * 100; y < rect.bottom; y += 100)
	{
		if (y != 0)
		{
			pDC->MoveTo(rect.left, y);
			pDC->LineTo(rect.right, y);
		}
	}

	// Outlines  왼 위 부터 시게 방향으로 선을 그린다. 
	CPen penSolid;
	penSolid.CreatePen(PS_SOLID, 1, m_gridColor);
	pDC->SelectObject(&penSolid);
	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right, rect.top);
	pDC->LineTo(rect.right, rect.bottom);
	pDC->LineTo(rect.left, rect.bottom);
	pDC->LineTo(rect.left, rect.top);

	pDC->SelectObject(pOldPen);
	pDC->SetBkColor(oldBkColor);
}

void CDrawView::OnInitialUpdate()
{
	CSize size = GetDocument()->GetSize();
	CClientDC dc(NULL);
	size.cx = MulDiv(size.cx, dc.GetDeviceCaps(LOGPIXELSX), 100);  // GetDeviceCaps : 다양한 디바이스 관련 정보를 검색
	size.cy = MulDiv(size.cy, dc.GetDeviceCaps(LOGPIXELSY), 100);
	SetScrollSizes(MM_TEXT, size);
}

void CDrawView::SetPageSize(CSize size)
{
	CClientDC dc(NULL);
	size.cx = MulDiv(size.cx, dc.GetDeviceCaps(LOGPIXELSX), 100);
	size.cy = MulDiv(size.cy, dc.GetDeviceCaps(LOGPIXELSY), 100); // GetDeviceCaps : 다양한 디바이스 관련 정보를 검색
	SetScrollSizes(MM_TEXT, size); // 스크롤에 대한 정보를 매핑 한다. 
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_WINDOW, NULL); //문서가 수정된 후 이 함수를 호출합니다.

}

/////////////////////////////////////////////////////////////////////////////
// CDrawView printing

BOOL CDrawView::OnPreparePrinting(CPrintInfo* pInfo) //문서를 미리 보거나 인쇄 하기 전에 프레임 워크에서 호출 됩니다.
{
	// default preparation
	return DoPreparePrinting(pInfo); //인쇄 또는 인쇄 미리 보기를 시작할 수 있는 경우 0이 아닌 경우 작업이 취소된 경우 0입니다.
}

void CDrawView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CScrollView::OnBeginPrinting(pDC,pInfo);

	// check page size -- user could have gone into print setup
	// from print dialog and changed paper or orientation
	GetDocument()->ComputePageSize(); // 고정된 인쇄 규격값 설정 
}

void CDrawView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// OLE Client support and commands

BOOL CDrawView::IsSelected(const CObject* pDocItem) const //노드가 선택되었는지 여부를 가져옵니다.
{
	CDrawObj* pDrawObj = (CDrawObj*)pDocItem;
	if (pDocItem->IsKindOf(RUNTIME_CLASS(CDrawItem))) // 런타임 클래스 구조를 가져옵니다.
		pDrawObj = ((CDrawItem*)pDocItem)->m_pDrawObj;
	return m_selection.Find(pDrawObj) != NULL; //문서가 수정된 후 이 함수를 호출합니다.

}

void CDrawView::OnInsertObject()
{
	// Invoke the standard Insert Object dialog box to obtain information
	//  for new CDrawItem object.
	COleInsertDialog dlg;
	if (dlg.DoModal() != IDOK) //다이얼로그가 반환되는 값을 검사 -> 종료 되었을 경우 IDOK 를반환
		return;

	BeginWaitCursor();

	// First create the C++ object
	CDrawOleObj* pObj = new CDrawOleObj(GetInitialPosition());
	ASSERT_VALID(pObj); // 해당하는 값이 아니면 프로그램 종료 
	CDrawItem* pItem = new CDrawItem(GetDocument(), pObj);
	ASSERT_VALID(pItem); // 해당하는 값이 아니면 프로그램 종료 
	pObj->m_pClientItem = pItem;

	// Now create the OLE object/item
	TRY
	{
		if (!dlg.CreateItem(pObj->m_pClientItem))
			AfxThrowMemoryException(); //메모리 할당 실패와 같은 예외를 발생  

		// add the object to the document
		GetDocument()->Add(pObj);

		// try to get initial presentation data
		pItem->UpdateLink(); //OLE 항목의 프레젠테이션 데이터를 즉시 업데이트 -> 성공하면 0이 아닌값 , 실패하면 0 
		pItem->UpdateExtent(); 

		// if insert new object -- initially show the object
		if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)//데이터 뷰 계층 구조에 새 노드를 만듭니다

			pItem->DoVerb(OLEIVERB_SHOW, this); // 지정된 동사를 실행하기 위한 호출 DoVerb 입니다.
	}
	CATCH_ALL(e)
	{
		// clean up item
		pItem->Delete();
		pObj->m_pClientItem = NULL;
		GetDocument()->Remove(pObj);
		pObj->Remove();

		AfxMessageBox(IDP_FAILED_TO_CREATE);
	}
	END_CATCH_ALL

	EndWaitCursor();//// 모래 시계 커서에서 이전 커서로 반환하도록 멤버 함수를 호출한 후 이 함수를 호출 BeginWaitCursor 합니다.
}

// The following command handler provides the standard keyboard
//  user interface to cancel an in-place editing session.
void CDrawView::OnCancelEdit()
{
	// deactivate any in-place active item on this view!
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this); //pWnd로 식별된 보기를 포함하는 프레임 창에서 현재 활성화된 OLE 항목을 가져오려면 이 함수를 호출합니다.


	if (pActiveItem != NULL)
	{
		// if we found one, deactivate it
		pActiveItem->Close();
	}
	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);

	// escape also brings us back into select mode
	ReleaseCapture();

	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
	{
		pTool->OnCancel();
	}

	CDrawTool::c_drawShape = selection;
}

void CDrawView::OnSetFocus(CWnd* pOldWnd)
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this); //pWnd로 식별된 보기를 포함하는 프레임 창에서 현재 활성화된 OLE 항목을 가져오려면 이 함수를 호출합니다.
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState) // 이 함수를 호출하여 OLE 항목의 현재 상태를 가져옵니다.


	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != NULL)
		{
			pWnd->SetFocus(); //입력 포커스를 클레임합니다.
			return;
		}
	}

	CScrollView::OnSetFocus(pOldWnd);
}

CRect CDrawView::GetInitialPosition()
{
	CRect rect(10, 10, 10, 10); // 사각형 포지션 초기화 
	ClientToDoc(rect); 
	return rect;
}

void CDrawView::ClientToDoc(CPoint& point)
{
	CClientDC dc(this); //CClientDC  객체 생성 
	OnPrepareDC(&dc, NULL);  // 화면 표시를 위해 멤버 함수가 OnDraw 호출되기 전과 인쇄 또는 인쇄 미리 보기 중에 각 페이지에 대해 멤버 함수가 호출되기 전에 OnPrint 프레임워크에서 호출됩니다.
	dc.DPtoLP(&point);
}

void CDrawView::ClientToDoc(CRect& rect)
{
	CClientDC dc(this);//CClientDC  객체 생성 
	OnPrepareDC(&dc, NULL);   //화면 표시를 위해 멤버 함수가 OnDraw 호출되기 전과 인쇄 또는 인쇄 미리 보기 중에 각 페이지에 대해 멤버 함수가 호출되기 전에 OnPrint 프레임워크에서 호출됩니다.
	dc.DPtoLP(rect); //디바이스 단위를 논리 단위로 변환합니다.
	ASSERT(rect.left <= rect.right); // 괄호 사항이 해당되지 않는다면 FALSE 
	ASSERT(rect.bottom <= rect.top); //top > bottom , right > left을 만족하는지 확인  
}

void CDrawView::DocToClient(CPoint& point)
{
	CClientDC dc(this); // dc객체 생성 
	OnPrepareDC(&dc, NULL);
	//화면 표시를 위해 멤버 함수가 OnDraw 호출되기 전과 인쇄 또는 인쇄 미리 보기 중에 각 페이지에 대해 멤버 함수가 호출되기 전에 OnPrint 프레임워크에서 호출됩니다.
	dc.LPtoDP(&point);//디바이스 단위를 논리 단위로 변환합니다.
}

void CDrawView::DocToClient(CRect& rect)
{
	CClientDC dc(this);// dc객체 생성 
	OnPrepareDC(&dc, NULL);
	dc.LPtoDP(rect);
	rect.NormalizeRect(); //CRect높이와 너비가 모두 양수가 되도록 정규화 합니다.
}

void CDrawView::Select(CDrawObj* pObj, BOOL bAdd)
{
	if (!bAdd)
	{
		OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL); // 뷰에서 해당 수정 내용을 반영하도록 디스플레이를 업데이트
		m_selection.RemoveAll(); // 해당 mselection 요소를 전체 삭제
	}

	if (pObj == NULL || IsSelected(pObj)) 
		return;

	m_selection.AddTail(pObj); //  뒷부분에 새 요소 또는 요소 목록을 추가 
	InvalObj(pObj); 
}

// rect is in device coordinates
void CDrawView::SelectWithinRect(CRect rect, BOOL bAdd)
{
	if (!bAdd)
		Select(NULL);

	ClientToDoc(rect);

	CDrawObjList* pObList = GetDocument()->GetObjects();
	POSITION posObj = pObList->GetHeadPosition(); // 첫번째 포지션을 선택한다. 
	while (posObj != NULL)
	{
		CDrawObj* pObj = pObList->GetNext(posObj); 
		if (pObj->Intersects(rect))  // 사각형의 겹치는 부분을 체크 
			Select(pObj, TRUE);
	}
}

void CDrawView::Deselect(CDrawObj* pObj)
{
	POSITION pos = m_selection.Find(pObj); 
	if (pos != NULL)
	{
		InvalObj(pObj);
		m_selection.RemoveAt(pos); 
	}
}

void CDrawView::CloneSelection()
{
	POSITION pos = m_selection.GetHeadPosition();  // 목록의 첫번째 데이터를 얻는다. 
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos); // 다음 데이터를 찾으면서 움직임  
		pObj->Clone(pObj->m_pDocument); //현재 EnumValue와 동일한 EnumValue 클래스의 인스턴스를 만듭니다.
			// copies object and adds it to the document
	}
}

void CDrawView::UpdateActiveItem()
{
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this); //pWnd로 식별된 보기를 포함하는 프레임 창에서 현재 활성화된 OLE 항목을 가져옴
	if (pActiveItem != NULL && 
		pActiveItem->GetItemState() == COleClientItem::activeUIState) //GetItemState : OLE 항목의 현재 상태를 가져옵니다. activeUIState: 
	{
		// this will update the item rectangles by calling
		//  OnGetPosRect & OnGetClipRect.
		pActiveItem->SetItemRects(); //경계 사각형 또는 OLE 항목의 표시되는 사각형을 설정
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDrawView message handlers
// down, up , move , dbclick 모두 CDrawTool::FindTool(CDrawTool::c_drawShape)에 의해 결정 
void CDrawView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_bActive)  //view 가 활성화 되어 있지 않다면 
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape); // FindTool의 c_drawShape 
	if (pTool != NULL) // c_drawShape 이 null 이 아니면 
	{
		pTool->OnLButtonDown(this, nFlags, point);
	}
}
		
void CDrawView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape); //지정된 명령 ID와 연결된 개체에 대한 포인터 CMFCUserTool 를 반환합니다. 
	if (pTool != NULL)
		pTool->OnLButtonUp(this, nFlags, point); 
}

void CDrawView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
		pTool->OnMouseMove(this, nFlags, point);
}

void CDrawView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!m_bActive)
		return;
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
	if (pTool != NULL)
		pTool->OnLButtonDblClk(this, nFlags, point);
}

void CDrawView::OnDestroy()
{
	CScrollView::OnDestroy(); 

	// deactivate the inplace active item on this view
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this); //pWnd로 식별된 보기를 포함하는 프레임 창에서 현재 활성화된 OLE 항목을 가져오려면 이 함수를 호출합니다.
	if (pActiveItem != NULL && pActiveItem->GetActiveView() == this) //항목이 현재 위치로 활성화된 보기를 반환
	{
		pActiveItem->Deactivate(); //항목을 비활성화합니다.
		ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
	}
}
//enum DrawShape 내에 있는 변수를 불러온다. 
void CDrawView::OnDrawSelect()
{
	CDrawTool::c_drawShape = selection; 
}

void CDrawView::OnDrawRoundRect()
{
	CDrawTool::c_drawShape = roundRect;
}

void CDrawView::OnDrawRect()
{
	CDrawTool::c_drawShape = rect;
}

void CDrawView::OnDrawLine()
{
	CDrawTool::c_drawShape = line;
}

void CDrawView::OnDrawEllipse()
{
	CDrawTool::c_drawShape = ellipse;
}

void CDrawView::OnDrawPolygon()
{
	CDrawTool::c_drawShape = poly;
}

void CDrawView::OnUpdateDrawEllipse(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == ellipse);
	//SetRadio:	이 명령에 대한 사용자 인터페이스 항목의 라디오 그룹 내의 확인 상태를 설정
}

void CDrawView::OnUpdateDrawLine(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == line);
}

void CDrawView::OnUpdateDrawRect(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == rect);
}

void CDrawView::OnUpdateDrawRoundRect(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == roundRect);
}

void CDrawView::OnUpdateDrawSelect(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == selection);
}

void CDrawView::OnUpdateSingleSelect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_selection.GetCount() == 1); // 
	//Enable : 이 명령에 대한 사용자 인터페이스 항목을 사용하거나 사용하지 않도록 설정합니다.
}

void CDrawView::OnEditSelectAll()
{
	CDrawObjList* pObList = GetDocument()->GetObjects(); //이 목록의 요소 수를 가져옵니다.
	POSITION pos = pObList->GetHeadPosition();
	while (pos != NULL)
		Select(pObList->GetNext(pos), TRUE);
}

void CDrawView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetObjects()->GetCount() != 0);
}

void CDrawView::OnEditClear()
{
	// update all the views before the selection goes away
	GetDocument()->UpdateAllViews(NULL, HINT_DELETE_SELECTION, &m_selection); // Doc 으로 부터 전체 데이터 출력 
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);

	// now remove the selection from the document
	POSITION pos = m_selection.GetHeadPosition(); // mselection의 첫번쨰 정보를 가져온다. 
	while (pos != NULL) // 위치가 0 아니먄 
	{
		CDrawObj* pObj = m_selection.GetNext(pos); // 옆으로 이동하면서 
		GetDocument()->Remove(pObj); // doc 의 해당 객체를 지워나간다. 
		pObj->Remove();
	}
	//Cleanup Tool members such as CPolyTool::m_pDrawObj, that should be NULL at this point.
	CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape); //finde tool 의 그리기 툴 멤버를 찾는다.
	if (pTool != NULL)
	{
		pTool->OnCancel(); //  대화 상자에서 취소 를 클릭할 때 발생 하는 이벤트의 역할
	}
	m_selection.RemoveAll(); // 이 목록에서 모든 요소를 제거합니다.
}

void CDrawView::OnUpdateAnySelect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnUpdateDrawPolygon(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(CDrawTool::c_drawShape == poly);
}

void CDrawView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy); // CWnd 의 크기가 변경된 후 호출됩니다.
	UpdateActiveItem();
}

void CDrawView::OnViewGrid()
{
	m_bGrid = !m_bGrid;
	Invalidate(FALSE); // 화면에 다시 그림을 그린다.  -> 깜빡임 현상 생김
	/*
	Invalidate(FALSE)  ->  WM_PAINT
	Invalidate(TRUE)   ->  WM_ERASEBKGND   -> WM_PAINT


	FALSE 인자는 백그라운드(현재 그려저 있는 거 포함)를 지우지 않고 그냥 그린다.
	TRUE는 백그라운드를 다 지우고 그린다.
	*/
}

void CDrawView::OnUpdateViewGrid(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_bGrid); //이 함수는 라디오 단추 또는 체크 박스를 설정하거나 다시 설정한다

}

BOOL CDrawView::OnEraseBkgnd(CDC*)
{
	return TRUE;
}

void CDrawView::OnObjectFillColor()
{
	CColorDialog dlg;
	if (dlg.DoModal() != IDOK)  //모달 다이얼로그의 경우는 다이얼로그가 생성되면 자신을 생성시킨 다이얼로그가 활성화되지 않습니다.

		return;

	COLORREF color = dlg.GetColor(); //현재 선택한 색을 반환합니다

	POSITION pos = m_selection.GetHeadPosition(); // 첫번쨰 위치 반환
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos);
		pObj->SetFillColor(color); // 해당 색 채우기 
	}
}

void CDrawView::OnObjectLineColor()
{
	CColorDialog dlg;
	if (dlg.DoModal() != IDOK)//모달 다이얼로그의 경우는 다이얼로그가 생성되면 자신을 생성시킨 다이얼로그가 활성화되지 않습니다.
		return;

	COLORREF color = dlg.GetColor();//현재 선택한 색을 반환합니다 

	POSITION pos = m_selection.GetHeadPosition();
	while (pos != NULL)
	{
		CDrawObj* pObj = m_selection.GetNext(pos);//반복할 다음 요소를 가져옵니다.
		pObj->SetLineColor(color);// 각 선 색을 칠한다. 
	}
}

void CDrawView::OnObjectMoveBack()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead();
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	if (pos != pObjects->GetHeadPosition())
	{
		POSITION posPrev = pos;
		pObjects->GetPrev(posPrev);//반복할 이전 요소를 가져옵니다.
		pObjects->RemoveAt(pos); //특정 인덱스의 요소를 제거합니다
		pObjects->InsertBefore(posPrev, pObj); //지정된 위치 앞에 새 요소를 삽입합니다.
		InvalObj(pObj);
	}
}

void CDrawView::OnObjectMoveForward()
{
	CDrawDoc* pDoc = GetDocument(); // 이 함수를 호출하여 보기의 문서에 대한 포인터를 가져옵니다.
	CDrawObj* pObj = m_selection.GetHead(); //목록의 맨처음 요소를 리턴함  
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	if (pos != pObjects->GetTailPosition())// 마지막요소의 위치를 반환 
	{
		POSITION posNext = pos;
		pObjects->GetNext(posNext); //rPosition으로 식별되는 목록 요소를 가져오고 목록에서 다음 항목의 POSITION 값으로 rPosition을 설정합니다.
		pObjects->RemoveAt(pos);
		pObjects->InsertAfter(posNext, pObj);  //지정된 위치 뒤에 새 요소를 삽입합니다.
		InvalObj(pObj);
	}
}

void CDrawView::OnObjectMoveToBack()
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead();
	CDrawObjList* pObjects = pDoc->GetObjects();
	POSITION pos = pObjects->Find(pObj);
	ASSERT(pos != NULL);
	pObjects->RemoveAt(pos);
	pObjects->AddHead(pObj); // 요소 앞 인덱스에 추가 
	InvalObj(pObj);
}

void CDrawView::OnObjectMoveToFront() // 그리거나 붙인 인스턴스를 맨 앞으로 
{
	CDrawDoc* pDoc = GetDocument();
	CDrawObj* pObj = m_selection.GetHead(); //이 목록의 헤드 요소를 나타내는 포인터를 가져옵니다.
	CDrawObjList* pObjects = pDoc->GetObjects(); // 
	POSITION pos = pObjects->Find(pObj); //목록을 순차적으로 검색하여 지정된 CObject 포인터와 일치하는 첫 번째 CObject 포인터를 찾습니다.
	ASSERT(pos != NULL);   
	pObjects->RemoveAt(pos); //이 목록에서 지정된 요소를 제거합니다.
	pObjects->AddTail(pObj); // 마지막 요소의 위치 (position값) 를 반환한가. 
	InvalObj(pObj);
}

void CDrawView::OnEditCopy() 
{
	ASSERT_VALID(this);//개체 내부 상태의 유효성에 대한 가정을 테스트
	ASSERT(m_cfDraw != NULL); // 해당 값이 조건에 맞는지 평가 

	// Create a shared file and associate a CArchive with it
	CSharedFile file;
	CArchive ar(&file, CArchive::store);

	// Serialize selected objects to the archive
	m_selection.Serialize(ar); // file save load 를 연결해서 하는것 : 직렬화  

	ar.Close(); // 버퍼에 남아 있는 모든 데이터를 플러시하고, 보관 파일을 닫고, 파일에서 보관의 연결을 끊습니다.

	COleDataSource* pDataSource = NULL;
	TRY
	{
		pDataSource = new COleDataSource;
		// put on local format instead of or in addation to
		pDataSource->CacheGlobalData(m_cfDraw, file.Detach());
		//CacheGlobalData : 데이터 전송 작업 중에 데이터가 제공되는 형식을 지정
		//CSharedFile :: Detach : 공유 메모리 파일을 닫고 해당 메모리 블록의 핸들을 반환합니다.
		// if only one item and it is a COleClientItem then also
		// paste in that format
		CDrawObj* pDrawObj = m_selection.GetHead(); // 목록의 첫번째 요소를 가져온다. 
		if (m_selection.GetCount() == 1 &&
			pDrawObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj))) //지정된 클래스에 대한 이 개체의 관계를 테스트
		{
			CDrawOleObj* pDrawOle = (CDrawOleObj*)pDrawObj;
			pDrawOle->m_pClientItem->GetClipboardData(pDataSource, FALSE); //CopyToClipboard 멤버 함수를 호출하여 COleDataSource클립보드에 배치되는 모든 데이터를 포함 하는 개체를 가져오려면 이 함수를 호출합니다.
		}

		pDataSource->SetClipboard();  
		//SetClipboard : COleDataSource, DelayRenderData 또는 DelayRenderFileData 함수 중 하나를 호출한 후 클립보드의 개체에 포함된 COleDataSource 데이터를 넣습니다.
		delete pDataSource; 
		
	}
	CATCH_ALL(e)
	{
		delete pDataSource;
		THROW_LAST(); //예외를 다음 외부 CATCH 블록으로 다시 throw합니다.
	}
	END_CATCH_ALL //CATCH_ALL 또는 AND_CATCH_ALL 블록의 끝을 표시합
}

void CDrawView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_selection.IsEmpty()); //  이 목록에 요소가 없는지 여부를 나타냅니다.


}

void CDrawView::OnEditCut()
{
	OnEditCopy();
	OnEditClear();
}

void CDrawView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnEditPaste()
{
	COleDataObject dataObject;
	dataObject.AttachClipboard(); //클립보드에 있는 데이터 개체를 연결합니다.

	// invalidate current selection since it will be deselected
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
	m_selection.RemoveAll();
	if (dataObject.IsDataAvailable(m_cfDraw)) //OLE 항목에서 데이터를 검색하는 데 특정 형식을 사용할 수 있는지 확인하려면 이 함수를 호출합니다.
	{
		PasteNative(dataObject);

		// now add all items in m_selection to document
		POSITION pos = m_selection.GetHeadPosition();
		while (pos != NULL)
			GetDocument()->Add(m_selection.GetNext(pos));
	}
	else
		PasteEmbedded(dataObject, GetInitialPosition().TopLeft() );

	GetDocument()->SetModifiedFlag();

	// invalidate new pasted stuff
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_SELECTION, &m_selection);
}

void CDrawView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	// determine if private or standard OLE formats are on the clipboard
	COleDataObject dataObject;
	BOOL bEnable = dataObject.AttachClipboard() &&
		(dataObject.IsDataAvailable(m_cfDraw) ||
		 COleClientItem::CanCreateFromData(&dataObject));
	//AttachClipboard : 현재 클립보드에 있는 데이터 개체를 개체에 연결하려면 이 함수를 COleDataObject 호출합니다.
	//IsDataAvailable : OLE 항목에서 데이터를 검색하는 데 특정 형식을 사용할 수 있는지 확인하려면 이 함수를 호출합니다.
	//CanCreateFromData : 컨테이너 애플리케이션이 지정된 COleDataObject 개체에서 포함된 개체를 만들 수 있는지 여부를 확인합니다.
	// enable command based on availability
	pCmdUI->Enable(bEnable);
}

void CDrawView::OnFilePrint()
{
	CScrollView::OnFilePrint();
	GetDocument()->ComputePageSize(); //doc 에 있는 page size 구하는 함수 
}

void CDrawView::OnViewShowObjects()
{
	CDrawOleObj::c_bShowItems = !CDrawOleObj::c_bShowItems;
	GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_OLE_ITEMS, NULL); //문서가 수정된 후 이 함수를 호출합니다.


}

void CDrawView::OnUpdateViewShowObjects(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(CDrawOleObj::c_bShowItems); //이 멤버 함수를 호출하여 이 명령에 대한 사용자 인터페이스 항목을 적절한 확인 상태로 설정합니다.


}

void CDrawView::OnEditProperties()
{
	if (m_selection.GetCount() == 1 && CDrawTool::c_drawShape == selection)
	{
		CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
		ASSERT(pTool != NULL);
		pTool->OnEditProperties(this);//사용자가 속성을 편집할 때 프레임워크에서 호출됩니다.
	}
}

void CDrawView::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_selection.GetCount() == 1 &&
				   CDrawTool::c_drawShape == selection);
//GetCount : 요소의 수를 가져옵니다. 

}

/////////////////////////////////////////////////////////////////////////////
// CDrawView diagnostics

#ifdef _DEBUG
void CDrawView::AssertValid() const
{
	CScrollView::AssertValid(); // 해당 객체의 무결성을 확인 한다.  
}

void CDrawView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc); 
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// new
// support for drag/drop
//OnCreate 함수는 WM_CREATE 메세지에 대한 핸들러 함수 
/*
윈도우 클래스가 윈도우로서 생성  -> WM_CREATE 메세지 발생 -> OnCreate 실행 되면서 윈도우 작업 수행  

*/
int CDrawView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1) 
		return -1; //	창 만들기의 일부로 호출됩니다.

	// register drop target
	if( m_dropTarget.Register( this ) ) //이 함수를 호출하여 OLE DLL에 창을 유효한 삭제 대상으로 등록합니다.
		return 0;
	else
		return -1;
}

BOOL CDrawView::GetObjectInfo(COleDataObject* pDataObject,
	CSize* pSize, CSize* pOffset)
{
	ENSURE(pSize != NULL); //데이터의 정확성 확인  

	// get object descriptor data
	HGLOBAL hObjDesc = pDataObject->GetGlobalData(m_cfObjectDescriptor); //전역 메모리 블록을 할당하고 지정된 형식의 데이터를 HGLOBAL로 검색하려면 이 함수를 호출
	if (hObjDesc == NULL)
	{
		if (pOffset != NULL)
			*pOffset = CSize(0, 0); // fill in defaults instead
		*pSize = CSize(0, 0);
		return FALSE;
	}
	ASSERT(hObjDesc != NULL);

	// otherwise, got CF_OBJECTDESCRIPTOR ok.  Lock it down and extract size.
	LPOBJECTDESCRIPTOR pObjDesc = (LPOBJECTDESCRIPTOR)GlobalLock(hObjDesc);
	ENSURE(pObjDesc != NULL);
	pSize->cx = (int)pObjDesc->sizel.cx;
	pSize->cy = (int)pObjDesc->sizel.cy;
	if (pOffset != NULL)
	{
		pOffset->cx = (int)pObjDesc->pointl.x;
		pOffset->cy = (int)pObjDesc->pointl.y;
	}
	GlobalUnlock(hObjDesc); //전역 메모리 개체를 잠금해제,  개체 메모리 블록의 첫 번째 바이트에 대한 포인터를 반환합니다
	GlobalFree(hObjDesc); //지정된 전역 메모리 개체를 해제하고 해당 핸들을 무효화합니다.

	// successfully retrieved pSize & pOffset info
	return TRUE;
}

DROPEFFECT CDrawView::OnDragEnter(COleDataObject* pDataObject,
	DWORD grfKeyState, CPoint point)
{
	ASSERT(m_prevDropEffect == DROPEFFECT_NONE);
	m_bDragDataAcceptable = FALSE;
	if (!COleClientItem::CanCreateFromData(pDataObject)) //컨테이너 애플리케이션이 지정된 COleDataObject 개체에서 포함된 개체를 만들 수 있는지 여부를 확인합니다.
		return DROPEFFECT_NONE; //놓기 대상은 데이터를 수락할 수 없습니다.

	m_bDragDataAcceptable = TRUE;
	GetObjectInfo(pDataObject, &m_dragSize, &m_dragOffset);
	CClientDC dc(NULL);
	dc.HIMETRICtoDP(&m_dragSize); //크기를 OLE에서 픽셀로 변환 HIMETRIC 할 때 이 함수를 사용합니다.


	dc.HIMETRICtoDP(&m_dragOffset);

	return OnDragOver(pDataObject, grfKeyState, point); //커서를 창 위로 끌 때 프레임워크에서 호출됩니다.


}

DROPEFFECT CDrawView::OnDragOver(COleDataObject*,
	DWORD grfKeyState, CPoint point) 
{
	if(m_bDragDataAcceptable == FALSE)
		return DROPEFFECT_NONE; //놓기 대상은 데이터를 수락할 수 없습니다.

	point -= m_dragOffset;  // adjust target rect by original cursor offset

	// check for point outside logical area -- i.e. in hatched region
	// GetTotalSize() returns the size passed to SetScrollSizes
	CRect rectScroll(CPoint(0, 0), GetTotalSize());
	//스크롤 보기의 현재 가로 및 세로 크기를 검색하려면 호출 GetTotalSize 합니다.


	CRect rectItem(point,m_dragSize);
	rectItem.OffsetRect(GetDeviceScrollPosition());
	//OffsetRect: CRect지정 된 오프셋으로 이동 합니다.


	DROPEFFECT de = DROPEFFECT_NONE;
	CRect rectTemp;
	if (rectTemp.IntersectRect(rectScroll, rectItem)) //두 기존 사각형의 교집합과 동일하게 만듭니다
	{
		// check for force link
		if ((grfKeyState & (MK_CONTROL|MK_SHIFT)) == (MK_CONTROL|MK_SHIFT)) //놓기 대상은 데이터를 수락할 수 없습니다.
			de = DROPEFFECT_NONE; // DRAWCLI isn't a linking container
		// check for force copy
		else if ((grfKeyState & MK_CONTROL) == MK_CONTROL)
			de = DROPEFFECT_COPY; // 결과를 복사본에 놓습니다. 원래 데이터는 끌기 원본에 의해 그대로 유지됩니다.
		// check for force move
		else if ((grfKeyState & MK_ALT) == MK_ALT)
			de = DROPEFFECT_MOVE; // 끌어서 원본은 데이터를 제거해야 합니다.
		// default -- recommended action is move
		else
			de = DROPEFFECT_MOVE;
	}

	if (point == m_dragPoint)
		return de;

	// otherwise, cursor has moved -- need to update the drag feedback
	CClientDC dc(this);
	if (m_prevDropEffect != DROPEFFECT_NONE)
	{
		// erase previous focus rect
		dc.DrawFocusRect(CRect(m_dragPoint, m_dragSize)); //사각형에 포커스가 있음을 나타내는 데 사용되는 스타일로 사각형을 그립니다.
	}
	m_prevDropEffect = de;
	if (m_prevDropEffect != DROPEFFECT_NONE)
	{
		m_dragPoint = point;
		dc.DrawFocusRect(CRect(point, m_dragSize));
	}
	return de;
}

BOOL CDrawView::OnDrop(COleDataObject* pDataObject,
	DROPEFFECT /*dropEffect*/, CPoint point)
{
	ASSERT_VALID(this); //개체 내부 상태의 유효성에 대한 가정을 테스트

	// clean up focus rect
	OnDragLeave(); //끌어오기 작업이 적용되는 동안 커서가 창을 떠날 때 프레임워크에서 호출됩니다.



	// offset point as appropriate for dragging
	GetObjectInfo(pDataObject, &m_dragSize, &m_dragOffset);
	CClientDC dc(NULL);
	dc.HIMETRICtoDP(&m_dragSize); //크기를 OLE에서 픽셀로 변환 HIMETRIC 할 때 이 함수를 사용합니다.


	dc.HIMETRICtoDP(&m_dragOffset);
	point -= m_dragOffset;

	// invalidate current selection since it will be deselected
	OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
	m_selection.RemoveAll();
	if (m_bDragDataAcceptable)
		PasteEmbedded(*pDataObject, point);

	// update the document and views
	GetDocument()->SetModifiedFlag(); //문서를 수정한 후 이 함수를 호출합니다.
	GetDocument()->UpdateAllViews(NULL, 0, NULL);      // including this view

	return TRUE;
}

void CDrawView::OnDragLeave()
{
	CClientDC dc(this); // CClientDC의 객체 생성 
	if (m_prevDropEffect != DROPEFFECT_NONE) // DROPEFFECT 상수 : 끌어서 놓기 작업을 수행하는 함수  DROPEFFECT_NONE : 놓기 대상은 데이터를 수락할 수 없다. 
	{
		dc.DrawFocusRect(CRect(m_dragPoint,m_dragSize)); // erase previous focus rect
		m_prevDropEffect = DROPEFFECT_NONE;
	}
}


void CDrawView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// make sure window is active
	GetParentFrame()->ActivateFrame(); //사용자가 볼 수 있고 사용할 수 있도록 프레임 창을 활성화하고 복원

	CPoint local = point;
	ScreenToClient(&local); // 화면에서 지정된 지점의 화면 좌표를 클라이언트 영역 좌표로 변환
	ClientToDoc(local);  

	CDrawObj* pObj;
	pObj = GetDocument()->ObjectAt(local); //지정된 인덱스에 있는 개체를 반환합니다
	if(pObj != NULL)
	{
		if(!IsSelected(pObj))
			Select( pObj, FALSE );          // reselect item if appropriate
		UpdateWindow(); //컨트롤을  위해  창을 업데이트 시킨다. 
		 
		CMenu menu;
		if (menu.LoadMenu(ID_POPUP_MENU)) //애플리케이션의 실행 파일에서 메뉴 리소스를 로드하고 개체에 CMenu 연결합니다.
		{
			CMenu* pPopup = menu.GetSubMenu(0); //지정된 메뉴 항목에 의해 활성화된 드롭다운 메뉴 또는 하위 메뉴로 핸들을 검색
			ENSURE(pPopup != NULL); //데이터 정확성을 검증하는 데 사용합니다.

			pPopup->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN,
								   point.x, point.y,
								   AfxGetMainWnd()); // route commands through main window
		//TrackPopupMenu : 지정된 위치에 부동 팝업 메뉴를 표시하고 팝업 메뉴에서 항목 선택을 추적
		}
	}
}

void CDrawView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	if (pInfo->m_bPreview == FALSE)
		((CDrawDoc*)GetDocument())->m_pSummInfo->RecordPrintDate();//summinfo 
	OnDraw(pDC);
}
