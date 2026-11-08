// AKRAVON: haritadan secilen noktaya otomatik yuruyus - arazi engellerinden kacan yol bulucu (A*) ve yerde cizilen ayak izleri.
// Python: background.FindPath(x1, y1, x2, y2) -> ((x, y), ...)  (santimetre, dunya)
//         background.SetFootprints(((x, y, aci_derece, sol0_sag1), ...))   (bos tuple = temizle)
#include "StdAfx.h"
#include "PythonBackground.h"
#include "../GameLib/MapOutdoor.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/ResourceManager.h"
#include "../EterLib/GrpImage.h"
#include "../EterLib/GrpBase.h"
#include "../PRTerrainLib/Terrain.h"
#include <queue>
#include <vector>
#include <math.h>

namespace
{
	const int kCell = 100;		// cm: yol bulma izgarasi (arazi attribute hucresi 50 cm)
	const int kMaxCells = 220;	// en fazla ~220 m yaricap

	struct Node
	{
		int x, y;
		float f;
		bool operator<(const Node& o) const { return f > o.f; }
	};

	struct Footprint
	{
		float x, y, deg;
		int right;
	};
	std::vector<Footprint> g_footprints;
	CGraphicImage* g_footImage[2] = { NULL, NULL };
	bool g_footLoadTried = false;
}

static bool CynexIsBlocked(CMapOutdoor& rkMap, float fx, float fy)
{
	BYTE attr = 0;
	if (!rkMap.GetAttr(fx, fy, &attr))
		return true;	// harita disi / yuklenmemis = gecilmez
	return (attr & (CTerrainImpl::ATTRIBUTE_BLOCK | CTerrainImpl::ATTRIBUTE_WATER)) != 0;
}

// bir izgara hucresi (100 cm) icindeki 4 attribute ornegine bak: biri bloksa gecilmez
static bool CynexCellBlocked(CMapOutdoor& rkMap, int cx, int cy, int originX, int originY)
{
	const float baseX = (float)(originX + cx * kCell);
	const float baseY = (float)(originY + cy * kCell);
	return CynexIsBlocked(rkMap, baseX + 25.0f, baseY + 25.0f) || CynexIsBlocked(rkMap, baseX + 75.0f, baseY + 25.0f) ||
		CynexIsBlocked(rkMap, baseX + 25.0f, baseY + 75.0f) || CynexIsBlocked(rkMap, baseX + 75.0f, baseY + 75.0f);
}

static bool CynexLineClear(CMapOutdoor& rkMap, float x1, float y1, float x2, float y2)
{
	const float dx = x2 - x1, dy = y2 - y1;
	const float len = sqrtf(dx * dx + dy * dy);
	const int steps = (int)(len / 40.0f) + 1;
	for (int i = 1; i <= steps; ++i)
	{
		const float t = (float)i / steps;
		const float px = x1 + dx * t, py = y1 + dy * t;
		if (CynexIsBlocked(rkMap, px, py))
			return false;
		// karakter genisligi: yanlara 30 cm
		const float nx = -dy / (len > 1.0f ? len : 1.0f) * 30.0f, ny = dx / (len > 1.0f ? len : 1.0f) * 30.0f;
		if (CynexIsBlocked(rkMap, px + nx, py + ny) || CynexIsBlocked(rkMap, px - nx, py - ny))
			return false;
	}
	return true;
}

PyObject* backgroundFindPath(PyObject* poSelf, PyObject* poArgs)
{
	float sx, sy, ex, ey;
	if (!PyTuple_GetFloat(poArgs, 0, &sx) || !PyTuple_GetFloat(poArgs, 1, &sy) ||
		!PyTuple_GetFloat(poArgs, 2, &ex) || !PyTuple_GetFloat(poArgs, 3, &ey))
		return Py_BuildException();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	if (!rkBG.IsMapReady())
		return Py_BuildValue("()");
	CMapOutdoor& rkMap = rkBG.GetMapOutdoorRef();

	// yol bulma izgarasi baslangic / hedefin etrafinda
	const float midX = (sx + ex) * 0.5f, midY = (sy + ey) * 0.5f;
	const float span = fmaxf(fabsf(ex - sx), fabsf(ey - sy)) * 0.5f + 3000.0f;	// +30 m dolasma payi
	int half = (int)(span / kCell) + 1;
	if (half > kMaxCells)
		half = kMaxCells;
	const int originX = (int)(midX - half * kCell), originY = (int)(midY - half * kCell);
	const int n = half * 2;
	auto toCell = [&](float v, int origin) { return (int)floorf((v - origin) / kCell); };
	int scx = toCell(sx, originX), scy = toCell(sy, originY);
	int ecx = toCell(ex, originX), ecy = toCell(ey, originY);
	if (scx < 0 || scy < 0 || scx >= n || scy >= n || ecx < 0 || ecy < 0 || ecx >= n || ecy >= n)
		return Py_BuildValue("()");

	// hedef engelin icindeyse en yakin serbest hucreyi ara
	if (CynexCellBlocked(rkMap, ecx, ecy, originX, originY))
	{
		bool found = false;
		for (int r = 1; r <= 8 && !found; ++r)
			for (int dy = -r; dy <= r && !found; ++dy)
				for (int dx = -r; dx <= r && !found; ++dx)
				{
					if (abs(dx) != r && abs(dy) != r)
						continue;
					const int cx = ecx + dx, cy = ecy + dy;
					if (cx < 0 || cy < 0 || cx >= n || cy >= n)
						continue;
					if (!CynexCellBlocked(rkMap, cx, cy, originX, originY))
					{
						ecx = cx; ecy = cy; found = true;
					}
				}
		if (!found)
			return Py_BuildValue("()");
	}

	std::vector<float> gScore((size_t)n * n, 1e30f);
	std::vector<int> parent((size_t)n * n, -1);
	std::vector<unsigned char> state((size_t)n * n, 0);	// 0 bilinmiyor, 1 acik, 2 kapali, 3 engelli
	std::priority_queue<Node> open;

	auto idx = [&](int x, int y) { return y * n + x; };
	auto heuristic = [&](int x, int y) { const float dx = (float)(x - ecx), dy = (float)(y - ecy); return sqrtf(dx * dx + dy * dy); };

	gScore[idx(scx, scy)] = 0.0f;
	Node start = { scx, scy, heuristic(scx, scy) };
	open.push(start);
	state[idx(scx, scy)] = 1;
	int expanded = 0;
	bool reached = false;

	static const int dxs[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };
	static const int dys[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
	while (!open.empty() && expanded < 60000)
	{
		const Node cur = open.top();
		open.pop();
		const int ci = idx(cur.x, cur.y);
		if (state[ci] == 2)
			continue;
		state[ci] = 2;
		++expanded;
		if (cur.x == ecx && cur.y == ecy)
		{
			reached = true;
			break;
		}
		for (int k = 0; k < 8; ++k)
		{
			const int nx = cur.x + dxs[k], ny = cur.y + dys[k];
			if (nx < 0 || ny < 0 || nx >= n || ny >= n)
				continue;
			const int ni = idx(nx, ny);
			if (state[ni] == 2 || state[ni] == 3)
				continue;
			if (state[ni] == 0)
			{
				if (CynexCellBlocked(rkMap, nx, ny, originX, originY) && !(nx == scx && ny == scy))
				{
					state[ni] = 3;
					continue;
				}
			}
			if (k >= 4)
			{
				// capraz: iki yan hucre de serbest olmali (kose kesmesin)
				if (CynexCellBlocked(rkMap, cur.x + dxs[k], cur.y, originX, originY) || CynexCellBlocked(rkMap, cur.x, cur.y + dys[k], originX, originY))
					continue;
			}
			const float step = (k >= 4) ? 1.4142f : 1.0f;
			const float g = gScore[ci] + step;
			if (g < gScore[ni])
			{
				gScore[ni] = g;
				parent[ni] = ci;
				state[ni] = 1;
				Node nn = { nx, ny, g + heuristic(nx, ny) };
				open.push(nn);
			}
		}
	}

	if (!reached)
		return Py_BuildValue("()");

	// yolu geri sar
	std::vector<std::pair<float, float> > pts;
	for (int c = idx(ecx, ecy); c != -1; c = parent[c])
	{
		const int cx = c % n, cy = c / n;
		pts.push_back(std::make_pair((float)(originX + cx * kCell + kCell / 2), (float)(originY + cy * kCell + kCell / 2)));
	}
	std::reverse(pts.begin(), pts.end());
	pts[0] = std::make_pair(sx, sy);
	if (ecx == toCell(ex, originX) && ecy == toCell(ey, originY))
		pts.back() = std::make_pair(ex, ey);

	// duzlestirme: gorus hatti acik oldukca ara noktalari at
	std::vector<std::pair<float, float> > smooth;
	smooth.push_back(pts[0]);
	size_t anchor = 0;
	while (anchor + 1 < pts.size())
	{
		size_t farIdx = anchor + 1;
		for (size_t j = pts.size() - 1; j > anchor + 1; --j)
		{
			if (CynexLineClear(rkMap, pts[anchor].first, pts[anchor].second, pts[j].first, pts[j].second))
			{
				farIdx = j;
				break;
			}
		}
		smooth.push_back(pts[farIdx]);
		anchor = farIdx;
	}

	PyObject* poTuple = PyTuple_New((Py_ssize_t)smooth.size());
	for (size_t i = 0; i < smooth.size(); ++i)
		PyTuple_SetItem(poTuple, (Py_ssize_t)i, Py_BuildValue("(ff)", smooth[i].first, smooth[i].second));
	return poTuple;
}

PyObject* backgroundSetFootprints(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poList = NULL;
	if (!PyArg_ParseTuple(poArgs, "O", &poList))
		return Py_BuildException();
	g_footprints.clear();
	const Py_ssize_t count = PySequence_Size(poList);
	for (Py_ssize_t i = 0; i < count && i < 96; ++i)
	{
		PyObject* poItem = PySequence_GetItem(poList, i);
		Footprint fp = { 0, 0, 0, 0 };
		if (poItem && PyArg_ParseTuple(poItem, "fffi", &fp.x, &fp.y, &fp.deg, &fp.right))
			g_footprints.push_back(fp);
		Py_XDECREF(poItem);
	}
	return Py_BuildNone();
}

void CynexRenderFootprints()
{
	if (g_footprints.empty())
		return;
	CPythonBackground& rkBG = CPythonBackground::Instance();
	if (!rkBG.IsMapReady())
		return;

	if (!g_footLoadTried)
	{
		g_footLoadTried = true;
		static const char* names[2] = { "mobile_ui/akravon/atlas/foot_l.png", "mobile_ui/akravon/atlas/foot_r.png" };
		for (int i = 0; i < 2; ++i)
		{
			g_footImage[i] = static_cast<CGraphicImage*>(CResourceManager::Instance().GetResourcePointer(names[i]));
			if (g_footImage[i])
				g_footImage[i]->AddReference();
		}
	}
	if (!g_footImage[0] || !g_footImage[1])
		return;

	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	STATEMANAGER.SaveTransform(D3DTS_WORLD, &matWorld);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	const float kHalfW = 34.0f, kHalfH = 50.0f;	// cm (gorsel 22x32 oranli)
	for (size_t i = 0; i < g_footprints.size(); ++i)
	{
		const Footprint& fp = g_footprints[i];
		CGraphicImage* pImg = g_footImage[fp.right ? 1 : 0];
		if (!pImg || !pImg->GetTexturePointer())
			continue;
		STATEMANAGER.SetTexture(0, pImg->GetTexturePointer()->GetD3DTexture());

		const float rad = D3DXToRadian(fp.deg);
		const float sn = sinf(rad), cs = cosf(rad);
		// yonu ileri (sx, sy), yan (-sy, sx)
		const float fx = sn, fy = -cs;
		const float rx = cs, ry = sn;
		const float cz = rkBG.GetHeight(fp.x, fp.y) + 6.0f;
		const float fade = (i < 3) ? (0.4f + 0.2f * i) : 1.0f;
		const DWORD color = ((DWORD)(fade * 235.0f) << 24) | 0x00FFFFFF;

		TPDTVertex v[4];
		const float corners[4][2] = { { -kHalfW, -kHalfH }, { kHalfW, -kHalfH }, { -kHalfW, kHalfH }, { kHalfW, kHalfH } };
		const float uvs[4][2] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } };
		for (int k = 0; k < 4; ++k)
		{
			// motor dunyasi: +y kuzey (ekranda yukari), doku ustu = ileri
			const float lx = corners[k][0], ly = -corners[k][1];
			v[k].position.x = fp.x + rx * lx + fx * ly;
			v[k].position.y = -(fp.y + ry * lx + fy * ly);
			v[k].position.z = cz;
			v[k].diffuse = color;
			v[k].texCoord.x = uvs[k][0];
			v[k].texCoord.y = uvs[k][1];
		}
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(TPDTVertex));
	}

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreTransform(D3DTS_WORLD);
}
