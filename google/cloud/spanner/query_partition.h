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

#ifndef GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_QUERY_PARTITION_H_
#define GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_QUERY_PARTITION_H_

#include "google/cloud/spanner/connection.h"
#include "google/cloud/spanner/sql_statement.h"
#include "google/cloud/status_or.h"
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace spanner {
inline namespace SPANNER_CLIENT_NS {

class QueryPartition;

/**
 * Serializes an instance of `QueryPartition` for transmission to another
 * process.
 *
 * @param query_partition - instance to be serialized.
 *
 * @par Example
 *
 * @code
 * spanner::SqlStatement stmt("select * from Albums");
 * std::vector<spanner::QueryPartition> partitions =
 *   spanner_client.PartitionSql(stmt);
 * for (auto const& partition : partitions) {
 *   auto serialized_partition = spanner::SerializeQueryPartition(partition);
 *   if (serialized_partition.ok()) {
 *     SendToRemoteMachine(*serialized_partition);
 *   }
 * }
 * @endcode
 */
StatusOr<std::string> SerializeQueryPartition(
    QueryPartition const& query_partition);

/**
 * Deserializes the provided string into a `QueryPartition`, if able.
 *
 * Returned `Status` should be checked to determine if deserialization was
 * successful.
 *
 * @param serialized_query_partition
 *
 * @par Example
 *
 * @code
 * std::string serialized_partition = ReceiveFromRemoteMachine();
 * spanner::QueryPartition partition =
 *   spanner::DeserializeQueryPartition(serialized_partition);
 * auto rows = spanner_client.ExecuteSql(partition);
 * @endcode
 */
StatusOr<QueryPartition> DeserializeQueryPartition(
    std::string const& serialized_query_partition);

// Internal implementation details that callers should not use.
namespace internal {

QueryPartition MakeQueryPartition(std::string const& transaction_id,
                                  std::string const& session_id,
                                  std::string const& partition_token,
                                  SqlStatement const& sql_statement);
Connection::ExecuteSqlParams MakeExecuteSqlParams(
    QueryPartition const& query_partition);

}  // namespace internal

/**
 * The `QueryPartition` class is a regular type that represents a single slice
 * of a parallel SQL read.
 *
 * Instances of `QueryPartition` are created by `Client::PartitionSql`. Once
 * created, `QueryPartition` objects can be serialized, transmitted to separate
 * process, and used to read data in parallel using `Client::ExecuteSql`.
 */
class QueryPartition {
 public:
  /**
   * Constructs an instance of `QueryPartition` that is not associated with any
   * `SqlStatement`.
   */
  QueryPartition() = default;

  /// @name Copy and move
  ///@{
  QueryPartition(QueryPartition const&) = default;
  QueryPartition(QueryPartition&&) = default;
  QueryPartition& operator=(QueryPartition const&) = default;
  QueryPartition& operator=(QueryPartition&&) = default;
  ///@}

  /**
   * Accessor for the `SqlStatement` associated with this `QueryPartition`.
   */
  SqlStatement const& sql_statement() const { return sql_statement_; }

  /// @name Equality
  ///@{
  friend bool operator==(QueryPartition const& a, QueryPartition const& b);
  friend bool operator!=(QueryPartition const& a, QueryPartition const& b) {
    return !(a == b);
  }
  ///@}

 private:
  friend class QueryPartitionTester;
  friend QueryPartition internal::MakeQueryPartition(
      std::string const& transaction_id, std::string const& session_id,
      std::string const& partition_token, SqlStatement const& sql_statement);
  friend Connection::ExecuteSqlParams internal::MakeExecuteSqlParams(
      QueryPartition const& query_partition);
  friend StatusOr<std::string> SerializeQueryPartition(
      QueryPartition const& query_partition);
  friend StatusOr<QueryPartition> DeserializeQueryPartition(
      std::string const& serialized_query_partition);

  explicit QueryPartition(std::string transaction_id, std::string session_id,
                          std::string partition_token,
                          SqlStatement sql_statement);

  // Accessor methods for use by friends.
  std::string const& partition_token() const { return partition_token_; }
  std::string const& session_id() const { return session_id_; }
  std::string const& transaction_id() const { return transaction_id_; }

  std::string transaction_id_;
  std::string session_id_;
  std::string partition_token_;
  SqlStatement sql_statement_;
};

}  // namespace SPANNER_CLIENT_NS
}  // namespace spanner
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_SPANNER_GOOGLE_CLOUD_SPANNER_QUERY_PARTITION_H_