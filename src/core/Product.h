#pragma once

#include <string>

class Product {
public:
    virtual ~Product() = default;
    virtual std::string name() const = 0;
};

class RawFeed : public Product {
public:
    std::string name() const override { return "RawFeed"; }
};

class Intermediate : public Product {
public:
    std::string name() const override { return "Intermediate"; }
};

class FinishedProduct : public Product {
public:
    std::string name() const override { return "FinishedProduct"; }
};
