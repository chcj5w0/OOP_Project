#pragma once

#include "SimObject.h"
#include "Product.h"
#include "bridge/ConnectorSnap.h"
#include <memory>
#include <string>
#include <vector>

class Connector : public SimObject {
public:
    explicit Connector(int id) : SimObject(id) {}
    ~Connector() override = default;

    virtual bool push(std::unique_ptr<Product> p) = 0;
    virtual std::unique_ptr<Product> pop() = 0;
    virtual int  size()     const = 0;
    virtual int  capacity() const = 0;

    ConnectorSnap snapshot() const;

    float loadRatio() const {
        return capacity() ? float(size()) / capacity() : 0.f;
    }

    std::string getInfo() const override {
        return "Connector[" + std::to_string(id()) + "] " +
               std::to_string(size()) + "/" + std::to_string(capacity());
    }

protected:
    virtual const char* typeName() const = 0;
    // Product names per position, exit end first ("" = empty slot).
    virtual std::vector<std::string> slotNames() const = 0;
};
