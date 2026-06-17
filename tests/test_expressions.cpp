#include "gtest/gtest.h"
#include "src/execution/expressions.h"
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

std::string GetResultValue(std::shared_ptr<Column> column, size_t index, Type column_type) {
    switch (column_type) {
        case Type::Char:
            return std::string(1, static_cast<const ColumnTyped<char>&>(*column).GetData()[index]);
        case Type::Int16:
            return ToString<int16_t>(static_cast<const ColumnTyped<int16_t>&>(*column).GetData()[index]);
        case Type::Int32:
            return ToString<int32_t>(static_cast<const ColumnTyped<int32_t>&>(*column).GetData()[index]);
        case Type::Int64:
            return ToString<int64_t>(static_cast<const ColumnTyped<int64_t>&>(*column).GetData()[index]);
        case Type::Float:
            return ToString<float>(static_cast<const ColumnTyped<float>&>(*column).GetData()[index]);
        case Type::Double:
            return ToString<double>(static_cast<const ColumnTyped<double>&>(*column).GetData()[index]);
        case Type::String:
            return static_cast<const ColumnTyped<std::string>&>(*column).GetData()[index];
        case Type::Timestamp:
            return ToString<Timestamp>(static_cast<const ColumnTyped<Timestamp>&>(*column).GetData()[index]);
        case Type::Date:
            return ToString<Date>(static_cast<const ColumnTyped<Date>&>(*column).GetData()[index]);
        default:
            throw std::runtime_error("Unknown column type in GetResultValue");
    }
}

TEST(Expressions, Equal) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            4,
            {Type::Int32},
            {"id"},
            {{"1", "2", "3", "2"}}
        );
        EqualExpression expr("id", "2");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 4u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,3, Type::Char), "1");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Int32},
            {"x"},
            {{"1", "2"}}
        );
        EqualExpression expr("missing", "1");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, NotEqual) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            4,
            {Type::String},
            {"name"},
            {{"alice", "", "bob", ""}}
        );
        NotEqualExpression expr("name", "");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 4u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,3, Type::Char), "0");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Int32},
            {"x"},
            {{"1", "2"}}
        );
        NotEqualExpression expr("missing", "0");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, Contains) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            4,
            {Type::String},
            {"url"},
            {{"https://google.com", "https://example.com", "http://google.ru", "https://bing.com"}}
        );
        ContainsExpression expr("url", "google");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 4u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,3, Type::Char), "0");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::String},
            {"s"},
            {{"a", "b"}}
        );
        ContainsExpression expr("missing", "a");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, NotContains) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            4,
            {Type::String},
            {"url"},
            {{"https://google.com", "https://example.com", "http://google.ru", "https://bing.com"}}
        );
        NotContainsExpression expr("url", "google");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 4u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,3, Type::Char), "1");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::String},
            {"s"},
            {{"a", "b"}}
        );
        NotContainsExpression expr("missing", "a");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, GreaterOrEqual) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            4,
            {Type::Int32},
            {"score"},
            {{"5", "10", "3", "10"}}
        );
        GreaterOrEqualExpression expr("score", "10");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 4u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,3, Type::Char), "1");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::Date},
            {"d"},
            {{"2013-07-01", "2013-06-30", "2013-07-15"}}
        );
        GreaterOrEqualExpression expr("d", "2013-07-01");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::String},
            {"name"},
            {{"apple", "banana", "cherry"}}
        );
        GreaterOrEqualExpression expr("name", "banana");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Int32},
            {"x"},
            {{"1", "2"}}
        );
        GreaterOrEqualExpression expr("missing", "1");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, LessOrEqual) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            4,
            {Type::Int32},
            {"score"},
            {{"5", "10", "3", "10"}}
        );
        LessOrEqualExpression expr("score", "5");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 4u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,3, Type::Char), "0");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::Date},
            {"d"},
            {{"2013-07-31", "2013-08-01", "2013-07-15"}}
        );
        LessOrEqualExpression expr("d", "2013-07-31");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::String},
            {"name"},
            {{"apple", "banana", "cherry"}}
        );
        LessOrEqualExpression expr("name", "banana");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,1, Type::Char), "1");
        EXPECT_EQ(GetResultValue(result,2, Type::Char), "0");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Int32},
            {"x"},
            {{"1", "2"}}
        );
        LessOrEqualExpression expr("missing", "1");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, Constant) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        3,
        {Type::String},
        {"url"},
        {{"a", "b", "c"}}
    );
    ConstantExpression expr("42");
    std::shared_ptr<Column> result = expr.Eval(batch);
    ASSERT_EQ(result->GetSize(), 3u);
    EXPECT_EQ(GetResultValue(result,0, Type::String), "42");
    EXPECT_EQ(GetResultValue(result,1, Type::String), "42");
    EXPECT_EQ(GetResultValue(result,2, Type::String), "42");
}

TEST(Expressions, SumExpression) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::Int64},
            {"x"},
            {{"10", "20", "30"}}
        );
        SumExpression expr("x", 5);
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Int64), "15");
        EXPECT_EQ(GetResultValue(result,1, Type::Int64), "25");
        EXPECT_EQ(GetResultValue(result,2, Type::Int64), "35");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::Int64},
            {"ip"},
            {{"100", "200", "300"}}
        );
        SumExpression expr("ip", -1);
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Int64), "99");
        EXPECT_EQ(GetResultValue(result,1, Type::Int64), "199");
        EXPECT_EQ(GetResultValue(result,2, Type::Int64), "299");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Int64},
            {"x"},
            {{"1", "2"}}
        );
        SumExpression expr("missing", -1);
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, And) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::String, Type::String},
        {"url", "phrase"},
        {{"https://google.com", "https://example.com", "http://google.ru", "https://bing.com"},
         {"google", "", "maps", ""}}
    );
    AndExpression expr(std::make_shared<ContainsExpression>("url", "google"), std::make_shared<NotEqualExpression>("phrase", ""));
    std::shared_ptr<Column> result = expr.Eval(batch);
    ASSERT_EQ(result->GetSize(), 4u);
    EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
    EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
    EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
    EXPECT_EQ(GetResultValue(result,3, Type::Char), "0");
}

TEST(Expressions, Or) {
    SetBatchSize(2, 2);

    std::shared_ptr<Batch> batch = MakeBatch(
        4,
        {Type::String, Type::String},
        {"url", "phrase"},
        {{"https://google.com", "https://example.com", "http://google.ru", "https://bing.com"}, {"google", "", "", ""}}
    );
    OrExpression expr(std::make_shared<ContainsExpression>("url", "google"), std::make_shared<NotEqualExpression>("phrase", ""));
    std::shared_ptr<Column> result = expr.Eval(batch);
    ASSERT_EQ(result->GetSize(), 4u);
    EXPECT_EQ(GetResultValue(result,0, Type::Char), "1");
    EXPECT_EQ(GetResultValue(result,1, Type::Char), "0");
    EXPECT_EQ(GetResultValue(result,2, Type::Char), "1");
    EXPECT_EQ(GetResultValue(result,3, Type::Char), "0");
}

TEST(Expressions, Length) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::String},
            {"url"},
            {{"hello", "world!", "ab"}}
        );
        LengthExpression expr("url");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::Int64), "5");
        EXPECT_EQ(GetResultValue(result,1, Type::Int64), "6");
        EXPECT_EQ(GetResultValue(result,2, Type::Int64), "2");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::String},
            {"x"},
            {{"a", "b"}}
        );
        LengthExpression expr("missing");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, RegexpReplace) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::String},
            {"ref"},
            {{"http://www.google.com/search", "https://example.org/page/1", "http://test.com/x"}}
        );
        RegexpReplaceExpression expr("ref", R"(^https?://(?:www\.)?([^/]+)/.*$)", "\\1");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::String), "google.com");
        EXPECT_EQ(GetResultValue(result,1, Type::String), "example.org");
        EXPECT_EQ(GetResultValue(result,2, Type::String), "test.com");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::String},
            {"ref"},
            {{"not_a_url", "also_not"}}
        );
        RegexpReplaceExpression expr("ref", R"(^https?://(?:www\.)?([^/]+)/.*$)", "\\1");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 2u);
        EXPECT_EQ(GetResultValue(result,0, Type::String), "not_a_url");
        EXPECT_EQ(GetResultValue(result,1, Type::String), "also_not");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::String},
            {"x"},
            {{"a", "b"}}
        );
        RegexpReplaceExpression expr("missing", ".*", "\\0");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, CaseWhen) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            3,
            {Type::Int32, Type::String},
            {"x", "name"},
            {{"1", "2", "1"}, {"alice", "bob", "carol"}}
        );
        CaseWhenExpression expr(std::make_shared<EqualExpression>("x", "1"), "name", "unknown");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 3u);
        EXPECT_EQ(GetResultValue(result,0, Type::String), "alice");
        EXPECT_EQ(GetResultValue(result,1, Type::String), "unknown");
        EXPECT_EQ(GetResultValue(result,2, Type::String), "carol");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Int32},
            {"x"},
            {{"1", "2"}}
        );
        CaseWhenExpression expr(std::make_shared<EqualExpression>("x", "1"), "missing", "default");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, ExtractMinute) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Timestamp},
            {"ts"},
            {{"2021-01-01 00:37:45", "2021-01-01 01:02:10"}}
        );
        ExtractMinuteExpression expr("ts");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 2u);
        EXPECT_EQ(GetResultValue(result,0, Type::Int64), "37");
        EXPECT_EQ(GetResultValue(result,1, Type::Int64), "2");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            1,
            {Type::Timestamp},
            {"ts"},
            {{"1970-01-01 00:00:00"}});
        ExtractMinuteExpression expr("missing");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}

TEST(Expressions, TruncateToMinute) {
    SetBatchSize(2, 2);

    {
        std::shared_ptr<Batch> batch = MakeBatch(
            2,
            {Type::Timestamp},
            {"ts"},
            {{"2021-01-01 00:37:45", "2021-01-01 01:02:10"}}
        );
        TruncateToMinuteExpression expr("ts");
        std::shared_ptr<Column> result = expr.Eval(batch);
        ASSERT_EQ(result->GetSize(), 2u);
        EXPECT_EQ(GetResultValue(result,0, Type::Timestamp), "2021-01-01 00:37:00");
        EXPECT_EQ(GetResultValue(result,1, Type::Timestamp), "2021-01-01 01:02:00");
    }
    {
        std::shared_ptr<Batch> batch = MakeBatch(
            1,
            {Type::Timestamp},
            {"ts"},
            {{"1970-01-01 00:00:00"}});
        TruncateToMinuteExpression expr("missing");
        EXPECT_THROW(expr.Eval(batch), std::runtime_error);
    }
}
