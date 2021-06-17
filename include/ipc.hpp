#pragma once

#ifdef _WIN32
//win32 API
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
//UNIX API
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace yba
{
	/// <summary>
	/// Manage an IPC file in memory
	/// </summary>
	/// <typeparam name="layout">structure representing the binary layout of the IPC file</typeparam>
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

		/// <summary>
		/// Map the existing ipc file inside the address space of the process
		/// </summary>
		/// <returns>true if memory was properly mapped</returns>
		bool map()
		{
#ifdef _WIN32
			mapped_content = (layout*)MapViewOfFile(mapping, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(layout));
#else
			mapped_content = (layout*)mmap(0, sizeof(layout), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
#endif
			return mapped_content;
		}

	public:
		/// <summary>
		/// Create an IPC file
		/// </summary>
		/// <param name="name">A name for the file, should be common with all consume of the file</param>
		ipc(const char* name) : name(name)
		{}

		~ipc()
		{
#ifdef _WIN32
			UnmapViewOfFile(mapped_content);
			CloseHandle(mapping);
#else
			if (fd > 0)
			{
				if (mapped_content)
					munmap(mapped_content, sizeof(layout));
				close(fd);
				if (owns)
					shm_unlink(name);
			}
#endif
		}

		/// <summary>
		/// Determine if a file of the name passed in the ctor already exist. This will try to open the file. File will be closed if successful, and no mapping occur.
		/// </summary>
		/// <returns>State of the file. True if exist, False if not</returns>
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
			if (fd < 0)
				return false;
			close(fd);
			//shm_unlink(name);
			return true;

#endif
		}

		/// <summary>
		/// Create a shared memory file of the name passed to the ctor.
		/// </summary>
		/// <remarks>You should use check_exist to know if you need to create or open the file</remarks>
		/// <returns>true if file was created and mapped</returns>
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

		/// <summary>
		/// Open an exisisting shared memory file
		/// <remarks>You should use check_exist to know if you need to create or open the file</remarks>
		/// <returns>true if file was created and mapped</returns>
		bool open()
		{
#ifdef _WIN32
			mapping = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, name);
			if (!mapping)
				return false;
			return map();
#else
			fd = shm_open(name, O_RDWR /*| O_EXCL*/, 0600);
			if (fd < 0)
				return false;
			return map();
#endif
		}

		/// <summary>
		/// Get pointer to the shared file
		/// </summary>
		/// <returns>Mapped pointer if a successful mapping occurred beforehand, or nullptr</returns>
		[[nodiscard]] layout* get()
		{
			return mapped_content;
		}

		ipc(ipc&&) = delete;
		ipc(const ipc&) = delete;
		ipc& operator=(ipc&&) = delete;
		ipc& operator=(const ipc&) = delete;
	};
}
