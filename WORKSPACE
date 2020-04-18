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
    commit = "5899267c47c12ce0be664d913440fd9be50f20b3",
    build_file = "@nes//third_party:BUILD.fmtlib",
    shallow_since = "1586705259 -0700",
)
