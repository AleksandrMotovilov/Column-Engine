#include "src/convertion/from_csv_to_clmn.h"
#include "src/kernel/reader_writer_csv.h"
#include "src/kernel/reader_writer_clmn.h"

void ConvertFromCsvToClmn(const std::string& file_csv, const std::string& file_scheme, const std::string& file_clmn) {
    std::cout << "Converting " << file_csv << " with scheme " << file_scheme << " to " << file_clmn << std::endl;
    ReaderCsv reader(file_csv, file_scheme);
    reader.ReadSchema();
    WriterClmn writer(file_clmn);
    while (std::shared_ptr<Batch> batch = reader.ReadBatch()) {
        writer.WriteBatch(batch);
    }
    writer.WriteSchema(reader.GetSchema());
    writer.WriteFooter();
}
