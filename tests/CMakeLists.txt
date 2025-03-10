enable_testing()

add_executable(engine__tests_example example.cpp)
target_link_libraries(engine__tests_example PRIVATE engine)
add_test(NAME engine__tests_example COMMAND engine__tests_example)

add_custom_target(run_engine_tests COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS engine__tests_example)
