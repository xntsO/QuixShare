load("@rules_cc//cc:cc_library.bzl", "cc_library")

licenses(["notice"])

alias(
    name = "timestamp",
    actual = "@com_google_protobuf//:timestamp_proto",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "timestamp_cc_proto",
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:protobuf"],
)
