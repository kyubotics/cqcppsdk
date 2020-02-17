macro(cq_add_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_compile_definitions(${TEST_NAME} PUBLIC _CQ_TEST_MODE)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    # Group under the "tests" project folder in IDEs such as Visual Studio.
    set_property(TARGET ${TEST_NAME} PROPERTY FOLDER "tests")
endmacro()

cq_add_test(test_dolores
        test_dolores.cpp
        test_dolores_session.cpp
        test_dolores_string.cpp
        test_dolores_matcher.cpp)
