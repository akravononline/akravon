#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonWindowManager.h"

#ifdef ENABLE_FPS
#include "../GameLib/GameType.h"
#endif

#ifdef ENABLE_RENDER_TARGET
#include "../EterLib/CRenderTargetManager.h"
#endif

#ifdef ENABLE_GIF_SUPPORT
#include <gif_lib.h>
#include <wingdi.h>
#include <array>
#endif
#include <fmt/format.h>
#ifdef CYNEX_MOBILE
BOOL g_bOutlineBoxEnable = FALSE;	// Cynex: pencere hata ayiklama cercevesi mobilde kapali
#else
BOOL g_bOutlineBoxEnable = TRUE;
#endif

namespace UI
{
	CWindow::CWindow(PyObject * ppyObject) :
		m_x(0),
		m_y(0),
		m_lWidth(0),
		m_lHeight(0),
		m_poHandler(ppyObject),
		m_bShow(false),
		m_pParent(NULL),
		m_dwFlag(0),
		m_isUpdatingChildren(FALSE)
		#ifdef ENABLE_MOUSEWHEEL_EVENT
		, m_bIsScrollable(false)
		#endif
	{
		#ifdef _DEBUG
		static DWORD DEBUG_dwGlobalCounter=0;
		DEBUG_dwCounter=DEBUG_dwGlobalCounter++;

		m_strName = "!!debug";
		#endif

		//assert(m_poHandler != NULL);
		m_HorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_VerticalAlign = VERTICAL_ALIGN_TOP;
		m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
#if defined(__BL_CLIP_MASK__)
		m_bEnableMask = false;
		m_pMaskWindow = NULL;
		memset(&m_rMaskRect, 0, sizeof(m_rMaskRect));
#endif
		m_limitBiasRect.bottom = m_limitBiasRect.left = m_limitBiasRect.right = m_limitBiasRect.top = 0;
	}

	CWindow::~CWindow()
	{
	}

	DWORD CWindow::Type()
	{
		static DWORD s_dwType = GetCRC32("CWindow", strlen("CWindow"));
		return (s_dwType);
	}

	BOOL CWindow::IsType(DWORD dwType)
	{
		return OnIsType(dwType);
	}

	BOOL CWindow::OnIsType(DWORD dwType)
	{
		if (CWindow::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	struct FClear
	{
		void operator () (CWindow * pWin)
		{
			pWin->Clear();
		}
	};

	void CWindow::Clear()
	{
#if defined(__BL_CLIP_MASK__)
		m_pMaskWindow = NULL;
#endif
		std::for_each(m_pChildList.begin(), m_pChildList.end(), FClear());
		m_pChildList.clear();

		m_pParent = NULL;
		DestroyHandle();
		Hide();
	}

	void CWindow::DestroyHandle()
	{
		m_poHandler = NULL;
	}

	void CWindow::Show()
	{
		m_bShow = true;
	}

	void CWindow::Hide()
	{
		m_bShow = false;
	}

	bool CWindow::IsRendering()
	{
		if (!IsShow())
			return false;

		if (!m_pParent)
			return true;

		return m_pParent->IsRendering();
	}

	void CWindow::__RemoveReserveChildren()
	{
		if (m_pReserveChildList.empty())
			return;

		TWindowContainer::iterator it;
		for(it = m_pReserveChildList.begin(); it != m_pReserveChildList.end(); ++it)
		{
			m_pChildList.remove(*it);
		}
		m_pReserveChildList.clear();
	}

	// Cynex: telefonda mantiksal ekran kucuk (~600 yuksek); 900'e gore tasarlanmis buyuk pencereler alta/saga tasmasin
	void CWindow::CynexClampChildren(LONG sw, LONG sh)
	{
		for (auto& c : m_pChildList)
		{
			if (!c->IsShow() || c->IsFlag(FLAG_NOT_PICK))
				continue;
			const LONG w = c->GetWidth(), h = c->GetHeight();
			if (w < 160 || h < 160)
				continue;
			if (w >= sw && h >= sh)
				continue;	// tam ekran (perde, arka plan)
			const LONG x = c->GetPositionX(), y = c->GetPositionY();
			if (y < 0 || y >= sh - 40 || x <= -w + 40 || x >= sw - 40)
				continue;	// park edilmis / ekran disi
			LONG nx = x, ny = y;
			if (y + h > sh)
				ny = h <= sh ? sh - h : 0;
			// yatay: pencereler kenardan kayarak girebilir (envanter animasyonu) - yalniz dikeyde ekran icinde tut
			if (nx != x || ny != y)
				c->SetPosition(nx, ny);
		}
	}

	void CWindow::Update()
	{
		if (!IsShow())
			return;

		__RemoveReserveChildren();

		OnUpdate();

		m_isUpdatingChildren = TRUE;

		for (auto& Child : m_pChildList)
			Child->Update();

		m_isUpdatingChildren = FALSE;
	}

	void CWindow::Render()
	{
		if (!IsShow())
			return;

		OnRender();

        if (g_bOutlineBoxEnable)
        {
            if (CWindowManager::instance().GetPointWindow() == this)
            {
                // Render parent rect with blue outline
                const auto parentRect = m_pParent ? m_pParent->m_rect : RECT{ 0, 0, 0, 0 };
                CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 1.0f);
                CPythonGraphic::Instance().RenderBox2d(parentRect.left, parentRect.top, parentRect.right, parentRect.bottom);

                // Render this rect with green outline
                CPythonGraphic::Instance().SetDiffuseColor(0.0f, 1.0f, 0.0f);
                CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

                // Render info text with window name and flags
                const auto buffer = fmt::format("{} ({})", m_strName, Type());
                CPythonGraphic::Instance().RenderTextLine(m_rect.left + 5, m_rect.top - 30, buffer.c_str(), 0xFFFFFF00);

                const auto flags = m_dwFlag;
                std::string flagInfo;
                if (flags & FLAG_MOVABLE)
                    flagInfo += "FLAG_MOVABLE ";
                if (flags & FLAG_SNAP)
                    flagInfo += "FLAG_SNAP ";
                if (flags & FLAG_DRAGABLE)
                    flagInfo += "FLAG_DRAGABLE ";
                if (flags & FLAG_ATTACH)
                    flagInfo += "FLAG_ATTACH ";
                if (flags & FLAG_RESTRICT_X)
                    flagInfo += "FLAG_RESTRICT_X ";
                if (flags & FLAG_RESTRICT_Y)
                    flagInfo += "FLAG_RESTRICT_Y ";
                if (flags & FLAG_FLOAT)
                    flagInfo += "FLAG_FLOAT ";
                if (flags & FLAG_NOT_PICK)
                    flagInfo += "FLAG_NOT_PICK ";
                if (flags & FLAG_IGNORE_SIZE)
                    flagInfo += "FLAG_IGNORE_SIZE ";
                if (flags & FLAG_RTL)
                    flagInfo += "FLAG_RTL ";

                const auto parentName = m_pParent ? m_pParent->GetName() : "None";

                const auto stRectInfo = fmt::format("Pos: ({0}, {1}), Size: {2}x{3}, Flags: {4}, Parent: {5}",
                    m_rect.left, m_rect.top, m_rect.right - m_rect.left, m_rect.bottom - m_rect.top,
                    flagInfo, parentName
                );
                CPythonGraphic::Instance().RenderTextLine(m_rect.left + 5, m_rect.top - 15, stRectInfo.c_str(), 0xFFFFFFFF);
            }
        }

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::void_mem_fun(&CWindow::Render));
	}

	void CWindow::OnUpdate()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		static PyObject* poFuncName_OnUpdate = PyString_InternFromString("OnUpdate");

		//PyCallClassMemberFunc(m_poHandler, "OnUpdate", BuildEmptyTuple());
		PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());

	}

	void CWindow::OnRender()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		//PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}

	void CWindow::SetName(const char * c_szName)
	{
		m_strName = c_szName;
	}

	void CWindow::SetSize(LONG width, LONG height)
	{
		m_lWidth = width;
		m_lHeight = height;

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;
	}

	void CWindow::SetHorizontalAlign(DWORD dwAlign)
	{
		m_HorizontalAlign = (EHorizontalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetVerticalAlign(DWORD dwAlign)
	{
		m_VerticalAlign = (EVerticalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetPosition(LONG x, LONG y)
	{
		m_x = x;
		m_y = y;

		UpdateRect();
	}

	void CWindow::GetPosition(LONG * plx, LONG * ply)
	{
		*plx = m_x;
		*ply = m_y;
	}

	LONG CWindow::UpdateRect()
	{
		m_rect.top		= m_y;
		if (m_pParent)
		{
			switch (m_VerticalAlign)
			{
				case VERTICAL_ALIGN_BOTTOM:
					m_rect.top = m_pParent->GetHeight() - m_rect.top;
					break;
				case VERTICAL_ALIGN_CENTER:
					m_rect.top = (m_pParent->GetHeight() - GetHeight()) / 2 + m_rect.top;
					break;
			}
			m_rect.top += m_pParent->m_rect.top;
		}
		m_rect.bottom	= m_rect.top + m_lHeight;

#if defined( _USE_CPP_RTL_FLIP )
		if( m_pParent == NULL ) {
			m_rect.left		= m_x;
			m_rect.right	= m_rect.left + m_lWidth;
		} else {
			if( m_pParent->IsFlag(UI::CWindow::FLAG_RTL) == true ) {
				m_rect.left = m_pParent->GetWidth() - m_lWidth - m_x;
				switch (m_HorizontalAlign)
				{
					case HORIZONTAL_ALIGN_RIGHT:
						m_rect.left = - m_x;
						break;
					case HORIZONTAL_ALIGN_CENTER:
						m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() - m_x;
						break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			} else {
				m_rect.left		= m_x;
				switch (m_HorizontalAlign)
				{
					case HORIZONTAL_ALIGN_RIGHT:
						m_rect.left = m_pParent->GetWidth() - m_rect.left;
						break;
					case HORIZONTAL_ALIGN_CENTER:
						m_rect.left = (m_pParent->GetWidth() - GetWidth()) / 2 + m_rect.left;
						break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			}
		}
#else
		m_rect.left		= m_x;
		if (m_pParent)
		{
			switch (m_HorizontalAlign)
			{
				case HORIZONTAL_ALIGN_RIGHT:
					m_rect.left = ::abs(m_pParent->GetWidth()) - m_rect.left;
					break;
				case HORIZONTAL_ALIGN_CENTER:
					m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() / 2 + m_rect.left;
					break;
			}
			m_rect.left += 0L < m_pParent->GetWidth() ? m_pParent->m_rect.left : m_pParent->m_rect.right + ::abs(m_pParent->GetWidth());
		}
		m_rect.right = m_rect.left + m_lWidth;
#endif
		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

		OnChangePosition();

		return 1;
	}

	void CWindow::GetLocalPosition(LONG & rlx, LONG & rly)
	{
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::GetMouseLocalPosition(LONG & rlx, LONG & rly)
	{
		CWindowManager::Instance().GetMousePosition(rlx, rly);
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::AddChild(CWindow * pWin)
	{
		m_pChildList.push_back(pWin);
		pWin->m_pParent = this;
	}

	CWindow * CWindow::GetRoot()
	{
		if (m_pParent)
			if (m_pParent->IsWindow())
				return m_pParent->GetRoot();

		return this;
	}

	CWindow * CWindow::GetParent()
	{
		return m_pParent;
	}

	bool CWindow::IsChild(CWindow * pWin)
	{
		auto itor = m_pChildList.begin();

		while (itor != m_pChildList.end())
		{
			if (*itor == pWin)
				return true;

			++itor;
		}

		return false;
	}

	void CWindow::DeleteChild(CWindow * pWin)
	{
		if (m_isUpdatingChildren)
		{
			m_pReserveChildList.push_back(pWin);
		}
		else
		{
			m_pChildList.remove(pWin);
		}
	}

	void CWindow::SetTop(CWindow * pWin)
	{
		if (!pWin->IsFlag(CWindow::FLAG_FLOAT))
			return;

		TWindowContainer::iterator itor = std::find(m_pChildList.begin(), m_pChildList.end(), pWin);
		if (m_pChildList.end() != itor)
		{
			m_pChildList.push_back(*itor);
			m_pChildList.erase(itor);

			pWin->OnTop();
		}
		else
		{
			TraceError(" CWindow::SetTop - Failed to find child window\n");
		}
	}

	void CWindow::OnMouseDrag(LONG lx, LONG ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseDrag", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnMoveWindow(LONG lx, LONG ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMoveWindow", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnSetFocus()
	{
		//PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
		PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
	}

	void CWindow::OnKillFocus()
	{
		PyCallClassMemberFunc(m_poHandler, "OnKillFocus", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverIn()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverOut()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}

	void CWindow::OnMouseOver()
	{
	}

	void CWindow::OnDrop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnDrop", BuildEmptyTuple());
	}

	void CWindow::OnTop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnTop", BuildEmptyTuple());
	}

	void CWindow::OnIMEUpdate()
	{
		PyCallClassMemberFunc(m_poHandler, "OnIMEUpdate", BuildEmptyTuple());
	}

#if defined(__BL_CLIP_MASK__)
	void CWindow::SetClippingMaskRect(const RECT& rMask)
	{
		m_bEnableMask = true;
		m_rMaskRect = rMask;

		TWindowContainer::const_reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow)
				pWindow->SetClippingMaskRect(rMask);
		}
	}

	void CWindow::SetClippingMaskWindow(CWindow* pMaskWindow)
	{
		if (!pMaskWindow)
			return;

		m_bEnableMask = true;
		m_pMaskWindow = pMaskWindow;

		TWindowContainer::const_reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow)
				pWindow->SetClippingMaskWindow(pMaskWindow);
		}
	}
#endif
	BOOL CWindow::RunIMETabEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMETabEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMETabEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEReturnEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEReturnEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMEReturnEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEKeyDownEvent(int ikey)
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEKeyDownEvent(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMEKeyDownEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	CWindow * CWindow::RunKeyDownEvent(int ikey)
	{
		if (OnKeyDown(ikey))
			return this;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			{
				CWindow * pProcessedWindow = pWindow->RunKeyDownEvent(ikey);
				if (NULL != pProcessedWindow)
				{
					return pProcessedWindow;
				}
			}
		}

		return NULL;
	}

	BOOL CWindow::RunKeyUpEvent(int ikey)
	{
		if (OnKeyUp(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			if (pWindow->RunKeyUpEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunPressEscapeKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			if (pWindow->RunPressEscapeKeyEvent())
				return TRUE;
		}

		if (OnPressEscapeKey())
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::RunPressExitKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunPressExitKeyEvent())
				return TRUE;

			if (pWindow->IsShow())
			if (pWindow->OnPressExitKey())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonDown()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonUp()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonUp", BuildEmptyTuple());
		return TRUE;
	}

	BOOL CWindow::OnMouseLeftButtonDoubleClick()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDown()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonUp()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDoubleClick()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonDown()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonUp()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	#ifdef ENABLE_MOUSEWHEEL_EVENT
	bool CWindow::OnMouseWheelScroll(int16_t wDelta)
	{
		#ifdef _DEBUG
		Tracenf("Mouse Wheel Scroll : wDelta %d ",wDelta);
		#endif

		PyCallClassMemberFunc(m_poHandler , "OnMouseWheelScroll", Py_BuildValue("(s)" , wDelta > 0 ? "UP":"DOWN") );
		return m_bIsScrollable;
	}

	void CWindow::SetScrollable()
	{
		m_bIsScrollable = true;
	}
	#endif

	#ifdef ENABLE_MOUSEWHEEL_EVENT_OLD
	BOOL CWindow::RunMouseWheelEvent(LONG nLen)
	{
		bool bValue = false;
		if (PyCallClassMemberFunc(m_poHandler, "OnRunMouseWheel", Py_BuildValue("(l)", nLen), &bValue))
			return bValue;
		return bValue;
	}
	#endif

	BOOL CWindow::OnIMETabEvent()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMETab", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEReturnEvent()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEReturn", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEKeyDownEvent(int ikey)
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEChangeCodePage()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEChangeCodePage", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenCandidateListEvent()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseCandidateListEvent()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenReadingWndEvent()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseReadingWndEvent()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyDown(int ikey)
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyUp(int ikey)
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyUp", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressEscapeKey()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressEscapeKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressExitKey()
	{
		LONG lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressExitKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	/////

	bool CWindow::IsIn(LONG x, LONG y)
	{
		if (x >= m_rect.left && x <= m_rect.right)
			if (y >= m_rect.top && y <= m_rect.bottom)
				return true;

		return false;
	}

	bool CWindow::IsIn()
	{
		LONG lx, ly;
		UI::CWindowManager::Instance().GetMousePosition(lx, ly);

		return IsIn(lx, ly);
	}

	CWindow * CWindow::PickWindow(LONG x, LONG y)
	{
		auto ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow * pWin = *ritor;
			if (pWin->IsShow())
			{
				if (!pWin->IsFlag(CWindow::FLAG_IGNORE_SIZE))
				{
					if (!pWin->IsIn(x, y)) {
						if (0L <= pWin->GetWidth()) {
							continue;
						}
					}
#ifdef CYNEX_MOBILE
					// Cynex: boyutsuz pencere (dugme yazisi) tam ortasina dokununca dugmeyi yutmasin
					else if (0L == pWin->GetWidth() || 0L == pWin->GetHeight())
						continue;
#endif
				}

				CWindow * pResult = pWin->PickWindow(x, y);
				if (pResult)
					return pResult;
			}
		}

		if (IsFlag(CWindow::FLAG_NOT_PICK))
			return NULL;

		return (this);
	}

	CWindow * CWindow::PickTopWindow(LONG x, LONG y)
	{
		auto ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow * pWin = *ritor;
			if (pWin->IsShow())
				if (pWin->IsIn(x, y))
					if (!pWin->IsFlag(CWindow::FLAG_NOT_PICK))
						return pWin;
		}

		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CBox::CBox(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBox::~CBox()
	{
	}

	void CBox::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBox::OnRender()
	{
		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef ENABLE_RENDER_TARGET
	CUiRenderTarget::CUiRenderTarget(PyObject* ppyObject) : CWindow(ppyObject)
	{
		m_dwIndex = -1;
		#ifdef ENABLE_WIKI
		memset(&rect_ex, 0, sizeof(rect_ex));
		#endif
	}

	DWORD CUiRenderTarget::Type()
	{
		static DWORD s_dwType = GetCRC32("CUiRenderTarget", strlen("CUiRenderTarget"));
		return (s_dwType);
	}
	BOOL CUiRenderTarget::OnIsType(DWORD dwType)
	{
		if (CUiRenderTarget::Type() == dwType)
			return TRUE;
		return FALSE;
	}

	CUiRenderTarget::~CUiRenderTarget()
	{
		if (m_dwIndex != 1)
		{
			//TraceError("removing index %d", m_dwIndex);
			std::shared_ptr<CRenderTarget> t = CRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
			if (t)
			{
				t->ResetModel();
			}
		}
	};

	bool CUiRenderTarget::SetRenderTarget(int index)
	{
		if (!CRenderTargetManager::Instance().GetRenderTarget(index))
		{
			if (!CRenderTargetManager::Instance().CreateRenderTarget(index, GetWidth(), GetHeight()))
			{
				TraceError("CRenderTargetManager could not create the texture. w %d h %d", GetWidth(), GetHeight());
				return false;
			}
		}
		m_dwIndex = index;

		UpdateRect();
		return true;
	}

	#ifdef ENABLE_WIKI
	void CUiRenderTarget::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		memset(&rect_ex, 0, sizeof(rect_ex));
		float fWidth = float(GetWidth());
		float fHeight = float(GetHeight());

		rect_ex.left = fWidth * fLeft;
		rect_ex.top = fHeight * fTop;
		rect_ex.right = fWidth * fRight;
		rect_ex.bottom = fHeight * fBottom;
	}
	#endif

	void CUiRenderTarget::OnRender()
	{
		auto target = CRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
		if (!target)
		{
			TraceError("SetRenderingRect -> target empty!");
			return;
		}

		#ifdef ENABLE_WIKI
		RECT rect_real;
		rect_real.left = m_rect.left - rect_ex.left;
		rect_real.top = m_rect.top - rect_ex.top;
		rect_real.right = m_rect.right + rect_ex.right;
		rect_real.bottom = m_rect.bottom + rect_ex.bottom;
		target->SetRenderingRect(&rect_real);
		#else
		target->SetRenderingRect(&m_rect);
		#endif
		target->RenderTexture();
	}
	#endif

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CBar::CBar(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBar::~CBar()
	{
	}

	void CBar::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBar::OnRender()
	{
		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CLine::CLine(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CLine::~CLine()
	{
	}

	void CLine::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CLine::OnRender()
	{
		CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();
		rkpyGraphic.SetDiffuseColor(m_dwColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CBar3D::Type()
	{
		static DWORD s_dwType = GetCRC32("CBar3D", strlen("CBar3D"));
		return (s_dwType);
	}

	CBar3D::CBar3D(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_dwLeftColor = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
		m_dwRightColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
		m_dwCenterColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	}
	CBar3D::~CBar3D()
	{
	}

	void CBar3D::SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter)
	{
		m_dwLeftColor = dwLeft;
		m_dwRightColor = dwRight;
		m_dwCenterColor = dwCenter;
	}

	void CBar3D::OnRender()
	{
		CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();

		rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
		rkpyGraphic.RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.top);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwRightColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom);
		rkpyGraphic.RenderLine2d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CTextLine::CTextLine(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_TextInstance.SetColor(0.78f, 0.78f, 0.78f);
		m_TextInstance.SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		m_TextInstance.SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_TOP);
	}
	CTextLine::~CTextLine()
	{
		m_TextInstance.Destroy();
	}

	void CTextLine::SetMax(int iMax)
	{
		m_TextInstance.SetMax(iMax);
	}
	void CTextLine::SetHorizontalAlign(int iType)
	{
		m_TextInstance.SetHorizonalAlign(iType);
	}
	void CTextLine::SetVerticalAlign(int iType)
	{
		m_TextInstance.SetVerticalAlign(iType);
	}
	void CTextLine::SetSecret(BOOL bFlag)
	{
		m_TextInstance.SetSecret(bFlag ? true : false);
	}
	void CTextLine::SetOutline(BOOL bFlag)
	{
		m_TextInstance.SetOutline(bFlag ? true : false);
	}
	void CTextLine::SetFeather(BOOL bFlag)
	{
		m_TextInstance.SetFeather(bFlag ? true : false);
	}
	void CTextLine::SetMultiLine(BOOL bFlag)
	{
		m_TextInstance.SetMultiLine(bFlag ? true : false);
	}
	void CTextLine::SetFontName(const char * c_szFontName)
	{
		std::string stFontName = c_szFontName;
		stFontName += ".fnt";

		CResourceManager& rkResMgr=CResourceManager::Instance();
		CResource* pkRes = rkResMgr.GetTypeResourcePointer(stFontName.c_str());
		CGraphicText* pkResFont=static_cast<CGraphicText*>(pkRes);
		m_TextInstance.SetTextPointer(pkResFont);
	}
	void CTextLine::SetFontColor(DWORD dwColor)
	{
		m_TextInstance.SetColor(dwColor);
	}
	void CTextLine::SetLimitWidth(float fWidth)
	{
		m_TextInstance.SetLimitWidth(fWidth);
	}
	void CTextLine::SetText(const char * c_szText)
	{
		OnSetText(c_szText);
	}
	void CTextLine::GetTextSize(int* pnWidth, int* pnHeight)
	{
		m_TextInstance.GetTextSize(pnWidth, pnHeight);
	}
	const char * CTextLine::GetText()
	{
		return m_TextInstance.GetValueStringReference().c_str();
	}
	void CTextLine::ShowCursor()
	{
		m_TextInstance.ShowCursor();
	}
	void CTextLine::HideCursor()
	{
		m_TextInstance.HideCursor();
	}
	int CTextLine::GetCursorPosition()
	{
		LONG lx, ly;
		CWindow::GetMouseLocalPosition(lx, ly);
		return m_TextInstance.PixelPositionToCharacterPosition(lx);
	}

	void CTextLine::OnSetText(const char * c_szText)
	{
		m_TextInstance.SetValue(c_szText);
		m_TextInstance.Update();
	}

#if defined(__BL_CLIP_MASK__)
	void CTextLine::OnUpdate()
	{
		if (!IsShow())
			return;

		if (m_bEnableMask && m_pMaskWindow)
			m_rMaskRect = m_pMaskWindow->GetRect();

		m_TextInstance.Update();
	}
	void CTextLine::OnRender()
	{
		if (IsShow())
			m_TextInstance.Render(m_bEnableMask ? &m_rMaskRect : NULL);
	}
#else
	void CTextLine::OnUpdate()
	{
		if (IsShow())
			m_TextInstance.Update();
	}
	void CTextLine::OnRender()
	{
		if (IsShow())
			m_TextInstance.Render();
	}
#endif

	void CTextLine::OnChangePosition()
	{
		// FOR_ARABIC_ALIGN
		//if (m_TextInstance.GetHorizontalAlign() == CGraphicTextInstance::HORIZONTAL_ALIGN_ARABIC)
		if( GetDefaultCodePage() == CP_ARABIC )
		{
			m_TextInstance.SetPosition(m_rect.right, m_rect.top);
		}
		else
		{
			m_TextInstance.SetPosition(m_rect.left, m_rect.top);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CNumberLine::CNumberLine(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;
	}
	CNumberLine::CNumberLine(CWindow * pParent) : CWindow(NULL)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;

		m_pParent = pParent;
	}
	CNumberLine::~CNumberLine()
	{
		ClearNumber();
	}

	void CNumberLine::SetPath(const char * c_szPath)
	{
		m_strPath = c_szPath;
	}
	void CNumberLine::SetHorizontalAlign(int iType)
	{
		m_iHorizontalAlign = iType;
	}
	void CNumberLine::SetNumber(const char * c_szNumber)
	{
		if (0 == m_strNumber.compare(c_szNumber))
			return;

		ClearNumber();

		m_strNumber = c_szNumber;

		for (DWORD i = 0; i < m_strNumber.size(); ++i)
		{
			char cChar = m_strNumber[i];
			std::string strImageFileName;

			if (':' == cChar)
			{
				strImageFileName = m_strPath + "colon.sub";
			}
			else if ('?' == cChar)
			{
				strImageFileName = m_strPath + "questionmark.sub";
			}
			else if ('/' == cChar)
			{
				strImageFileName = m_strPath + "slash.sub";
			}
			else if ('%' == cChar)
			{
				strImageFileName = m_strPath + "percent.sub";
			}
			else if ('+' == cChar)
			{
				strImageFileName = m_strPath + "plus.sub";
			}
			else if ('m' == cChar)
			{
				strImageFileName = m_strPath + "m.sub";
			}
			else if ('g' == cChar)
			{
				strImageFileName = m_strPath + "g.sub";
			}
			else if ('p' == cChar)
			{
				strImageFileName = m_strPath + "p.sub";
			}
			else if (cChar >= '0' && cChar <= '9')
			{
				strImageFileName = m_strPath;
				strImageFileName += cChar;
				strImageFileName += ".sub";
			}
			else
				continue;

			if (!CResourceManager::Instance().IsFileExist(strImageFileName.c_str()))
				continue;

			CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(strImageFileName.c_str());

			CGraphicImageInstance * pInstance = CGraphicImageInstance::New();
			pInstance->SetImagePointer(pImage);
			m_ImageInstanceVector.push_back(pInstance);

			m_dwWidthSummary += pInstance->GetWidth();
		}
	}

	void CNumberLine::ClearNumber()
	{
        for (auto ins : m_ImageInstanceVector)
        {
            CGraphicImageInstance::Delete(ins);
        } /* Clear number memory leak fix */

		m_ImageInstanceVector.clear();
		m_dwWidthSummary = 0;
		m_strNumber = "";
	}

	void CNumberLine::OnRender()
	{
		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->Render();
		}
	}

	void CNumberLine::OnChangePosition()
	{
		int ix = m_x;
		int iy = m_y;

		if (m_pParent)
		{
			ix = m_rect.left;
			iy = m_rect.top;
		}

		switch (m_iHorizontalAlign)
		{
			case HORIZONTAL_ALIGN_LEFT:
				break;
			case HORIZONTAL_ALIGN_CENTER:
				ix -= int(m_dwWidthSummary) / 2;
				break;
			case HORIZONTAL_ALIGN_RIGHT:
				ix -= int(m_dwWidthSummary);
				break;
		}

		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->SetPosition(ix, iy);
			ix += m_ImageInstanceVector[i]->GetWidth();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CImageBox::CImageBox(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_pImageInstance = NULL;
	}
	CImageBox::~CImageBox()
	{
		OnDestroyInstance();
	}

	#ifdef ENABLE_RENDER_TARGET
	void CImageBox::LeftRightReverse()
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->LeftRightReverse();
	}
	#endif

	void CImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicImageInstance::New();
	}
	void CImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance=NULL;
		}
	}

	BOOL CImageBox::LoadImage(const char * c_szFileName)
	{
		if (!c_szFileName[0])
			return FALSE;

		OnCreateInstance();

		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource)
			return FALSE;
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_pImageInstance->IsEmpty())
			return FALSE;

		SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
		UpdateRect();

		return TRUE;
	}

	void CImageBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	int CImageBox::GetWidth()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetWidth();
	}

	int CImageBox::GetHeight()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetHeight();
	}

#if defined(__BL_CLIP_MASK__)
	void CImageBox::OnUpdate()
	{
		if (!m_pImageInstance)
			return;

		if (!IsShow())
			return;

		if (!m_bEnableMask)
			return;

		if (!m_pMaskWindow)
			return;

		m_rMaskRect = m_pMaskWindow->GetRect();
	}
	void CImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
			m_pImageInstance->Render(m_bEnableMask ? &m_rMaskRect : NULL);
	}
#else
	void CImageBox::OnUpdate()
	{
	}
	void CImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
			m_pImageInstance->Render();
	}
#endif
	void CImageBox::OnChangePosition()
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetPosition(m_rect.left, m_rect.top);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	CMarkBox::CMarkBox(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_pMarkInstance = NULL;
	}

	CMarkBox::~CMarkBox()
	{
		OnDestroyInstance();
	}

	void CMarkBox::OnCreateInstance()
	{
		OnDestroyInstance();
		m_pMarkInstance = CGraphicMarkInstance::New();
	}

	void CMarkBox::OnDestroyInstance()
	{
		if (m_pMarkInstance)
		{
			CGraphicMarkInstance::Delete(m_pMarkInstance);
			m_pMarkInstance=NULL;
		}
	}

	void CMarkBox::LoadImage(const char * c_szFilename)
	{
		OnCreateInstance();

		m_pMarkInstance->SetImageFileName(c_szFilename);
		m_pMarkInstance->Load();
		SetSize(m_pMarkInstance->GetWidth(), m_pMarkInstance->GetHeight());

		UpdateRect();
	}

	void CMarkBox::SetScale(FLOAT fScale)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetScale(fScale);
	}

	void CMarkBox::SetIndex(UINT uIndex)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetIndex(uIndex);
	}

	void CMarkBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	void CMarkBox::OnUpdate()
	{
	}

	void CMarkBox::OnRender()
	{
		if (!m_pMarkInstance)
			return;

		if (IsShow())
			m_pMarkInstance->Render();
	}

	void CMarkBox::OnChangePosition()
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetPosition(m_rect.left, m_rect.top);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CExpandedImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CExpandedImageBox", strlen("CExpandedImageBox"));
		return (s_dwType);
	}

	BOOL CExpandedImageBox::OnIsType(DWORD dwType)
	{
		if (CExpandedImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CExpandedImageBox::CExpandedImageBox(PyObject * ppyObject) : CImageBox(ppyObject)
	{
	}
	CExpandedImageBox::~CExpandedImageBox()
	{
		OnDestroyInstance();
	}

	void CExpandedImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicExpandedImageInstance::New();
	}
	void CExpandedImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicExpandedImageInstance::Delete((CGraphicExpandedImageInstance*)m_pImageInstance);
			m_pImageInstance=NULL;
		}
	}

	void CExpandedImageBox::SetScale(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetScale(fx, fy);
		CWindow::SetSize(LONG(float(GetWidth())*fx), LONG(float(GetHeight())*fy));
	}
	void CExpandedImageBox::SetOrigin(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetOrigin(fx, fy);
	}
	void CExpandedImageBox::SetRotation(float fRotation)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRotation(fRotation);
	}
	void CExpandedImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}

	void CExpandedImageBox::SetRenderingMode(int iMode)
	{
		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingMode(iMode);
	}

	#if defined(ENABLE_IMAGE_CLIP_RECT) || defined(ENABLE_BATTLE_PASS)
	void CExpandedImageBox::SetImageClipRect(float fLeft, float fTop, float fRight, float fBottom, bool bIsVertical)
	{
		if (!m_pImageInstance)
			return;

		const RECT& c_rRect = GetRect();

		float fDifLeft = (c_rRect.left < fLeft) ? -(float(fLeft - c_rRect.left) / float(GetWidth())) : 0.0f;
		float fDifTop = (c_rRect.top < fTop) ? -(float(fTop - c_rRect.top) / float(GetHeight())) : 0.0f;
		float fDifRight = (c_rRect.right > fRight) ? -(float(c_rRect.right - fRight) / float(GetWidth())) : 0.0f;
		float fDifBottom = (c_rRect.bottom > fBottom) ? -(float(c_rRect.bottom - fBottom) / float(GetHeight())) : 0.0f;

		if (bIsVertical)
			((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fDifTop, fRight, fDifBottom);
		else
			((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fDifLeft, fDifTop, fDifRight, fDifBottom);
	}
	#endif

#ifdef __BL_CLIP_MASK__
	void CExpandedImageBox::OnUpdate()
	{
		if (!m_pImageInstance)
			return;

		if (!IsShow())
			return;

		if (!m_bEnableMask)
			return;

		if (!m_pMaskWindow)
			return;

		m_rMaskRect = m_pMaskWindow->GetRect();
	}
	void CExpandedImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
			m_pImageInstance->Render(m_bEnableMask ? &m_rMaskRect : NULL);
	}

#else
	void CExpandedImageBox::OnUpdate()
	{
	}
	void CExpandedImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
			m_pImageInstance->Render();
	}
#endif
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CAniImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CAniImageBox", strlen("CAniImageBox"));
		return (s_dwType);
	}

	BOOL CAniImageBox::OnIsType(DWORD dwType)
	{
		if (CAniImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CAniImageBox::CAniImageBox(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_bycurDelay(0),
			m_byDelay(4),
			m_bycurIndex(0)
	{
		m_ImageVector.clear();
	}
	CAniImageBox::~CAniImageBox()
	{
		for_each(m_ImageVector.begin(), m_ImageVector.end(), CGraphicExpandedImageInstance::DeleteExpandedImageInstance);
	}

	void CAniImageBox::SetDelay(int iDelay)
	{
		m_byDelay = iDelay;
	}

	#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	void CAniImageBox::SetDiffuseColor(const D3DXCOLOR & color)
	{
		for (auto & elem : m_ImageVector)
			elem->SetDiffuseColor(color.r, color.g, color.b, color.a);
	}

	//AniImageBoxFix
	/*void CAniImageBox::SetDiffuseColor(float fR, float fG, float fB, float fA)
	{
		m_SetDiffuseColor = std::bind(&CGraphicExpandedImageInstance::SetDiffuseColor, std::placeholders::_1, fR, fG, fB, fA);
		std::for_each(m_ImageVector.begin(), m_ImageVector.end(), m_SetDiffuseColor);
	}*/
	//AniImageBoxFix
	#endif

	void CAniImageBox::AppendImage(const char * c_szFileName)
	{
		m_ImageFileNames.push(c_szFileName);//AniImageBoxFix
	}
	/*{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return;

		CGraphicExpandedImageInstance * pImageInstance = CGraphicExpandedImageInstance::New();
		pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));

		if (pImageInstance->IsEmpty())
		{
			CGraphicExpandedImageInstance::Delete(pImageInstance);
			return;
		}

		m_ImageVector.push_back(pImageInstance);
		m_bycurIndex = rand() % m_ImageVector.size();
	}*/

	struct FSetRenderingRect
	{
		float fLeft, fTop, fRight, fBottom;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom);
		}
	};

	/*void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		FSetRenderingRect setRenderingRect;
		setRenderingRect.fLeft = fLeft;
		setRenderingRect.fTop = fTop;
		setRenderingRect.fRight = fRight;
		setRenderingRect.fBottom = fBottom;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingRect);
	}*/

	//AniImageBoxFix
	void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		m_SetRenderingRect = std::bind(&CGraphicExpandedImageInstance::SetRenderingRect, std::placeholders::_1, fLeft, fTop, fRight, fBottom);
		std::for_each(m_ImageVector.begin(), m_ImageVector.end(), m_SetRenderingRect);
	}
	//AniImageBoxFix

	struct FSetRenderingMode
	{
		int iMode;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetRenderingMode(iMode);
		}
	};

	/*void CAniImageBox::SetRenderingMode(int iMode)
	{
		FSetRenderingMode setRenderingMode;
		setRenderingMode.iMode = iMode;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingMode);
	}*/

	//AniImageBoxFix
	void CAniImageBox::SetRenderingMode(int iMode)
	{
		m_SetRenderingMode = std::bind(&CGraphicExpandedImageInstance::SetRenderingMode, std::placeholders::_1, iMode);
		std::for_each(m_ImageVector.begin(), m_ImageVector.end(), m_SetRenderingMode);
	}
	//AniImageBoxFix

	void CAniImageBox::ResetFrame()
	{
		m_bycurIndex = 0;
	}

	void CAniImageBox::OnUpdate()
	{
		//AniImageBoxFix
		if (!m_ImageFileNames.empty())
		{
			const std::string& stFileName = m_ImageFileNames.front();

			CResource* pResource = CResourceManager::Instance().GetResourcePointer(stFileName.c_str());
			if (pResource->IsType(CGraphicImage::Type()))
			{
				CGraphicExpandedImageInstance* pImageInstance = CGraphicExpandedImageInstance::New();
				pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));

				if (pImageInstance->IsEmpty())
				{
					CGraphicExpandedImageInstance::Delete(pImageInstance);
				}
				else
				{
					pImageInstance->SetPosition(m_rect.left, m_rect.top);

					if (m_SetRenderingRect)
						m_SetRenderingRect(pImageInstance);

					if (m_SetRenderingMode)
						m_SetRenderingMode(pImageInstance);

					if (m_SetDiffuseColor)
						m_SetDiffuseColor(pImageInstance);

					m_ImageVector.push_back(pImageInstance);
				}
			}

			m_ImageFileNames.pop();
		}
		//AniImageBoxFix

		++m_bycurDelay;

		#ifdef ENABLE_FPS
		if (g_fGameFPS > 65)
		{
			if (m_bycurDelay < m_byDelay + (m_byDelay * ((g_fGameFPS / 60.0) - 1)))
				return;
		}
		else
		#endif
		{
			if (m_bycurDelay < m_byDelay)
				return;
		}

		m_bycurDelay = 0;

		++m_bycurIndex;
		if (m_bycurIndex >= m_ImageVector.size())
		{
			m_bycurIndex = 0;

			OnEndFrame();
		}
	}
	void CAniImageBox::OnRender()
	{
		if (m_bycurIndex < m_ImageVector.size())
		{
			CGraphicExpandedImageInstance * pImage = m_ImageVector[m_bycurIndex];
			pImage->Render();
		}
	}

	struct FChangePosition
	{
		float fx, fy;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetPosition(fx, fy);
		}
	};

	/*void CAniImageBox::OnChangePosition()
	{
		FChangePosition changePosition;
		changePosition.fx = m_rect.left;
		changePosition.fy = m_rect.top;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), changePosition);
	}*/

	//AniImageBoxFix
	void CAniImageBox::OnChangePosition()
	{
		std::for_each(m_ImageVector.begin(), m_ImageVector.end(),
			std::bind(&CGraphicExpandedImageInstance::SetPosition, std::placeholders::_1, m_rect.left, m_rect.top));
	}
	//AniImageBoxFix

	void CAniImageBox::OnEndFrame()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndFrame", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	/*
	#ifdef ENABLE_GIF_SUPPORT
	uint32_t CGifImageBox::Type()
	{
		static uint32_t s_dwType = GetCRC32("CGifImageBox", strlen("CGifImageBox"));
		return (s_dwType);
	}

	BOOL CGifImageBox::OnIsType(uint32_t dwType)
	{
		if (CGifImageBox::Type() == dwType)
			return TRUE;
		return FALSE;
	}

	CGifImageBox::CGifImageBox(PyObject* ppyObject)
		: CAniImageBox(ppyObject)
	{
	}
	CGifImageBox::~CGifImageBox()
	{
	}

	bool RawFrameToBmpImage(SavedImage* currentFrame, GifFileType* gifFile, std::vector <uint8_t>& outData)
	{
		if (!currentFrame || !gifFile)
		{
			TraceError("RawFrameToBmpImage: Invalid parameters.");
			return false;
		}

		auto& imageDesc = currentFrame->ImageDesc;
		auto* colorMap = (imageDesc.ColorMap) ? imageDesc.ColorMap : gifFile->SColorMap;
		if (!colorMap)
		{
			TraceError("RawFrameToBmpImage: ColorMap is null.");
			return false;
		}

		const int width = imageDesc.Width;
		const int height = imageDesc.Height;
		const int rowPadding = (4 - (width * 3 % 4)) % 4;
		const std::size_t rowDataSize = static_cast<std::size_t>(width * 3 + rowPadding) * height;

		BITMAPFILEHEADER fileHeader = {
			0x4D42, // "BM"
			static_cast<uint32_t>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + rowDataSize),
			0, // Reserved1
			0, // Reserved2
			sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
		};

		BITMAPINFOHEADER infoHeader = {
			sizeof(BITMAPINFOHEADER),
			width,
			height,
			1, // Planes
			24, // BitCount
			0, // Compression
			static_cast<uint32_t>(rowDataSize),
			0, // XPelsPerMeter
			0, // YPelsPerMeter
			0, // ClrUsed
			0  // ClrImportant
		};

		outData.resize(fileHeader.bfSize);
		std::memcpy(outData.data(), &fileHeader, sizeof(fileHeader));
		std::memcpy(outData.data() + sizeof(fileHeader), &infoHeader, sizeof(infoHeader));

		auto pixelData = outData.begin() + fileHeader.bfOffBits;
		for (int y = height - 1; y >= 0; --y)
		{
			for (int x = 0; x < width; ++x)
			{
				const int index = y * width + x;
				const GifByteType colorIndex = currentFrame->RasterBits[index];
				const GifColorType& gifColor = colorMap->Colors[colorIndex];

				*pixelData++ = gifColor.Blue;
				*pixelData++ = gifColor.Green;
				*pixelData++ = gifColor.Red;
			}
			pixelData = std::fill_n(pixelData, rowPadding, 0x00);
		}
	}

	bool CGifImageBox::LoadGif(const char* c_szFileName)
	{
		if (!c_szFileName || !*c_szFileName)
			return false;

		// Open GIF file
		int error;
		GifFileType* gif = DGifOpenFileName(c_szFileName, &error);
		if (!gif)
		{
			const char* c_szError = GifErrorString(error);
			TraceError("Failed to open GIF file: %s with error: %s", c_szFileName, c_szError);
			return false;
		}

		// Read the GIF into memory
		if (DGifSlurp(gif) == GIF_ERROR)
		{
			TraceError("Failed to read GIF file: %s", c_szFileName);
			DGifCloseFile(gif, &error);
			return false;
		}

		// Iterate through the frames
		for (int i = 0; i < gif->ImageCount; ++i)
		{
			// Get the current frame
			SavedImage* currentImage = &gif->SavedImages[i];

			// Convert the frame to BMP image
			std::vector <uint8_t> bmpBuffer;
			if (!RawFrameToBmpImage(currentImage, gif, bmpBuffer))
			{
				TraceError("Failed to convert GIF(%s) frame #%d to BMP image.", c_szFileName, i);
				continue;
			}

			// Create name for the frame
			char frameFileName[256]{ '\0' };
			snprintf(frameFileName, sizeof(frameFileName), "gif_frame%d_%s", i, c_szFileName);

			// Create the image instance
			CGraphicImage* pImage = new CGraphicImage(frameFileName, bmpBuffer.data(), bmpBuffer.size(), D3DX_FILTER_LINEAR);
			if (!pImage)
			{
				TraceError("Failed to create image instance for GIF(%s) frame #%d.", c_szFileName, i);
				continue;
			}

			// Create the expanded image instance
			CGraphicExpandedImageInstance* pImageInstance = CGraphicExpandedImageInstance::New();
			pImageInstance->SetImagePointer(pImage);

			// Add the image instance to the vector if it's not empty
			if (pImageInstance->IsEmpty())
			{
				CGraphicExpandedImageInstance::Delete(pImageInstance);
			}
			else
			{
				pImageInstance->SetPosition(m_rect.left, m_rect.top);
				m_ImageVector.push_back(pImageInstance);
			}
		}

		// Clean up
		DGifCloseFile(gif, &error);
		return true;
	}

	bool CGifImageBox::UnloadGif()
	{
		for (auto pImage : m_ImageVector)
			CGraphicExpandedImageInstance::Delete(pImage);

		m_ImageVector.clear();
		return true;
	}
	#endif
	*/

	#ifdef ENABLE_GIF_SUPPORT
	uint32_t CGifImageBox::Type()
	{
		static uint32_t s_dwType = GetCRC32("CGifImageBox", strlen("CGifImageBox"));
		return (s_dwType);
	}

	BOOL CGifImageBox::OnIsType(uint32_t dwType)
	{
		if (CGifImageBox::Type() == dwType)
			return TRUE;
		return FALSE;
	}

	CGifImageBox::CGifImageBox(PyObject* ppyObject)
		: CAniImageBox(ppyObject)
	{
	}
	CGifImageBox::~CGifImageBox()
	{
	}

	bool RawFrameToTGAImage(SavedImage* currentFrame, GifFileType* gifFile, std::vector <uint8_t>& outData)
	{
		if (!currentFrame || !gifFile)
		{
			TraceError("RawFrameToTGAImage: Invalid parameters.");
			return false;
		}

		auto& imageDesc = currentFrame->ImageDesc;
		auto* colorMap = (imageDesc.ColorMap) ? imageDesc.ColorMap : gifFile->SColorMap;
		if (!colorMap)
		{
			TraceError("RawFrameToTGAImage: ColorMap is null.");
			return false;
		}

		const int width = imageDesc.Width;
		const int height = imageDesc.Height;

		// TGA Header
		std::array<uint8_t, 18> header = {0};
		header[2] = 2; // Uncompressed RGB
		header[12] = width & 0xFF;
		header[13] = (width >> 8) & 0xFF;
		header[14] = height & 0xFF;
		header[15] = (height >> 8) & 0xFF;
		// header[16] = 32; // 32 bits per pixel
		header[16] = 32; // 32 bits per pixel

		outData.insert(outData.end(), header.begin(), header.end());

		// Get transparency information from the GIF
		GraphicsControlBlock gcb;
		DGifSavedExtensionToGCB(gifFile, currentFrame - gifFile->SavedImages, &gcb);

		for (int y = height - 1; y >= 0; --y)
		{
			for (int x = 0; x < width; ++x)
			{
				const int index = y * width + x;
				const GifByteType colorIndex = currentFrame->RasterBits[index];
				if (colorIndex < colorMap->ColorCount)
				{
					const GifColorType& gifColor = colorMap->Colors[colorIndex];
					outData.push_back(gifColor.Blue);
					outData.push_back(gifColor.Green);
					outData.push_back(gifColor.Red);
					// If the pixel is transparent in the GIF, make it transparent in the TGA
					outData.push_back((gcb.TransparentColor && colorIndex == gcb.TransparentColor) ? 0x00 : 0xFF);
				}
				else
				{
					TraceError("RawFrameToTGAImage: Color index out of range.");
					return false;
				}
			}
		}

		return true;
	}

	bool CGifImageBox::LoadGif(const char* c_szFileName)
	{
		if (!c_szFileName || !*c_szFileName)
			return false;

		// Open GIF file
		int error;
		GifFileType* gif = DGifOpenFileName(c_szFileName, &error);
		if (!gif)
		{
			const char* c_szError = GifErrorString(error);
			TraceError("Failed to open GIF file: %s with error: %s", c_szFileName, c_szError);
			return false;
		}

		// Read the GIF into memory
		if (DGifSlurp(gif) == GIF_ERROR)
		{
			TraceError("Failed to read GIF file: %s", c_szFileName);
			DGifCloseFile(gif, &error);
			return false;
		}

		// Iterate through the frames
		for (int i = 0; i < gif->ImageCount; ++i)
		{
			// Get the current frame
			SavedImage* currentImage = &gif->SavedImages[i];

			// Convert the frame to TGA image
			std::vector <uint8_t> tgaBuffer;
			if (!RawFrameToTGAImage(currentImage, gif, tgaBuffer))
			{
				TraceError("Failed to convert GIF(%s) frame #%d to TGA image.", c_szFileName, i);
				continue;
			}

			// Create name for the frame
			char frameFileName[256]{ '\0' };
			snprintf(frameFileName, sizeof(frameFileName), "gif_frame%d_%s", i, c_szFileName);

			// Create the image instance
			CGraphicImage* pImage = new CGraphicImage(frameFileName, tgaBuffer.data(), tgaBuffer.size(), D3DX_FILTER_LINEAR);
			if (!pImage)
			{
				TraceError("Failed to create image instance for GIF(%s) frame #%d.", c_szFileName, i);
				continue;
			}

			// Create the expanded image instance
			CGraphicExpandedImageInstance* pImageInstance = CGraphicExpandedImageInstance::New();
			pImageInstance->SetImagePointer(pImage);

			// Add the image instance to the vector if it's not empty
			if (pImageInstance->IsEmpty())
			{
				CGraphicExpandedImageInstance::Delete(pImageInstance);
			}
			else
			{
				pImageInstance->SetPosition(m_rect.left, m_rect.top);
				m_ImageVector.push_back(pImageInstance);
			}
		}

		// Clean up
		DGifCloseFile(gif, &error);
		return true;
	}

	bool CGifImageBox::UnloadGif()
	{
		for (auto pImage : m_ImageVector)
			CGraphicExpandedImageInstance::Delete(pImage);

		m_ImageVector.clear();
		return true;
	}
	#endif

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CButton::CButton(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_pcurVisual(NULL),
			m_bEnable(TRUE),
			m_isPressed(FALSE),
			m_isFlash(FALSE)
			, m_isEnableFlash(FALSE)
	{
		CWindow::AddFlag(CWindow::FLAG_NOT_CAPTURE);
	}
	CButton::~CButton()
	{
	}

	#ifdef ENABLE_RENDER_TARGET
	void CButton::LeftRightReverse()
	{
		m_upVisual.LeftRightReverse();
		m_overVisual.LeftRightReverse();
		m_downVisual.LeftRightReverse();
		m_disableVisual.LeftRightReverse();
	}
	#endif

	BOOL CButton::SetUpVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_upVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_upVisual.IsEmpty())
			return FALSE;

		SetSize(m_upVisual.GetWidth(), m_upVisual.GetHeight());
		//
		SetCurrentVisual(&m_upVisual);
		//

		return TRUE;
	}
	BOOL CButton::SetOverVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_overVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_overVisual.IsEmpty())
			return FALSE;

		SetSize(m_overVisual.GetWidth(), m_overVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDownVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_downVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_downVisual.GetWidth(), m_downVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDisableVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_disableVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_disableVisual.GetWidth(), m_disableVisual.GetHeight());

		return TRUE;
	}

	const char * CButton::GetUpVisualFileName()
	{
		return m_upVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char * CButton::GetOverVisualFileName()
	{
		return m_overVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char * CButton::GetDownVisualFileName()
	{
		return m_downVisual.GetGraphicImagePointer()->GetFileName();
	}

	void CButton::Flash()
	{
		m_isFlash = TRUE;
	}

	void CButton::EnableFlash()
	{
		m_isEnableFlash = TRUE;
	}

	void CButton::Enable()
	{
		SetUp();
		m_bEnable = TRUE;
	}

	void CButton::Disable()
	{
		m_bEnable = FALSE;
		if (!m_disableVisual.IsEmpty())
			SetCurrentVisual(&m_disableVisual);
	}

	BOOL CButton::IsDisable()
	{
		return m_bEnable;
	}

	void CButton::SetUp()
	{
		SetCurrentVisual(&m_upVisual);
		m_isPressed = FALSE;
	}
	void CButton::Up()
	{
		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);

		PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
	}
	void CButton::Over()
	{
		SetCurrentVisual(&m_overVisual);
	}
	void CButton::Down()
	{
		m_isPressed = TRUE;
		SetCurrentVisual(&m_downVisual);
		PyCallClassMemberFunc(m_poHandler, "DownEvent", BuildEmptyTuple());
	}

	void CButton::SetScale(float fx, float fy)
	{
		if (m_upVisual.IsEmpty())
		{
			return;
		}

		if (m_overVisual.IsEmpty())
		{
			return;
		}

		if (m_downVisual.IsEmpty())
		{
			return;
		}

		m_upVisual.SetScale (fx, fy);
		m_overVisual.SetScale (fx, fy);
		m_downVisual.SetScale (fx, fy);

		CWindow::SetSize (LONG (float (GetWidth()) * fx), LONG (float (GetHeight()) * fy));
	}
#if defined(__BL_CLIP_MASK__)
	void CButton::OnUpdate()
	{
		if (!m_bEnableMask)
			return;

		if (!m_pMaskWindow)
			return;

		m_rMaskRect = m_pMaskWindow->GetRect();
	}
	void CButton::OnRender()
	{
		if (!IsShow())
			return;

		if (m_pcurVisual)
		{
			if (m_isFlash)
				if (!IsIn())
					if (int(timeGetTime() / 500) % 2)
					{
						return;
					}
			m_pcurVisual->Render(m_bEnableMask ? &m_rMaskRect : NULL);
		}

		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}
#else
	void CButton::OnUpdate()
	{
	}
	void CButton::OnRender()
	{
		if (!IsShow())
			return;

		if (m_pcurVisual)
		{
			if (m_isFlash)
			if (!IsIn())
			if (int(timeGetTime() / 500)%2)
			{
				return;
			}
			if (m_isEnableFlash && !IsIn())
			{
				if (int(timeGetTime() / 500) % 2)
					Over();
				if (int(timeGetTime() / (500 - 15)) % 2)
					Down();
			}
			m_pcurVisual->Render();
		}

		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}
#endif
	void CButton::OnChangePosition()
	{
		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		m_isPressed = TRUE;
		Down();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonDoubleClick()
	{
		if (!IsEnable())
			return TRUE;

		OnMouseLeftButtonDown();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		if (!IsPressed())
			return TRUE;

		m_isPressed = FALSE;
		Up();

		return TRUE;
	}
	void CButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		Over();
		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		SetUp();
		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CButton", strlen("CButton"));
		return (s_dwType);
	}
	BOOL CButton::OnIsType(DWORD dwType)
	{
		if (CButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
	void CButton::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		m_upVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
		m_overVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
		m_downVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}
	void CButton::SetCurrentVisual(CGraphicExpandedImageInstance* pVisual)
#else
	void CButton::SetCurrentVisual(CGraphicImageInstance* pVisual)
#endif
	{
		m_pcurVisual = pVisual;
		m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::IsEnable()
	{
		return m_bEnable;
	}

	BOOL CButton::IsPressed()
	{
		return m_isPressed;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CRadioButton::CRadioButton(PyObject * ppyObject) : CButton(ppyObject)
	{
	}
	CRadioButton::~CRadioButton()
	{
	}
#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CRadioButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CRadioButton", strlen("CRadioButton"));
		return (s_dwType);
	}
	BOOL CRadioButton::OnIsType(DWORD dwType)
	{
		if (CRadioButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
#endif
	BOOL CRadioButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (!m_isPressed)
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CRadioButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}
	void CRadioButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CRadioButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CToggleButton::CToggleButton(PyObject * ppyObject) : CButton(ppyObject)
	{
	}
	CToggleButton::~CToggleButton()
	{
	}
#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CToggleButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CToggleButton", strlen("CToggleButton"));
		return (s_dwType);
	}
	BOOL CToggleButton::OnIsType(DWORD dwType)
	{
		if (CToggleButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
#endif
	BOOL CToggleButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (m_isPressed)
		{
			SetUp();
			if (IsIn())
				SetCurrentVisual(&m_overVisual);
			else
				SetCurrentVisual(&m_upVisual);
			PyCallClassMemberFunc(m_poHandler, "OnToggleUp", BuildEmptyTuple());
		}
		else
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "OnToggleDown", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CToggleButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}

	void CToggleButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CToggleButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CDragButton::CDragButton(PyObject * ppyObject) : CButton(ppyObject)
	{
		CWindow::RemoveFlag(CWindow::FLAG_NOT_CAPTURE);
		m_restrictArea.left = 0;
		m_restrictArea.top = 0;
		m_restrictArea.right = CWindowManager::Instance().GetScreenWidth();
		m_restrictArea.bottom = CWindowManager::Instance().GetScreenHeight();
	}
	CDragButton::~CDragButton()
	{
	}

	void CDragButton::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
	{
		m_restrictArea.left = ix;
		m_restrictArea.top = iy;
		m_restrictArea.right = ix + iwidth;
		m_restrictArea.bottom = iy + iheight;
	}

	void CDragButton::OnChangePosition()
	{
		m_x = max(m_x, m_restrictArea.left);
		m_y = max(m_y, m_restrictArea.top);
		m_x = min(m_x, max(0, m_restrictArea.right - m_lWidth));
		m_y = min(m_y, max(0, m_restrictArea.bottom - m_lHeight));

		m_rect.left = m_x;
		m_rect.top = m_y;

		if (m_pParent)
		{
			const RECT & c_rRect = m_pParent->GetRect();
			m_rect.left += c_rRect.left;
			m_rect.top += c_rRect.top;
		}

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

		if (IsPressed())
			PyCallClassMemberFunc(m_poHandler, "OnMove", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverIn()
	{
		if (!IsEnable())

			return;

		CButton::OnMouseOverIn();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		CButton::OnMouseOverIn();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
