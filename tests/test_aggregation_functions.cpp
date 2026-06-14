#include <cstring>
#include "gtest/gtest.h"
#include "src/execution/aggregation_functions.h"
#include "src/kernel/column_utils.h"

std::shared_ptr<Batch> MakeBatch(
    size_t rows,
    std::vector<Type> types,
    std::vector<std::string> names,
    std::vector<std::vector<std::string>> values)
{
    std::vector<std::shared_ptr<Column>> cols;
    for (size_t j = 0; j < types.size(); j++) {
        cols.push_back(MakeColumnFromStrings(types[j], values[j]));
    }
    return std::make_shared<Batch>(rows, std::make_shared<Schema>(std::move(names), std::move(types)), std::move(cols));
}

std::string GetResultString(AggregationFunction& agg) {
    std::vector<char> buf;
    agg.AppendResultBytes(buf);
    switch (agg.GetType()) {
        case Type::Int16: {
            int16_t val;
            std::memcpy(&val, buf.data(), sizeof(val));
            return ToString<int16_t>(val);
        }
        case Type::Int32: {
            int32_t val;
            std::memcpy(&val, buf.data(), sizeof(val));
            return ToString<int32_t>(val);
        }
        case Type::Int64: {
            int64_t val;
            std::memcpy(&val, buf.data(), sizeof(val));
            return ToString<int64_t>(val);
        }
        case Type::Float: {
            float val;
            std::memcpy(&val, buf.data(), sizeof(val));
            return ToString<float>(val);
        }
        case Type::Double: {
            double val;
            std::memcpy(&val, buf.data(), sizeof(val));
            return ToString<double>(val);
        }
        case Type::Date: {
            int32_t raw;
            std::memcpy(&raw, buf.data(), sizeof(raw));
            return ToString<Date>(Date(raw));
        }
        case Type::Timestamp: {
            int64_t raw;
            std::memcpy(&raw, buf.data(), sizeof(raw));
            return ToString<Timestamp>(Timestamp(raw));
        }
        case Type::Char: {
            char val;
            std::memcpy(&val, buf.data(), sizeof(val));
            return ToString<char>(val);
        }
        case Type::String: {
            size_t len;
            std::memcpy(&len, buf.data(), sizeof(size_t));
            return std::string(buf.data() + sizeof(size_t), len);
        }
        default:
            throw std::runtime_error("Unknown type in GetResultString");
    }
}

TEST(AggregationFunctions, CountRows) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        3,
        {Type::Int32},
        {"x"},
        {{"1", "2", "3"}}
    );
    CountRowsAggregation aggregation;
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "3");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "count(*)");
}

TEST(AggregationFunctions, CountDistinct) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        5,
        {Type::String},
        {"name"},
        {{"alice", "bob", "alice", "carol", "bob"}}
    );
    CountDistinctAggregation aggregation("name");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "3");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "count(distinct name)");
}

TEST(AggregationFunctions, CountDistinct_column_not_found) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(2, {Type::Int32}, {"x"}, {{"1", "2"}});
    CountDistinctAggregation aggregation("missing");
    EXPECT_THROW(aggregation.Update(batch), std::runtime_error);
}

TEST(AggregationFunctions, Sum) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::Int64},
        {"val"},
        {{"10", "20", "30", "40"}}
    );
    SumAggregation aggregation("val");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "100");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "sum(val)");
}

TEST(AggregationFunctions, Sum_column_not_found) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Int64}, {"val"}, {{"1", "2", "3"}});
    SumAggregation aggregation("missing");
    EXPECT_THROW(aggregation.Update(batch), std::runtime_error);
}

TEST(AggregationFunctions, Avg) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        3,
        {Type::Int64},
        {"score"},
        {{"10", "20", "30"}});
    AvgAggregation aggregation("score");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "20");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "avg(score)");
}

TEST(AggregationFunctions, Avg_integer_division) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(2, {Type::Int64}, {"v"}, {{"1", "2"}});
    AvgAggregation aggregation("v");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "1");
}

TEST(AggregationFunctions, Avg_column_not_found) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Int64}, {"score"}, {{"10", "20", "30"}});
    AvgAggregation aggregation("missing");
    EXPECT_THROW(aggregation.Update(batch), std::runtime_error);
}

TEST(AggregationFunctions, Min_int) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::Int64},
        {"n"},
        {{"5", "1", "3", "2"}});
    MinAggregation aggregation("n");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "1");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "min(n)");
}

TEST(AggregationFunctions, Min_column_not_found) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Int64}, {"n"}, {{"5", "1", "3"}});
    MinAggregation aggregation("missing");
    EXPECT_THROW(aggregation.Update(batch), std::runtime_error);
}

TEST(AggregationFunctions, Max_int) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::Int64},
        {"n"},
        {{"5", "1", "3", "2"}});
    MaxAggregation aggregation("n");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "5");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "max(n)");
}

TEST(AggregationFunctions, Max_column_not_found) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Int64}, {"n"}, {{"5", "1", "3"}});
    MaxAggregation aggregation("missing");
    EXPECT_THROW(aggregation.Update(batch), std::runtime_error);
}

TEST(AggregationFunctions, Min_date) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Date}, {"d"}, {{"2020-01-01", "2019-06-15", "2021-03-10"}});
    MinAggregation aggregation("d");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "2019-06-15");
    EXPECT_EQ(aggregation.GetType(), Type::Date);
}

TEST(AggregationFunctions, Max_date) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Date}, {"d"}, {{"2020-01-01", "2019-06-15", "2021-03-10"}});
    MaxAggregation aggregation("d");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "2021-03-10");
    EXPECT_EQ(aggregation.GetType(), Type::Date);
}

TEST(AggregationFunctions, Min_string) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::String},
        {"url"},
        {{"http://bing.com", "http://apple.com", "http://zz.com", "http://google.com"}}
    );
    MinAggregation aggregation("url");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "http://apple.com");
    EXPECT_EQ(aggregation.GetType(), Type::String);
    EXPECT_EQ(aggregation.GetName(), "min(url)");
}

TEST(AggregationFunctions, Max_string) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::String},
        {"url"},
        {{"http://bing.com", "http://apple.com", "http://zz.com", "http://google.com"}}
    );
    MaxAggregation aggregation("url");
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "http://zz.com");
    EXPECT_EQ(aggregation.GetType(), Type::String);
    EXPECT_EQ(aggregation.GetName(), "max(url)");
}

TEST(AggregationFunctions, SumWithOffset_zero) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Int32}, {"v"}, {{"10", "20", "30"}});
    SumWithOffsetAggregation aggregation("v", 0);
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "60");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "sum(v+0)");
}

TEST(AggregationFunctions, SumWithOffset_nonzero) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(3, {Type::Int32}, {"v"}, {{"10", "20", "30"}});
    SumWithOffsetAggregation aggregation("v", 5);
    aggregation.Update(batch);
    EXPECT_EQ(GetResultString(aggregation), "75");
    EXPECT_EQ(aggregation.GetType(), Type::Int64);
    EXPECT_EQ(aggregation.GetName(), "sum(v+5)");
}
