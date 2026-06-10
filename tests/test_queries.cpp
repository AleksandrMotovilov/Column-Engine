#include <filesystem>
#include "gtest/gtest.h"

#include "src/convertion/from_csv_to_clmn.h"
#include "src/convertion/from_clmn_to_csv.h"
#include "src/execution/operators.h"

static bool CompareFiles(const std::string& file_left, const std::string& file_right) {
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

static void RunQuery(
    const std::string& test_dir,
    const std::string& csv_data,
    const std::string& schema,
    const std::function<std::shared_ptr<Operator>(
        const std::string& input_clmn,
        const std::string& result_csv,
        const std::string& result_schema)>& build_plan,
    const std::string& expected,
    size_t column_batch_size = 2
) {
    SetBatchSize(column_batch_size, 2);

    std::filesystem::create_directories(test_dir);

    std::string file_csv = test_dir + "/data.csv";
    std::ofstream f_csv(file_csv);
    f_csv << csv_data;
    f_csv.close();

    std::string file_schema = test_dir + "/schema.csv";
    std::ofstream f_schema(file_schema);
    f_schema << schema;
    f_schema.close();

    std::string file_clmn = test_dir + "/data.clmn";
    ConvertFromCsvToClmn(file_csv, file_schema, file_clmn);

    std::string file_result_clmn = test_dir + "/result.clmn";
    std::string file_result_schema = test_dir + "/result_schema.csv";
    std::string file_result_csv = test_dir + "/result.csv";

    std::shared_ptr<Operator> plan = build_plan(file_clmn, file_result_csv, file_result_schema);
    plan->Next();

    std::string file_expected = test_dir + "/expected.csv";
    std::ofstream f_expected(file_expected);
    f_expected << expected;
    f_expected.close();

    ASSERT_TRUE(CompareFiles(file_result_csv, file_expected));
}

// Q0: SELECT COUNT(*) FROM hits;
TEST(Test_Query, Q0) {
    RunQuery(
        "test_q0",
        "1,2\n3,4\n5,6\n",
        "a,int64\nb,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountRowsAggregation>()
                    }
                )
            );
        },
        "3\n"
    );
}

// Q1: SELECT COUNT(*) FROM hits WHERE AdvEngineID <> 0;
TEST(Test_Query, Q1) {
    RunQuery(
        "test_q1",
        "0,1\n3,4\n5,6\n",
        "AdvEngineID,int16\nOtherCol,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"AdvEngineID"}
                        ),
                        std::make_shared<NotEqualExpression>("AdvEngineID", "0")
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountRowsAggregation>()
                    }
                )
            );
        },
        "2\n"
    );
}

// Q2: SELECT SUM(AdvEngineID), COUNT(*), AVG(ResolutionWidth) FROM hits;
TEST(Test_Query, Q2) {
    RunQuery(
        "test_q2",
        "1,10\n2,20\n3,30\n",
        "AdvEngineID,int16\nResolutionWidth,int16",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"AdvEngineID", "ResolutionWidth"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<SumAggregation>("AdvEngineID"),
                        std::make_shared<CountRowsAggregation>(),
                        std::make_shared<AvgAggregation>("ResolutionWidth")
                    }
                )
            );
        },
        "6,3,20\n"
    );
}

// Q3: SELECT AVG(UserID) FROM hits;
TEST(Test_Query, Q3) {
    RunQuery(
        "test_q3",
        "100\n200\n300\n",
        "UserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"UserID"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<AvgAggregation>("UserID")
                    }
                )
            );
        },
        "200\n"
    );
}

// Q4: SELECT COUNT(DISTINCT UserID) FROM hits;
TEST(Test_Query, Q4) {
    RunQuery(
        "test_q4",
        "100\n200\n100\n300\n",
        "UserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"UserID"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountDistinctAggregation>("UserID")
                    }
                )
            );
        },
        "3\n"
    );
}

// Q5: SELECT COUNT(DISTINCT SearchPhrase) FROM hits;
TEST(Test_Query, Q5) {
    RunQuery(
        "test_q5",
        "hello\ngoogle\nhello\n",
        "SearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"SearchPhrase"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountDistinctAggregation>("SearchPhrase")
                    }
                )
            );
        },
        "2\n"
    );
}

// Q6: SELECT MIN(EventDate), MAX(EventDate) FROM hits;
TEST(Test_Query, Q6) {
    RunQuery(
        "test_q6",
        "2013-07-01\n2013-07-15\n2013-07-31\n",
        "EventDate,date",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"EventDate"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<MinAggregation>("EventDate"),
                        std::make_shared<MaxAggregation>("EventDate")
                    }
                )
            );
        },
        "2013-07-01,2013-07-31\n"
    );
}

// Q7: SELECT AdvEngineID, COUNT(*) FROM hits WHERE AdvEngineID <> 0 GROUP BY AdvEngineID ORDER BY COUNT(*) DESC;
TEST(Test_Query, Q7) {
    RunQuery(
        "test_q7",
        "0\n1\n2\n1\n2\n2\n",
        "AdvEngineID,int16",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<SortOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"AdvEngineID"}
                            ),
                            std::make_shared<NotEqualExpression>("AdvEngineID", "0")
                        ),
                        std::vector<std::string>{"AdvEngineID"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    "count(*)", true
                )
            );
        },
        "2,3\n1,2\n"
    );
}

// Q8: SELECT RegionID, COUNT(DISTINCT UserID) AS u FROM hits GROUP BY RegionID ORDER BY u DESC LIMIT 10;
TEST(Test_Query, Q8) {
    RunQuery(
        "test_q8",
        "1,10\n2,20\n2,30\n3,40\n3,50\n3,60\n",
        "RegionID,int32\nUserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"RegionID", "UserID"}
                        ),
                        std::vector<std::string>{"RegionID"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountDistinctAggregation>("UserID")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(distinct UserID)"}, 10, true
                )
            );
        },
        "3,3\n2,2\n1,1\n"
    );
}

// Q9: SELECT RegionID, SUM(AdvEngineID), COUNT(*) AS c, AVG(ResolutionWidth), COUNT(DISTINCT UserID) FROM hits GROUP BY RegionID ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q9) {
    RunQuery(
        "test_q9",
        "1,5,100,10\n2,3,200,20\n2,7,400,30\n",
        "RegionID,int32\nAdvEngineID,int16\nResolutionWidth,int16\nUserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"RegionID", "AdvEngineID", "ResolutionWidth", "UserID"}
                        ),
                        std::vector<std::string>{"RegionID"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<SumAggregation>("AdvEngineID"),
                                std::make_shared<CountRowsAggregation>(),
                                std::make_shared<AvgAggregation>("ResolutionWidth"),
                                std::make_shared<CountDistinctAggregation>("UserID")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "2,10,2,300,2\n1,5,1,100,1\n"
    );
}

// Q10: SELECT MobilePhoneModel, COUNT(DISTINCT UserID) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhoneModel ORDER BY u DESC LIMIT 10;
TEST(Test_Query, Q10) {
    RunQuery(
        "test_q10",
        ",10\niPhone,20\nGalaxy,30\niPhone,40\niPhone,20\n",
        "MobilePhoneModel,string\nUserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"MobilePhoneModel", "UserID"}
                            ),
                            std::make_shared<NotEqualExpression>("MobilePhoneModel", "")
                        ),
                        std::vector<std::string>{"MobilePhoneModel"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountDistinctAggregation>("UserID")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(distinct UserID)"}, 10, true
                )
            );
        },
        "iPhone,2\nGalaxy,1\n"
    );
}

// Q11: SELECT MobilePhone, MobilePhoneModel, COUNT(DISTINCT UserID) AS u FROM hits WHERE MobilePhoneModel <> '' GROUP BY MobilePhone, MobilePhoneModel ORDER BY u DESC LIMIT 10;
TEST(Test_Query, Q11) {
    RunQuery(
        "test_q11",
        "1,,10\n1,iPhone,20\n2,Galaxy,30\n1,iPhone,40\n1,iPhone,20\n",
        "MobilePhone,int32\nMobilePhoneModel,string\nUserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"MobilePhone", "MobilePhoneModel", "UserID"}
                            ),
                            std::make_shared<NotEqualExpression>("MobilePhoneModel", "")
                        ),
                        std::vector<std::string>{"MobilePhone", "MobilePhoneModel"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountDistinctAggregation>("UserID")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(distinct UserID)"}, 10, true
                )
            );
        },
        "1,iPhone,2\n2,Galaxy,1\n"
    );
}

// Q12: SELECT SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q12) {
    RunQuery(
        "test_q12",
        "google\nbing\ngoogle\nbing\ngoogle\n\n",
        "SearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"SearchPhrase"}
                            ),
                            std::make_shared<NotEqualExpression>("SearchPhrase", "")
                        ),
                        std::vector<std::string>{"SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "google,3\nbing,2\n"
    );
}

// Q13: SELECT SearchPhrase, COUNT(DISTINCT UserID) AS u FROM hits WHERE SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY u DESC LIMIT 10;
TEST(Test_Query, Q13) {
    RunQuery(
        "test_q13",
        ",99\ngoogle,10\nbing,30\ngoogle,20\ngoogle,10\n",
        "SearchPhrase,string\nUserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"SearchPhrase", "UserID"}
                            ),
                            std::make_shared<NotEqualExpression>("SearchPhrase", "")
                        ),
                        std::vector<std::string>{"SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountDistinctAggregation>("UserID")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(distinct UserID)"}, 10, true
                )
            );
        },
        "google,2\nbing,1\n"
    );
}

// Q14: SELECT SearchEngineID, SearchPhrase, COUNT(*) AS c FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, SearchPhrase ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q14) {
    RunQuery(
        "test_q14",
        "1,google\n2,bing\n1,google\n1,google\n2,bing\n0,\n1,bing\n",
        "SearchEngineID,int16\nSearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"SearchEngineID", "SearchPhrase"}
                            ),
                            std::make_shared<NotEqualExpression>("SearchPhrase", "")
                        ),
                        std::vector<std::string>{"SearchEngineID", "SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "1,google,3\n2,bing,2\n1,bing,1\n"
    );
}

// Q15: SELECT UserID, COUNT(*) FROM hits GROUP BY UserID ORDER BY COUNT(*) DESC LIMIT 10;
TEST(Test_Query, Q15) {
    RunQuery(
        "test_q15",
        "1\n2\n1\n3\n1\n2\n",
        "UserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"UserID"}
                        ),
                        std::vector<std::string>{"UserID"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "1,3\n2,2\n3,1\n"
    );
}

// Q16: SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase ORDER BY COUNT(*) DESC LIMIT 10;
TEST(Test_Query, Q16) {
    RunQuery(
        "test_q16",
        "2,bing\n1,google\n1,google\n1,bing\n1,google\n1,bing\n",
        "UserID,int64\nSearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"UserID", "SearchPhrase"}
                        ),
                        std::vector<std::string>{"UserID", "SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "1,google,3\n1,bing,2\n2,bing,1\n"
    );
}

// Q17: SELECT UserID, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, SearchPhrase LIMIT 10;
TEST(Test_Query, Q17) {
    RunQuery(
        "test_q17",
        "2,bing\n1,google\n1,google\n1,bing\n1,google\n1,bing\n",
        "UserID,int64\nSearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<LimitOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"UserID", "SearchPhrase"}
                        ),
                        std::vector<std::string>{"UserID", "SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    10
                )
            );
        },
        "2,bing,1\n1,google,3\n1,bing,2\n"
    );
}

// Q18: SELECT UserID, extract(minute FROM EventTime) AS m, SearchPhrase, COUNT(*) FROM hits GROUP BY UserID, m, SearchPhrase ORDER BY COUNT(*) DESC LIMIT 10;
TEST(Test_Query, Q18) {
    RunQuery(
        "test_q18",
        "1,2023-01-01 10:05:00,google\n"
        "1,2023-01-01 10:05:30,google\n"
        "1,2023-01-01 11:05:00,google\n"
        "2,2023-01-01 10:10:00,bing\n"
        "2,2023-01-01 10:10:30,bing\n"
        "3,2023-01-01 10:15:00,google\n",
        "UserID,int64\nEventTime,timestamp\nSearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ProjectOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"UserID", "EventTime", "SearchPhrase"}
                            ),
                            std::make_shared<ExtractMinuteExpression>("EventTime"),
                            "m"
                        ),
                        std::vector<std::string>{"UserID", "m", "SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "1,5,google,3\n2,10,bing,2\n3,15,google,1\n"
    );
}

// Q19: SELECT UserID FROM hits WHERE UserID = 435090932899640449;
TEST(Test_Query, Q19) {
    RunQuery(
        "test_q19",
        "100\n200\n100\n300\n",
        "UserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<FilterOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"UserID"}
                    ),
                    std::make_shared<EqualExpression>("UserID", "100")
                )
            );
        },
        "100\n100\n"
    );
}

// Q20: SELECT COUNT(*) FROM hits WHERE URL LIKE '%google%';
TEST(Test_Query, Q20) {
    RunQuery(
        "test_q20",
        "http://google.com\nhttps://bing.com\nhttps://www.google.com/search\n",
        "URL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"URL"}
                        ),
                        std::make_shared<ContainsExpression>("URL", "google")
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountRowsAggregation>()
                    }
                )
            );
        },
        "2\n"
    );
}

// Q21: SELECT SearchPhrase, MIN(URL), COUNT(*) AS c FROM hits WHERE URL LIKE '%google%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q21) {
    RunQuery(
        "test_q21",
        "http://google.com,google\n"
        "http://bing.com,bing\n"
        "http://google.com/maps,maps\n"
        "http://google.com,google\n",
        "URL,string\nSearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"URL", "SearchPhrase"}
                            ),
                            std::make_shared<AndExpression>(
                                std::make_shared<ContainsExpression>("URL", "google"),
                                std::make_shared<NotEqualExpression>("SearchPhrase", "")
                            )
                        ),
                        std::vector<std::string>{"SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<MinAggregation>("URL"),
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "google,http://google.com,2\n"
        "maps,http://google.com/maps,1\n"
    );
}

// Q22: SELECT SearchPhrase, MIN(URL), MIN(Title), COUNT(*) AS c, COUNT(DISTINCT UserID) FROM hits WHERE Title LIKE '%Google%' AND URL NOT LIKE '%.google.%' AND SearchPhrase <> '' GROUP BY SearchPhrase ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q22) {
    RunQuery(
        "test_q22",
        "Google Search,http://bing.com,hello,10\n"
        "Google Maps,http://maps.google.com/,maps,20\n"
        "Google Search,http://docs.google.com/,hello,10\n"
        "Google News,http://news.example.com,news,30\n",
        "Title,string\nURL,string\nSearchPhrase,string\nUserID,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"Title", "URL", "SearchPhrase", "UserID"}
                            ),
                            std::make_shared<AndExpression>(
                                std::make_shared<AndExpression>(
                                    std::make_shared<ContainsExpression>("Title", "Google"),
                                    std::make_shared<NotContainsExpression>("URL", ".google.")
                                ),
                                std::make_shared<NotEqualExpression>("SearchPhrase", "")
                            )
                        ),
                        std::vector<std::string>{"SearchPhrase"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<MinAggregation>("URL"),
                                std::make_shared<MinAggregation>("Title"),
                                std::make_shared<CountRowsAggregation>(),
                                std::make_shared<CountDistinctAggregation>("UserID")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "hello,http://bing.com,Google Search,1,1\n"
        "news,http://news.example.com,Google News,1,1\n",
        4
    );
}

// Q23: SELECT * FROM hits WHERE URL LIKE '%google%' ORDER BY EventTime LIMIT 10;
TEST(Test_Query, Q23) {
    RunQuery(
        "test_q23",
        "10,2021-01-01 00:05:00,http://google.com,Google\n"
        "20,2021-01-01 00:01:00,http://bing.com,Bing\n"
        "30,2021-01-01 00:03:00,http://google.com/maps,Maps\n"
        "40,2021-01-01 00:07:00,http://yahoo.com,Yahoo\n"
        "50,2021-01-01 00:02:00,http://google.ru,RuGoogle\n",
        "WatchID,int64\nEventTime,timestamp\nURL,string\nTitle,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{}, true
                        ),
                        std::make_shared<ContainsExpression>("URL", "google")
                    ),
                    std::vector<std::string>{"EventTime"}, 10, false
                )
            );
        },
        "50,2021-01-01 00:02:00,http://google.ru,RuGoogle\n"
        "30,2021-01-01 00:03:00,http://google.com/maps,Maps\n"
        "10,2021-01-01 00:05:00,http://google.com,Google\n",
        4
    );
}

// Q24: SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime LIMIT 10;
TEST(Test_Query, Q24) {
    RunQuery(
        "test_q24",
        ",2021-01-01 00:01:00\n"
        "apple,2021-01-01 00:05:00\n"
        ",2021-01-01 00:03:00\n"
        "banana,2021-01-01 00:02:00\n"
        "cherry,2021-01-01 00:04:00\n",
        "SearchPhrase,string\nEventTime,timestamp",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"SearchPhrase", "EventTime"}
                        ),
                        std::make_shared<NotEqualExpression>("SearchPhrase", "")
                    ),
                    std::vector<std::string>{"EventTime"}, 10, false
                )
            );
        },
        "banana,2021-01-01 00:02:00\n"
        "cherry,2021-01-01 00:04:00\n"
        "apple,2021-01-01 00:05:00\n"
    );
}

// Q25: SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY SearchPhrase LIMIT 10;
TEST(Test_Query, Q25) {
    RunQuery(
        "test_q25",
        "apple\nbanana\n\ncherry\napple\n",
        "SearchPhrase,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"SearchPhrase"}
                        ),
                        std::make_shared<NotEqualExpression>("SearchPhrase", "")
                    ),
                    std::vector<std::string>{"SearchPhrase"}, 10, false
                )
            );
        },
        "apple\napple\nbanana\ncherry\n"
    );
}

// Q30: SELECT SearchEngineID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY SearchEngineID, ClientIP ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q30) {
    RunQuery(
        "test_q30",
        ",1,1,0,100\n"
        "google,1,1,0,150\n"
        "google,1,1,1,200\n"
        "bing,1,2,1,150\n"
        "bing,2,2,1,250\n"
        "bing,2,2,0,300\n"
        "bing,2,2,1,350\n",
        "SearchPhrase,string\nSearchEngineID,int32\nClientIP,int64\nIsRefresh,int16\nResolutionWidth,int16",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{
                                    "SearchPhrase", "SearchEngineID", "ClientIP",
                                    "IsRefresh", "ResolutionWidth"}
                            ),
                            std::make_shared<NotEqualExpression>("SearchPhrase", "")
                        ),
                        std::vector<std::string>{"SearchEngineID", "ClientIP"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>(),
                                std::make_shared<SumAggregation>("IsRefresh"),
                                std::make_shared<AvgAggregation>("ResolutionWidth")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "2,2,3,2,300\n"
        "1,1,2,1,175\n"
        "1,2,1,1,150\n"
    );
}

// Q31: SELECT WatchID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits WHERE SearchPhrase <> '' GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q31) {
    RunQuery(
        "test_q31",
        ",1,1,0,100\n"
        "google,10,1,1,200\n"
        "bing,10,1,0,300\n"
        "google,20,2,1,150\n",
        "SearchPhrase,string\nWatchID,int64\nClientIP,int64\nIsRefresh,int16\nResolutionWidth,int16",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{
                                    "SearchPhrase", "WatchID", "ClientIP",
                                    "IsRefresh", "ResolutionWidth"}
                            ),
                            std::make_shared<NotEqualExpression>("SearchPhrase", "")
                        ),
                        std::vector<std::string>{"WatchID", "ClientIP"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>(),
                                std::make_shared<SumAggregation>("IsRefresh"),
                                std::make_shared<AvgAggregation>("ResolutionWidth")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "10,1,2,1,250\n20,2,1,1,150\n"
    );
}

// Q32: SELECT WatchID, ClientIP, COUNT(*) AS c, SUM(IsRefresh), AVG(ResolutionWidth) FROM hits GROUP BY WatchID, ClientIP ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q32) {
    RunQuery(
        "test_q32",
        "10,1,0,100\n"
        "10,1,1,200\n"
        "10,1,0,300\n"
        "20,2,1,400\n"
        "20,2,0,500\n"
        "10,2,1,600\n",
        "WatchID,int64\nClientIP,int64\nIsRefresh,int16\nResolutionWidth,int16",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{
                                "WatchID", "ClientIP", "IsRefresh", "ResolutionWidth"}
                        ),
                        std::vector<std::string>{"WatchID", "ClientIP"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>(),
                                std::make_shared<SumAggregation>("IsRefresh"),
                                std::make_shared<AvgAggregation>("ResolutionWidth")
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "10,1,3,1,200\n"
        "20,2,2,1,450\n"
        "10,2,1,1,600\n"
    );
}

// Q33: SELECT URL, COUNT(*) AS c FROM hits GROUP BY URL ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q33) {
    RunQuery(
        "test_q33",
        "http://google.com\nhttp://bing.com\nhttp://google.com\nhttp://bing.com\nhttp://google.com\n",
        "URL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"URL"}
                        ),
                        std::vector<std::string>{"URL"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "http://google.com,3\nhttp://bing.com,2\n"
    );
}

// Q34: SELECT 1, URL, COUNT(*) AS c FROM hits GROUP BY 1, URL ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q34) {
    RunQuery(
        "test_q34",
        "http://google.com\nhttp://bing.com\nhttp://google.com\nhttp://bing.com\nhttp://google.com\n",
        "URL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ProjectOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{"URL"}
                            ),
                            std::make_shared<ConstantExpression>("1"),
                            "1"
                        ),
                        std::vector<std::string>{"1", "URL"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "1,http://google.com,3\n1,http://bing.com,2\n"
    );
}

// Q35: SELECT ClientIP, ClientIP - 1, ClientIP - 2, ClientIP - 3, COUNT(*) AS c FROM hits GROUP BY ClientIP, ClientIP - 1, ClientIP - 2, ClientIP - 3 ORDER BY c DESC LIMIT 10;
TEST(Test_Query, Q35) {
    RunQuery(
        "test_q35",
        "100\n200\n100\n200\n200\n",
        "ClientIP,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<ProjectOperator>(
                            std::make_shared<ProjectOperator>(
                                std::make_shared<ProjectOperator>(
                                    std::make_shared<ScanOperator>(
                                        input_clmn,
                                        std::vector<std::string>{"ClientIP"}
                                    ),
                                    std::make_shared<SumExpression>("ClientIP", -1LL),
                                    "ClientIP - 1"
                                ),
                                std::make_shared<SumExpression>("ClientIP", -2LL),
                                "ClientIP - 2"
                            ),
                            std::make_shared<SumExpression>("ClientIP", -3LL),
                            "ClientIP - 3"
                        ),
                        std::vector<std::string>{"ClientIP", "ClientIP - 1", "ClientIP - 2", "ClientIP - 3"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "200,199,198,197,3\n100,99,98,97,2\n"
    );
}

// Q36: SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND URL <> '' GROUP BY URL ORDER BY PageViews DESC LIMIT 10;
TEST(Test_Query, Q36) {
    RunQuery(
        "test_q36",
        "62,2013-07-15,0,0,http://example.com\n"
        "62,2013-07-20,0,0,http://example.com\n"
        "62,2013-07-20,1,0,http://example.com\n"
        "62,2013-06-30,0,0,http://old.com\n"
        "99,2013-07-10,0,0,http://other.com\n"
        "62,2013-07-10,0,0,\n"
        "62,2013-07-10,0,0,http://another.com\n",
        "CounterID,int32\nEventDate,date\nDontCountHits,int16\nIsRefresh,int16\nURL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{
                                    "CounterID", "EventDate", "DontCountHits",
                                    "IsRefresh", "URL"}
                            ),
                            std::make_shared<AndExpression>(
                                std::make_shared<AndExpression>(
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<EqualExpression>("CounterID", "62"),
                                                std::make_shared<GreaterOrEqualExpression>(
                                                    "EventDate", "2013-07-01")
                                            ),
                                            std::make_shared<LessOrEqualExpression>(
                                                "EventDate", "2013-07-31")
                                        ),
                                        std::make_shared<EqualExpression>("DontCountHits", "0")
                                    ),
                                    std::make_shared<EqualExpression>("IsRefresh", "0")
                                ),
                                std::make_shared<NotEqualExpression>("URL", "")
                            )
                        ),
                        std::vector<std::string>{"URL"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "http://example.com,2\nhttp://another.com,1\n",
        5
    );
}

// Q37: SELECT Title, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND DontCountHits = 0 AND IsRefresh = 0 AND Title <> '' GROUP BY Title ORDER BY PageViews DESC LIMIT 10;
TEST(Test_Query, Q37) {
    RunQuery(
        "test_q37",
        "62,2013-07-15,0,0,Welcome\n"
        "62,2013-07-20,0,0,Welcome\n"
        "62,2013-07-20,1,0,Welcome\n"
        "62,2013-06-30,0,0,Old\n"
        "99,2013-07-10,0,0,Other\n"
        "62,2013-07-10,0,0,\n"
        "62,2013-07-10,0,0,About\n",
        "CounterID,int32\nEventDate,date\nDontCountHits,int16\nIsRefresh,int16\nTitle,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<GroupByAggregationOperator>(
                        std::make_shared<FilterOperator>(
                            std::make_shared<ScanOperator>(
                                input_clmn,
                                std::vector<std::string>{
                                    "CounterID", "EventDate", "DontCountHits",
                                    "IsRefresh", "Title"}
                            ),
                            std::make_shared<AndExpression>(
                                std::make_shared<AndExpression>(
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<EqualExpression>("CounterID", "62"),
                                                std::make_shared<GreaterOrEqualExpression>(
                                                    "EventDate", "2013-07-01")
                                            ),
                                            std::make_shared<LessOrEqualExpression>(
                                                "EventDate", "2013-07-31")
                                        ),
                                        std::make_shared<EqualExpression>("DontCountHits", "0")
                                    ),
                                    std::make_shared<EqualExpression>("IsRefresh", "0")
                                ),
                                std::make_shared<NotEqualExpression>("Title", "")
                            )
                        ),
                        std::vector<std::string>{"Title"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                )
            );
        },
        "Welcome,2\nAbout,1\n",
        5
    );
}

// Q26: SELECT SearchPhrase FROM hits WHERE SearchPhrase <> '' ORDER BY EventTime, SearchPhrase LIMIT 10;
TEST(Test_Query, Q26) {
    RunQuery(
        "test_q26",
        "apple,2021-01-01 00:05:00\n"
        ",2021-01-01 00:01:00\n"
        "banana,2021-01-01 00:02:00\n"
        "cherry,2021-01-01 00:01:00\n"
        "date,2021-01-01 00:01:00\n",
        "SearchPhrase,string\nEventTime,timestamp",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"SearchPhrase", "EventTime"}
                        ),
                        std::make_shared<NotEqualExpression>("SearchPhrase", "")
                    ),
                    std::vector<std::string>{"EventTime", "SearchPhrase"}, 10, false
                )
            );
        },
        "cherry,2021-01-01 00:01:00\n"
        "date,2021-01-01 00:01:00\n"
        "banana,2021-01-01 00:02:00\n"
        "apple,2021-01-01 00:05:00\n"
    );
}

// Q27: SELECT CounterID, AVG(STRLEN(URL)) AS l, COUNT(*) AS c FROM hits WHERE URL <> '' GROUP BY CounterID HAVING COUNT(*) > 2 ORDER BY l DESC LIMIT 5;
TEST(Test_Query, Q27) {
    RunQuery(
        "test_q27",
        "1,abc\n"
        "1,abcd\n"
        "1,abcde\n"
        "2,ab\n"
        "2,abcdef\n"
        "2,abcd\n"
        "2,abcdefgh\n"
        "3,a\n"
        "3,abc\n",
        "CounterID,int32\nURL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<ProjectOperator>(
                                std::make_shared<FilterOperator>(
                                    std::make_shared<ScanOperator>(
                                        input_clmn,
                                        std::vector<std::string>{"CounterID", "URL"}
                                    ),
                                    std::make_shared<NotEqualExpression>("URL", "")
                                ),
                                std::make_shared<LengthExpression>("URL"),
                                "strlen(URL)"
                            ),
                            std::vector<std::string>{"CounterID"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<AvgAggregation>("strlen(URL)"),
                                    std::make_shared<CountRowsAggregation>()
                                };
                            }
                        ),
                        std::make_shared<GreaterOrEqualExpression>("count(*)", "3")
                    ),
                    std::vector<std::string>{"avg(strlen(URL))"}, 5, true
                )
            );
        },
        "2,5,4\n1,4,3\n",
        3
    );
}

// Q28: SELECT REGEXP_REPLACE(Referer, ...) AS k, AVG(STRLEN(Referer)) AS l, COUNT(*) AS c, MIN(Referer) FROM hits WHERE Referer <> '' GROUP BY k HAVING COUNT(*) > 1 ORDER BY l DESC LIMIT 5;
TEST(Test_Query, Q28) {
    RunQuery(
        "test_q28",
        "http://www.google.com/search?q=test\n"
        "https://example.org/page/1\n"
        "http://www.google.com/maps\n"
        "https://example.org/page/2\n"
        "http://test.com/something\n",
        "Referer,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<ProjectOperator>(
                                std::make_shared<ProjectOperator>(
                                    std::make_shared<FilterOperator>(
                                        std::make_shared<ScanOperator>(
                                            input_clmn,
                                            std::vector<std::string>{"Referer"}
                                        ),
                                        std::make_shared<NotEqualExpression>("Referer", "")
                                    ),
                                    std::make_shared<RegexpReplaceExpression>(
                                        "Referer",
                                        R"(^https?://(?:www\.)?([^/]+)/.*$)",
                                        "$1"
                                    ),
                                    "k"
                                ),
                                std::make_shared<LengthExpression>("Referer"),
                                "strlen(Referer)"
                            ),
                            std::vector<std::string>{"k"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<AvgAggregation>("strlen(Referer)"),
                                    std::make_shared<CountRowsAggregation>(),
                                    std::make_shared<MinAggregation>("Referer")
                                };
                            }
                        ),
                        std::make_shared<GreaterOrEqualExpression>("count(*)", "2")
                    ),
                    std::vector<std::string>{"avg(strlen(Referer))"}, 5, true
                )
            );
        },
        "google.com,30,2,http://www.google.com/maps\n"
        "example.org,26,2,https://example.org/page/1\n",
        4
    );
}

// Q29: SELECT SUM(ResolutionWidth), SUM(ResolutionWidth+1), ..., SUM(ResolutionWidth+89) FROM hits;
TEST(Test_Query, Q29) {
    std::string expected;
    for (int64_t i = 0; i <= 89; i++) {
        if (i > 0) {
            expected += ",";
        }
        expected += std::to_string(4224 + 3 * i);
    }
    expected += "\n";

    RunQuery(
        "test_q29",
        "1280\n1920\n1024\n",
        "ResolutionWidth,int32",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            std::vector<std::shared_ptr<AggregationFunction>> aggs;
            for (int64_t i = 0; i <= 89; i++) {
                aggs.push_back(
                    std::make_shared<SumWithOffsetAggregation>("ResolutionWidth", i));
            }
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"ResolutionWidth"}
                    ),
                    aggs
                )
            );
        },
        expected
    );
}

// Q38: SELECT URL, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND IsLink <> 0 AND IsDownload = 0 GROUP BY URL ORDER BY PageViews DESC LIMIT 2 OFFSET 1;
TEST(Test_Query, Q38) {
    RunQuery(
        "test_q38",
        "62,2013-07-15,0,1,0,http://a.com\n"
        "62,2013-07-16,0,1,0,http://a.com\n"
        "62,2013-07-17,0,1,0,http://a.com\n"
        "62,2013-07-18,0,1,0,http://b.com\n"
        "62,2013-07-19,0,1,0,http://b.com\n"
        "62,2013-07-20,0,1,0,http://c.com\n"
        "62,2013-07-21,1,1,0,http://d.com\n"
        "62,2013-07-22,0,0,0,http://e.com\n",
        "CounterID,int32\nEventDate,date\nIsRefresh,int16\nIsLink,int16\nIsDownload,int16\nURL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<OffsetOperator>(
                    std::make_shared<TopKOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<FilterOperator>(
                                std::make_shared<ScanOperator>(
                                    input_clmn,
                                    std::vector<std::string>{
                                        "CounterID", "EventDate", "IsRefresh",
                                        "IsLink", "IsDownload", "URL"}
                                ),
                                std::make_shared<AndExpression>(
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<EqualExpression>("CounterID", "62"),
                                                std::make_shared<GreaterOrEqualExpression>(
                                                    "EventDate", "2013-07-01")
                                            ),
                                            std::make_shared<LessOrEqualExpression>(
                                                "EventDate", "2013-07-31")
                                        ),
                                        std::make_shared<EqualExpression>("IsRefresh", "0")
                                    ),
                                    std::make_shared<AndExpression>(
                                        std::make_shared<NotEqualExpression>("IsLink", "0"),
                                        std::make_shared<EqualExpression>("IsDownload", "0")
                                    )
                                )
                            ),
                            std::vector<std::string>{"URL"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<CountRowsAggregation>()
                                };
                            }
                        ),
                        std::vector<std::string>{"count(*)"}, 3, true
                    ),
                    1
                )
            );
        },
        "http://b.com,2\nhttp://c.com,1\n",
        6
    );
}

// Q39: SELECT TraficSourceID, SearchEngineID, AdvEngineID, CASE WHEN (SearchEngineID = 0 AND AdvEngineID = 0) THEN Referer ELSE '' END AS Src, URL AS Dst, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 GROUP BY TraficSourceID, SearchEngineID, AdvEngineID, Src, Dst ORDER BY PageViews DESC LIMIT 2 OFFSET 1;
TEST(Test_Query, Q39) {
    RunQuery(
        "test_q39",
        "62,2013-07-15,0,1,0,0,http://ref1.com,http://a.com\n"
        "62,2013-07-15,0,1,0,0,http://ref1.com,http://a.com\n"
        "62,2013-07-15,0,1,0,0,http://ref1.com,http://a.com\n"
        "62,2013-07-15,0,2,1,0,http://ref2.com,http://b.com\n"
        "62,2013-07-15,0,2,1,0,http://ref2.com,http://b.com\n"
        "62,2013-07-15,0,3,0,1,http://ref3.com,http://c.com\n",
        "CounterID,int32\nEventDate,date\nIsRefresh,int16\n"
        "TraficSourceID,int32\nSearchEngineID,int32\nAdvEngineID,int32\n"
        "Referer,string\nURL,string",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<OffsetOperator>(
                    std::make_shared<TopKOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<ProjectOperator>(
                                std::make_shared<FilterOperator>(
                                    std::make_shared<ScanOperator>(
                                        input_clmn,
                                        std::vector<std::string>{
                                            "CounterID", "EventDate", "IsRefresh",
                                            "TraficSourceID", "SearchEngineID",
                                            "AdvEngineID", "Referer", "URL"}
                                    ),
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<EqualExpression>("CounterID", "62"),
                                                std::make_shared<GreaterOrEqualExpression>(
                                                    "EventDate", "2013-07-01")
                                            ),
                                            std::make_shared<LessOrEqualExpression>(
                                                "EventDate", "2013-07-31")
                                        ),
                                        std::make_shared<EqualExpression>("IsRefresh", "0")
                                    )
                                ),
                                std::make_shared<CaseWhenExpression>(
                                    std::make_shared<AndExpression>(
                                        std::make_shared<EqualExpression>("SearchEngineID", "0"),
                                        std::make_shared<EqualExpression>("AdvEngineID", "0")
                                    ),
                                    "Referer", ""
                                ),
                                "Src"
                            ),
                            std::vector<std::string>{
                                "TraficSourceID", "SearchEngineID", "AdvEngineID", "Src", "URL"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<CountRowsAggregation>()
                                };
                            }
                        ),
                        std::vector<std::string>{"count(*)"}, 3, true
                    ),
                    1
                )
            );
        },
        "2,1,0,,http://b.com,2\n3,0,1,,http://c.com,1\n",
        8
    );
}

// Q40: SELECT URLHash, EventDate, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND TraficSourceID IN (-1, 6) AND RefererHash = 12345 GROUP BY URLHash, EventDate ORDER BY PageViews DESC LIMIT 2 OFFSET 1;
TEST(Test_Query, Q40) {
    RunQuery(
        "test_q40",
        "62,2013-07-15,0,6,12345,1001\n"
        "62,2013-07-15,0,6,12345,1001\n"
        "62,2013-07-15,0,6,12345,1001\n"
        "62,2013-07-18,0,-1,12345,1002\n"
        "62,2013-07-18,0,-1,12345,1002\n"
        "62,2013-07-20,0,6,12345,1003\n"
        "62,2013-07-21,0,1,12345,1004\n"
        "62,2013-07-22,0,6,99999,1005\n",
        "CounterID,int32\nEventDate,date\nIsRefresh,int16\n"
        "TraficSourceID,int32\nRefererHash,int64\nURLHash,int64",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<OffsetOperator>(
                    std::make_shared<TopKOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<FilterOperator>(
                                std::make_shared<ScanOperator>(
                                    input_clmn,
                                    std::vector<std::string>{
                                        "CounterID", "EventDate", "IsRefresh",
                                        "TraficSourceID", "RefererHash", "URLHash"}
                                ),
                                std::make_shared<AndExpression>(
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<EqualExpression>("CounterID", "62"),
                                                std::make_shared<GreaterOrEqualExpression>(
                                                    "EventDate", "2013-07-01")
                                            ),
                                            std::make_shared<LessOrEqualExpression>(
                                                "EventDate", "2013-07-31")
                                        ),
                                        std::make_shared<EqualExpression>("IsRefresh", "0")
                                    ),
                                    std::make_shared<AndExpression>(
                                        std::make_shared<OrExpression>(
                                            std::make_shared<EqualExpression>("TraficSourceID", "-1"),
                                            std::make_shared<EqualExpression>("TraficSourceID", "6")
                                        ),
                                        std::make_shared<EqualExpression>("RefererHash", "12345")
                                    )
                                )
                            ),
                            std::vector<std::string>{"URLHash", "EventDate"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<CountRowsAggregation>()
                                };
                            }
                        ),
                        std::vector<std::string>{"count(*)"}, 3, true
                    ),
                    1
                )
            );
        },
        "1002,2013-07-18,2\n1003,2013-07-20,1\n",
        6
    );
}

// Q41: SELECT WindowClientWidth, WindowClientHeight, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-01' AND EventDate <= '2013-07-31' AND IsRefresh = 0 AND DontCountHits = 0 AND URLHash = 2868770270353813622 GROUP BY WindowClientWidth, WindowClientHeight ORDER BY PageViews DESC LIMIT 2 OFFSET 1;
TEST(Test_Query, Q41) {
    RunQuery(
        "test_q41",
        "62,2013-07-15,0,0,2868770270353813622,1280,720\n"
        "62,2013-07-16,0,0,2868770270353813622,1280,720\n"
        "62,2013-07-17,0,0,2868770270353813622,1280,720\n"
        "62,2013-07-18,0,0,2868770270353813622,1920,1080\n"
        "62,2013-07-19,0,0,2868770270353813622,1920,1080\n"
        "62,2013-07-20,0,0,2868770270353813622,1024,768\n"
        "62,2013-07-21,1,0,2868770270353813622,1280,720\n"
        "62,2013-07-22,0,1,2868770270353813622,1280,720\n"
        "62,2013-07-23,0,0,9999999999,1280,720\n",
        "CounterID,int32\nEventDate,date\nIsRefresh,int16\nDontCountHits,int16\n"
        "URLHash,int64\nWindowClientWidth,int16\nWindowClientHeight,int16",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<OffsetOperator>(
                    std::make_shared<TopKOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<FilterOperator>(
                                std::make_shared<ScanOperator>(
                                    input_clmn,
                                    std::vector<std::string>{
                                        "CounterID", "EventDate", "IsRefresh",
                                        "DontCountHits", "URLHash",
                                        "WindowClientWidth", "WindowClientHeight"}
                                ),
                                std::make_shared<AndExpression>(
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<AndExpression>(
                                                    std::make_shared<EqualExpression>(
                                                        "CounterID", "62"),
                                                    std::make_shared<GreaterOrEqualExpression>(
                                                        "EventDate", "2013-07-01")
                                                ),
                                                std::make_shared<LessOrEqualExpression>(
                                                    "EventDate", "2013-07-31")
                                            ),
                                            std::make_shared<EqualExpression>("IsRefresh", "0")
                                        ),
                                        std::make_shared<EqualExpression>("DontCountHits", "0")
                                    ),
                                    std::make_shared<EqualExpression>(
                                        "URLHash", "2868770270353813622")
                                )
                            ),
                            std::vector<std::string>{"WindowClientWidth", "WindowClientHeight"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<CountRowsAggregation>()};
                            }
                        ),
                        std::vector<std::string>{"count(*)"}, 3, true
                    ),
                    1
                )
            );
        },
        "1920,1080,2\n1024,768,1\n",
        7
    );
}

// Q42: SELECT DATE_TRUNC('minute', EventTime) AS M, COUNT(*) AS PageViews FROM hits WHERE CounterID = 62 AND EventDate >= '2013-07-14' AND EventDate <= '2013-07-15' AND IsRefresh = 0 AND DontCountHits = 0 GROUP BY M ORDER BY M LIMIT 2 OFFSET 1;
TEST(Test_Query, Q42) {
    RunQuery(
        "test_q42",
        "62,2013-07-14,0,0,2013-07-14 10:30:05\n"
        "62,2013-07-14,0,0,2013-07-14 10:30:45\n"
        "62,2013-07-14,0,0,2013-07-14 10:31:00\n"
        "62,2013-07-14,0,0,2013-07-14 10:31:20\n"
        "62,2013-07-14,0,0,2013-07-14 10:31:59\n"
        "62,2013-07-14,0,0,2013-07-14 10:32:10\n"
        "62,2013-07-15,1,0,2013-07-15 10:30:05\n"
        "62,2013-07-15,0,1,2013-07-15 10:30:05\n",
        "CounterID,int32\nEventDate,date\nIsRefresh,int16\nDontCountHits,int16\nEventTime,timestamp",
        [](const std::string& input_clmn, const std::string& result_csv, const std::string& result_schema) {
            return std::make_shared<WriteOperator>(
                result_csv,
                result_schema,
                std::make_shared<OffsetOperator>(
                    std::make_shared<TopKOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<ProjectOperator>(
                                std::make_shared<FilterOperator>(
                                    std::make_shared<ScanOperator>(
                                        input_clmn,
                                        std::vector<std::string>{
                                            "CounterID", "EventDate", "IsRefresh",
                                            "DontCountHits", "EventTime"}
                                    ),
                                    std::make_shared<AndExpression>(
                                        std::make_shared<AndExpression>(
                                            std::make_shared<AndExpression>(
                                                std::make_shared<AndExpression>(
                                                    std::make_shared<EqualExpression>(
                                                        "CounterID", "62"),
                                                    std::make_shared<GreaterOrEqualExpression>(
                                                        "EventDate", "2013-07-14")
                                                ),
                                                std::make_shared<LessOrEqualExpression>(
                                                    "EventDate", "2013-07-15")
                                            ),
                                            std::make_shared<EqualExpression>("IsRefresh", "0")
                                        ),
                                        std::make_shared<EqualExpression>("DontCountHits", "0")
                                    )
                                ),
                                std::make_shared<TruncateToMinuteExpression>("EventTime"),
                                "M"
                            ),
                            std::vector<std::string>{"M"},
                            []() {
                                return std::vector<std::shared_ptr<AggregationFunction>>{
                                    std::make_shared<CountRowsAggregation>()};
                            }
                        ),
                        std::vector<std::string>{"M"}, 3, false
                    ),
                    1
                )
            );
        },
        "2013-07-14 10:31:00,3\n2013-07-14 10:32:00,1\n",
        5
    );
}
