#include "src/convertion/from_clmn_to_csv.h"

void ConvertFromClmnToCsv(const std::string& file_clmn, const std::string& file_scheme, const std::string& file_csv) {
    std::cout << "Converting " << file_clmn << " to " << file_csv << " with scheme " << file_scheme << std::endl;
    ReaderClmn reader(file_clmn);
    reader.ReadSchema();
    WriterCsv writer(file_csv, file_scheme);
    while (std::shared_ptr<Batch> batch = reader.ReadBatch()) {
        writer.WriteBatch(batch);
    }
    writer.WriteSchema(reader.GetSchema());
    writer.WriteFooter();
}
