cmake_minimum_required(VERSION 3.20)
project(RelogicGNSWrapperRecomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GameNetworkingSockets CONFIG REQUIRED)

add_library(GNS_Wrapper SHARED
    src/gns_wrapper.cpp
    src/gns_wrapper.def
)

set_target_properties(GNS_Wrapper PROPERTIES
    PREFIX ""
    OUTPUT_NAME "GNS_Wrapper"
)

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
endif()
