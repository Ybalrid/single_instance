#pragma once

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace yba
{
	template<typename layout> class ipc
	{
		layout* mapped_content = nullptr;
		const char* const name;

#ifdef _WIN32
		HANDLE mapping = INVALID_HANDLE_VALUE;
#endif

		bool map()
		{
#ifdef _WIN32
			mapped_content = (layout*)MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(layout));
			return mapped_content;
#endif
		}
	public:
		ipc(const char* name) : name(name)
		{
		}

		~ipc()
		{
			UnmapViewOfFile(mapped_content);
			CloseHandle(mapping);
		}

		bool check_exist()
		{

#ifdef _WIN32
			mapping = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
			if (mapping == NULL)
				return false;

			CloseHandle(mapping);
			return true;
#endif
		}

		bool create()
		{
#ifdef _WIN32
			mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(layout), name);
			if (!mapping)
				return false;
			return map();
#endif
		}

		bool open()
		{
#ifdef _WIN32
			mapping = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
			if (!mapping)
				return false;
			return map();
#endif
		}

		layout* get()
		{
			return mapped_content;
		}
	};
}
