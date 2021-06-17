#include <single_instance.hpp>

#include <cstdio>
#include <cstdlib>
#include <csignal>

struct my_handler : yba::single_instance::argument_handler
{
	void operator()(int argc, char* argv[]) final
	{
		std::printf("Received %d arguments\n", argc);
		for(int i = 0; i < argc; ++i)
		{
			printf("argv[%d] = \"%s\"\n", i, argv[i]);
		}
	}
};

bool server_is_running = true;

void signal_handler(int sig)
{
	std::printf("handling signal %d\n", sig);
	server_is_running = false;
}


int main(int argc, char* argv[])
{
	my_handler handler;
	handler(argc, argv);
	yba::single_instance instance(argc, argv, "test_server", &handler);
	if(instance.check_single_instance())
	{
		std::printf("this program is the original instance.\n");
		signal(SIGTERM, signal_handler);
		signal(SIGINT, signal_handler);
#ifndef _WIN32
		signal(SIGQUIT, signal_handler);
#endif
		while (server_is_running)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	else
	{
		std::printf("this program is another instance of the same program.\n");
		instance.forward_arguments();
		return 0;
	}

	return 0;
}
