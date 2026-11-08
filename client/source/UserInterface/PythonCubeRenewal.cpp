#include "StdAfx.h"
#include "PythonCubeRenewal.h"
#include "PythonNetworkStream.h"
#include "Packet.h"


CPythonCubeRenewal::CPythonCubeRenewal()
	: m_CubeRenewalHandler(NULL)
{
	ClearList();
}

CPythonCubeRenewal::~CPythonCubeRenewal()
{
	ClearList();
}


void CPythonCubeRenewal::LoadingList()
{
	if (m_CubeRenewalHandler)
		PyCallClassMemberFunc(m_CubeRenewalHandler, "BINARY_CUBE_RENEWAL_LOADING", Py_BuildValue("()"));
}

void CPythonCubeRenewal::ClearList()
{
	cube_renewal_list.clear();
}

void CPythonCubeRenewal::ReceiveList(const TInfoDateCubeRenewal & TInfoCR)
{
	cube_renewal_list.push_back(TInfoCR);
}

int CPythonCubeRenewal::GetCountList()
{
	return cube_renewal_list.size();
}

const CPythonCubeRenewal::TInfoStrucCubeRenewal & CPythonCubeRenewal::GetList()
{
	return cube_renewal_list;
}

PyObject * GetCountCubeRenewalList(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonCubeRenewal::Instance().GetCountList());
}

PyObject * GetCubeRenewalDates(PyObject * poSelf, PyObject * poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	char szMaterials[1024] = { '\0' };
	char *tempMaterials = szMaterials;

	const CPythonCubeRenewal::TInfoStrucCubeRenewal & CRItemVector = CPythonCubeRenewal::Instance().GetList();
	if (DWORD(index) >= CRItemVector.size())
	{
		for (int i = 0; i < CUBE_MATERIAL_MAX_NUM; i++)
			tempMaterials += sprintf(tempMaterials, "%u,%d%s", 0, 0, (i < CUBE_MATERIAL_MAX_NUM - 1 ? "|" : ""));
		return Py_BuildValue("iibsKis", 0, 0, false, szMaterials, 0, 0, "None");
	}

	const TInfoDateCubeRenewal & CRenewalItemVector = CRItemVector[index];
	for (int i = 0; i < CUBE_MATERIAL_MAX_NUM; i++)
		tempMaterials += sprintf(tempMaterials, "%u,%d%s", CRenewalItemVector.vnum_material[i], CRenewalItemVector.count_material[i], (i < CUBE_MATERIAL_MAX_NUM - 1 ? "|" : ""));

	return Py_BuildValue("iibsKis",
		CRenewalItemVector.vnum_reward,
		CRenewalItemVector.count_reward,
		CRenewalItemVector.item_reward_stackable,
		szMaterials,
		CRenewalItemVector.gold,
		CRenewalItemVector.percent,
		CRenewalItemVector.category
	);
}

PyObject * SetCubeRenewalHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject * poEventHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poEventHandler))
		return Py_BuildException();

	CPythonCubeRenewal::Instance().SetCubeRenewalHandler(poEventHandler);
	return Py_BuildNone();
}

PyObject * CubeRenewalMakeItem(PyObject* poSelf, PyObject* poArgs)
{
	int index_craft;
	if (!PyTuple_GetInteger(poArgs, 0, &index_craft))
		return Py_BuildException();

	int count_item;
	if (!PyTuple_GetInteger(poArgs, 1, &count_item))
		return Py_BuildException();

	int index_item_improve;
	if (!PyTuple_GetInteger(poArgs, 2, &index_item_improve))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.CubeRenewalMakeItem(index_craft,count_item,index_item_improve);
	return Py_BuildNone();
}

PyObject * CubeRenewalClose(PyObject* poSelf, PyObject* poArgs)
{

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.CubeRenewalClose();
	return Py_BuildNone();

}

void intcuberenewal()
{
	static PyMethodDef s_methods[] =
	{
		{ "CountDates",						GetCountCubeRenewalList,					METH_VARARGS },
		{ "GetDates", 						GetCubeRenewalDates, 						METH_VARARGS },
		{ "SetCubeRenewalHandler",			SetCubeRenewalHandler,						METH_VARARGS },
		{ "CubeRenewalMakeItem",			CubeRenewalMakeItem,						METH_VARARGS },
		{ "CubeRenewalClose",				CubeRenewalClose,							METH_VARARGS},

		{ NULL,								NULL,								NULL },
	};

	PyObject * poModule = Py_InitModule("cube_renewal", s_methods);
}
