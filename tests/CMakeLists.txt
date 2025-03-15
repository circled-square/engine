#enable_testing() # this doesn't need to be called since we call it in the root CMakeLists.txt, because otherwise QtCreator does not find our tests

add_executable(engine__tests_example example.cpp)
target_link_libraries(engine__tests_example PRIVATE engine)
add_test(NAME engine__tests_example COMMAND engine__tests_example)

add_executable(engine__tests_rm rm.cpp)
target_link_libraries(engine__tests_rm PRIVATE engine)
add_test(NAME engine__tests_rm COMMAND engine__tests_rm)

add_custom_target(run_engine_tests COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS engine__tests_example engine__tests_rm)
