include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        d141cdbeb0fb422a3fb7173b285fd38e0d1772dc # 8.0.1
)

FetchContent_MakeAvailable(fmt)
