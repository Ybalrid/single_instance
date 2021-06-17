cmake_minimum_required(VERSION 3.18)
project(single_instance)

file(GLOB dev_code CONFIGURE_DEPENDS "test/*.cpp" )
file(GLOB header_only_code CONFIGURE_DEPENDS "include/*.hpp" )

add_executable(test_me ${dev_code} ${header_only_code})

target_include_directories(test_me PUBLIC ${CMAKE_SOURCE_DIR}/include/)

if(UNIX)
find_package(Threads REQUIRED)
target_link_libraries(test_me PRIVATE Threads::Threads rt)
endif()
