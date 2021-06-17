#include <single_instance.hpp>

#include <cstdio>
#include <cstdlib>

struct my_handler : yba::single_instance::argument_handler
{
	void operator()(int argc, char* argv[]) final
	{
		std::printf("Received %d arguments\n");
		for(int i = 0; i < argc; ++i)
		{
			printf("argv[%d] = \"%s\"\n", i, argv[i]);
		}
	}
};

int main(int argc, char* argv[])
{
	my_handler handler;
	yba::single_instance instance(argc, argv, "test_server", &handler);
	if(instance.check_single_instance())
	{
		std::printf("this program is the original instance.\n");
		for (;;);
	}
	else
	{
		std::printf("this program is another instance of the same program.\n");
		instance.forward_arguments();
		return 0;
	}

	std::system("pause");
	return 0;
}
