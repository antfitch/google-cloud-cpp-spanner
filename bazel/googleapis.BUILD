# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # Apache 2.0

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_proto_library(
    name = "spanner_cc_proto",
    deps = ["//google/spanner/v1:spanner_proto"],
)

cc_grpc_library(
    name = "grpc_spanner_proto",
    srcs = ["//google/spanner/v1:spanner_proto"],
    deps = [":spanner_cc_proto"],
    grpc_only = True,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "spanner_admin_instance_cc_proto",
    deps = ["//google/spanner/admin/instance/v1:spanner_admin_instance_proto"],
)

# Instance Admin
cc_grpc_library(
    name = "google_spanner_admin_instance_v1_spanner_instance_admin",
    srcs = ["//google/spanner/admin/instance/v1:spanner_admin_instance_proto"],
    deps = [
        ":spanner_admin_instance_cc_proto",
        ":spanner_cc_proto",
        "//google/longrunning:longrunning_cc_grpc",
    ],
    grpc_only = True,
    well_known_protos = True,
    use_external = True,
)

cc_proto_library(
    name = "spanner_admin_database_cc_proto",
    deps = ["//google/spanner/admin/database/v1:spanner_admin_database_proto"],
)

# Database Admin
cc_grpc_library(
    name = "google_spanner_admin_database_v1_spanner_database_admin",
    srcs = ["//google/spanner/admin/database/v1:spanner_admin_database_proto"],
    deps = [
        ":spanner_admin_database_cc_proto",
        "//google/longrunning:longrunning_cc_grpc",
    ],
    grpc_only = True,
    well_known_protos = True,
    use_external = True,
)

cc_library(
    name = "spanner_protos",
    deps = [
        ":grpc_spanner_proto",
        ":google_spanner_admin_instance_v1_spanner_instance_admin",
        ":google_spanner_admin_database_v1_spanner_database_admin",
        "@com_github_grpc_grpc//:grpc++",
    ],
    includes = ["."],
)

cc_library(
    name = "grpc_utils_protos",
    includes = [
        ".",
    ],
    deps = [
        "@com_github_grpc_grpc//:grpc++",
        "//google/rpc:status_cc_proto",
    ],
)
