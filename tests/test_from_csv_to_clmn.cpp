#include "gtest/gtest.h"
#include <filesystem>
#include <memory>
#include "from_csv_to_clmn.cpp"

bool CompareTables(std::shared_ptr<Table> table_left, std::shared_ptr<Table> table_right) {
    if (table_left->ColumnsNumber() != table_right->ColumnsNumber() || table_left->RowsNumber() != table_right->RowsNumber()) {
        return false;
    }
    size_t n = table_left->RowsNumber();
    size_t m = table_left->ColumnsNumber();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < m; j++) {
            if (table_left->GetValueString(i, j) != table_right->GetValueString(i, j)) {
                return false;
            }
        }
    }
    return true;
}

TEST(Test_Reader, Reader_read) {
    std::string task_dir = "test_reader_read";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << "1,2,first,4\n";
    f1 << "5,1,second,2\n";
    f1 << "8,17,third,2\n";
    f1.close();
    std::string file2 = task_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2 << "a,int64\n";
    f2 << "b,int64\n";
    f2 << "name123,string\n";
    f2 << "d,int64\n";
    f2.close();
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCSV(file2);
    CSVReader reader(file1, schema);
    std::shared_ptr<Table> table(reader.Read());
    std::shared_ptr<Table> table_ans = std::make_shared<Table>(schema);
    table_ans->Push(0, "1", Type::INT64);
    table_ans->Push(0, "5", Type::INT64);
    table_ans->Push(0, "8", Type::INT64);
    table_ans->Push(1, "2", Type::INT64);
    table_ans->Push(1, "1", Type::INT64);
    table_ans->Push(1, "17", Type::INT64);
    table_ans->Push(2, "first", Type::STRING);
    table_ans->Push(2, "second", Type::STRING);
    table_ans->Push(2, "third", Type::STRING);
    table_ans->Push(3, "4", Type::INT64);
    table_ans->Push(3, "2", Type::INT64);
    table_ans->Push(3, "2", Type::INT64);
    ASSERT_TRUE(CompareTables(table, table_ans));
}

bool CompareFiles(const std::string& file_left, const std::string& file_right) {
    bool ans = true;
    std::ifstream fleft(file_left);
    std::ifstream fright(file_right);
    while (!fleft.eof() || !fright.eof()) {
        int a = fleft.get();
        int b = fright.get();
        if (a != b) {
            ans = false;
            std::cout << "Difference: " << static_cast<char>(a) << ' ' << static_cast<char>(b) << std::endl;
            break;
        }
    }
    fleft.close();
    fright.close();
    return ans;
}

TEST(Test_Writer, Writer_write) {
    std::string task_dir = "test_writer_write";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << "1,2,first,4\n";
    f1 << "5,1,second,2\n";
    f1 << "8,17,third,2\n";
    f1.close();
    std::string file2 = task_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2 << "a,int64\n";
    f2 << "b,int64\n";
    f2 << "name123,string\n";
    f2 << "d,int64\n";
    f2.close();
    std::string file3 = task_dir + "/columnar.clmn";
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCSV(file2);
    CSVReader reader(file1, schema);
    std::shared_ptr<Table> table(reader.Read());
    CLMNWriter writer(file3, schema);
    writer.Write(table);
    std::string file4 = task_dir + "/columnar_ans.clmn";
    std::ofstream f3(file4);
    f3 << "1,5,8\n";
    f3 << "2,1,17\n";
    f3 << "first,second,third\n";
    f3 << "4,2,2\n";
    f3.close();
    ASSERT_TRUE(CompareFiles(file3, file4));
}

TEST(Test_Convert, Writer_write_schema) {
    std::string task_dir = "test_convert_from_csv_to_clmn";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << "1,2,first,4\n";
    f1 << "5,1,second,2\n";
    f1 << "8,17,third,2\n";
    f1.close();
    std::string file2 = task_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2 << "a,int64\n";
    f2 << "b,int64\n";
    f2 << "name123,string\n";
    f2 << "d,int64\n";
    f2.close();
    std::string file3 = task_dir + "/columnar.clmn";
    std::shared_ptr<Schema> schema = std::make_shared<Schema>();
    schema->FromCSV(file2);
    CSVReader reader(file1, schema);
    std::shared_ptr<Table> table(reader.Read());
    CLMNWriter writer(file3, schema);
    writer.Write(table);
    writer.WriteSchema();
    std::string file4 = task_dir + "/columnar_ans.clmn";
    std::ofstream f3(file4);
    f3 << "1,5,8\n";
    f3 << "2,1,17\n";
    f3 << "first,second,third\n";
    f3 << "4,2,2\n";
    f3 << "a,int64\n";
    f3 << "b,int64\n";
    f3 << "name123,string\n";
    f3 << "d,int64\n";
    std::string s = "a,int64\nb,int64\nname123,string\nd,int64\n";
    int64_t offset = s.size() + 8;
    f3.write(reinterpret_cast<char*>(&offset), sizeof(offset));
    f3.close();
    ASSERT_TRUE(CompareFiles(file3, file4));
}

TEST(Test_Convert, ConvertFromCsvToClmn) {
    std::string task_dir = "test_convert_from_csv_to_clmn";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << "1,2,first,4\n";
    f1 << "5,1,second,2\n";
    f1 << "8,17,third,2\n";
    f1.close();
    std::string file2 = task_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2 << "a,int64\n";
    f2 << "b,int64\n";
    f2 << "name123,string\n";
    f2 << "d,int64\n";
    f2.close();
    std::string file3 = task_dir + "/columnar.clmn";
    ConvertFromCsvToClmn(file1, file2, file3);
    std::string file4 = task_dir + "/columnar_ans.clmn";
    std::ofstream f3(file4);
    f3 << "1,5,8\n";
    f3 << "2,1,17\n";
    f3 << "first,second,third\n";
    f3 << "4,2,2\n";
    f3 << "a,int64\n";
    f3 << "b,int64\n";
    f3 << "name123,string\n";
    f3 << "d,int64\n";
    std::string s = "a,int64\nb,int64\nname123,string\nd,int64\n";
    int64_t offset = s.size() + 8;
    f3.write(reinterpret_cast<char*>(&offset), sizeof(offset));
    f3.close();
    ASSERT_TRUE(CompareFiles(file3, file4));
}

