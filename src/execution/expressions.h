#pragma once

#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include "src/batch.h"

class Expression {
public:
    virtual ~Expression() = default;
    virtual std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const = 0;
    virtual Type GetType() const = 0;
};

class EqualExpression : public Expression {
public:
    EqualExpression(std::string column_name, std::string constant_value);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::string value_;
};

class NotEqualExpression : public Expression {
public:
    NotEqualExpression(std::string column_name, std::string constant_value);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::string value_;
};

class GreaterOrEqualExpression : public Expression {
public:
    GreaterOrEqualExpression(std::string column_name, std::string value);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::string value_;
};

class LessOrEqualExpression : public Expression {
public:
    LessOrEqualExpression(std::string column_name, std::string value);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::string value_;
};

class ContainsExpression : public Expression {
public:
    ContainsExpression(std::string column_name, std::string substring);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::string substring_;
};

class NotContainsExpression : public Expression {
public:
    NotContainsExpression(std::string column_name, std::string substring);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::string substring_;
};

class AndExpression : public Expression {
public:
    AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
};

class OrExpression : public Expression {
public:
    OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
};

class ConstantExpression : public Expression {
public:
    explicit ConstantExpression(std::string value);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string value_;
};

class SumExpression : public Expression {
public:
    SumExpression(std::string column_name, int64_t constant);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    int64_t constant_;
};

class LengthExpression : public Expression {
public:
    explicit LengthExpression(std::string column_name);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
};

class RegexpReplaceExpression : public Expression {
public:
    RegexpReplaceExpression(std::string column_name, std::string pattern, std::string replacement);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
    std::regex pattern_;
    std::string replacement_;
};

class CaseWhenExpression : public Expression {
public:
    CaseWhenExpression(std::shared_ptr<Expression> condition, std::string then_column, std::string else_value);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::shared_ptr<Expression> condition_;
    std::string then_column_;
    std::string else_value_;
};

class ExtractMinuteExpression : public Expression {
public:
    explicit ExtractMinuteExpression(std::string column_name);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
};

class TruncateToMinuteExpression : public Expression {
public:
    explicit TruncateToMinuteExpression(std::string column_name);
    std::shared_ptr<Column> Eval(std::shared_ptr<Batch> batch) const override;
    Type GetType() const override;

private:
    std::string column_name_;
};
