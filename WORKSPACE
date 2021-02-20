workspace(name = "nes")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

# v1.10.0 + fix for build failing due to std::result_of being removed in C++20.
# See: https://github.com/google/googletest/commit/61f010d703b32de9bfb20ab90ece38ab2f25977f
git_repository(
    name = "gtest",
    commit = "61f010d703b32de9bfb20ab90ece38ab2f25977f",
    remote = "https://github.com/google/googletest",
    shallow_since = "1585697018 -0400",
)

new_git_repository(
    name = "fmtlib",
    remote = "https://github.com/fmtlib/fmt",
    commit = "4fe0b11195b7cd71f39253c44db2c9dddf6b82d4",
    build_file = "@nes//third_party:BUILD.fmtlib",
    shallow_since = "1603647862 -0700",
)
