cc_library(
  name = "augmented-treap-map",
  hdrs = ["augmented-treap-map.h"],
  deps = ["//util/treap:treap-internal"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "indexed-vector",
  hdrs = ["indexed-vector.h"],
  visibility = ["//visibility:public"],
)

cc_test(
  name = "indexed-vector_test",
  srcs = ["indexed-vector_test.cc"],
  deps = ["@com_google_googletest//:gtest_main",
          ":indexed-vector"]
)

cc_library(
  name = "status-util",
  deps = ["@com_google_absl//absl/status:status",
          "@com_google_absl//absl/status:statusor",],
  hdrs = ["status-util.h"],
  visibility = ["//visibility:public"],
)