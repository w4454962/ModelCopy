#include "Helper.h"
#include "fp_call.h"
#include "storm_mpq.h"
#include <map>
Helper* getHelper()
{
	static Helper instance;
	return &instance;
}

uintptr_t Helper::getAddress(uintptr_t addr)
{
	const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
	return addr - 0x00400000 + base;
}

EditorData* Helper::getEditorData()
{
	uintptr_t addr = *(uintptr_t*)getAddress(0x803cb0);

	uintptr_t count = *(uintptr_t*)(addr + 0x1b0);

	uintptr_t object = *(uintptr_t*)(*(uintptr_t*)(addr + 0x1a8) + count * 4);

	if (*(uintptr_t*)(object + 0x114))
	{
		uintptr_t uknow = *(uintptr_t*)(object + 0x118);

		return *(EditorData**)uknow;
	}
	MessageBoxA(0, "获取不到编辑器对象", "错误！！！", MB_OK);
	return 0;
}

std::string Helper::getUnitData(const char* unitId, const char* field)
{
	std::string result;

	auto worldData = getEditorData();

	if (!worldData || !worldData->units)
		return result;

	char buffer[0x104];
	ZeroMemory(buffer, sizeof(buffer));

	this_call<bool>(getAddress(0x005174C0), *(uintptr_t*)(((uint32_t)worldData->units) + 0x140), unitId, field, buffer, 0x104, 0);

	result = buffer;
	return result;
}

fs::path Helper::getUnitModelPath(const char* unitId)
{
	std::string result;

	auto worldData = getEditorData();

	if (!worldData || !worldData->units)
		return result;

	char buffer[0x104];
	ZeroMemory(buffer, sizeof(buffer));


	this_call<bool>(getAddress(0x00500470), *(uintptr_t*)(((uint32_t)worldData->units) + 0x140), *(uint32_t*)unitId, buffer, 0x104);
	
	result = buffer;
	return result;
}

fs::path Helper::getDoodasModelPath(const char* doodaId)
{
	std::string result;

	auto worldData = getEditorData();

	if (!worldData || !worldData->doodas)
		return result;

	char buffer[0x104];
	ZeroMemory(buffer, sizeof(buffer));


	this_call<bool>(getAddress(0x005029A0), *(uintptr_t*)(((uint32_t)worldData->doodas) + 0x140), *(uint32_t*)doodaId, buffer, 0x104, 0);

	result = buffer;
	return result;
}

const char* Helper::getCurrentMapPath()
{
	uintptr_t addr = *(uintptr_t*)getAddress(0x803cb0);

	uintptr_t count = *(uintptr_t*)(addr + 0x1b0);

	uintptr_t object = *(uintptr_t*)(*(uintptr_t*)(addr + 0x1a8) + count * 4);

	return (const char*)object;
}



void Helper::copyUpdate()
{
	auto worldData = getEditorData();

	if (!worldData || !worldData->units)
		return;

	storm_mpq mpq;

	if (!mpq.Open(getCurrentMapPath()))
		return;


	int count = 0;
	//遍历单位

	fs::path root = fs::temp_directory_path() / "model";

	fs::remove_all(root);

	fs::create_directories(root);

	std::map<fs::path, bool> file_map;

	auto units = worldData->units;
	for (size_t i = 0; i < units->unit_count; i++)
	{
		Unit* unit = &units->array[i];

		//类型 不是单位 或者 是玩家开始点 则跳过
		if (unit->type != 0 || strncmp(unit->name, "sloc", 4) == 0)
			continue;

		//如果被选中
		if (this_call<bool>(*(uintptr_t*)((*(uintptr_t*)units) + 0x24), units, i))
		{
			std::string id = std::string(unit->name, 0x4);
			std::string name = u2a(getUnitData(unit->name, "Name"));
			fs::path path = getUnitModelPath(unit->name);
			if (path.extension().string() == ".mdl")
			{
				std::string str = path.string();
				str[str.length() - 1] = 'x';
				path = str;
			}

			fs::path temp = root / path.filename();

			std::vector<fs::path> list;

			if (file_map.find(path) == file_map.end() && mpq.ExportFile(path, temp, &list))
			{
				file_map[path] = true;
				printf("复制模型 %s %s %s\n", id.c_str(), name.c_str(), path.string().c_str());

				//解析贴图
				for (auto& texture_path : list)
				{
					fs::path texture_temp = root / texture_path;
					if (file_map.find(texture_path) == file_map.end() && mpq.ExportFile(texture_path, texture_temp))
					{
						file_map[texture_path] = true;
						printf("复制贴图 %s \n", texture_path.string().c_str());
						count++;
					}
				}


				count++;
			}
		}
	}
	auto doodas = worldData->doodas;
	for (size_t i = 0; i < doodas->unit_count; i++)
	{
		Unit* unit = &doodas->array[i];

		//如果被选中
		if (this_call<bool>(*(uintptr_t*)((*(uintptr_t*)doodas) + 0x24), doodas, i))
		{
			std::string id = std::string(unit->name, 0x4);

			fs::path path = getDoodasModelPath(unit->name);
			if (path.extension().string() == ".mdl")
			{
				std::string str = path.string();
				str[str.length() - 1] = 'x';
				path = str;
			}

			fs::path temp = root / path.filename();

			std::vector<fs::path> list;

			if (file_map.find(path) == file_map.end() && mpq.ExportFile(path, temp, &list))
			{
				file_map[path] = true;
				printf("复制装饰物模型 %s  %s\n", id.c_str(), path.string().c_str());

				//解析贴图
				for (auto& texture_path : list)
				{
					fs::path texture_temp = root / texture_path;
					if (file_map.find(texture_path) == file_map.end() && mpq.ExportFile(texture_path, texture_temp))
					{
						file_map[texture_path] = true;
						printf("复制贴图 %s \n", texture_path.string().c_str());
						count++;
					}
				}
				count++;;
			}
		}
	}

	printf("文件数量 %i \n", count);


	mpq.Close();

	std::vector<std::string> filelist;
	for (auto& iter : fs::directory_iterator(root))
	{
		filelist.push_back(iter.path().string());
	}

	CopyFilesToClip(filelist);

}


void Helper::attach()
{


	char buffer[MAX_PATH];

	GetModuleFileNameA(nullptr, buffer, MAX_PATH);

	std::string name = fs::path(buffer).filename().string();
	if (std::string::npos == name.find("worldedit")
		&& std::string::npos == name.find("WorldEdit"))
		return;

	enableConsole();

	printf("7哥的模型复制工具2.0， 有空记得加群692125060\n");
	printf("按 Shift + C 键可以多选复制地形上 单位&装饰物 的模型\n");
	printf("在外面按 Ctrl + V 键黏贴出模型贴图文件了。 \n");
	printf("按 Shift + H 键 隐藏控制台\n");
	printf("按 Shift + S 键 显示控制台\n");

	

	m_hwnd = *(HWND*)((uint32_t)GetModuleHandleA(name.c_str()) + 0x403C9C);

	m_oldWinProc = (WNDPROC)SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG)Helper::windowProc);

	//SetTimer(m_hwnd, 10086, 5000, [](HWND hwnd, UINT, UINT_PTR, DWORD) {
	//	::ShowWindow(::GetConsoleWindow() , SW_HIDE);
	//	::KillTimer(getHelper()->m_hwnd, 10086);
	//});
}


LRESULT CALLBACK Helper::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		// shift + c 键
		if ((GetKeyState(0x10) & 0x8000) && GetKeyState('C'))
		{
			getHelper()->copyUpdate();
		}
		// shift + s 键
		if ((GetKeyState(0x10) & 0x8000) && GetKeyState('S'))
		{
			getHelper()->enableConsole();
		}
		// shift + h 键
		if ((GetKeyState(0x10) & 0x8000) && GetKeyState('H'))
		{
			::ShowWindow(::GetConsoleWindow(), SW_HIDE);
		}


		break;
	}
	case WM_SYSKEYUP:
	case WM_KEYUP:
	{

	}
	default:
		break;
	}


	return CallWindowProc(getHelper()->m_oldWinProc, hWnd, message, wParam, lParam);
}


void Helper::enableConsole()
{
	static HWND v_hwnd_console = ::GetConsoleWindow();

	if (nullptr != v_hwnd_console)
	{
		::ShowWindow(v_hwnd_console, SW_SHOW);

	}
	else
	{
		auto v_is_ok = AllocConsole();
		if (v_is_ok)
		{
			FILE* fp;
			//freopen_s(&fp, "CONOUT$", "r", stdin);
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);
			fclose(fp);
			std::cout.clear();
		}
	}
	v_hwnd_console = ::GetConsoleWindow();
	if (v_hwnd_console)
	{
		::DeleteMenu(::GetSystemMenu(v_hwnd_console, FALSE), SC_CLOSE, MF_BYCOMMAND);
		::DrawMenuBar(v_hwnd_console);
		::SetWindowTextA(v_hwnd_console, "7哥的模型复制工具2.0， 有空记得加群692125060");
	}
}
