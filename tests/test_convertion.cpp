#include <filesystem>
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
            std::cout << "Difference: " << a << ' ' << b << std::endl;
            break;
        }
    }
    fleft.close();
    fright.close();
    return ans;
}

void TestConvertion(const std::string& test_dir, const std::string& text_data, const std::string& text_schema) {
    std::filesystem::create_directories(test_dir);
    std::string file1 = test_dir + "/data.csv";
    std::ofstream f1(file1);
    f1 << text_data;
    f1.close();
    std::string file2 = test_dir + "/schema.csv";
    std::ofstream f2(file2);
    f2 << text_schema;
    f2.close();
    std::string file3 = test_dir + "/columnar.clmn";
    ConvertFromCsvToClmn(file1, file2, file3);
    std::string file4 = test_dir + "/data_new.csv";
    std::string file5 = test_dir + "/schema_new.csv";
    ConvertFromClmnToCsv(file3, file5, file4);
    ASSERT_TRUE(CompareFiles(file1, file4));
    ASSERT_TRUE(CompareFiles(file2, file5));
}

TEST(Test_Convertion, Convertion_empty) {
    std::string test_dir = "test_convertion_empty";
    std::string text_data;
    std::string text_schema;
    TestConvertion(test_dir, text_data, text_schema);
}

TEST(Test_Convertion, Convertion_one_column) {
    std::string test_dir = "test_convertion_one_column";
    std::string text_data = "1\n2\n8";
    std::string text_schema = "a,int64";
    TestConvertion(test_dir, text_data, text_schema);
}

TEST(Test_Convertion, Convertion_one_row) {
    std::string test_dir = "test_convertion_one_row";
    std::string text_data = "1,2,4";
    std::string text_schema = "a,int64\nb,int64\nc,int64";
    TestConvertion(test_dir, text_data, text_schema);
}

TEST(Test_Convertion, Convertion_table) {
    std::string test_dir = "test_convertion_table";
    std::string text_data = "1,2,first,4\n5,1,second,2\n8,17,third,2";
    std::string text_schema = "a,int64\nb,int64\nc,string\nd,int64";
    TestConvertion(test_dir, text_data, text_schema);
}

TEST(Test_Convertion, Convertion_all_types) {
    std::string test_dir = "test_convertion_table";
    std::string text_data = "1,1,1,1.1,1.1,a,str\n1,1,1,1.1,1.1,a,str";
    std::string text_schema = "a,int16\nb,int32\nc,int64\nd,float\ne,double\nf,char\ng,string";
    TestConvertion(test_dir, text_data, text_schema);
}