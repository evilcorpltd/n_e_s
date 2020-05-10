include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        567ed03f88d493b51e5efc24f9f4cad47dc5c76d # 6.2.0 with build fix for -Wshadow
)

FetchContent_MakeAvailable(fmt)
