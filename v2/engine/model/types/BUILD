cc_library(
    name = "decimal-number",
    deps = ["@com_google_absl//absl/strings"],
    srcs = ["decimal-number.cc"],
    hdrs = ["decimal-number.h", "decimal-number.hpp"],
    visibility = ["//model:__pkg__"],
)

cc_library(
    name = "date",
    deps = ["@com_google_absl//absl/time"],
    hdrs = ["date.h"],
    visibility = ["//model:__pkg__"],
)
cc_test(
    name = "date_test",
    deps = ["@com_google_absl//absl/time",
            "@com_google_googletest//:gtest_main",
            ":date"],
    srcs = ["date_test.cc"],
)