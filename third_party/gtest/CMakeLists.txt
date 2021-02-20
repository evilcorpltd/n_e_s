include(FetchContent)

# v1.10.0 + fix for build failing due to std::result_of being removed in C++20.
# See: https://github.com/google/googletest/commit/61f010d703b32de9bfb20ab90ece38ab2f25977f
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        61f010d703b32de9bfb20ab90ece38ab2f25977f
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

