cmake_minimum_required (VERSION 3.14)
project (Porc)
ENABLE_TESTING()

file(GLOB TESTS_SOURCES
  tests/*.c
)

file(GLOB )

file(GLOB TEST_PARSER_SOURCES
	tests/test_output/*.cpp)

add_library(interpreter_common STATIC ${LIB_SOURCES})
add_library(porc_backend STATIC ${SOURCES})

SET ( APP_EXE Porc )

add_executable(${APP_EXE} ${APP_SOURCES})
target_link_libraries(${APP_EXE} interpreter_common)
target_link_libraries(${APP_EXE} porc_backend)

add_executable(test_output ${TEST_PARSER_SOURCES})
target_link_libraries(test_output porc_backend)
target_link_libraries(test_output Catch2::Catch2)

include(CTest)
include(lib/Catch2/contrib/Catch.cmake)
catch_discover_tests(test_output)
