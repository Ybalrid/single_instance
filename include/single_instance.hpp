#pragma once

#include "ipc.hpp"

#include <thread>
#include <atomic>
#include <climits>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#pragma warning(push)
#pragma warning (disable:4996)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define YBA_SINGLE_INSTANCE_CRT_WARNINGS_UNDO
#endif
#endif

namespace yba
{
	/// <summary>
	/// Represent, and manage a single instance of the program
	/// </summary>
	class single_instance
	{
		//Asynchronous background checks for receiveing messages from other attempts are running this program
		std::atomic_bool bg_run;
		std::thread bg_checker_thread;

		static constexpr int args_size = 4096U;
		struct application_presence
		{
			bool child_written;
			char args[args_size];
		};

		ipc<application_presence> ipc_file;

		// Storage for this process's arg list
		int argc;
		char** argv;

		/// <summary>
		/// Check the arguments present in the ipc file
		/// </summary>
		void check_arguments()
		{
			int new_argc = 0;

			application_presence* presence = ipc_file.get();
			if (!presence)
				return;

			if (presence->args[0] == '\0')
				return;

			int index = 0;
			do
			{
				index++;
				if (presence->args[index] == '\0')
					new_argc++;
			} while (index < args_size - 2 && !(presence->args[index] == '\0' && presence->args[index + 1] == 0));

			char** new_argv = (char**)std::calloc(new_argc, sizeof(char*));
			index = 0;
			for(int i = 0; i < new_argc; ++i)
			{
				new_argv[i] = strdup(&presence->args[index]);
				index += (int)strlen(new_argv[i]) +1;
			}

			if(handler)
				(*handler)(new_argc, new_argv);

			for(int i = 0; i < new_argc; ++i)
				free(new_argv[i]);
			free(new_argv);
		}

	public:

		/// <summary>
		/// A callable type you can override, to contain code that processes command line arguments
		/// </summary>
		struct argument_handler
		{
			virtual void operator()(int argc, char* argv[])
			{
			}
		};


		/// <summary>
		/// Create a single_instance holder
		/// </summary>
		/// <param name="argc">argument count passed to this program instance</param>
		/// <param name="argv">argument list passed to this program instance</param>
		/// <param name="unique_name">a unique string name for this program</param>
		/// <param name="handler">pointer to a yba::single_instance::argument_handler. Call operator may be called by a background thread containing an argument list sent by another instance of this program</param>
		single_instance(int argc, char* argv[], const char* unique_name, argument_handler* handler = nullptr) : ipc_file(unique_name), argc(argc), argv(argv), handler(handler)
		{
			bg_run = true;
			bg_checker_thread = std::thread
			{[&]
				{
					while (bg_run)
					{
						application_presence* file = ipc_file.get();
						if (file)
						{
							if (file->child_written)
							{
								check_arguments();
								file->child_written = false;
							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
			};
		}

		~single_instance()
		{
			bg_run = false;
			if (bg_checker_thread.joinable())
				bg_checker_thread.join();
		}

		/// <summary>
		/// Check if this program is currently the main single instance or not
		/// </summary>
		bool check_single_instance()
		{
			if (!ipc_file.check_exist())
				return ipc_file.create();

			ipc_file.open();
			return false;
		}

		/// <summary>
		/// Write all arguments to the ipc file created by the single instance
		/// </summary>
		void forward_arguments()
		{
			int len = 0;
			if (!ipc_file.get()) return;
			application_presence& presence = *ipc_file.get();
			presence.child_written = false;
			memset(presence.args, 0, args_size);
			for (int i = 1;
				i < argc;
				++i)
			{
				char* arg = argv[i];
				strcpy(presence.args + len, arg);
				len += (int)strlen(arg) + 1;
			}

			presence.child_written = true;
		}

	private:
		argument_handler* handler = nullptr;
	};
}

#ifdef _WIN32
#ifdef YBA_SINGLE_INSTANCE_CRT_WARNINGS_UNDO
#undef _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(pop)
#endif
