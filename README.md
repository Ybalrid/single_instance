# single_instance

`yba::single_instance` is a really small single-header library to assist 
you in making a single-instance program.

This may be used for URL handlers, or application that needs to make sure only 
one instance of it is running.

This library is compatible with Windows, and any UNIX platform supporinng the 
`shm_open`, `mmap`, `munmap`, `ftruncate`, `shm_unlink` and `close` systemcalls.

## How to

Add the `inlcude` directory of this repository to your compiler search path.

On UNIX you will need to link to `pthread` and `rt`.

The `yba::single_instance` is the main type of this library.

```cpp
yba::single_instance(int argc, char* argv[], 
                     const char* unqiue_name, 
                     yba::single_instance::argument_handler& handler = nullptr);
```

Its constructor takes the command line arguments of the application 
(that may be forwarded to the first instance of the program), a unique identifier string, 
and optionally a pointer to  an `argument_handler` sturcture.

The pointer `handler` has to be an address of an instance of a type that inherits from `argument_handler`
```cpp
struct argument_handler
{
	virtual void operator()(int argc, char* argv[]) {}
};
```

Example:
```cpp
struct my_handler : yba::single_instance::argument_handler
{
  void operator()(int argc, char* argv[]) final
  {
    std::printf("Received %d arguments\n", argc);
    for(int i = 0; i < argc; ++i)
    {
      std::printf("argv[%d] = \"%s\"\n", i, argv[i]);
    }
  }
};

int main(int argc, char* argv[])
{
  my_handler handler;
  yba::single_instance instance(argc, argv, "test_server", &handler);
  
  // Calling this permit you to perform the check to know if this program is the original instance,
  // or not.
  if(instance.check_single_instance())
  {
    std::printf("this program is the original instance.\n");
  }
  else
  {
    std::printf("this program is another instance of the same program.\n");
    
    // Calling this function will send the list of arguments passed form the 
    // 2nd instance of the program to the orignal instance. 
    //
    // In the orignal instance of the program, my_handler::operator() will be called with
    // *this program*'s `argc` and `argv`
    
    instance.forward_arguments();
    return 0;
  }
  
  while(true) { /* this is the original instance doing it's work here */ }
  
  return 0;
}
```
