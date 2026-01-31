#include "gtest/gtest.h"
#include <filesystem>
#include <memory>
#include "from_csv_to_clmn.cpp"
#include "from_clmn_to_csv.cpp"

bool CompareSchema(std::shared_ptr<Schema> schema_left, std::shared_ptr<Schema> schema_right) {
    if (schema_left->ColumnsNumber() != schema_right->ColumnsNumber()) {
        return false;
    }
    size_t n = schema_left->ColumnsNumber();
    for (size_t i = 0; i < n; i++) {
        if (schema_left->GetName(i) != schema_right->GetName(i) || schema_left->GetType(i) != schema_right->GetType(i)) {
            return false;
        }
    }
    return true;
}

TEST(Test_Schema, Schema_From_CSV) {
    std::string task_dir = "test_schema_from_csv";
    std::filesystem::create_directories(task_dir);
    std::string file = task_dir + "/schema.csv";
    std::ofstream f1(file);
    f1 << "a,int64\n";
    f1 << "b,int64\n";
    f1 << "name123,string\n";
    f1 << "d,int64\n";
    f1.close();
    std::shared_ptr<Schema> schema_ans = std::make_shared<Schema>();
    schema_ans->Push("a", Type::INT64);
    schema_ans->Push("b", Type::INT64);
    schema_ans->Push("name123", Type::STRING);
    schema_ans->Push("d", Type::INT64);
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCSV(file);
    ASSERT_TRUE(CompareSchema(schema, schema_ans));
}

TEST(Test_Schema, Schema_From_CLMN) {
    std::string task_dir = "test_schema_from_clmn";
    std::filesystem::create_directories(task_dir);
    std::string file = task_dir + "/columnar.clmn";
    std::ofstream f(file);
    f << "1,5,8\n";
    f << "2,1,17\n";
    f << "first,second,third\n";
    f << "4,2,2\n";
    f << "a,int64\n";
    f << "b,int64\n";
    f << "name123,string\n";
    f << "d,int64\n";
    std::string s = "a,int64\nb,int64\nname123,string\nd,int64\n";
    int64_t offset = s.size() + 8;
    f.write(reinterpret_cast<char*>(&offset), sizeof(offset));
    f.close();
    std::shared_ptr<Schema> schema_ans = std::make_shared<Schema>();
    schema_ans->Push("a", Type::INT64);
    schema_ans->Push("b", Type::INT64);
    schema_ans->Push("name123", Type::STRING);
    schema_ans->Push("d", Type::INT64);
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCLMN(file);
    ASSERT_TRUE(CompareSchema(schema, schema_ans));
}

TEST(Test_Schema, Schema_String) {
    std::string task_dir = "test_schema_string";
    std::filesystem::create_directories(task_dir);
    std::string file = task_dir + "/schema.csv";
    std::ofstream f1(file);
    f1 << "a,int64\n";
    f1 << "b,int64\n";
    f1 << "name123,string\n";
    f1 << "d,int64\n";
    f1.close();
    std::string ans = "a,int64\nb,int64\nname123,string\nd,int64\n";
    std::shared_ptr<Schema> schema_ans = std::make_shared<Schema>();
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCSV(file);
    ASSERT_TRUE(schema->GetString() == ans);
}
