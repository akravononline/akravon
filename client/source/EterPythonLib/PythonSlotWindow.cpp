#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "../eterBase/Filename.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"

#ifdef CYNEX_MOBILE
// Cynex: wndMgr.SetSlotScale / SetSlotUnitSize ikon olcegi
void CynexSlot_BeforeIcon(UI::CSlotWindow* pWnd, DWORD dwSlotNumber, CGraphicImageInstance* pInstance, float fPosX, float fPosY);
void CynexSlot_BeforeBase(UI::CSlotWindow* pWnd, CGraphicImageInstance* pInstance);
float CynexSlot_CoolHalf(UI::CSlotWindow* pWnd, DWORD dwSlotNumber);
bool CynexSlot_IsRound(UI::CSlotWindow* pWnd);
bool CynexSlot_CellSize(UI::CSlotWindow* pWnd, DWORD dwSlotNumber, int iPlacedX, int iPlacedY, float* pfW, float* pfH);
void CynexSlot_Forget(UI::CSlotWindow* pWnd);
#endif

//#define __RENDER_SLOT_AREA__

using namespace UI;

class UI::CSlotWindow::CSlotButton : public CButton
{
	public:
		enum ESlotButtonType
		{
			SLOT_BUTTON_TYPE_PLUS,
			SLOT_BUTTON_TYPE_COVER,
		};

	public:
		CSlotButton(ESlotButtonType dwType, DWORD dwSlotNumber, CSlotWindow * pParent) : CButton(NULL)
		{
			m_dwSlotButtonType = dwType;
			m_dwSlotNumber = dwSlotNumber;
			m_pParent = pParent;
		}
		virtual ~CSlotButton()
		{
		}

		BOOL OnMouseLeftButtonDown()
		{
			if (!IsEnable())
				return TRUE;
			if (UI::CWindowManager::Instance().IsAttaching())
				return TRUE;

			m_isPressed = TRUE;
			Down();

			return TRUE;
		}
		BOOL OnMouseLeftButtonUp()
		{
			if (!IsEnable())
				return TRUE;
			if (!IsPressed())
				return TRUE;

			if (IsIn())
				SetCurrentVisual(&m_overVisual);
			else
				SetCurrentVisual(&m_upVisual);

			m_pParent->OnPressedSlotButton(m_dwSlotButtonType, m_dwSlotNumber);

			return TRUE;
		}

		void OnMouseOverIn()
		{
			if (IsEnable())
			{
				SetCurrentVisual(&m_overVisual);
			}
			m_pParent->OnOverInItem(m_dwSlotNumber);
		}
		void OnMouseOverOut()
		{
			if (IsEnable())
			{
				SetUp();
				SetCurrentVisual(&m_upVisual);
			}
			m_pParent->OnOverOutItem();
		}

	protected:
		ESlotButtonType m_dwSlotButtonType;
		DWORD m_dwSlotNumber;
		CSlotWindow * m_pParent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class UI::CSlotWindow::CCoverButton : public CSlotButton
{
	public:
		CCoverButton(ESlotButtonType dwType, DWORD dwSlotNumber, CSlotWindow * pParent) : CSlotButton(dwType, dwSlotNumber, pParent)
		{
			m_bLeftButtonEnable = TRUE;
			m_bRightButtonEnable = TRUE;
		}
		virtual ~CCoverButton()
		{
		}

		void SetLeftButtonEnable(BOOL bEnable)
		{
			m_bLeftButtonEnable = bEnable;
		}
		void SetRightButtonEnable(BOOL bEnable)
		{
			m_bRightButtonEnable = bEnable;
		}

		void OnRender()
		{
		}

		void RenderButton()
		{
			CButton::OnRender();
		}

		BOOL OnMouseLeftButtonDown()
		{
			if (!IsEnable())
				return TRUE;
			if (m_bLeftButtonEnable)
			{
				CButton::OnMouseLeftButtonDown();
			}
			else
			{
				m_pParent->OnMouseLeftButtonDown();
			}
			return TRUE;
		}
		BOOL OnMouseLeftButtonUp()
		{
			if (!IsEnable())
				return TRUE;
			if (m_bLeftButtonEnable)
			{
				CButton::OnMouseLeftButtonUp();
				m_pParent->OnMouseLeftButtonDown();
			}
			else
			{
				m_pParent->OnMouseLeftButtonUp();
			}
			return TRUE;
		}

		BOOL OnMouseRightButtonDown()
		{
			if (!IsEnable())
				return TRUE;
			if (m_bRightButtonEnable)
				CButton::OnMouseLeftButtonDown();
			return TRUE;
		}
		BOOL OnMouseRightButtonUp()
		{
			if (!IsEnable())
				return TRUE;
			m_pParent->OnMouseRightButtonDown();
			if (m_bRightButtonEnable)
				CButton::OnMouseLeftButtonUp();
			return TRUE;
		}

	protected:
		BOOL m_bLeftButtonEnable;
		BOOL m_bRightButtonEnable;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class UI::CSlotWindow::CCoolTimeFinishEffect : public CAniImageBox
{
	public:
		CCoolTimeFinishEffect(CSlotWindow * pParent, DWORD dwSlotIndex) : CAniImageBox(NULL)
		{
			m_pParent = pParent;
			m_dwSlotIndex = dwSlotIndex;
		}
		virtual ~CCoolTimeFinishEffect()
		{
		}

		void OnEndFrame()
		{
			((CSlotWindow *)m_pParent)->ReserveDestroyCoolTimeFinishEffect(m_dwSlotIndex);
		}

	protected:
		DWORD m_dwSlotIndex;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// Set & Append

void CSlotWindow::SetSlotType(DWORD dwType)
{
	m_dwSlotType = dwType;
}


// AKRAVON: yuvarlak kose cizim yardimcilari (1 px satirlarla)
static float CynexRoundInset(float r, int dy)
{
	if (dy >= int(r))
		return 0.0f;
	const float t = r - float(dy) - 0.5f;
	const float v = r * r - t * t;
	return r - sqrtf(v > 0.0f ? v : 0.0f);
}

// dolu yuvarlak dikdortgen
static void CynexFillRound(float x0, float y0, float x1, float y1, float r)
{
	const int h = int(y1 - y0);
	for (int i = 0; i < h; ++i)
	{
		const int dy = i < h - 1 - i ? i : h - 1 - i;
		const float in = CynexRoundInset(r, dy);
		CPythonGraphic::Instance().RenderBar2d(x0 + in, y0 + float(i), x1 - in, y0 + float(i) + 1.0f);
	}
}

// yuvarlak dikdortgen cerceve (ince)
static void CynexFrameRound(float x0, float y0, float x1, float y1, float r, float thick)
{
	const int h = int(y1 - y0);
	for (int i = 0; i < h; ++i)
	{
		const int dy = i < h - 1 - i ? i : h - 1 - i;
		const float in = CynexRoundInset(r, dy);
		if (dy < int(thick))
		{
			CPythonGraphic::Instance().RenderBar2d(x0 + in, y0 + float(i), x1 - in, y0 + float(i) + 1.0f);
		}
		else
		{
			const float next = CynexRoundInset(r, dy + 1);
			const float w = thick + (in > next ? in - next : 0.0f);
			CPythonGraphic::Instance().RenderBar2d(x0 + in, y0 + float(i), x0 + in + w, y0 + float(i) + 1.0f);
			CPythonGraphic::Instance().RenderBar2d(x1 - in - w, y0 + float(i), x1 - in, y0 + float(i) + 1.0f);
		}
	}
}

void CSlotWindow::SetSlotStyle(DWORD dwStyle)
{
	m_dwSlotStyle = dwStyle;
}

void CSlotWindow::AppendSlot(DWORD dwIndex, int ixPosition, int iyPosition, int ixCellSize, int iyCellSize)
{
	TSlot Slot;
	Slot.pInstance = NULL;
	Slot.pNumberLine = NULL;
	Slot.pCoverButton = NULL;
	Slot.pSlotButton = NULL;
	Slot.pSignImage = NULL;
	Slot.pFinishCoolTimeEffect = NULL;

	ClearSlot(&Slot);
	Slot.dwSlotNumber = dwIndex;
	Slot.dwCenterSlotNumber = dwIndex;
	Slot.ixPosition = ixPosition;
	Slot.iyPosition = iyPosition;
	Slot.ixCellSize = ixCellSize;
	Slot.iyCellSize = iyCellSize;
	m_SlotList.push_back(Slot);
}

void CSlotWindow::SetCoverButton(DWORD dwIndex, const char * c_szUpImageName, const char * c_szOverImageName, const char * c_szDownImageName, const char * c_szDisableImageName, BOOL bLeftButtonEnable, BOOL bRightButtonEnable)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	CCoverButton *& rpCoverButton = pSlot->pCoverButton;

	if (!rpCoverButton)
	{
		rpCoverButton = new CCoverButton(CSlotButton::SLOT_BUTTON_TYPE_COVER, pSlot->dwSlotNumber, this);
		CWindowManager::Instance().SetParent(rpCoverButton, this);
	}

	rpCoverButton->SetLeftButtonEnable(bLeftButtonEnable);
	rpCoverButton->SetRightButtonEnable(bRightButtonEnable);
	rpCoverButton->SetUpVisual(c_szUpImageName);
	rpCoverButton->SetOverVisual(c_szOverImageName);
	rpCoverButton->SetDownVisual(c_szDownImageName);
	rpCoverButton->SetDisableVisual(c_szDisableImageName);
	rpCoverButton->Enable();
	rpCoverButton->Show();

	if (pSlot->pSlotButton)
	{
		SetTop(pSlot->pSlotButton);
	}
}

void CSlotWindow::EnableCoverButton(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Enable();
}

void CSlotWindow::DisableCoverButton(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (!pSlot->pCoverButton)
		return;

	pSlot->pCoverButton->Disable();
}

void CSlotWindow::SetAlwaysRenderCoverButton(DWORD dwIndex, bool bAlwaysRender)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (bAlwaysRender)
		SET_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
	else
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_ALWAYS_RENDER_COVER);
}

void CSlotWindow::ShowSlotBaseImage(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = true;
}

void CSlotWindow::HideSlotBaseImage(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bRenderBaseSlotImage = false;
}

BOOL CSlotWindow::IsDisableCoverButton(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return FALSE;

	if (!pSlot->pCoverButton)
		return FALSE;

	return pSlot->pCoverButton->IsDisable();
}

void CSlotWindow::SetSlotBaseImage(const char * c_szFileName, float fr, float fg, float fb, float fa)
{
	__CreateBaseImage(c_szFileName, fr, fg, fb, fa);
}

void CSlotWindow::AppendSlotButton(const char * c_szUpImageName, const char * c_szOverImageName, const char * c_szDownImageName)
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;
		CSlotButton *& rpSlotButton = rSlot.pSlotButton;

		if (!rpSlotButton)
		{
			rpSlotButton = new CSlotButton(CSlotButton::SLOT_BUTTON_TYPE_PLUS, rSlot.dwSlotNumber, this);
			rpSlotButton->AddFlag(FLAG_FLOAT);
			CWindowManager::Instance().SetParent(rpSlotButton, this);
		}

		rpSlotButton->SetUpVisual(c_szUpImageName);
		rpSlotButton->SetOverVisual(c_szOverImageName);
		rpSlotButton->SetDownVisual(c_szDownImageName);
		rpSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		rpSlotButton->Hide();
	}
}

void CSlotWindow::AppendRequirementSignImage(const char * c_szImageName)
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;
		CImageBox *& rpSignImage = rSlot.pSignImage;

		if (!rpSignImage)
		{
			rpSignImage = new CImageBox(NULL);
			CWindowManager::Instance().SetParent(rpSignImage, this);
		}

		rpSignImage->LoadImage(c_szImageName);
		rpSignImage->Hide();
	}
}

BOOL CSlotWindow::HasSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return FALSE;

	return TRUE;
}
#ifdef SPIN_WHEEL
DWORD CSlotWindow::GetSlot(DWORD dwIndex)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return -1;

	return pSlot->dwItemIndex;
}
#endif
void CSlotWindow::SetSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, CGraphicImage * pImage, D3DXCOLOR& diffuseColor)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
	if (pSlot->dwItemIndex == dwVirtualNumber)
	{
		pSlot->dwState = 0;
		pSlot->isItem = TRUE;
		if (pImage && pSlot->pInstance)
		{
			pSlot->pInstance->SetImagePointer(pImage);
		}
		return;
	}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(NULL == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
		pSlot->pInstance->SetImagePointer(pImage);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}
}

#ifdef ENABLE_OKEY_KART_EVENT
void CSlotWindow::SetCardSlot(DWORD dwIndex, DWORD dwVirtualNumber, BYTE byWidth, BYTE byHeight, const char * c_szFileName, D3DXCOLOR& diffuseColor)
{
	CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(c_szFileName);
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (pSlot->isItem)
	if (pSlot->dwItemIndex == dwVirtualNumber)
	{
		pSlot->dwState = 0;
		pSlot->isItem = TRUE;
		if (pImage && pSlot->pInstance)
		{
			pSlot->pInstance->SetImagePointer(pImage);
		}
		return;
	}

	ClearSlot(pSlot);
	pSlot->dwState = 0;
	pSlot->isItem = TRUE;
	pSlot->dwItemIndex = dwVirtualNumber;

	if (pImage)
	{
		assert(NULL == pSlot->pInstance);
		pSlot->pInstance = CGraphicImageInstance::New();
		pSlot->pInstance->SetImagePointer(pImage);
		pSlot->pInstance->SetDiffuseColor(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	}

	pSlot->byxPlacedItemSize = byWidth;
	pSlot->byyPlacedItemSize = byHeight;

	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Show();
	}
}
#endif

void CSlotWindow::SetSlotCount(DWORD dwIndex, DWORD dwCount)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = NULL;
		}
	}
	else
	{
		char szCount[16+1];
		_snprintf(szCount, sizeof(szCount), "%d", dwCount);

		if (!pSlot->pNumberLine)
		{
			CNumberLine * pNumberLine = new UI::CNumberLine(this);
			pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlotCountNew(DWORD dwIndex, DWORD dwGrade, DWORD dwCount)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwCount <= 0)
	{
		if (pSlot->pNumberLine)
		{
			delete pSlot->pNumberLine;
			pSlot->pNumberLine = NULL;
		}
	}
	else
	{
		char szCount[16+1];

		switch (dwGrade)
		{
			case 0:
				_snprintf(szCount, sizeof(szCount), "%d", dwCount);
				break;
			case 1:
				_snprintf(szCount, sizeof(szCount), "m%d", dwCount);
				break;
			case 2:
				_snprintf(szCount, sizeof(szCount), "g%d", dwCount);
				break;
			case 3:
				_snprintf(szCount, sizeof(szCount), "p");
				break;
		}

		if (!pSlot->pNumberLine)
		{
			CNumberLine * pNumberLine = new UI::CNumberLine(this);
			pNumberLine->SetHorizontalAlign(CNumberLine::HORIZONTAL_ALIGN_RIGHT);
			pNumberLine->Show();
			pSlot->pNumberLine = pNumberLine;
		}

		pSlot->pNumberLine->SetNumber(szCount);
	}
}

void CSlotWindow::SetSlotCoolTime(DWORD dwIndex, float fCoolTime, float fElapsedTime)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->fCoolTime = fCoolTime;
	pSlot->fStartCoolTime = CTimer::Instance().GetCurrentSecond() - fElapsedTime;
}

void CSlotWindow::ActivateSlot(DWORD dwIndex, const D3DXCOLOR & color)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = TRUE;
	#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	pSlot->Color = color;
	#endif

	if (!m_pSlotActiveEffect
		#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
		|| !m_pSlotActiveEffectSlot2 || !m_pSlotActiveEffectSlot3
		#endif
	)
		__CreateSlotEnableEffect();
}

void CSlotWindow::DeactivateSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->bActive = FALSE;
}

#ifdef ENABLE_SLOT_WINDOW_EX
float CSlotWindow::GetSlotCoolTime(DWORD dwIndex, float * fElapsedTime)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return 0.0;

	*fElapsedTime = CTimer::Instance().GetCurrentSecond() - pSlot->fStartCoolTime;
	return pSlot->fCoolTime;
}

bool CSlotWindow::IsActivatedSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return false;
	return pSlot->bActive;
}
#endif

void CSlotWindow::ClearSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	ClearSlot(pSlot);
}

void CSlotWindow::ClearSlot(TSlot * pSlot)
{
	pSlot->bActive = FALSE;
	pSlot->byxPlacedItemSize = 1;
	pSlot->byyPlacedItemSize = 1;

	pSlot->isItem = FALSE;
	pSlot->dwState = 0;
	pSlot->fCoolTime = 0.0f;
	pSlot->fStartCoolTime = 0.0f;
	pSlot->dwCenterSlotNumber = 0xffffffff;

	pSlot->dwItemIndex = 0;
	pSlot->bRenderBaseSlotImage = true;

	if (pSlot->pInstance)
	{
		CGraphicImageInstance::Delete(pSlot->pInstance);
		pSlot->pInstance = NULL;
	}
	if (pSlot->pCoverButton)
	{
		pSlot->pCoverButton->Hide();
	}
	if (pSlot->pSlotButton)
	{
		pSlot->pSlotButton->Hide();
	}
	if (pSlot->pSignImage)
	{
 		pSlot->pSignImage->Hide();
	}
	if (pSlot->pFinishCoolTimeEffect)
	{
		pSlot->pFinishCoolTimeEffect->Hide();
	}
}

void CSlotWindow::ClearAllSlot()
{
	Destroy();
}

void CSlotWindow::RefreshSlot()
{
	OnRefreshSlot();

	if (IsRendering())
	{
		TSlot * pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			OnOverOutItem();
			OnOverInItem(pSlot->dwSlotNumber);
		}
	}
}

void CSlotWindow::OnRefreshSlot()
{
}

DWORD CSlotWindow::GetSlotCount()
{
	return m_SlotList.size();
}

void CSlotWindow::LockSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_LOCK;
}
void CSlotWindow::UnlockSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_LOCK;
}
void CSlotWindow::SetCantUseSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState |= SLOT_STATE_CANT_USE;
}
void CSlotWindow::SetUseSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	pSlot->dwState ^= SLOT_STATE_CANT_USE;
}
void CSlotWindow::EnableSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState |= SLOT_STATE_DISABLE;
}
void CSlotWindow::DisableSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;
	SET_BIT(pSlot->dwState, SLOT_STATE_DISABLE);
	//pSlot->dwState ^= SLOT_STATE_DISABLE;
}

#ifdef ENABLE_TRADE_DELETE_SLOT_EFFECT
void CSlotWindow::SetSlotHighlightedGreeen(DWORD dwIndex, DWORD dwColor)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwColor == 0)
		SET_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_GREEN);
	else
		SET_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_RED);
}

void CSlotWindow::DisableSlotHighlightedGreen(DWORD dwIndex, DWORD dwColor)
{
	TSlot* pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	if (dwColor == 0)
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_GREEN);
	else
		REMOVE_BIT(pSlot->dwState, SLOT_STATE_HIGHLIGHT_RED);
}
#endif
// Select

void CSlotWindow::SelectSlot(DWORD dwSelectingIndex)
{
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end();)
	{
		if (dwSelectingIndex == *itor)
		{
			itor = m_dwSelectedSlotIndexList.erase(itor);
			return;
		}
		else
		{
			++itor;
		}
	}

	TSlot * pSlot;
	if (GetSlotPointer(dwSelectingIndex, &pSlot))
	{
		if (!pSlot->isItem)
			return;

		m_dwSelectedSlotIndexList.push_back(dwSelectingIndex);
	}
}

BOOL CSlotWindow::isSelectedSlot(DWORD dwIndex)
{
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == *itor)
			return TRUE;
	}

	return FALSE;
}

void CSlotWindow::ClearSelected()
{
	m_dwSelectedSlotIndexList.clear();
}

DWORD CSlotWindow::GetSelectedSlotCount()
{
	return m_dwSelectedSlotIndexList.size();
}

DWORD CSlotWindow::GetSelectedSlotNumber(DWORD dwIndex)
{
	if (dwIndex >= m_dwSelectedSlotIndexList.size())
		return DWORD(-1);

	DWORD dwCount = 0;
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		if (dwIndex == dwCount)
			break;

		++dwCount;
	}

	return *itor;
}

void CSlotWindow::ShowSlotButton(DWORD dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (pSlot->pSlotButton)
	{
		pSlot->pSlotButton->Show();
	}
}

void CSlotWindow::HideAllSlotButton()
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		if (rSlot.pSlotButton)
		{
			rSlot.pSlotButton->Hide();
		}
	}
}

void CSlotWindow::SetSlotBound(DWORD dwSlotNumber, BOOL bBound)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (bBound)
		pSlot->dwState |= SLOT_STATE_BOUND;
	else
		pSlot->dwState &= ~SLOT_STATE_BOUND;
}

void CSlotWindow::ShowRequirementSign(DWORD dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Show();
}

void CSlotWindow::HideRequirementSign(DWORD dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return;

	if (!pSlot->pSignImage)
		return;

	pSlot->pSignImage->Hide();
}

// Event

BOOL CSlotWindow::OnMouseLeftButtonDown()
{
	TSlot * pSlot;
	if (!GetPickedSlotPointer(&pSlot))
	{
		UI::CWindowManager::Instance().DeattachIcon();
		return TRUE;
	}

	#ifdef ENABLE_TRADABLE_ICON
	if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
	{
		if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
		{
			OnSelectItemSlot(pSlot->dwSlotNumber);
		}
		else
		{
			OnSelectEmptySlot(pSlot->dwSlotNumber);
		}
	}
	#else
	if (pSlot->isItem && !(pSlot->dwState & SLOT_STATE_LOCK))
	{
		OnSelectItemSlot(pSlot->dwSlotNumber);
	}
	else
	{
		OnSelectEmptySlot(pSlot->dwSlotNumber);
	}
	#endif

	return TRUE;
}

BOOL CSlotWindow::OnMouseLeftButtonUp()
{
#ifdef CYNEX_MOBILE
	// AKRAVON: dokunus ile esya elde kalmasin - esya yalniz basili tutup surukleyerek tasinir, birakinca yerine oturur
	if (UI::CWindowManager::Instance().IsAttaching() && !UI::CWindowManager::Instance().IsDragging())
	{
		UI::CWindowManager::Instance().DeattachIcon();
		return TRUE;
	}
#endif
	if (UI::CWindowManager::Instance().IsAttaching())
	if (UI::CWindowManager::Instance().IsDragging())
	if (IsIn())
	{
		UI::CWindow * pWin = UI::CWindowManager::Instance().GetPointWindow();

		if (pWin)
		if (pWin->IsType(UI::CSlotWindow::Type()))
		{
			UI::CSlotWindow * pSlotWin = (UI::CSlotWindow *)pWin;

			TSlot * pSlot;
			if (!pSlotWin->GetPickedSlotPointer(&pSlot))
			{
				UI::CWindowManager::Instance().DeattachIcon();
				return TRUE;
			}

			if (pSlot->isItem)
				pSlotWin->OnSelectItemSlot(pSlot->dwSlotNumber);
			else
				pSlotWin->OnSelectEmptySlot(pSlot->dwSlotNumber);

			return TRUE;
		}

		return FALSE;
	}

	return FALSE;
}

BOOL CSlotWindow::OnMouseRightButtonDown()
{
	TSlot * pSlot;
	if (!GetPickedSlotPointer(&pSlot))
		return TRUE;

	#ifdef ENABLE_TRADABLE_ICON
	if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
	{
		if (pSlot->isItem)
		{
			OnUnselectItemSlot(pSlot->dwSlotNumber);
		}
		else
		{
			OnUnselectEmptySlot(pSlot->dwSlotNumber);
		}
	}
	#else
	if (pSlot->isItem)
	{
		OnUnselectItemSlot(pSlot->dwSlotNumber);
	}
	else
	{
		OnUnselectEmptySlot(pSlot->dwSlotNumber);
	}
	#endif

	return TRUE;
}

BOOL CSlotWindow::OnMouseLeftButtonDoubleClick()
{
	#ifdef ENABLE_TRADABLE_ICON
	TSlot * pSlot;
	if (!GetPickedSlotPointer(&pSlot))
		return TRUE;

	if (!(pSlot->dwState & SLOT_STATE_CANT_MOUSE_EVENT))
		OnUseSlot();
	#else
	OnUseSlot();
	#endif

	return TRUE;
}

void CSlotWindow::OnMouseOverOut()
{
	OnOverOutItem();
}

void CSlotWindow::OnMouseOver()
{
//	if (UI::CWindowManager::Instance().IsCapture())
//	if (!UI::CWindowManager::Instance().IsAttaching())
//		return;

	CWindow * pPointWindow = UI::CWindowManager::Instance().GetPointWindow();
	if (this == pPointWindow)
	{
		TSlot * pSlot;
		if (GetPickedSlotPointer(&pSlot))
		{
			if (OnOverInItem(pSlot->dwSlotNumber))
				return;
		}
	}

	OnOverOutItem();
}

void CSlotWindow::OnSelectEmptySlot(int iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnSelectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnSelectItemSlot(int iSlotNumber)
{
//	OnOverOutItem();
	PyCallClassMemberFunc(m_poHandler, "OnSelectItemSlot", Py_BuildValue("(i)", iSlotNumber));

	if (UI::CWindowManager::Instance().IsAttaching())
		OnOverOutItem();
}
void CSlotWindow::OnUnselectEmptySlot(int iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectEmptySlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUnselectItemSlot(int iSlotNumber)
{
	PyCallClassMemberFunc(m_poHandler, "OnUnselectItemSlot", Py_BuildValue("(i)", iSlotNumber));
}
void CSlotWindow::OnUseSlot()
{
	TSlot * pSlot;
	if (GetPickedSlotPointer(&pSlot))
	if (pSlot->isItem)
	{
		PyCallClassMemberFunc(m_poHandler, "OnUseSlot", Py_BuildValue("(i)", pSlot->dwSlotNumber));
	}
}

BOOL CSlotWindow::OnOverInItem(DWORD dwSlotNumber)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwSlotNumber, &pSlot))
		return FALSE;

	if (!pSlot->isItem)
		return FALSE;

	if (pSlot->dwSlotNumber == m_dwToolTipSlotNumber)
		return TRUE;

	m_dwToolTipSlotNumber = dwSlotNumber;
	PyCallClassMemberFunc(m_poHandler, "OnOverInItem", Py_BuildValue("(i)", dwSlotNumber));

	return TRUE;
}

void CSlotWindow::OnOverOutItem()
{
	if (SLOT_NUMBER_NONE == m_dwToolTipSlotNumber)
		return;

	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;
	PyCallClassMemberFunc(m_poHandler, "OnOverOutItem", Py_BuildValue("()"));
}

void CSlotWindow::OnPressedSlotButton(DWORD dwType, DWORD dwSlotNumber, BOOL isLeft)
{
	if (CSlotButton::SLOT_BUTTON_TYPE_PLUS == dwType)
	{
		PyCallClassMemberFunc(m_poHandler, "OnPressedSlotButton", Py_BuildValue("(i)", dwSlotNumber));
	}
	else if (CSlotButton::SLOT_BUTTON_TYPE_COVER == dwType)
	{
		if (isLeft)
		{
			OnMouseLeftButtonDown();
		}
	}
}

void CSlotWindow::OnUpdate()
{
	for (std::deque<DWORD>::iterator itor = m_ReserveDestroyEffectDeque.begin(); itor != m_ReserveDestroyEffectDeque.end(); ++itor)
	{
		DWORD dwSlotIndex = *itor;
		TSlot * pSlot;
		if (!GetSlotPointer(dwSlotIndex, &pSlot))
			continue;
		__DestroyFinishCoolTimeEffect(pSlot);
	}
	m_ReserveDestroyEffectDeque.clear();

	if (m_pSlotActiveEffect)
		m_pSlotActiveEffect->Update();
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	if (m_pSlotActiveEffectSlot2)
		m_pSlotActiveEffectSlot2->Update();
	if (m_pSlotActiveEffectSlot3)
		m_pSlotActiveEffectSlot3->Update();
#endif
}

void CSlotWindow::OnRender()
{
	RenderSlotBaseImage();

	switch (m_dwSlotStyle)
	{
		case SLOT_STYLE_PICK_UP:
			OnRenderPickingSlot();
			break;
		case SLOT_STYLE_SELECT:
			OnRenderSelectedSlot();
			break;
	}

	std::for_each(m_pChildList.begin(), m_pChildList.end(), std::void_mem_fun(&CWindow::OnRender));

	TSlotListIterator itor;

	//
	//////////////////////////////////////////////////////////////////////////
#ifdef __RENDER_SLOT_AREA__
	CPythonGraphic::Instance().SetDiffuseColor(0.5f, 0.5f, 0.5f);
	for (itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;
		CPythonGraphic::Instance().RenderBox2d(m_rect.left + rSlot.ixPosition,
											   m_rect.top + rSlot.iyPosition,
											   m_rect.left + rSlot.ixPosition + rSlot.ixCellSize,
											   m_rect.top + rSlot.iyPosition + rSlot.iyCellSize);
	}
	CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
#endif
	//////////////////////////////////////////////////////////////////////////

	for (itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		if (rSlot.pSlotButton)
		{
			rSlot.pSlotButton->SetPosition(rSlot.ixPosition + 1, rSlot.iyPosition + 19);
		}
		if (rSlot.pSignImage)
		{
			rSlot.pSignImage->SetPosition(rSlot.ixPosition - 7, rSlot.iyPosition + 10);
		}

		if (rSlot.pInstance && rSlot.isItem && !CynexSlot_IsRound(this))
		{
			// AKRAVON: dolu slot hafif icerde - cok hucreli esyalarda ara cizgi gorunmez, tek parca derinlik
			float fw = 0.0f, fh = 0.0f;
			if (CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &fw, &fh))
			{
				if (fw < float(rSlot.ixCellSize)) fw = float(rSlot.ixCellSize);
				if (fh < float(rSlot.iyCellSize)) fh = float(rSlot.iyCellSize);
				const float x0 = float(m_rect.left + rSlot.ixPosition) + 2.0f, y0 = float(m_rect.top + rSlot.iyPosition) + 2.0f;
				const float x1 = float(m_rect.left + rSlot.ixPosition) + fw - 2.0f, y1 = float(m_rect.top + rSlot.iyPosition) + fh - 2.0f;
				// tek parca, hafif icerde yuva: serin lacivert taban, ust/sol ic golge, alt/sag ince isik
				CPythonGraphic::Instance().SetDiffuseColor(0.085f, 0.11f, 0.17f, 0.97f);
				CynexFillRound(x0, y0, x1, y1, 6.0f);
				CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.55f);
				CPythonGraphic::Instance().RenderBar2d(x0 + 5.0f, y0, x1 - 5.0f, y0 + 1.0f);
				CPythonGraphic::Instance().RenderBar2d(x0 + 3.0f, y0 + 1.0f, x1 - 3.0f, y0 + 2.0f);
				CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.32f);
				CPythonGraphic::Instance().RenderBar2d(x0 + 3.0f, y0 + 2.0f, x1 - 3.0f, y0 + 4.0f);
				CPythonGraphic::Instance().RenderBar2d(x0, y0 + 4.0f, x0 + 3.0f, y1 - 4.0f);
				CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.16f);
				CPythonGraphic::Instance().RenderBar2d(x0 + 3.0f, y0 + 4.0f, x1 - 3.0f, y0 + 6.0f);
				CPythonGraphic::Instance().RenderBar2d(x0 + 3.0f, y0 + 6.0f, x0 + 5.0f, y1 - 4.0f);
				CPythonGraphic::Instance().SetDiffuseColor(0.65f, 0.78f, 1.0f, 0.10f);
				CPythonGraphic::Instance().RenderBar2d(x0 + 5.0f, y1 - 1.0f, x1 - 5.0f, y1);
				CPythonGraphic::Instance().RenderBar2d(x1 - 1.0f, y0 + 5.0f, x1, y1 - 5.0f);
			}
		}

		if (rSlot.pInstance)
		{
			rSlot.pInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
#ifdef CYNEX_MOBILE
			CynexSlot_BeforeIcon(this, rSlot.dwSlotNumber, rSlot.pInstance, float(m_rect.left + rSlot.ixPosition), float(m_rect.top + rSlot.iyPosition));
#endif
			rSlot.pInstance->Render();
		}

		if (rSlot.pInstance && rSlot.isItem && IS_SET(rSlot.dwState, SLOT_STATE_BOUND))
		{
			// AKRAVON: bagli esya rozeti - slotun sol ust kosesinde, hucre olcusune gore olceklenen B ikonu (ymir work/ui/akravon/bagli_rozet.png, 64x64)
			static CGraphicImageInstance * s_pBoundBadge = NULL;
			if (!s_pBoundBadge)
			{
				CGraphicImage * pBadgeImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer("ymir work/ui/akravon/bagli_rozet.png");
				s_pBoundBadge = CGraphicImageInstance::New();
				s_pBoundBadge->SetImagePointer(pBadgeImage);
			}
			float fw = float(rSlot.ixCellSize), fh = float(rSlot.iyCellSize);
			CynexSlot_CellSize(this, rSlot.dwSlotNumber, 1, 1, &fw, &fh);
			const float cell = (fw < fh) ? fw : fh;
			float s = cell * 0.26f;
			if (s < 10.0f) s = 10.0f;
			s_pBoundBadge->SetScale(s / 64.0f, s / 64.0f);
			s_pBoundBadge->SetPosition(float(m_rect.left + rSlot.ixPosition) + 5.0f, float(m_rect.top + rSlot.iyPosition) + 5.0f);
			s_pBoundBadge->Render();
		}

		if (!rSlot.isItem)
		{
			if (IS_SET(rSlot.dwState, SLOT_STATE_ALWAYS_RENDER_COVER))
			{
				rSlot.pCoverButton->Show();
				rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
				rSlot.pCoverButton->RenderButton();
			}

			continue;
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_DISABLE))
		{
			{
				// AKRAVON: kullanilamaz alan - olcekli hucre, yuvarlak kenar
				int px = rSlot.isItem ? rSlot.byxPlacedItemSize : 1, py = rSlot.isItem ? rSlot.byyPlacedItemSize : 1;
				float fw = float(rSlot.ixCellSize * px), fh = float(rSlot.iyCellSize * py);
				float sw = 0.0f, sh = 0.0f;
				if (CynexSlot_CellSize(this, rSlot.dwSlotNumber, px, py, &sw, &sh))
				{
					if (sw > fw) fw = sw;
					if (sh > fh) fh = sh;
				}
				const float x0 = float(m_rect.left + rSlot.ixPosition) + 2.0f, y0 = float(m_rect.top + rSlot.iyPosition) + 2.0f;
				CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.1f, 0.1f, 0.30f);
				CynexFillRound(x0, y0, x0 + fw - 4.0f, y0 + fh - 4.0f, 6.0f);
			}
		}


		#ifdef ENABLE_TRADABLE_ICON
		if (IS_SET(rSlot.dwState, SLOT_STATE_CANT_MOUSE_EVENT))
		{
			{
				float fw = float(rSlot.byxPlacedItemSize * ITEM_WIDTH), fh = float(rSlot.byyPlacedItemSize * ITEM_HEIGHT);
				CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &fw, &fh);
				if (fw < float(rSlot.ixCellSize)) fw = float(rSlot.ixCellSize);
				if (fh < float(rSlot.iyCellSize)) fh = float(rSlot.iyCellSize);
				const float x0 = float(m_rect.left + rSlot.ixPosition) + 2.0f, y0 = float(m_rect.top + rSlot.iyPosition) + 2.0f;
				CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.1f, 0.1f, 0.30f);
				CynexFillRound(x0, y0, x0 + fw - 4.0f, y0 + fh - 4.0f, 6.0f);
			}
		}

		if (IS_SET(rSlot.dwState, SLOT_STATE_UNUSABLE))
		{
			// AKRAVON: kullanilamaz/surukleniyor durumu - olcekli, yuvarlak kenarli soluk kaplama (eski beyaz 32 px kutu yerine)
			float fw = float(rSlot.byxPlacedItemSize * ITEM_WIDTH), fh = float(rSlot.byyPlacedItemSize * ITEM_HEIGHT);
			CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &fw, &fh);
			if (fw < float(rSlot.ixCellSize)) fw = float(rSlot.ixCellSize);
			if (fh < float(rSlot.iyCellSize)) fh = float(rSlot.iyCellSize);
			const float x0 = float(m_rect.left + rSlot.ixPosition) + 2.0f, y0 = float(m_rect.top + rSlot.iyPosition) + 2.0f;
			CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.45f);
			CynexFillRound(x0, y0, x0 + fw - 4.0f, y0 + fh - 4.0f, 6.0f);
		}
		#endif

		#ifdef ENABLE_TRADE_DELETE_SLOT_EFFECT
		if (IS_SET(rSlot.dwState, SLOT_STATE_HIGHLIGHT_GREEN) || IS_SET(rSlot.dwState, SLOT_STATE_HIGHLIGHT_RED))
		{
			// AKRAVON: yesil / kirmizi vurgu - bizim hucre olcusu, yuvarlak kose, ince cerceve (eski ham kare yerine)
			const bool greenState = IS_SET(rSlot.dwState, SLOT_STATE_HIGHLIGHT_GREEN);
			float fw = float(rSlot.byxPlacedItemSize * ITEM_WIDTH), fh = float(rSlot.byyPlacedItemSize * ITEM_HEIGHT);
			CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &fw, &fh);
			if (fw < float(rSlot.ixCellSize)) fw = float(rSlot.ixCellSize);
			if (fh < float(rSlot.iyCellSize)) fh = float(rSlot.iyCellSize);
			const float hx = float(m_rect.left + rSlot.ixPosition) + 1.0f, hy = float(m_rect.top + rSlot.iyPosition) + 1.0f;
			if (greenState)
				CPythonGraphic::Instance().SetDiffuseColor(0.29f, 1.0f, 0.29f, 0.12f);
			else
				CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.1f, 0.1f, 0.12f);
			CynexFillRound(hx, hy, hx + fw - 2.0f, hy + fh - 2.0f, 6.0f);
			if (greenState)
				CPythonGraphic::Instance().SetDiffuseColor(0.35f, 1.0f, 0.35f, 0.75f);
			else
				CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.25f, 0.25f, 0.75f);
			CynexFrameRound(hx, hy, hx + fw - 2.0f, hy + fh - 2.0f, 6.0f, 1.0f);
		}
		#endif

		if (rSlot.fCoolTime != 0.0f)
		{
			float fcurTime = CTimer::Instance().GetCurrentSecond();
			float fPercentage = (fcurTime - rSlot.fStartCoolTime) / rSlot.fCoolTime;
#ifdef CYNEX_MOBILE
			const float fCynexHalf = CynexSlot_CoolHalf(this, rSlot.dwSlotNumber);
			CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + rSlot.ixPosition + fCynexHalf, m_rect.top + rSlot.iyPosition + fCynexHalf, fCynexHalf, fPercentage);
#else
			CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + rSlot.ixPosition + 16.0f, m_rect.top + rSlot.iyPosition + 16.0f, 16.0f, fPercentage);
#endif

			if (fcurTime - rSlot.fStartCoolTime >= rSlot.fCoolTime)
			{
				if ((fcurTime - rSlot.fStartCoolTime) - rSlot.fCoolTime < 1.0f)
					__CreateFinishCoolTimeEffect(&rSlot);

				rSlot.fCoolTime = 0.0f;
				rSlot.fStartCoolTime = 0.0f;
			}
		}

		if (rSlot.pCoverButton)
		{
			rSlot.pCoverButton->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pCoverButton->RenderButton();
		}

		float fCntW = 0.0f, fCntH = 0.0f;
		if (rSlot.pNumberLine && CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &fCntW, &fCntH))
		{
			// AKRAVON: esya adedi - alt sag kosede okunakli yazi tipiyle
			static CGraphicTextInstance* s_pCount = NULL;
			if (!s_pCount)
			{
				s_pCount = new CGraphicTextInstance;
				s_pCount->SetTextPointer(static_cast<CGraphicText*>(CResourceManager::Instance().GetResourcePointer("NotoSansSb:17.fnt")));
				s_pCount->SetOutline(true);
			}
			s_pCount->SetValue(rSlot.pNumberLine->GetNumber().c_str());
			s_pCount->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			s_pCount->Update();
			int tw = 0, th = 0;
			s_pCount->GetTextSize(&tw, &th);
			s_pCount->SetPosition(float(m_rect.left + rSlot.ixPosition) + fCntW - float(tw) - 5.0f, float(m_rect.top + rSlot.iyPosition) + fCntH - float(th) - 3.0f);
			s_pCount->Render();
		}
		else if (rSlot.pNumberLine)
		{
			int ix = rSlot.byxPlacedItemSize*ITEM_WIDTH + rSlot.ixPosition - 4;
			int iy = rSlot.iyPosition + rSlot.byyPlacedItemSize*ITEM_HEIGHT - 12 + 2;
			rSlot.pNumberLine->SetPosition(ix, iy);
			rSlot.pNumberLine->Update();
			rSlot.pNumberLine->Render();
		}

		if (rSlot.pFinishCoolTimeEffect)
		{
			rSlot.pFinishCoolTimeEffect->SetPosition(rSlot.ixPosition, rSlot.iyPosition);
			rSlot.pFinishCoolTimeEffect->Update();
			rSlot.pFinishCoolTimeEffect->Render();
		}

		if (rSlot.bActive)
		{
			{
				// AKRAVON: kullanimda gostergesi - dalgalanan ic parilti + cevrede tam donen iz
				float fw = float(rSlot.ixCellSize), fh = float(rSlot.iyCellSize);
				CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &fw, &fh);
				const bool round = CynexSlot_IsRound(this);
				const float tm = float(CTimer::Instance().GetCurrentSecond());
				const float x0 = float(m_rect.left + rSlot.ixPosition) + 1.0f, y0 = float(m_rect.top + rSlot.iyPosition) + 1.0f;
				fw -= 2.0f; fh -= 2.0f;
				const float pulse = 0.5f + 0.5f * sinf(tm * 4.0f);
				if (round)
				{
					const float r = (fw < fh ? fw : fh) * 0.5f * 1.0f - 1.0f;
					const float cx = x0 + fw * 0.5f, cy = y0 + fh * 0.5f;
					// dalgali dolgu: yatay seritler
					for (float yy = -r; yy < r; yy += 2.0f)
					{
						const float hw = sqrtf(r * r - yy * yy);
						const float wave = 0.5f + 0.5f * sinf(tm * 5.0f - yy * 0.25f);
						CPythonGraphic::Instance().SetDiffuseColor(0.45f, 0.72f, 1.0f, 0.08f + 0.22f * wave * (0.6f + 0.4f * pulse));
						CPythonGraphic::Instance().RenderBar2d(cx - hw, cy + yy, cx + hw, cy + yy + 2.0f);
					}
				}
				else if (rSlot.Color.r > 0.9f && rSlot.Color.g < 0.4f && rSlot.Color.b < 0.4f)
				{
					// AKRAVON: toplu satis secimi - ince kirmizi dalga + ince cerceve
					for (float yy = 0.0f; yy < fh; yy += 2.0f)
					{
						const float wave = 0.5f + 0.5f * sinf(tm * 4.0f - yy * 0.22f);
						const float inset = CynexRoundInset(5.0f, int(yy < fh - yy ? yy : fh - yy));
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.18f, 0.18f, 0.06f + 0.10f * wave);
						CPythonGraphic::Instance().RenderBar2d(x0 + inset, y0 + yy, x0 + fw - inset, y0 + yy + 2.0f);
					}
					CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.25f, 0.25f, 0.55f);
					CynexFrameRound(x0, y0, x0 + fw, y0 + fh, 6.0f, 1.0f);
				}
				else if (rSlot.Color.b < 0.5f && rSlot.Color.r > 0.9f)
				{
					// AKRAVON: yeni alinan esya - sari dalga + kenarda gezen yildizlar (hafifletildi)
					for (float yy = 0.0f; yy < fh; yy += 2.0f)
					{
						const float wave = 0.5f + 0.5f * sinf(tm * 4.0f - yy * 0.22f);
						const float inset = CynexRoundInset(5.0f, int(yy < fh - yy ? yy : fh - yy));
						CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.82f, 0.25f, 0.04f + 0.09f * wave);
						CPythonGraphic::Instance().RenderBar2d(x0 + inset, y0 + yy, x0 + fw - inset, y0 + yy + 2.0f);
					}
					CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.85f, 0.3f, 0.30f);
					CynexFrameRound(x0, y0, x0 + fw, y0 + fh, 6.0f, 1.0f);
					const float per = 2.0f * (fw + fh);
					for (int k = 0; k < 4; ++k)
					{
						// 4 isik birbirini kovalar; her birinin arkasinda soluk iz
						for (int t = 0; t < 5; ++t)
						{
							float d = fmodf(tm * 0.45f + float(k) * 0.25f - float(t) * 0.012f + 4.0f, 1.0f) * per;
							float px, py;
							if (d < fw) { px = x0 + d; py = y0; }
							else if (d < fw + fh) { px = x0 + fw; py = y0 + (d - fw); }
							else if (d < 2.0f * fw + fh) { px = x0 + fw - (d - fw - fh); py = y0 + fh; }
							else { px = x0; py = y0 + fh - (d - 2.0f * fw - fh); }
							const float a2 = (1.0f - float(t) / 5.0f) * 0.8f;
							const float sz = 2.0f - float(t) * 0.3f;
							CPythonGraphic::Instance().SetDiffuseColor(1.0f, 0.88f, 0.4f, a2 * a2);
							CPythonGraphic::Instance().RenderBar2d(px - sz, py - sz, px + sz, py + sz);
							if (t == 0)
							{
								CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 0.9f, 1.0f);
								CPythonGraphic::Instance().RenderBar2d(px - 1.0f, py - 1.0f, px + 1.0f, py + 1.0f);
							}
						}
					}
				}
				else
				{
					// dalgali dolgu: tek efekt, hucrenin tamamini kaplar (cerceve / donen iz yok)
					for (float yy = 0.0f; yy < fh; yy += 2.0f)
					{
						const float wave = 0.5f + 0.5f * sinf(tm * 4.0f - yy * 0.22f);
						const float wave2 = 0.5f + 0.5f * sinf(tm * 2.3f + yy * 0.12f + 1.3f);
						const float inset = CynexRoundInset(5.0f, int(yy < fh - yy ? yy : fh - yy));
						CPythonGraphic::Instance().SetDiffuseColor(0.45f, 0.72f, 1.0f, 0.12f + 0.22f * (0.6f * wave + 0.4f * wave2));
						CPythonGraphic::Instance().RenderBar2d(x0 + inset, y0 + yy, x0 + fw - inset, y0 + yy + 2.0f);
					}
				}
			}
			if (false && m_pSlotActiveEffect && rSlot.byyPlacedItemSize==1)
			{
				int ix = m_rect.left + rSlot.ixPosition;
				int iy = m_rect.top + rSlot.iyPosition;
				#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
				m_pSlotActiveEffect->SetDiffuseColor(rSlot.Color);
				#endif
				m_pSlotActiveEffect->SetPosition(ix, iy);
				m_pSlotActiveEffect->Render();
			}
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
			else if (false && m_pSlotActiveEffectSlot2 && rSlot.byyPlacedItemSize==2)
			{
				int ix = m_rect.left + rSlot.ixPosition;
				int iy = m_rect.top + rSlot.iyPosition;
				m_pSlotActiveEffectSlot2->SetDiffuseColor(rSlot.Color);
				m_pSlotActiveEffectSlot2->SetPosition(ix, iy);
				m_pSlotActiveEffectSlot2->Render();
			}
			else if (false && m_pSlotActiveEffectSlot3 && rSlot.byyPlacedItemSize==3)
			{
				int ix = m_rect.left + rSlot.ixPosition;
				int iy = m_rect.top + rSlot.iyPosition;
				m_pSlotActiveEffectSlot3->SetDiffuseColor(rSlot.Color);
				m_pSlotActiveEffectSlot3->SetPosition(ix, iy);
				m_pSlotActiveEffectSlot3->Render();
			}
#endif
		}
	}

	RenderLockedSlot();
}

void CSlotWindow::RenderSlotBaseImage()
{
	if (!m_pBaseImageInstance)
		return;

	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		if (!rSlot.bRenderBaseSlotImage)
			continue;

		m_pBaseImageInstance->SetPosition(m_rect.left + rSlot.ixPosition, m_rect.top + rSlot.iyPosition);
#ifdef CYNEX_MOBILE
		CynexSlot_BeforeBase(this, m_pBaseImageInstance);
#endif
		m_pBaseImageInstance->Render();
	}
}

void CSlotWindow::AkrRenderDropTarget(std::list<TSlot*> & rSlots, int iMode)
{
	// AKRAVON: eski ham kare (32 px hucre, beyaz / kirmizi) yerine bizim hucre olcumuz + yuvarlak kose + ince cerceve
	float left = 1.0e9f, top = 1.0e9f, right = -1.0e9f, bottom = -1.0e9f;
	for (std::list<TSlot*>::iterator itor = rSlots.begin(); itor != rSlots.end(); ++itor)
	{
		TSlot * pSlot = *itor;
		float fw = float(pSlot->ixCellSize * pSlot->byxPlacedItemSize), fh = float(pSlot->iyCellSize * pSlot->byyPlacedItemSize);
		CynexSlot_CellSize(this, pSlot->dwSlotNumber, pSlot->byxPlacedItemSize, pSlot->byyPlacedItemSize, &fw, &fh);
		if (fw < float(pSlot->ixCellSize)) fw = float(pSlot->ixCellSize);
		if (fh < float(pSlot->iyCellSize)) fh = float(pSlot->iyCellSize);
		const float x0 = float(m_rect.left + pSlot->ixPosition), y0 = float(m_rect.top + pSlot->iyPosition);
		if (x0 < left) left = x0;
		if (y0 < top) top = y0;
		if (x0 + fw > right) right = x0 + fw;
		if (y0 + fh > bottom) bottom = y0 + fh;
	}
	if (right <= left || bottom <= top)
		return;
	float r = 1.0f, g = 1.0f, b = 1.0f;
	if (iMode == 1 || iMode == 3) { r = 1.0f; g = 0.2f; b = 0.2f; }
	else if (iMode == 2) { r = 1.0f; g = 0.88f; b = 0.25f; }
	CPythonGraphic::Instance().SetDiffuseColor(r, g, b, 0.12f);
	CynexFillRound(left + 1.0f, top + 1.0f, right - 1.0f, bottom - 1.0f, 6.0f);
	CPythonGraphic::Instance().SetDiffuseColor(r, g, b, 0.75f);
	CynexFrameRound(left + 1.0f, top + 1.0f, right - 1.0f, bottom - 1.0f, 6.0f, 1.0f);
}

void CSlotWindow::OnRenderPickingSlot()
{
	if (!UI::CWindowManager::Instance().IsAttaching())
		return;

	TSlot * pSlot;
	if (!GetSelectedSlotPointer(&pSlot))
		return;

	{
		// AKRAVON: secim hucresi - ince, yuvarlak kenarli mavi cerceve
		float fw = float(pSlot->ixCellSize), fh = float(pSlot->iyCellSize);
		CynexSlot_CellSize(this, pSlot->dwSlotNumber, pSlot->byxPlacedItemSize, pSlot->byyPlacedItemSize, &fw, &fh);
		if (fw < float(pSlot->ixCellSize)) fw = float(pSlot->ixCellSize);
		if (fh < float(pSlot->iyCellSize)) fh = float(pSlot->iyCellSize);
		const float x0 = float(m_rect.left + pSlot->ixPosition) + 1.0f, y0 = float(m_rect.top + pSlot->iyPosition) + 1.0f;
		CPythonGraphic::Instance().SetDiffuseColor(0.45f, 0.72f, 1.0f, 0.10f);
		CynexFillRound(x0, y0, x0 + fw - 2.0f, y0 + fh - 2.0f, 6.0f);
		CPythonGraphic::Instance().SetDiffuseColor(0.55f, 0.80f, 1.0f, 0.85f);
		CynexFrameRound(x0, y0, x0 + fw - 2.0f, y0 + fh - 2.0f, 6.0f, 1.0f);
	}
}

void CSlotWindow::OnRenderSelectedSlot()
{
	std::list<DWORD>::iterator itor = m_dwSelectedSlotIndexList.begin();
	for (; itor != m_dwSelectedSlotIndexList.end(); ++itor)
	{
		TSlot * pSlot;
		if (!GetSlotPointer(*itor, &pSlot))
			continue;

		float fw = float(pSlot->ixCellSize), fh = float(pSlot->iyCellSize);
		CynexSlot_CellSize(this, pSlot->dwSlotNumber, pSlot->byxPlacedItemSize, pSlot->byyPlacedItemSize, &fw, &fh);
		if (fw < float(pSlot->ixCellSize)) fw = float(pSlot->ixCellSize);
		if (fh < float(pSlot->iyCellSize)) fh = float(pSlot->iyCellSize);
		const float x0 = float(m_rect.left + pSlot->ixPosition) + 1.0f, y0 = float(m_rect.top + pSlot->iyPosition) + 1.0f;
		CPythonGraphic::Instance().SetDiffuseColor(0.45f, 0.72f, 1.0f, 0.10f);
		CynexFillRound(x0, y0, x0 + fw - 2.0f, y0 + fh - 2.0f, 6.0f);
		CPythonGraphic::Instance().SetDiffuseColor(0.55f, 0.80f, 1.0f, 0.85f);
		CynexFrameRound(x0, y0, x0 + fw - 2.0f, y0 + fh - 2.0f, 6.0f, 1.0f);
	}
}

void CSlotWindow::RenderLockedSlot()
{
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.45f);
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		if (!rSlot.isItem)
			continue;

		if (rSlot.dwState & SLOT_STATE_LOCK)
		{
			// AKRAVON: kilitli (suruklenen) hucre - olcekli, yuvarlak kenarli koyu kaplama
			float fw = float(rSlot.ixCellSize * rSlot.byxPlacedItemSize), fh = float(rSlot.iyCellSize * rSlot.byyPlacedItemSize);
			float sw = 0.0f, sh = 0.0f;
			if (CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.byxPlacedItemSize, rSlot.byyPlacedItemSize, &sw, &sh))
			{
				if (sw > fw) fw = sw;
				if (sh > fh) fh = sh;
			}
			const float x0 = float(m_rect.left + rSlot.ixPosition) + 2.0f, y0 = float(m_rect.top + rSlot.iyPosition) + 2.0f;
			CynexFillRound(x0, y0, x0 + fw - 4.0f, y0 + fh - 4.0f, 6.0f);
		}
	}
}

// Check Slot

BOOL CSlotWindow::GetSlotPointer(DWORD dwIndex, TSlot ** ppSlot)
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		if (dwIndex == rSlot.dwSlotNumber)
		{
			*ppSlot = &rSlot;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSlotWindow::GetSelectedSlotPointer(TSlot ** ppSlot)
{
	LONG lx, ly;
	GetMouseLocalPosition(lx, ly);

	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		float fcw = float(rSlot.ixCellSize), fch = float(rSlot.iyCellSize);
		CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.isItem ? rSlot.byxPlacedItemSize : 1, rSlot.isItem ? rSlot.byyPlacedItemSize : 1, &fcw, &fch);	// AKRAVON: olcekli hucre
		if (lx >= rSlot.ixPosition)
		if (ly >= rSlot.iyPosition)
		if (lx <= rSlot.ixPosition + fcw)
		if (ly <= rSlot.iyPosition + fch)
		{
			*ppSlot = &rSlot;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CSlotWindow::GetPickedSlotPointer(TSlot ** ppSlot)
{
	LONG lx, ly;
	CWindowManager::Instance().GetMousePosition(lx, ly);

	int ixLocal = lx - m_rect.left;
	int iyLocal = ly - m_rect.top;

	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		int ixCellSize = rSlot.ixCellSize;
		int iyCellSize = rSlot.iyCellSize;

		if (rSlot.isItem)
		{
			ixCellSize = max(rSlot.ixCellSize, int(rSlot.byxPlacedItemSize * ITEM_WIDTH));
			iyCellSize = max(rSlot.iyCellSize, int(rSlot.byyPlacedItemSize * ITEM_HEIGHT));
		}
		{
			// AKRAVON: olcekli hucre (envanter 46 px) - tiklama alani gorunen hucreyle ayni olsun
			float fcw = 0.0f, fch = 0.0f;
			if (CynexSlot_CellSize(this, rSlot.dwSlotNumber, rSlot.isItem ? rSlot.byxPlacedItemSize : 1, rSlot.isItem ? rSlot.byyPlacedItemSize : 1, &fcw, &fch))
			{
				ixCellSize = int(fcw);
				iyCellSize = int(fch);
			}
		}

		if (ixLocal >= rSlot.ixPosition)
		if (iyLocal >= rSlot.iyPosition)
		if (ixLocal <= rSlot.ixPosition + ixCellSize)
		if (iyLocal <= rSlot.iyPosition + iyCellSize)
		{
			*ppSlot = &rSlot;
			return TRUE;
		}
	}

	return FALSE;
}

void CSlotWindow::SetUseMode(BOOL bFlag)
{
	m_isUseMode = bFlag;
}

void CSlotWindow::SetUsableItem(BOOL bFlag)
{
	m_isUsableItem = bFlag;
}

void CSlotWindow::ReserveDestroyCoolTimeFinishEffect(DWORD dwSlotIndex)
{
	m_ReserveDestroyEffectDeque.push_back(dwSlotIndex);
}

DWORD CSlotWindow::Type()
{
	static int s_Type = GetCRC32("CSlotWindow", strlen("CSlotWindow"));
	return s_Type;
}

BOOL CSlotWindow::OnIsType(DWORD dwType)
{
	if (CSlotWindow::Type() == dwType)
		return TRUE;

	return CWindow::OnIsType(dwType);
}

void CSlotWindow::__CreateToggleSlotImage()
{
	__DestroyToggleSlotImage();

	m_pToggleSlotImage = new CImageBox(NULL);
	m_pToggleSlotImage->LoadImage("d:/ymir work/ui/public/slot_toggle.sub");
	m_pToggleSlotImage->Show();
}

void CSlotWindow::__CreateSlotEnableEffect()
{
	__DestroySlotEnableEffect();

	m_pSlotActiveEffect = new CAniImageBox(NULL);
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/00.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/01.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/02.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/03.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/04.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/05.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/06.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/07.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/08.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/09.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/10.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/11.sub");
	m_pSlotActiveEffect->AppendImage("d:/ymir work/ui/public/slotactiveeffect/12.sub");
	m_pSlotActiveEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffect->Show();

#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	m_pSlotActiveEffectSlot2 = new CAniImageBox(NULL);
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/00.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/01.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/02.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/03.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/04.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/05.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/06.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/07.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/08.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/09.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/10.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/11.sub");
	m_pSlotActiveEffectSlot2->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot2/12.sub");
	m_pSlotActiveEffectSlot2->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffectSlot2->Show();

	m_pSlotActiveEffectSlot3 = new CAniImageBox(NULL);
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/00.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/01.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/02.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/03.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/04.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/05.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/06.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/07.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/08.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/09.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/10.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/11.sub");
	m_pSlotActiveEffectSlot3->AppendImage("d:/ymir work/ui/public/slotactiveeffect/slot3/12.sub");
	m_pSlotActiveEffectSlot3->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	m_pSlotActiveEffectSlot3->Show();
#endif
}

void CSlotWindow::__CreateFinishCoolTimeEffect(TSlot * pSlot)
{
	__DestroyFinishCoolTimeEffect(pSlot);

	CAniImageBox * pFinishCoolTimeEffect = new CCoolTimeFinishEffect(this, pSlot->dwSlotNumber);
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/00.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/01.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/02.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/03.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/04.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/05.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/06.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/07.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/08.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/09.sub");
	pFinishCoolTimeEffect->AppendImage("d:/ymir work/ui/public/slotfinishcooltimeeffect/10.sub");
	pFinishCoolTimeEffect->SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	pFinishCoolTimeEffect->ResetFrame();
	pFinishCoolTimeEffect->SetDelay(2);
	pFinishCoolTimeEffect->Show();

	pSlot->pFinishCoolTimeEffect = pFinishCoolTimeEffect;
}

void CSlotWindow::__CreateBaseImage(const char * c_szFileName, float fr, float fg, float fb, float fa)
{
	__DestroyBaseImage();

	CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(c_szFileName);
	m_pBaseImageInstance = CGraphicImageInstance::New();
	m_pBaseImageInstance->SetImagePointer(pImage);
	m_pBaseImageInstance->SetDiffuseColor(fr, fg, fb, fa);
}

void CSlotWindow::__DestroyToggleSlotImage()
{
	if (m_pToggleSlotImage)
	{
		delete m_pToggleSlotImage;
		m_pToggleSlotImage = NULL;
	}
}

void CSlotWindow::__DestroySlotEnableEffect()
{
	if (m_pSlotActiveEffect)
	{
		delete m_pSlotActiveEffect;
		m_pSlotActiveEffect = NULL;
	}
#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	if (m_pSlotActiveEffectSlot2)
	{
		delete m_pSlotActiveEffectSlot2;
		m_pSlotActiveEffectSlot2 = NULL;
	}
	if (m_pSlotActiveEffectSlot3)
	{
		delete m_pSlotActiveEffectSlot3;
		m_pSlotActiveEffectSlot3 = NULL;
	}
#endif
}

void CSlotWindow::__DestroyFinishCoolTimeEffect(TSlot * pSlot)
{
	if (pSlot->pFinishCoolTimeEffect)
	{
		delete pSlot->pFinishCoolTimeEffect;
		pSlot->pFinishCoolTimeEffect = NULL;
	}
}

void CSlotWindow::__DestroyBaseImage()
{
	if (m_pBaseImageInstance)
	{
		CGraphicImageInstance::Delete(m_pBaseImageInstance);
		m_pBaseImageInstance = NULL;
	}
}


#ifdef ENABLE_TRADABLE_ICON
void CSlotWindow::SetCanMouseEventSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
	//pSlot->dwState |= SLOT_STATE_CANT_MOUSE_EVENT;
}

void CSlotWindow::SetCantMouseEventSlot(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_CANT_MOUSE_EVENT);
	//pSlot->dwState ^= SLOT_STATE_CANT_MOUSE_EVENT;
}

void CSlotWindow::SetUsableSlotOnTopWnd(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	REMOVE_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE);
	//pSlot->dwState |= SLOT_STATE_UNUSABLE;
}

void CSlotWindow::SetUnusableSlotOnTopWnd(DWORD dwIndex)
{
	TSlot * pSlot;
	if (!GetSlotPointer(dwIndex, &pSlot))
		return;

	SET_BIT(pSlot->dwState, SLOT_STATE_UNUSABLE);
	//pSlot->dwState ^= SLOT_STATE_UNUSABLE;
}
#endif

void CSlotWindow::__Initialize()
{
	m_dwSlotType = 0;
	m_dwSlotStyle = SLOT_STYLE_PICK_UP;
	m_dwToolTipSlotNumber = SLOT_NUMBER_NONE;

	m_isUseMode = FALSE;
	m_isUsableItem = FALSE;

	m_pToggleSlotImage = NULL;
	m_pSlotActiveEffect = NULL;
	#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	m_pSlotActiveEffectSlot2 = NULL;
	m_pSlotActiveEffectSlot3 = NULL;
	#endif
	m_pBaseImageInstance = NULL;
}

void CSlotWindow::Destroy()
{
	for (TSlotListIterator itor = m_SlotList.begin(); itor != m_SlotList.end(); ++itor)
	{
		TSlot & rSlot = *itor;

		ClearSlot(&rSlot);

		if (rSlot.pNumberLine)
		{
			delete rSlot.pNumberLine;
			rSlot.pNumberLine = NULL;
		}
		if (rSlot.pCoverButton)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pCoverButton);
		}
		if (rSlot.pSlotButton)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pSlotButton);
		}
		if (rSlot.pSignImage)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pSignImage);
		}
		if (rSlot.pFinishCoolTimeEffect)
		{
			CWindowManager::Instance().DestroyWindow(rSlot.pFinishCoolTimeEffect);
		}
	}

	m_SlotList.clear();
#ifdef CYNEX_MOBILE
	CynexSlot_Forget(this);
#endif

	__DestroyToggleSlotImage();
	__DestroySlotEnableEffect();
	__DestroyBaseImage();

	__Initialize();
}

CSlotWindow::CSlotWindow(PyObject * ppyObject) : CWindow(ppyObject)
{
	__Initialize();
}

CSlotWindow::~CSlotWindow()
{
	Destroy();
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
