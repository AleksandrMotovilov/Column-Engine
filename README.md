# Column-Engine

Колоночный движок хранения данных на C++. Реализует собственный бинарный формат `.clmn`, оптимизированный для чтения отдельных колонок без сканирования всего файла, и набор операторов в стиле pull-based pipeline для выполнения аналитических запросов. Проект разработан как учебная реализация идей колоночных СУБД.

---

## Формат `.clmn`

Файл состоит из **батчей** колонок, **секции схемы** и **футера**:

```
┌─ батч 0 ──────────────────────────────────────────────────────┐
│ metadata_offset : size_t                                       │
│ data_col_0      : raw bytes                                    │
│ data_col_1      : raw bytes                                    │
│ ...                                                            │
│ ── метаданные батча ───────────────────────────────────────── │
│ batch_rows      : size_t   <- число строк в батче              │
│ offset_col_0    : size_t   <- смещение от начала батча         │
│ offset_col_1    : size_t                                       │
│ ...                                                            │
└───────────────────────────────────────────────────────────────┘
┌─ батч 1 ─ ...
┌─ схема ────────────────────────────────────────────────────────┐
│ для каждой колонки:                                            │
│   type     : uint8_t                                           │
│   name_len : size_t                                            │
│   name     : char[name_len]                                    │
└───────────────────────────────────────────────────────────────┘
┌─ футер ────────────────────────────────────────────────────────┐
│ rows_number    : size_t    <- общее число строк                 │
│ columns_number : size_t    <- число колонок                     │
│ batches_number : size_t    <- число батчей                      │
│ schema_offset  : size_t    <- байт от конца файла до схемы     │
└───────────────────────────────────────────────────────────────┘
```

---

## Запуск тестов

Тесты запускаются из директории `build/` — там они создают временные файлы.

```bash
cd build
cmake ..   # только если менялись CMakeLists.txt
cmake --build . --target test_convertion test_queries test_aggregation_functions test_expressions
./tests/test_convertion
./tests/test_queries
./tests/test_aggregation_functions
./tests/test_expressions
```

- `test_convertion` — тесты `.csv` -> `.clmn` -> `.csv`.
- `test_queries` — тесты запросов Q0–Q42: строит план, выполняет, сравнивает результат с эталоном.
- `test_aggregation_functions` — тесты агрегаций.
- `test_expressions` — тесты выражений.

---

## Проверка корректности запросов

Скрипт сравнивает вывод каждого запроса с эталонными ответами из `clickbench/results/small/`.

**Шаг 1 — сконвертировать датасет:**

```bash
./script/convert.sh clickbench/hits_sample.csv clickbench/hits_sample.clmn
```

Третий аргумент — схема — не обязателен, по умолчанию `clickbench/hits.schema`.

**Шаг 2 — проверить все запросы:**

```bash
./script/check_queries.sh [путь_к_.clmn]
```

Для каждого Q0–Q42 выводит одну из трёх строк:

```
Q00: OK
Q01: WRONG  expected=1 got=0
Q21: NOT IMPLEMENTED
```

Аргумент по умолчанию: `clickbench/hits_sample.clmn`.

---

## Замер времени запросов

```bash
./script/bench_queries.sh [путь_к_.clmn]
```

Для каждого Q0–Q42 измеряет время выполнения `run_query` в секундах:

```
Q00: 0.012s
Q01: 0.008s
Q21: N/A
```

Нереализованные запросы выводятся как `N/A`.

---

## Структура проекта

```
src/
  kernel/
    types.h / types.cpp              — Type, Date, Timestamp, FromString/ToString/TypeOf
    column.h / column.cpp            — Column, ColumnTyped<T>
    column_utils.h / column_utils.cpp — GetStringValueAt, CopyRowsTyped,
                                        MakeSingleValueColumn, MakeColumnFromStrings,
                                        CompareStringValues
    schema.h / schema.cpp            — Schema
    batch.h / batch.cpp              — Batch, kColumnBatchSize, kRowBatchSize, SetBatchSize
    reader_writer_clmn.h / .cpp      — ReaderClmn, WriterClmn
    reader_writer_csv.h / .cpp       — ReaderCsv, WriterCsv
  convertion/
    from_csv_to_clmn.h / .cpp        — Конвертер .csv → .clmn
    from_clmn_to_csv.h / .cpp        — Конвертер .clmn → .csv
  execution/
    operators.h                      — базовый класс Operator
    expressions.h / .cpp             — все Expression
    aggregation_functions.h / .cpp   — все AggregationFunction
    *_operators.h / .cpp             — Scan, Write, Filter, Project, GlobalAgg,
                                       GroupByAgg, TopK, Sort, Limit
exe/
  convert.cpp                        — CLI: конвертация .csv → .clmn
  run_query.cpp                      — CLI: выполнение запросов Q0–Q42
tests/                               — GoogleTest (4 набора)
clickbench/                          — датасет, схема, SQL-запросы, эталонные ответы
script/                              — shell-скрипты для сборки, конвертации, проверки
```
