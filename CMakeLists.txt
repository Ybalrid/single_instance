cmake_minimum_required(VERSION 3.19)
project(single_instance)

file(GLOB dev_code CONFIGURE_DEPENDS "test/*.cpp" )
file(GLOB header_only_code CONFIGURE_DEPENDS "include/*.hpp" )

add_executable(test_me ${dev_code} ${header_only_code})

target_include_directories(test_me PUBLIC ${CMAKE_SOURCE_DIR}/include/)
