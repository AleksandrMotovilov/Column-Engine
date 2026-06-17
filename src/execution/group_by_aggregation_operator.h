#pragma once

#include <algorithm>
#include <functional>
#include <unordered_set>
#include <memory>
#include <vector>

#include <unordered_map>
#include <boost/unordered_map.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_node_map.hpp>
#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"

#include "src/execution/aggregation_functions.h"
#include "src/execution/operator.h"
#include "src/kernel/batch.h"
#include "src/kernel/column_utils.h"

struct VectorCharHash {
    size_t operator()(const std::vector<char>& v) const;
};

#if defined(GROUP_BY_MAP_BOOST_UNORDERED)
using GroupByMap = boost::unordered_map<std::vector<char>, size_t, VectorCharHash>;
#elif defined(GROUP_BY_MAP_BOOST_FLAT)
using GroupByMap = boost::unordered_flat_map<std::vector<char>, size_t, VectorCharHash>;
#elif defined(GROUP_BY_MAP_BOOST_NODE)
using GroupByMap = boost::unordered_node_map<std::vector<char>, size_t, VectorCharHash>;
#elif defined(GROUP_BY_MAP_ABSL_FLAT)
using GroupByMap = absl::flat_hash_map<std::vector<char>, size_t, VectorCharHash>;
#elif defined(GROUP_BY_MAP_ABSL_NODE)
using GroupByMap = absl::node_hash_map<std::vector<char>, size_t, VectorCharHash>;
#else
using GroupByMap = std::unordered_map<std::vector<char>, size_t, VectorCharHash>;
#endif

void AppendToKey(std::vector<char>& key, std::shared_ptr<Column> column, Type type, size_t index);

class GroupByAggregationOperator : public Operator {
public:
    using AggregationFactory = std::function<std::vector<std::shared_ptr<AggregationFunction>>()>;

    GroupByAggregationOperator(std::shared_ptr<Operator> next, std::vector<std::string> group_by_columns, AggregationFactory aggregation_factory);
    std::shared_ptr<Batch> Next() override;

private:
    std::shared_ptr<Operator> next_;
    std::vector<std::string> group_by_columns_;
    AggregationFactory aggregation_factory_;
    bool done_;
};
