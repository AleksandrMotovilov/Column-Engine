#include <filesystem>
#include "from_clmn_to_csv.h"
#include "gtest/gtest.h"

#include "from_clmn_to_csv.cpp"
#include "from_csv_to_clmn.cpp"

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

TEST(Test_Convertion, Convertion_empty) {
    std::string task_dir = "test_convertion_empty";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1.close();
    std::string file2 = task_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2.close();
    std::string file3 = task_dir + "/columnar.clmn";
    ConvertFromCsvToClmn(file1, file2, file3);
    std::string file4 = task_dir + "/data_new.csv";
    std::string file5 = task_dir + "/schema_new.csv";
    ConvertFromClmnToCsv(file3, file4, file5);
    ASSERT_TRUE(CompareFiles(file1, file4) && CompareFiles(file2, file5));
}

TEST(Test_Convertion, Convertion_one_column) {
    std::string task_dir = "test_convertion_one_column";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << "1\n";
    f1 << "5\n";
    f1 << "8\n";
    f1.close();
    std::string file2 = task_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2 << "a,int64\n";
    f2.close();
    std::string file3 = task_dir + "/columnar.clmn";
    ConvertFromCsvToClmn(file1, file2, file3);
    std::string file4 = task_dir + "/data_new.csv";
    std::string file5 = task_dir + "/schema_new.csv";
    ConvertFromClmnToCsv(file3, file4, file5);
    ASSERT_TRUE(CompareFiles(file1, file4) && CompareFiles(file2, file5));
}

TEST(Test_Convertion, Convertion_one_row) {
    std::string task_dir = "test_convertion_one_row";
    std::filesystem::create_directories(task_dir);
    std::string file1 = task_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << "1,2,first,4,magic\n";
    f1.close();
    std::string file2 = "test_convertion/schema.csv";
    std::ofstream f2(file2);
    f2 << "a,int64\n";
    f2 << "b,int64\n";
    f2 << "c,string\n";
    f2 << "d,int64\n";
    f2 << "mag,string\n";
    f2.close();
    std::string file3 = task_dir + "/columnar.clmn";
    ConvertFromCsvToClmn(file1, file2, file3);
    std::string file4 = task_dir + "/data_new.csv";
    std::string file5 = task_dir + "/schema_new.csv";
    ConvertFromClmnToCsv(file3, file4, file5);
    ASSERT_TRUE(CompareFiles(file1, file4) && CompareFiles(file2, file5));
}

TEST(Test_Convertion, Convertion_table) {
    std::string task_dir = "test_convertion_table";
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
    std::string file4 = task_dir + "/data_new.csv";
    std::string file5 = task_dir + "/schema_new.csv";
    ConvertFromClmnToCsv(file3, file4, file5);
    ASSERT_TRUE(CompareFiles(file1, file4) && CompareFiles(file2, file5));
}