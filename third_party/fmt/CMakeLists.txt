include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        4fe0b11195b7cd71f39253c44db2c9dddf6b82d4 # 7.1.0
)

FetchContent_MakeAvailable(fmt)
