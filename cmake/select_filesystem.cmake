set (TEST_FS_LIB OFF)
include (CheckIncludeFileCXX)

if (CMAKE_BOOST_FILESYSTEM_FORCE)
    # https://cmake.org/cmake/help/git-master/module/FindBoost.html
    # Implicit dependencies such as Boost::filesystem requiring Boost::system will be automatically detected and satisfied,
    # even if system is not specified when using find_package and if Boost::system is not added to target_link_libraries.
    # If using Boost::thread, then Thread::Thread will also be added automatically.
    find_package(Boost REQUIRED COMPONENTS filesystem)
    if(Boost_FOUND)
        message (STATUS "Boost Filesystem header:      <boost/filesystem.hpp>")
        # target_include_directories  (${OUT_NAME} SYSTEM PUBLIC Boost::Boost)
        # message("boost found true")
        set (TEST_FS_LIB ON)
        set (Boost_USE_STATIC_LIBS ON)
        set (Boost_USE_STATIC_RUNTIME ON)
        set (CXXSTD_FS_TEST_SOURCE
                "#include <boost/filesystem.hpp>
                #include <boost/cerrno.hpp>
                #include <boost/throw_exception.hpp>
            int main()
            {
                boost::filesystem::path p{\"\tmp/\"};
                BOOST_THROW_EXCEPTION( boost::filesystem_error(\"Empty file name!\", boost::system::error_code(boost::system::errc::invalid_argument, boost::system::generic_category())) );
            }")
    endif()
else()
    check_include_file_cxx (filesystem HAVE_FILESYSTEM)
    if (HAVE_FILESYSTEM)
        message (STATUS "C++ Filesystem header:      <filesystem>")
        set (TEST_FS_LIB ON)
        set (CXXSTD_FS_TEST_SOURCE
                "#include <filesystem>
            int main()
            {
                std::filesystem::path p{\"\tmp/\"};
                throw std::filesystem::filesystem_error(\"Empty file name!\", std::make_error_code(std::errc::invalid_argument));
            }")
    else()
        check_include_file_cxx (experimental/filesystem HAVE_EXP_FILESYSTEM)
        if (HAVE_EXP_FILESYSTEM)
            message (STATUS "C++ Filesystem header:      <experimental/filesystem>")
            set (TEST_FS_LIB ON)
            set (CXXSTD_FS_TEST_SOURCE
                    "#include <experimental/filesystem>
                int main()
                {
                    std::experimental::filesystem::path p{\"/tmp/\"};
                    throw std::experimental::filesystem::filesystem_error(\"Empty file name!\", std::make_error_code(std::errc::invalid_argument));
                }")
        else ()
            message (SEND_ERROR "No std::filesystem include file found")
            target_compile_definitions(${OUT_NAME} PUBLIC STD_FILESYSTEM_NOT_SUPPORTED)
            set (TEST_FS_LIB OFF)
        endif ()
    endif ()
endif ()

if (TEST_FS_LIB)
    include (FindPackageMessage)
    include (CheckIncludeFileCXX)
    include (CheckCXXSourceCompiles)
    # CMAKE_REQUIRED_FLAGS = string of compile command line flags
    # CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
    # CMAKE_REQUIRED_INCLUDES = list of include directories
    set (CMAKE_REQUIRED_INCLUDES    ${CMAKE_INCLUDE_PATH})
    set (CMAKE_REQUIRED_FLAGS       ${CMAKE_CXX_FLAGS})
    set (CMAKE_REQUIRED_FLAGS_ORIGINAL ${CMAKE_REQUIRED_FLAGS})
    set (CMAKE_REQUIRED_LIBRARIES_ORIGINAL ${CMAKE_REQUIRED_LIBRARIES})

    if (CMAKE_BOOST_FILESYSTEM_FORCE)
        message (STATUS "Boost Filesystem library:    ${Boost_LIBRARIES}")
        target_compile_definitions(${OUT_NAME} PUBLIC BOOST_FILESYSTEM_AVAILABLE)
        # SYSTEM - ignore warnings from here
        target_include_directories(${OUT_NAME} SYSTEM PUBLIC "${Boost_INCLUDE_DIR}")    # ?? SYSTEM
        target_link_libraries     (${OUT_NAME} ${Boost_LIBRARIES})
    else()
        # c++: builtin
        set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_ORIGINAL}")
        set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_ORIGINAL})
        check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXXBuiltIn_FS_BuiltIn)
        if (CXXBuiltIn_FS_BuiltIn)
            message (STATUS "C++ Filesystem library:     builtin")
        else()
            set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES_ORIGINAL} stdc++fs")
            check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXXBuiltIn_FS_stdcppfs)
            if (CXXBuiltIn_FS_stdcppfs)
                message (STATUS "C++ Filesystem library:     stdc++fs")
                target_link_libraries     (${OUT_NAME} stdc++fs)
            else()
                set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES_ORIGINAL} c++fs")
                check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXXBuiltIn_FS_cppfs)
                if (CXXBuiltIn_FS_cppfs)
                    message (STATUS "C++ Filesystem library:     c++fs")
                    target_link_libraries     (${OUT_NAME} c++fs)
                else()

                    # c++:   -std=c++17
                    set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_ORIGINAL} -std=c++17")
                    set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_ORIGINAL})
                    check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXX_std__cpp17_FS_BuiltIn)
                    if (CXX_std__cpp17_FS_BuiltIn)
                        message (STATUS "C++: -std=c++17; Filesystem library: builtin")
                    else()
                        set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES_ORIGINAL} stdc++fs")
                        check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXX_std__cpp17_FS_stdcppfs)
                        if (CXX_std__cpp17_FS_stdcppfs)
                            message (STATUS "C++: -std=c++17; Filesystem library:   stdc++fs")
                            target_link_libraries     (${OUT_NAME} stdc++fs)
                        else()
                            set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES_ORIGINAL}  c++fs")
                            check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXX_std__cpp17_FS_cppfs)
                            if (CXX_std__cpp17_FS_cppfs)
                                message (STATUS "C++: -std=c++17; Filesystem library:    c++fs")
                                target_link_libraries     (${OUT_NAME}  c++fs)
                            else()

                                # c++:   /std:c++17
                                set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_ORIGINAL} /std:c++17")
                                set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_ORIGINAL})
                                check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXX_std_cpp17_FS_BuiltIn)
                                if (CXX_std_cpp17_FS_BuiltIn)
                                    message (STATUS "C++: /std:c++17; Filesystem library: builtin")
                                else()
                                    set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES_ORIGINAL} stdc++fs")
                                    check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXX_std_cpp17_FS_stdcppfs)
                                    if (CXX_std_cpp17_FS_stdcppfs)
                                        message (STATUS "C++: /std:c++17; Filesystem library:   stdc++fs")
                                        target_link_libraries     (${OUT_NAME} stdc++fs)
                                    else()
                                        set (CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES_ORIGINAL}  c++fs")
                                        check_cxx_source_compiles ("${CXXSTD_FS_TEST_SOURCE}" CXX_std_cpp17_FS_cppfs)
                                        if (CXX_std_cpp17_FS_cppfs)
                                            message (STATUS "C++: /std:c++17; Filesystem library:    c++fs")
                                            target_link_libraries     (${OUT_NAME}  c++fs)
                                        else ()
                                            if(NOT CMAKE_BOOST_FILESYSTEM_FORCE)
                                                set(CMAKE_BOOST_FILESYSTEM_FORCE TRUE)
                                                message (WARNING "Test std::filesystem failed. Retrying with boost::filesystem.")
                                                include(${_CQCPPSDK_DIR}/cmake/select_filesystem.cmake)
                                            else()
                                                message (SEND_ERROR "No boost::filesystem library found.")
                                            endif()
                                        endif ()
                                    endif ()
                                endif ()
                            endif ()
                        endif ()
                    endif ()
                endif ()
            endif ()
        endif ()
    endif()
endif (TEST_FS_LIB)


