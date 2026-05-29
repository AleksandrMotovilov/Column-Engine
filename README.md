# Column-Engine

Колоночный движок хранения данных на C++. Реализует собственный бинарный формат `.clmn`, оптимизированный для чтения отдельных колонок без сканирования всего файла, и набор операторов в стиле pull-based pipeline для выполнения аналитических запросов. Проект разработан как учебная реализация идей колоночных СУБД.

---

## Формат `.clmn`

Файл состоит из **батчей** колонок и **футера**:

```
┌─ батч 0 ──────────────────────────────────────────────────────┐
│ metadata_offset : uint64_t   <- смещение до секции метаданных  │
│ data_col_0      : raw bytes  <- данные первой колонки           │
│ data_col_1      : raw bytes                                    │
│ ...                                                            │
│ ── секция метаданных ──────────────────────────────────────── │
│ batch_size      : size_t     <- число колонок в батче           │
│ offset_col_0    : uint64_t   <- смещение от начала батча        │
│ type_col_0      : uint8_t    <- тип колонки (enum Type)         │
│ name_len_0      : size_t                                       │
│ name_0          : char[name_len_0]                             │
│ ... (аналогично для каждой колонки батча)                      │
└───────────────────────────────────────────────────────────────┘
┌─ батч 1 ─ ...
┌─ футер ───────────────────────────────────────────────────────┐
│ rows_number    : size_t      <- общее число строк в таблице     │
│ columns_number : size_t      <- общее число колонок             │
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
  types.h / types.cpp            — Type, Date и Timestamp, FromString/ToString
  column.h / column.cpp          — Column, ColumnTyped<T>, GetStringValueAt, CopyRowsTyped,
                                   MakeSingleValueColumn, MakeColumnFromStrings
  batch.h / batch.cpp            — Batch, kColumnBatchSize, kRowBatchSize, SetBatchSize
  convertion/                    — конвертеры .csv <-> .clmn
  execution/                     — операторы запросов (Scan, Filter, Project, Agg, Sort, …)
exe/
  convert.cpp                    — CLI: конвертация .csv -> .clmn
  run_query.cpp                  — CLI: выполнение запроса по номеру
tests/                           — GoogleTest
clickbench/                      — датасет, схема, SQL-запросы, эталонные ответы
script/                          — shell-скрипты для сборки, конвертации, проверки
```
