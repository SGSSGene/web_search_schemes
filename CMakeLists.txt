# SPDX-FileCopyrightText: 2025 Simon Gene Gottlieb
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required (VERSION 4.0)

project (search_schemes.js LANGUAGES CXX C
                 VERSION 1.0.0)

find_package(OpenMP QUIET)
include(cmake/CPM.cmake)
CPMAddPackage("gh:SGSSGene/cpm.dependencies@1.0.0")
CPMLoadDependenciesFile("${CMAKE_CURRENT_SOURCE_DIR}/cpm.dependencies")

add_executable(search_schemes.js
    main.cpp
)
target_link_options(search_schemes.js PUBLIC
    -sFETCH
    -sNO_DISABLE_EXCEPTION_CATCHING
    -sALLOW_MEMORY_GROWTH=1
    -sTOTAL_STACK=4MB
)

target_link_libraries(search_schemes.js
    fmindex-collection::fmindex-collection
    fmt::fmt
    ivio::ivio
    ivsigma::ivsigma
    clice::clice
    embind
)
target_compile_features(search_schemes.js PUBLIC cxx_std_23)

