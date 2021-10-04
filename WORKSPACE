workspace(name = "nes")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "gtest",
    commit = "e2239ee6043f73722e7aa812a459f54a28552929",
    remote = "https://github.com/google/googletest",
    shallow_since = "1623433346 -0700",
)

new_git_repository(
    name = "fmtlib",
    build_file = "@nes//third_party:fmtlib.BUILD",
    commit = "d141cdbeb0fb422a3fb7173b285fd38e0d1772dc",
    remote = "https://github.com/fmtlib/fmt",
    shallow_since = "1625267673 -0700",
)
