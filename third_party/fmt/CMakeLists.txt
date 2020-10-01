include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        cd4af11efc9c622896a3e4cb599fa28668ca3d05 # 7.0.3
)

FetchContent_MakeAvailable(fmt)
