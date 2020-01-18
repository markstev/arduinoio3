package(default_visibility = ["//visibility:public"])

cc_library(name = "arduino_simulator",
           srcs = ["arduino_simulator.cc"],
           hdrs = [
               "arduino.h",
               "arduino_simulator.h",
           ])

cc_test(name = "arduino_simulator_test",
        srcs = ["arduino_simulator_test.cc"],
        deps = [":arduino_simulator",
                "@google_googletest//:gtest",
                "@google_googletest//:gtest_main"
                ])

cc_library(
    name = "message",
    srcs = ["message.cc"],
    hdrs = ["message.h"],
)

cc_library(
    name = "uc_module",
    hdrs = ["uc_module.h"],
    deps = [":message"],
)

# cc_library(name = "motor_module",
#            srcs = ["motor_module.cc"],
#            hdrs = ["motor_module.h"],
#            deps = [
#                "//module_framework:message",
#                "//module_framework:uc_module",
#                "//hardware_abstraction:arduino_simulator",
#                ],
#            )

# cc_test(name = "motor_module_test",
#         srcs = ["motor_module_test.cc"],
#         deps = [
#             ":motor_module",
#             "//hardware_abstraction:arduino_simulator",
#             ],
#         )
cc_library(
    name = "serial_link",
    srcs = ["bidir_serial_module.cc"],
    hdrs = [
        "arduino.h",
        "bidir_serial_module.h",
    ],
    deps = [
        ":uc_module",
        ":message",
    ],
)

cc_test(
    name = "bidir_serial_module_test",
    srcs = ["bidir_serial_module_test.cc"],
    deps = [
        ":arduino_simulator",
        ":serial_link",
        "@google_googletest//:gtest",
        "@google_googletest//:gtest_main"
    ],
)

cc_binary(
    name = "bidir_serial_main_for_test",
    srcs = ["bidir_serial_main_for_test.cc"],
    deps = [
        ":serial_link",
        ":message",
        ":arduino_simulator",
    ],
)

cc_library(
    name = "arduinoio",
    srcs = ["arduinoio.cc"],
    hdrs = [
        "arduino.h",
        "arduinoio.h"
    ],
    deps = [
        ":message",
        ":uc_module",
        ":serial_link",
    ],
)
