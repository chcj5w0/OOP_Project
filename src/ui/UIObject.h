#pragma once

class UIObject {
    public:
        virtual ~UIObject() = default;
        virtual void render() = 0;
};