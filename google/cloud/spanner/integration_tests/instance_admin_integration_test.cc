// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/spanner/instance_admin_client.h"
#include "google/cloud/spanner/testing/random_instance_name.h"
#include "google/cloud/spanner/update_instance_request_builder.h"
#include "google/cloud/internal/getenv.h"
#include "google/cloud/internal/random.h"
#include "google/cloud/testing_util/assert_ok.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {
namespace {

using ::testing::HasSubstr;
using ::testing::UnorderedElementsAre;

/// @test Verify the basic read operations for instances work.
TEST(InstanceAdminClient, InstanceReadOperations) {
  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  auto instance_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_SPANNER_INSTANCE")
          .value_or("");
  ASSERT_FALSE(project_id.empty());
  ASSERT_FALSE(instance_id.empty());

  Instance in(project_id, instance_id);

  InstanceAdminClient client(MakeInstanceAdminConnection());
  auto instance = client.GetInstance(in);
  EXPECT_STATUS_OK(instance);
  EXPECT_THAT(instance->name(), HasSubstr(project_id));
  EXPECT_THAT(instance->name(), HasSubstr(instance_id));
  EXPECT_NE(0, instance->node_count());

  std::vector<std::string> instance_names = [client, project_id]() mutable {
    std::vector<std::string> names;
    for (auto instance : client.ListInstances(project_id, "")) {
      EXPECT_STATUS_OK(instance);
      if (!instance) break;
      names.push_back(instance->name());
    }
    return names;
  }();
  EXPECT_EQ(1, std::count(instance_names.begin(), instance_names.end(),
                          instance->name()));
}

/// @test Verify the basic CRUD operations for instances work.
TEST(InstanceAdminClient, InstanceCRUDOperations) {
  auto run_slow_integration_tests =
      google::cloud::internal::GetEnv("RUN_SLOW_INTEGRATION_TESTS")
          .value_or("");
  if (run_slow_integration_tests != "yes") {
    GTEST_SKIP();
  }
  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  auto generator = google::cloud::internal::MakeDefaultPRNG();
  std::string instance_id =
      google::cloud::spanner_testing::RandomInstanceName(generator);
  ASSERT_FALSE(project_id.empty());
  ASSERT_FALSE(instance_id.empty());
  Instance in(project_id, instance_id);

  InstanceAdminClient client(MakeInstanceAdminConnection());
  std::vector<std::string> instance_config_names = [client,
                                                    project_id]() mutable {
    std::vector<std::string> names;
    for (auto instance_config : client.ListInstanceConfigs(project_id)) {
      EXPECT_STATUS_OK(instance_config);
      if (!instance_config) break;
      names.push_back(instance_config->name());
    }
    return names;
  }();
  ASSERT_FALSE(instance_config_names.empty());
  // Use the name of the first element from the list of instance configs.
  auto instance_config = instance_config_names[0];

  future<StatusOr<google::spanner::admin::instance::v1::Instance>> f =
      client.CreateInstance(
          project_id, instance_id, "test-display-name", instance_config, 1,
          std::map<std::string, std::string>{{"label-key", "label-value"}});
  StatusOr<google::spanner::admin::instance::v1::Instance> instance = f.get();

  EXPECT_STATUS_OK(instance.status());
  EXPECT_THAT(instance->name(), HasSubstr(project_id));
  EXPECT_THAT(instance->name(), HasSubstr(instance_id));
  EXPECT_EQ("test-display-name", instance->display_name());
  EXPECT_NE(0, instance->node_count());
  EXPECT_NE(0, instance->labels_size());
  EXPECT_EQ(instance_config, instance->config());
  EXPECT_EQ("label-value", instance->labels().at("label-key"));

  // Then update the instance
  f = client.UpdateInstance(std::move(UpdateInstanceRequestBuilder(*instance)
                                          .SetDisplayName("New display name")
                                          .AddLabels({{"new-key", "new-value"}})
                                          .SetNodeCount(2))
                                .Build());
  instance = f.get();
  EXPECT_EQ("New display name", instance->display_name());
  EXPECT_EQ(2, instance->labels_size());
  EXPECT_EQ("new-value", instance->labels().at("new-key"));
  EXPECT_EQ(2, instance->node_count());
  auto status = client.DeleteInstance(in);
  EXPECT_STATUS_OK(status);
}

TEST(InstanceAdminClient, InstanceConfig) {
  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  ASSERT_FALSE(project_id.empty());

  InstanceAdminClient client(MakeInstanceAdminConnection());

  std::vector<std::string> instance_config_names = [client,
                                                    project_id]() mutable {
    std::vector<std::string> names;
    for (auto instance_config : client.ListInstanceConfigs(project_id)) {
      EXPECT_STATUS_OK(instance_config);
      if (!instance_config) break;
      names.push_back(instance_config->name());
    }
    return names;
  }();
  ASSERT_FALSE(instance_config_names.empty());
  // Use the name of the first element from the list of instance configs.
  auto instance_config = client.GetInstanceConfig(instance_config_names[0]);
  EXPECT_STATUS_OK(instance_config);
  EXPECT_THAT(instance_config->name(), HasSubstr(project_id));
  EXPECT_EQ(
      1, std::count(instance_config_names.begin(), instance_config_names.end(),
                    instance_config->name()));
}

TEST(InstanceAdminClient, InstanceIam) {
  auto run_slow_integration_tests =
      google::cloud::internal::GetEnv("RUN_SLOW_INTEGRATION_TESTS")
          .value_or("");
  auto project_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_PROJECT").value_or("");
  auto instance_id =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_SPANNER_INSTANCE")
          .value_or("");
  auto test_iam_service_account =
      google::cloud::internal::GetEnv("GOOGLE_CLOUD_CPP_SPANNER_IAM_TEST_SA")
          .value_or("");
  ASSERT_FALSE(project_id.empty());
  ASSERT_FALSE(instance_id.empty());
  ASSERT_FALSE(test_iam_service_account.empty());

  Instance in(project_id, instance_id);

  InstanceAdminClient client(MakeInstanceAdminConnection());

  auto actual_policy = client.GetIamPolicy(in);
  ASSERT_STATUS_OK(actual_policy);
  EXPECT_FALSE(actual_policy->etag().empty());

  if (run_slow_integration_tests == "yes") {
    // Set the policy to the existing value of the policy. While this changes
    // nothing it tests all the code in the client library.
    auto updated_policy = client.SetIamPolicy(in, *actual_policy);
    ASSERT_STATUS_OK(updated_policy);
    EXPECT_FALSE(actual_policy->etag().empty());
  }

  auto actual = client.TestIamPermissions(
      in, {"spanner.databases.list", "spanner.databases.get"});
  ASSERT_STATUS_OK(actual);
  EXPECT_THAT(
      actual->permissions(),
      UnorderedElementsAre("spanner.databases.list", "spanner.databases.get"));
}

}  // namespace
}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google