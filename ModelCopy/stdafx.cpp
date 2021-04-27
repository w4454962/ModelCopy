#include <locale> 
#include "stdafx.h"
#include <codecvt> 
#include <shlobj.h>

std::string a2u(std::string const& strGb2312)
{
	std::vector<wchar_t> buff(strGb2312.size());
#ifdef _MSC_VER
	std::locale loc("zh-CN");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	wchar_t* pwszNext = nullptr;
	const char* pszNext = nullptr;
	mbstate_t state = {};
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).in(state,
			strGb2312.data(), strGb2312.data() + strGb2312.size(), pszNext,
			buff.data(), buff.data() + buff.size(), pwszNext);

	if (std::codecvt_base::ok == res)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
		return cutf8.to_bytes(std::wstring(buff.data(), pwszNext));
	}

	return "";

}

std::string u2a(std::string const& strUtf8)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
	std::wstring wTemp = cutf8.from_bytes(strUtf8);
#ifdef _MSC_VER
	std::locale loc("zh-CN");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	const wchar_t* pwszNext = nullptr;
	char* pszNext = nullptr;
	mbstate_t state = {};

	std::vector<char> buff(wTemp.size() * 2);
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).out(state,
			wTemp.data(), wTemp.data() + wTemp.size(), pwszNext,
			buff.data(), buff.data() + buff.size(), pszNext);

	if (std::codecvt_base::ok == res)
	{
		return std::string(buff.data(), pszNext);
	}
	return "";
}


bool CopyFilesToClip(std::vector<std::string>& fileList)
{
	//复制文件到剪切板,对于多个文件,没有参数指定文件个数,
	//每个文件名用 0 分隔,所有文件名结束用 两个0表示
	DROPFILES* dropFiles;
	UINT memLen;
	HGLOBAL Gmem;
	char* pMem, * szFileList;

	//记算所需内存大小。
	memLen = sizeof(DROPFILES);

	for (auto& path : fileList)
	{
		memLen += path.length() + 1;
	}
	memLen++; //最后一个0的位置。

	Gmem = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, memLen);
	pMem = (char*)GlobalLock(Gmem);
	if (pMem == NULL)
	{
		GlobalFree(Gmem);
		return false;
	}

	dropFiles = (DROPFILES*)pMem;
	szFileList = pMem + sizeof(DROPFILES);

	dropFiles->pFiles = sizeof(DROPFILES);
	dropFiles->pt.x = 0;
	dropFiles->pt.y = 0;
	dropFiles->fNC = false;
	dropFiles->fWide = false;

	int nameLen = 0;

	for (auto& path : fileList)
	{
		nameLen += sprintf(szFileList + nameLen, path.c_str());
		nameLen++;//字符结尾0的位置。
	}

	szFileList[nameLen] = 0;//最后一个0。

	GlobalUnlock(Gmem);

	if (!OpenClipboard(NULL))
	{
		GlobalFree(Gmem);
		return false;
	}
	
	bool retValue = true;

	EmptyClipboard();

	if (NULL == SetClipboardData(CF_HDROP, Gmem))
		retValue = false;

	CloseClipboard();

	return retValue;
}