# Column-Engine

Колоночный движок хранения данных на C++. Реализует собственный бинарный формат `clmn`, оптимизированный для чтения отдельных колонок без сканирования всего файла, и набор операторов в стиле pull-based pipeline для выполнения аналитических запросов. Проект разработан как учебная реализация идей колоночных СУБД.

---

## Формат `clmn`

Файл состоит из **батчей** колонок, **секции схемы** и **footer'а**:

```
┌─ батч 0 ───────────────────────────────────────────────────────┐
│ metadata_offset : size_t                                       │
│ data_col_0      : raw bytes                                    │
│ data_col_1      : raw bytes                                    │
│ ...                                                            │
│ ── метаданные батча ────────────────────────────────────────── │
│ batch_rows      : size_t                                       │
│ offset_col_0    : size_t                                       │
│ offset_col_1    : size_t                                       │
│ ...                                                            │
└────────────────────────────────────────────────────────────────┘
 ...
┌─ схема ────────────────────────────────────────────────────────┐
│ для каждой колонки:                                            │
│   type     : uint8_t                                           │
│   name_len : size_t                                            │
│   name     : char[name_len]                                    │
└────────────────────────────────────────────────────────────────┘
┌─ footer ───────────────────────────────────────────────────────┐
│ rows_number       : size_t    <- общее число строк             │
│ columns_number    : size_t    <- число колонок                 │
│ batches_number    : size_t    <- число батчей                  │
│ compression_flags : size_t    <- флаги сжатия                  │
│ schema_offset     : size_t    <- байт от конца файла до схемы  │
└────────────────────────────────────────────────────────────────┘
```

---

## Сжатие

Каждая колонка кодируется независимо перед записью в `clmn`. По умолчанию включён **адаптивный режим** (`ENABLE_ADAPTIVE=ON`): кодирование выбирается в рантайме per-column путём оценки размера через `EstimateRle`/`EstimateDict`/`EstimateRaw`, выбранные флаги сохраняются в начале данных колонки.

`compression_flags` записывается в футер файла, поэтому читатель определяет схему декодирования из самого файла — независимо от своих compile-time флагов.

| CMake-флаг | По умолчанию | Типы | Описание |
|------------|-------------|------|----------|
| `ENABLE_DELTA_INT`  | OFF | Int16/Int32/Int64/Char/Date/Timestamp | Delta перед RLE (unsigned-арифметика) |
| `ENABLE_RLE_INT`    | ON  | Int16/Int32/Int64/Char/Date/Timestamp | RLE для целых |
| `ENABLE_RLE_FLOAT`  | ON  | Float/Double | RLE для вещественных |
| `ENABLE_RLE_STR`    | ON  | String | RLE для строк |
| `ENABLE_DICT_INT`   | ON  | Int16/Int32/Int64/Char/Date/Timestamp | Dictionary Encoding для целых |
| `ENABLE_DICT_FLOAT` | ON  | Float/Double | Dictionary Encoding для вещественных |
| `ENABLE_DICT_STR`   | ON  | String | Dictionary Encoding для строк |
| `ENABLE_BITPACK_INT`   | ON | Int16/Int32/Int64/Char/Date/Timestamp | Bit-packing значений и счётчиков RLE/Dict |
| `ENABLE_BITPACK_FLOAT` | ON | Float/Double | Bit-packing для RLE/Dict вещественных |
| `ENABLE_BITPACK_STR`   | ON | String | Bit-packing uint32-кодов Dictionary |
| `ENABLE_LZ4`        | ON  | все | LZ4 поверх закодированных байт: `[uncompressed_size][compressed_size][bytes]` |
| `ENABLE_ADAPTIVE`   | ON  | все целые, Float, String | Адаптивный выбор кодирования per-column в рантайме (игнорирует остальные флаги, кроме lz4) |

Изменить флаги и пересобрать:

```bash
# В script/build.sh и script/convert.sh отредактировать переменные в начале файла:
ENABLE_ADAPTIVE=OFF
ENABLE_RLE_INT=ON
ENABLE_DICT_STR=ON
ENABLE_LZ4=OFF

./script/build.sh
```

Или напрямую через CMake:

```bash
cmake -S . -B build -DENABLE_ADAPTIVE=OFF -DENABLE_LZ4=OFF
cmake --build build -j$(nproc)
```

---

## Зависимости

| Библиотека | Пакет (Ubuntu) | Назначение |
|------------|---------------|-----------|
| LZ4        | `liblz4-dev`  | Сжатие данных колонок |
| RE2        | `libre2-dev`  | `RegexpReplaceExpression` (Q28) |

```bash
apt-get install -y liblz4-dev libre2-dev
```

Или через `script/setup.sh` (также устанавливает clang-20 и cmake).

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

## Визуализация

Скрипты в `graph/` строят графики на основе данных, которые задаются вручную прямо в файле. Требуется Python с пакетами `matplotlib` и `numpy` (установлены в `venv/`).

```bash
source venv/bin/activate

# Перевести вывод bench_queries.sh в словарь {номер: мс}
python3 graph/parse_bench.py bench_output.txt

# Один запуск — время по всем запросам
python3 graph/queries_ms.py   # время в мс → graph/visualization/queries_ms_*.png
python3 graph/queries_s.py    # время в с  → graph/visualization/queries_s_*.png

# Горячий и холодный кэш
python3 graph/queries_runs_ms.py   # → queries_cold_ms_*.png, queries_hot_ms_*.png
python3 graph/queries_runs_s.py    # → queries_cold_s_*.png,  queries_hot_s_*.png

# Сравнение с DuckDB (горячий и холодный кэш)
python3 graph/compare_ms.py   # → compare_cold_ms_*.png, compare_hot_ms_*.png
python3 graph/compare_s.py    # → compare_cold_s_*.png,  compare_hot_s_*.png

# Размеры файлов
python3 graph/file_sizes.py   # → graph/visualization/file_sizes_*.png

deactivate
```

В каждом файле в начале задаются переменные `DATASET`, `BATCH_ROWS_NUMBER` и словари с данными — отредактировать и перезапустить.

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
    encoding.h / encoding.cpp        — EncodeColumn / DecodeColumn, GetCompressionFlags,
                                       EncodeIntegerVector, EncodeFloatVector, EncodeStringVector
    encoding_bitpack.h / .cpp        — BitsRequired, BitPackUnsigned, BitUnpackUnsigned
    encoding_lz4.h / .cpp            — Lz4Compress, Lz4Decompress
    encoding_base.h / .cpp             — ElementValueSize, WriteValues, ReadValues
    encoding_delta.h                 — DoDelta, UndoDelta, EncodeDelta, DecodeDelta (header-only)
    encoding_rle.h                   — EncodeRle, DecodeRle, EstimateRle (header-only)
    encoding_dict.h                  — EncodeDict, DecodeDict, EstimateDict (header-only)
    reader_writer_clmn.h / .cpp      — ReaderClmn, WriterClmn
    reader_writer_csv.h / .cpp       — ReaderCsv, WriterCsv
  convertion/
    from_csv_to_clmn.h / .cpp        — Конвертер .csv → .clmn
    from_clmn_to_csv.h / .cpp        — Конвертер .clmn → .csv
  execution/
    operators.h                      — базовый класс Operator
    expressions.h / .cpp             — все Expression; RegexpReplaceExpression использует RE2
    aggregation_functions.h / .cpp   — все AggregationFunction
    *_operators.h / .cpp             — Scan, Write, Filter, Project, GlobalAgg,
                                       GroupByAgg, TopK, Sort, Limit
exe/
  convert.cpp                        — CLI: конвертация .csv → .clmn
  run_query.cpp                      — CLI: выполнение запросов Q0–Q42
tests/                               — GoogleTest (4 набора)
clickbench/                          — датасет, схема, SQL-запросы, эталонные ответы
script/                              — shell-скрипты для сборки, конвертации, проверки
graph/
  parse_bench.py                     — парсер вывода bench_queries.sh → dict {номер: мс}
  queries_ms.py                      — график времени запросов (мс)
  queries_s.py                       — график времени запросов (с)
  queries_runs_ms.py                 — холодный и горячий кэш (мс), без DuckDB
  queries_runs_s.py                  — холодный и горячий кэш (с), без DuckDB
  compare_ms.py                      — сравнение с DuckDB, холодный и горячий кэш (мс)
  compare_s.py                       — сравнение с DuckDB, холодный и горячий кэш (с)
  file_sizes.py                      — сравнение размеров .clmn-файлов
  visualization/                     — сохранённые графики (.png)
```
