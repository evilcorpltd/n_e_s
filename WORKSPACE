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
    commit = "b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9",  # 8.1.1
    remote = "https://github.com/fmtlib/fmt",
    shallow_since = "1641508515 -0800",
)
