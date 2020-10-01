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
    commit = "cd4af11efc9c622896a3e4cb599fa28668ca3d05",
    build_file = "@nes//third_party:BUILD.fmtlib",
    shallow_since = "1593963827 -0700",
)
