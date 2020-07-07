include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        5173a76ba49936d252a85ee49b7eb96e3dff4033 # 7.0.0
)

FetchContent_MakeAvailable(fmt)
