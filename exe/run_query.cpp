#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "src/convertion/from_clmn_to_csv.h"
#include "src/execution/operators.h"

static void RunQuery(int query_num, const std::string& input_clmn, const std::string& output_dir) {
    std::filesystem::create_directories(output_dir);

    char padded[8];
    std::snprintf(padded, sizeof(padded), "q%02d", query_num);
    std::string output_schema = output_dir + "/schema.csv";
    std::string output_csv = output_dir + "/" + std::string(padded) + ".csv";

    std::shared_ptr<Operator> plan;
    switch (query_num) {
        case 0:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountRowsAggregation>()
                    }
                ));
            break;
        case 1:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 2:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 3:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"UserID"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<AvgAggregation>("UserID")
                    }
                ));
            break;
        case 4:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"UserID"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountDistinctAggregation>("UserID")
                    }
                ));
            break;
        case 5:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"SearchPhrase"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<CountDistinctAggregation>("SearchPhrase")
                    }
                ));
            break;
        case 6:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"EventDate"}
                    ),
                    std::vector<std::shared_ptr<AggregationFunction>>{
                        std::make_shared<MinAggregation>("EventDate"),
                        std::make_shared<MaxAggregation>("EventDate")
                    }
                ));
            break;
        case 7:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 8:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 9:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 10:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 11:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 12:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 13:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 14:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 15:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 16:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 17:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 18:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 19:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<FilterOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn,
                        std::vector<std::string>{"UserID"}
                    ),
                    std::make_shared<EqualExpression>("UserID", "435090932899640449")
                ));
            break;
        case 20:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 21:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 22:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 23:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{}, true
                        ),
                        std::make_shared<ContainsExpression>("URL", "google")
                    ),
                    std::vector<std::string>{"EventTime"}, 10, false
                ));
            break;
        case 24:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"SearchPhrase", "EventTime"}
                        ),
                        std::make_shared<NotEqualExpression>("SearchPhrase", "")
                    ),
                    std::vector<std::string>{"EventTime"}, 10, false
                ));
            break;
        case 25:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"SearchPhrase"}
                        ),
                        std::make_shared<NotEqualExpression>("SearchPhrase", "")
                    ),
                    std::vector<std::string>{"SearchPhrase"}, 10, false
                ));
            break;
        case 26:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<TopKOperator>(
                    std::make_shared<FilterOperator>(
                        std::make_shared<ScanOperator>(
                            input_clmn,
                            std::vector<std::string>{"SearchPhrase", "EventTime"}
                        ),
                        std::make_shared<NotEqualExpression>("SearchPhrase", "")
                    ),
                    std::vector<std::string>{"EventTime", "SearchPhrase"}, 10, false
                ));
            break;
        case 27:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                        std::make_shared<GreaterOrEqualExpression>("count(*)", "100001")
                    ),
                    std::vector<std::string>{"avg(strlen(URL))"}, 25, true
                ));
            break;
        case 28:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                        std::make_shared<GreaterOrEqualExpression>("count(*)", "100001")
                    ),
                    std::vector<std::string>{"avg(strlen(Referer))"}, 25, true
                ));
            break;
        case 29: {
            std::vector<std::shared_ptr<AggregationFunction>> aggs;
            for (int64_t i = 0; i <= 89; i++) {
                aggs.push_back(
                    std::make_shared<SumWithOffsetAggregation>("ResolutionWidth", i));
            }
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<GlobalAggregationOperator>(
                    std::make_shared<ScanOperator>(
                        input_clmn, std::vector<std::string>{"ResolutionWidth"}),
                    aggs));
            break;
        }
        case 30:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 31:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 32:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 33:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 34:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 35:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                        std::vector<std::string>{
                            "ClientIP", "ClientIP - 1", "ClientIP - 2", "ClientIP - 3"},
                        []() {
                            return std::vector<std::shared_ptr<AggregationFunction>>{
                                std::make_shared<CountRowsAggregation>()
                            };
                        }
                    ),
                    std::vector<std::string>{"count(*)"}, 10, true
                ));
            break;
        case 36:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 37:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                ));
            break;
        case 38:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                        std::vector<std::string>{"count(*)"}, 1010, true
                    ),
                    1000
                ));
            break;
        case 39:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                        std::vector<std::string>{"count(*)"}, 1010, true
                    ),
                    1000
                ));
            break;
        case 40:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                                        std::make_shared<EqualExpression>(
                                            "RefererHash", "3594120000172545465")
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
                        std::vector<std::string>{"count(*)"}, 110, true
                    ),
                    100
                ));
            break;
        case 41:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
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
                                    std::make_shared<CountRowsAggregation>()
                                };
                            }
                        ),
                        std::vector<std::string>{"count(*)"}, 10010, true
                    ),
                    10000
                ));
            break;
        case 42:
            plan = std::make_shared<WriteOperator>(
                output_csv,
                output_schema,
                std::make_shared<OffsetOperator>(
                    std::make_shared<TopKOperator>(
                        std::make_shared<GroupByAggregationOperator>(
                            std::make_shared<ProjectOperator>(
                                std::make_shared<FilterOperator>(
                                    std::make_shared<ScanOperator>(
                                        input_clmn,
                                        std::vector<std::string>{"CounterID", "EventDate", "IsRefresh", "DontCountHits", "EventTime"}
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
                                    std::make_shared<CountRowsAggregation>()
                                };
                            }
                        ),
                        std::vector<std::string>{"M"}, 1010, false
                    ),
                    1000
                ));
            break;
        default:
            std::cerr << "ERROR: Query " << query_num << " is not implemented\n";
            std::exit(1);
    }

    plan->Next();
}

int main(int argc, char** argv) {
    SetBatchSize(100000, 100000);

    std::string input_clmn;
    std::string output_dir;
    int query_num = -1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--input" && i + 1 < argc) {
            input_clmn = argv[++i];
        } else if (arg == "--output_dir" && i + 1 < argc) {
            output_dir = argv[++i];
        } else if (arg.substr(0, 9) == "--queries") {
            if (arg.size() > 9 && arg[9] == '=') {
                query_num = std::stoi(arg.substr(10));
            } else if (i + 1 < argc) {
                query_num = std::stoi(argv[++i]);
            }
        }
        // --schema accepted but unused: column types are embedded in .clmn metadata
    }

    if (input_clmn.empty() || output_dir.empty() || query_num < 0) {
        std::cerr << "Usage: run_query --input <clmn> [--schema <schema>]"
                     " --output_dir <dir> --queries=N\n";
        return 1;
    }

    RunQuery(query_num, input_clmn, output_dir);
    return 0;
}
