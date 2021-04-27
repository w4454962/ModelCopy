#pragma once
#include "stdafx.h"
#include "EditorData.h"
#include <string>




class Helper
{
public:

	HWND m_hwnd;


	void attach();

	uintptr_t getAddress(uintptr_t addr);
	EditorData* getEditorData();
	const char* getCurrentMapPath();

	void enableConsole();

	//��ȡ��λ���� ��λid ����ֶ� ����ֵ
	std::string getUnitData(const char* unitId, const char* field);
	fs::path getUnitModelPath(const char* unitId);
	fs::path getDoodasModelPath(const char* doodaId);


private:

	WNDPROC m_oldWinProc;

	void copyUpdate();

	static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

Helper* getHelper();
