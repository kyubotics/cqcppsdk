macro(cq_add_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    target_link_libraries(${TEST_NAME} cqcppsdk_test_mode)
    # Group under the "tests" project folder in IDEs such as Visual Studio.
    set_property(TARGET ${TEST_NAME} PROPERTY FOLDER "tests")
endmacro()

cq_add_test(test_dolores
        test_dolores.cpp
        test_dolores_anymap.cpp
        test_dolores_string.cpp
        test_dolores_matcher.cpp
        test_dolores_watashi.cpp
        test_dolores_traits.cpp
        test_dolores_handler.cpp)
