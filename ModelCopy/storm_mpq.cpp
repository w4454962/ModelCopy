#include "stormlib.h"
#include "storm_mpq.h"
#include <string>
#include <regex>
#include <MDX.h>
#include <vector>
 

storm_mpq::storm_mpq()
{
	m_hMpq = NULL;
	m_strPath = "";
}
storm_mpq::storm_mpq(const char* szPath)
{
	if (!SFileOpenArchive(szPath, 0, 0, &m_hMpq))
	{
		printf((std::string(szPath) + "-��mpqʧ��").c_str());
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
		printf((m_strPath + "-mpq�ر�ʧ��").c_str());
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
		printf((std::string(szPath) + "-��mpqʧ��").c_str());
		return 0;
	}
	m_strPath = szPath;
	return 1;
}
int storm_mpq::Close()
{
	if (!m_hMpq)
	{
		printf((std::string(m_strPath) + "-�ر�mpqʧ��-mpq��Ч").c_str());
		return 0;
	}
	if (!SFileCloseArchive(m_hMpq))
	{
		printf((m_strPath + "-mpq�ر�ʧ��").c_str());
		return 0;
	}
	m_hMpq = NULL;
	return 1;
}

int storm_mpq::HasFile(const char* szFileName)
{
	if (!m_hMpq)
	{
		printf("�ļ�������-mpq��Ч");
		return 0;
	}
	return SFileHasFile(m_hMpq, szFileName);
}

int storm_mpq::LoadFile(const char* szFileName, const char** pszBuffer, DWORD* pdwSize)
{
	HANDLE hFile;
	if (!m_hMpq)
	{
		printf("��ȡ�ļ�ʧ��-mpq��Ч");
		return 0;
	}
	if (!SFileHasFile(m_hMpq, szFileName))
	{
		//printf((std::string(szFileName) + "������").c_str());
		return 0;
	}
	if (!SFileOpenFileEx(m_hMpq, szFileName, 0, &hFile))
	{
		//printf(("���ļ�ʧ��-" + std::string(szFileName)).c_str());
		return 0;
	}
	DWORD dwSize = SFileGetFileSize(hFile, 0);
	if (!dwSize)
	{
		//printf("�ļ���СΪ0");
		return 0;
	}
	char* szFileData = new char[dwSize + 1];
	DWORD dwRead;
	if (!SFileReadFile(hFile, szFileData, dwSize, &dwRead, NULL))
	{
		//printf(("��ȡ�ļ�-" + std::string(szFileName) + "�ڴ�ʧ��").c_str());
		return 0;
	}
	if (!SFileCloseFile(hFile))
	{
		//printf(("�ر��ļ�-" + std::string(szFileName) + "ʧ��").c_str());
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
		printf("����ļ�ʧ��-mpq��Ч");
		return 0;
	}
	if (SFileHasFile(m_hMpq, szArchivePath)) //�ж�·���Ѵ����ļ�
	{
		if (!RemoveFile(szArchivePath)) //ɾ���Ѵ��ڵ��ļ�
		{
			return 0;
		}
	}
	if (!SFileAddFile(m_hMpq, szFilePath, szArchivePath, 0x00000200 | 0x80000000))
	{
		printf((std::string(szArchivePath) + "����ļ�ʧ��").c_str());
		return 0;
	}
	return 1;
} 
int storm_mpq::RemoveFile(const char* szFileName)
{
	if (!m_hMpq)
	{
		printf("ɾ���ļ�ʧ��-mpq��Ч");
		return 0;
	}
	if (!SFileRemoveFile(m_hMpq, szFileName, 0))
	{
		printf(("ɾ���ļ�-" + std::string(szFileName) + "ʧ��").c_str());
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

	if (find_texture) //�����ģ�� Ҫ������������ͼ·��
	{
		std::vector<uint8_t> data;
		data.resize(size);
		memcpy(&data[0], buffer, size);
		BinaryReader render(data);
		mdx::MDX model(render);
		
		for (auto& texture : model.textures)
		{
			find_texture->push_back(texture.file_name);
		}
	}

	UnLoadFile(buffer);

	return 1;
}