#pragma once

#include <memory>
#include <string>

// Own this after InitWindow, and destroy it before CloseWindow.
class ZepExample {
public:
    ZepExample();
    ~ZepExample();
    void draw();
    std::string text() const;
    bool quit_requested() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
