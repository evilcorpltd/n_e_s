cc_library(
    name = "fmtlib",
    srcs = [
        # "src/fmt.cc", # No C++ module support
        "src/format.cc",
        "src/os.cc",
    ],
    hdrs = glob([
        "include/**",
    ]),
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)
