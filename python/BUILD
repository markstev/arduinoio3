package(default_visibility = ["//visibility:public"])

py_library(
    name = "serial_control",
    srcs = ["serial_control.py"],
)

py_library(
    name = "hardware_abstraction",
    srcs = ["hardware_abstraction.py"],
)

py_library(
    name = "bidir_serial_control",
    srcs = ["bidir_serial_control.py"],
    deps = [
        ":hardware_abstraction",
        ":serial_control",
    ],
)

py_test(
    name = "hardware_abstraction_test",
    srcs = ["hardware_abstraction_test.py"],
    deps = [":hardware_abstraction"],
)

py_test(
    name = "bidir_serial_control_test",
    srcs = ["bidir_serial_control_test.py"],
    deps = [":bidir_serial_control",
            ":hardware_abstraction"],
)

py_test(
    name = "bidir_serial_integration_test",
    srcs = ["bidir_serial_integration_test.py"],
    deps = [":bidir_serial_control",
            ],
    data = ["//:bidir_serial_main_for_test"],
)


