workspace(name = "nes")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "gtest",
    commit = "703bd9caab50b139428cea1aaff9974ebee5742e",
    remote = "https://github.com/google/googletest",
    shallow_since = "1570114335 -0400",
)

new_git_repository(
    name = "fmtlib",
    remote = "https://github.com/fmtlib/fmt",
    commit = "4fe0b11195b7cd71f39253c44db2c9dddf6b82d4",
    build_file = "@nes//third_party:BUILD.fmtlib",
    shallow_since = "1603647862 -0700",
)
