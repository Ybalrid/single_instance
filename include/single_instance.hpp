#pragma once

#include "ipc.hpp"
#include <thread>
#include <atomic>
#include <climits>
#include <cstring>

namespace yba
{
	class single_instance
	{
		std::atomic_bool bg_run = true;
		std::thread bg_checker_thread;

		struct application_presence
		{
			bool child_written;
			char args[4096];
		};

		ipc<application_presence> ipc;

		int argc;
		char** argv;

		void check_arguments()
		{
			int new_argc = 0;

			application_presence* presence = ipc.get();
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
			} while (index < 4096 - 2 && !(presence->args[index] == '\0' && presence->args[index + 1] == 0));

			char** new_argv = (char**)std::calloc(new_argc, sizeof(char*));
			index = 0;
			for(int i = 0; i < new_argc; ++i)
			{
				new_argv[i] = strdup(&presence->args[index]);
				index += strlen(new_argv[i]) +1;
			}

			if(handler)
			{
				(*handler)(new_argc, new_argv);
			}

			for(int i = 0; i < new_argc; ++i)
			{
				free(new_argv[i]);
			}
			free(new_argv);
		}

	public:

		struct argument_handler
		{
			virtual void operator()(int argc, char* argv[])
			{
			}
		};


		single_instance(int argc, char* argv[], const char* unique_name, argument_handler* handler = nullptr) : ipc(unique_name), argc(argc), argv(argv), handler(handler)
		{
			bg_checker_thread = std::thread{
				[&]
				{
					while(bg_run)
					{
						application_presence* file = ipc.get();
						if(file)
						{
							if(file->child_written)
							{
								this->check_arguments();
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

		bool check_single_instance()
		{
			if(!ipc.check_exist())
				return ipc.create();

			ipc.open();
			return false;
		}

		void forward_arguments()
		{
			int len = 0;
			if (!ipc.get()) return;
			application_presence& presence = *ipc.get();
			presence.child_written = false;
			memset(presence.args, 0, 4096);
			for(int i = 1;
				i < argc;
				++i)
			{
				char* arg = argv[i];
				strcpy(presence.args + len, arg);
				len += strlen(arg) + 1;
			}

			presence.child_written = true;
		}

	private:
		argument_handler* handler = nullptr;
	};
}