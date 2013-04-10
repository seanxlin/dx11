#pragma once

namespace Application
{
    bool init();
    void destroy();

    int run();

    void updateScene(const float dt);

    void drawScene();
}