cc_library(
    name = "commodity-manager",
    deps = [":object-manager",
            ":validators",
            ":institution-manager",
            "@com_google_absl//absl/status:status",
            "//model/proto:commodity_cc_proto"],
    srcs = ["commodity-manager.cc"],
    hdrs = ["commodity-manager.h"]
)

cc_library(
    name = "icon-manager",
    deps = [":object-manager",
            "@com_google_absl//absl/status:status",
            "//model/proto:icon_cc_proto"],
    srcs = ["icon-manager.cc"],
    hdrs = ["icon-manager.h"]
)

cc_library(
    name = "institution-manager",
    deps = [":object-manager",
            ":validators",
            "@com_google_absl//absl/status:status",
            "//model/proto:institution_cc_proto"],
    srcs = ["institution-manager.cc"],
    hdrs = ["institution-manager.h"]
)

cc_library(
    name = "object-manager",
    deps = ["@com_google_absl//absl/strings",
            "@com_google_absl//absl/status:status",
            "@com_google_absl//absl/status:statusor",
            "//util:indexed-vector",
            "//util:status-util"],
    hdrs = ["object-manager.h"],
)

cc_test(
    name = "object-manager_test",
    deps = ["@com_google_googletest//:gtest_main",
            "//model/proto:institution_cc_proto",
            ":object-manager"],
    srcs = ["object-manager_test.cc"],
)

cc_library(
    name = "payee-manager",
    deps = [":object-manager",
            ":validators",
            "@com_google_absl//absl/status:status",
            "//model/proto:payee_cc_proto",
            "//model/proto:transaction_cc_proto"],
    srcs = ["payee-manager.cc"],
    hdrs = ["payee-manager.h"]
)

cc_library(
    name = "price-manager",
    deps = ["//model/types:date",
    "@com_google_absl//absl/strings",
    "@com_google_absl//absl/container:btree"],
    hdrs = ["price-manager.h"],
    srcs = ["price-manager.cc"],
)

cc_test(
    name = "price-manager_test",
    deps = ["@com_google_googletest//:gtest_main",
            ":price-manager"],
    srcs = ["price-manager_test.cc"],
)

cc_library(
    name = "validators",
    deps = ["@com_google_absl//absl/strings",
            "@com_google_absl//absl/status:status"],
    hdrs = ["validators.h"],
)

cc_test(
    name = "validators_test",
    deps = ["@com_google_googletest//:gtest_main",
            "//model/proto:institution_cc_proto",
            ":validators",
            ":object-manager"],
    srcs = ["validators_test.cc"],
)

