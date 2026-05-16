#pragma once

#include "SimObject.h"
#include "Product.h"
#include <memory>
#include <string>

class Connector : public SimObject {
public:
    explicit Connector(int id) : SimObject(id) {}
    ~Connector() override = default;

    virtual bool push(std::unique_ptr<Product> p) = 0;
    virtual std::unique_ptr<Product> pop() = 0;
    virtual int  size()     const = 0;
    virtual int  capacity() const = 0;

    float loadRatio() const {
        return capacity() ? float(size()) / capacity() : 0.f;
    }

    std::string getInfo() const override {
        return "Connector[" + std::to_string(id()) + "] " +
               std::to_string(size()) + "/" + std::to_string(capacity());
    }
};
