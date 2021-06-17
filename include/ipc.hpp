#pragma once

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace yba
{
	template<typename layout> class ipc
	{
		layout* mapped_content = nullptr;
		const char* const name;

#ifdef _WIN32
		HANDLE mapping = INVALID_HANDLE_VALUE;
#else
		int fd = -1;
		bool owns = false;
#endif

		bool map()
		{
#ifdef _WIN32
			mapped_content = (layout*)MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(layout));
#else
			mapped_content = (layout*)mmap(0, sizeof(layout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			return mapped_content;
#endif
		}
		public:
		ipc(const char* name) : name(name)
		{
		}

		~ipc()
		{
#ifdef _WIN32
			UnmapViewOfFile(mapped_content);
			CloseHandle(mapping);
#else
			if(fd > 0)
			{
				if(mapped_content)
					munmap(mapped_content, sizeof(layout));
				close(fd);
				if(owns)
					shm_unlink(name);
			}
#endif
		}

		bool check_exist()
		{

#ifdef _WIN32
			mapping = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
			if (mapping == NULL)
				return false;

			CloseHandle(mapping);
			return true;
#else
			fd = shm_open(name, O_RDWR | O_EXCL, 0600);
			if(fd < 0)
				return false;
			close(fd);
			//shm_unlink(name);
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
#else
			fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
			if (fd < 0)
				return false;
			ftruncate(fd, sizeof(layout));
			owns = true;
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
#else
			fd = shm_open(name, O_RDWR /*| O_EXCL*/, 0600);
			if(fd < 0)
				return false;
			return map();
#endif
		}

		layout* get()
		{
			return mapped_content;
		}

		ipc(ipc&&) = delete;
		ipc(const ipc&) = delete;
		ipc& operator=(ipc&&) = delete;
		ipc& operator=(const ipc&) = delete;
	};
}
