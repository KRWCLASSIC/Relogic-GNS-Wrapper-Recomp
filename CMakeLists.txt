cmake_minimum_required(VERSION 3.20)
project(RelogicGNSWrapperRecomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GameNetworkingSockets CONFIG REQUIRED)

set(GNS_WRAPPER_SOURCES
    src/gns_wrapper.cpp
)
if(WIN32)
    list(APPEND GNS_WRAPPER_SOURCES src/gns_wrapper.def)
endif()

add_library(GNS_Wrapper SHARED ${GNS_WRAPPER_SOURCES})

if(WIN32)
    set_target_properties(GNS_Wrapper PROPERTIES
        PREFIX ""
        OUTPUT_NAME "GNS_Wrapper"
    )
else()
    set_target_properties(GNS_Wrapper PROPERTIES
        PREFIX "lib"
        OUTPUT_NAME "GNS_Wrapper"
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Link against whichever imported target GameNetworkingSockets exports (shared preferred)
if(TARGET GameNetworkingSockets::GameNetworkingSockets)
    target_link_libraries(GNS_Wrapper PRIVATE GameNetworkingSockets::GameNetworkingSockets)
elseif(TARGET GameNetworkingSockets::shared)
    target_link_libraries(GNS_Wrapper PRIVATE GameNetworkingSockets::shared)
elseif(TARGET GameNetworkingSockets::GameNetworkingSockets_s)
    target_link_libraries(GNS_Wrapper PRIVATE GameNetworkingSockets::GameNetworkingSockets_s)
else()
    message(FATAL_ERROR "Could not locate GameNetworkingSockets CMake target.")
endif()

if(MSVC)
    target_compile_options(GNS_Wrapper PRIVATE /W4 /EHsc)
    target_compile_definitions(GNS_Wrapper PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(GNS_Wrapper PRIVATE -Wall -Wextra -fvisibility=hidden)
endif()
