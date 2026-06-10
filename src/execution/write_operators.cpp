#include "src/execution/write_operators.h"

WriteOperator::WriteOperator(const std::string& file_csv, const std::string& file_schema, std::shared_ptr<Operator> next) {
    next_ = std::move(next);
    done_ = false;
    writer_ = std::make_unique<WriterCsv>(file_csv, file_schema);
}

std::shared_ptr<Batch> WriteOperator::Next() {
    if (done_) {
        return nullptr;
    }
    done_ = true;
    std::shared_ptr<Schema> schema;
    while (std::shared_ptr<Batch> batch = next_->Next()) {
        if (!schema) {
            schema = batch->GetSchema();
        }
        writer_->WriteBatch(batch);
    }
    if (schema) {
        writer_->WriteSchema(schema);
    }
    writer_->WriteFooter();
    return nullptr;
}
