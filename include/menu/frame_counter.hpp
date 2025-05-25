#pragma once

#include "menu/basic_menu.hpp"

class FrameCounter : public BasicMenu {

  public:
    FrameCounter(const sf::Time& delta) : BasicMenu(delta) {
        resize({80, 30});
        setTitle("Frame Counter");
        setFlags(ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                 ImGuiWindowFlags_NoMove);
    }

    void process() override;
};