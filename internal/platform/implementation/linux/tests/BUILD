load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_test(
    name = "bluetooth_classic_medium_test",
    size = "small",
    srcs = ["bluetooth_classic_medium_test.cc"],
    deps = [
        "//internal/platform/implementation/linux:comm",
        "//internal/platform/implementation/linux:linux",
        "//internal/platform:cancellation_flag",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "network_safety_test",
    size = "small",
    srcs = ["network_safety_test.cc"],
    deps = [
        "//internal/platform/implementation:platform",
        "//internal/platform/implementation/linux:linux",
        "//internal/platform/implementation/linux:network_safety",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "stream_test",
    size = "small",
    srcs = ["stream_test.cc"],
    deps = [
        "//internal/platform/implementation/linux:comm",
        "//internal/platform/implementation/linux:linux",
        "//internal/platform:base",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "linux_connections_test",
    size = "small",
    srcs = [
        "atomic_boolean_test.cc",
        "atomic_reference_test.cc",
        "mutex_test.cc",
        "utils_test.cc",
    ],
    tags = ["notap"],
    deps = [
        "//internal/platform/implementation/linux:comm",
        "//internal/platform/implementation/linux:crypto",
        "//internal/platform/implementation/linux:linux",
        "//internal/platform/implementation/linux:test_utils",
        "//internal/platform/implementation/linux:types",
        "//internal/platform:base",
        "//internal/platform/implementation:comm",
        "//internal/platform/implementation:platform",
        "//internal/platform/implementation:types",
        "//internal/platform/implementation/shared:count_down_latch",
        "@com_github_protobuf_matchers//protobuf-matchers",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/time",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
        "@nlohmann_json//:json",
    ],
)
