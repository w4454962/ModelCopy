#include "stormlib.h"
#include "storm_mpq.h"
#include <string>
#include <regex>
 

storm_mpq::storm_mpq()
{
	m_hMpq = NULL;
	m_strPath = "";
}
storm_mpq::storm_mpq(const char* szPath)
{
	if (!SFileOpenArchive(szPath, 0, 0, &m_hMpq))
	{
		printf((std::string(szPath) + "-打开mpq失败").c_str());
	}
	m_strPath = szPath;
}
storm_mpq::~storm_mpq()
{
	if (!m_hMpq)
	{
		return;
	}
	if (!SFileCloseArchive(m_hMpq))
	{
		printf((m_strPath + "-mpq关闭失败").c_str());
	}
}

int storm_mpq::Open(const char* szPath)
{
	if (m_hMpq != NULL)
	{
		if (!Close())
		{
			return 0;
		}
	}
	if (!SFileOpenArchive(szPath, 0, 0x100, &m_hMpq))
	{
		printf((std::string(szPath) + "-打开mpq失败").c_str());
		return 0;
	}
	m_strPath = szPath;
	return 1;
}
int storm_mpq::Close()
{
	if (!m_hMpq)
	{
		printf((std::string(m_strPath) + "-关闭mpq失败-mpq无效").c_str());
		return 0;
	}
	if (!SFileCloseArchive(m_hMpq))
	{
		printf((m_strPath + "-mpq关闭失败").c_str());
		return 0;
	}
	m_hMpq = NULL;
	return 1;
}

int storm_mpq::HasFile(const char* szFileName)
{
	if (!m_hMpq)
	{
		printf("文件不存在-mpq无效");
		return 0;
	}
	return SFileHasFile(m_hMpq, szFileName);
}

int storm_mpq::LoadFile(const char* szFileName, const char** pszBuffer, DWORD* pdwSize)
{
	HANDLE hFile;
	if (!m_hMpq)
	{
		printf("读取文件失败-mpq无效");
		return 0;
	}
	if (!SFileHasFile(m_hMpq, szFileName))
	{
		//printf((std::string(szFileName) + "不存在").c_str());
		return 0;
	}
	if (!SFileOpenFileEx(m_hMpq, szFileName, 0, &hFile))
	{
		//printf(("打开文件失败-" + std::string(szFileName)).c_str());
		return 0;
	}
	DWORD dwSize = SFileGetFileSize(hFile, 0);
	if (!dwSize)
	{
		//printf("文件大小为0");
		return 0;
	}
	char* szFileData = new char[dwSize + 1];
	DWORD dwRead;
	if (!SFileReadFile(hFile, szFileData, dwSize, &dwRead, NULL))
	{
		//printf(("读取文件-" + std::string(szFileName) + "内存失败").c_str());
		return 0;
	}
	if (!SFileCloseFile(hFile))
	{
		//printf(("关闭文件-" + std::string(szFileName) + "失败").c_str());
	}
	szFileData[dwSize] = 0;
	*pdwSize = dwSize;
	*pszBuffer = szFileData;
	return 1;
}

int storm_mpq::UnLoadFile(const char* szFileData)
{
	if (NULL == szFileData)
		return 0;
	if (NULL == *szFileData)
		return 0;
	delete[] szFileData;
	return 1;
}
int storm_mpq::AddFile(const char* szFilePath)
{
	return AddFile(szFilePath, szFilePath);
}
int storm_mpq::AddFile(const char* szFilePath, const char* szArchivePath)
{
	if (!m_hMpq)
	{
		printf("添加文件失败-mpq无效");
		return 0;
	}
	if (SFileHasFile(m_hMpq, szArchivePath)) //判断路径已存在文件
	{
		if (!RemoveFile(szArchivePath)) //删除已存在的文件
		{
			return 0;
		}
	}
	if (!SFileAddFile(m_hMpq, szFilePath, szArchivePath, 0x00000200 | 0x80000000))
	{
		printf((std::string(szArchivePath) + "添加文件失败").c_str());
		return 0;
	}
	return 1;
} 
int storm_mpq::RemoveFile(const char* szFileName)
{
	if (!m_hMpq)
	{
		printf("删除文件失败-mpq无效");
		return 0;
	}
	if (!SFileRemoveFile(m_hMpq, szFileName, 0))
	{
		printf(("删除文件-" + std::string(szFileName) + "失败").c_str());
		return 0;
	}
	return 1;
}



int storm_mpq::ExportFile(const fs::path& path, const fs::path& outpath, std::vector<fs::path>* find_texture)
{
	const char* buffer = nullptr;
	size_t size = 0;

	if (!LoadFile(path.string().c_str(), &buffer, (DWORD*)&size))
		return 0;

	fs::create_directories(outpath.parent_path());

	FILE* file = fopen(outpath.string().c_str(), "wb");

	if (!file)
		return 0;

	fwrite(buffer, 1, size, file);
	fclose(file);

	if (find_texture) //如果是模型 要在里面搜索贴图路径
	{
		std::string str = std::string(buffer, size);
		std::regex reg("([\\w _~!@#$%^&()\\{\\}\\[\\]';,\\+\\-\\\\]+\\.[bBtT][lLgG][pPaA])");
	
		auto words_begin = std::sregex_iterator(str.begin(), str.end(), reg);
		auto words_end = std::sregex_iterator();
		//正则表达式匹配 全局jass中 符合初始化触发器名字的函数
		for (; words_begin != words_end; ++words_begin)
		{
			find_texture->push_back(words_begin->str(1));
		}
	}

	UnLoadFile(buffer);

	return 1;
}