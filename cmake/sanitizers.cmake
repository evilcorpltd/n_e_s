# Clang and gcc sanitizers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "Sanitizers:")

    option(ADDRESS_SANITIZER "Enable the address sanitizer" OFF)
    message(STATUS "  + ADDRESS_SANITIZER                     ${ADDRESS_SANITIZER}")
    if(ADDRESS_SANITIZER)
        add_compile_options(
           -fsanitize=address
           -fno-omit-frame-pointer
        )
        link_libraries(-fsanitize=address)
    endif()

    option(UNDEFINED_SANITIZER "Enable the undefined sanitizer" OFF)
    message(STATUS "  + UNDEFINED_SANITIZER                   ${UNDEFINED_SANITIZER}")
    if(UNDEFINED_SANITIZER)
        add_compile_options(-fsanitize=undefined)
        link_libraries(-fsanitize=undefined)
    endif()
endif()
