cc_library(
    name = "account-manager",
    deps = [":commodity-manager",
            ":icon-manager",
            ":institution-manager",
            ":object-manager",
            ":validator-helpers",
            ":validators",
            "@com_google_absl//absl/strings",
            "@com_google_absl//absl/status:status",
            "//model/proto:account_cc_proto"],
    srcs = ["account-manager.cc"],
    hdrs = ["account-manager.h"]
)

cc_test(
    name = "account-manager_test",
    deps = ["@com_google_googletest//:gtest_main",
            "//model/proto:account_cc_proto",
            ":account-manager"],
    srcs = ["account-manager_test.cc"],
)

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
    deps = [":icon-manager",
            ":object-manager",
            ":validators",
            ":validator-helpers",
            "@com_google_absl//absl/status:status",
            "//model/proto:institution_cc_proto"],
    srcs = ["institution-manager.cc"],
    hdrs = ["institution-manager.h"]
)

cc_library(
    name = "ledger",
    deps = ["@com_google_absl//absl/container:flat_hash_map",
            "@com_google_absl//absl/status:status",
            "//model/proto:transaction_cc_proto",
            "//model/types:date",
            "//util:augmented-treap-map"],
    srcs = ["ledger.cc"],
    hdrs = ["ledger.h"]
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
    deps = [":icon-manager",
            ":object-manager",
            ":validator-helpers",
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
    name = "transaction-manager",
    deps = [":account-manager",
            ":payee-manager",
            ":commodity-manager",
            ":ledger",
            ":object-manager",
            ":validator-helpers",
            "@com_google_absl//absl/status:status",
            "//model/proto:transaction_cc_proto",
            "//model/types:date"],
    srcs = ["transaction-manager.cc"],
    hdrs = ["transaction-manager.h"]
)

cc_library(
    name = "validator-helpers",
    deps = [":validators"],
    hdrs = ["validator-helpers.h"],
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

