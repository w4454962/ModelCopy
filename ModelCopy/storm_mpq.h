#pragma once
#include "stdafx.h"
#include "stormlib.h"
#include <string>

class storm_mpq
{
public:
	HANDLE m_hMpq;
	std::string m_strPath;

	storm_mpq();
	storm_mpq(const char* szPath);

	~storm_mpq();

	int Open(const char* szPath);
	int Close();

	int HasFile(const char* szFileName);
	int LoadFile(const char* szFileName, const char** pszBuffer, DWORD* pdwSize);
	int UnLoadFile(const char* szFileData);

	int AddFile(const char* szFilePath);
	int AddFile(const char* szFilePath, const char* szArchivePath);
	int RemoveFile(const char* szFileName);
	int ExportFile(const fs::path& path, const fs::path& outpath, std::vector<fs::path>* find_texture = nullptr);
};
