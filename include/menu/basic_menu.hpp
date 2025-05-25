#pragma once

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

class BasicMenu {
  public:
    BasicMenu(const sf::Time& delta) : delta(delta) {
        resize(sf::Vector2u(320, 320));
        move(sf::Vector2u(10, 10));
    }

    BasicMenu() : delta(sf::Time::Zero) {}
    virtual ~BasicMenu() = default;

    virtual void process() = 0;

    virtual void resize(const sf::Vector2u& size) {
        ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);
    }

    virtual void move(const sf::Vector2u& position) {
        ImGui::SetNextWindowPos(ImVec2(position.x, position.y),
                                ImGuiCond_Always);
    }

    virtual void setTitle(const std::string& new_title) {
        title = new_title;
    }

    virtual void setFlags(ImGuiWindowFlags new_flags) {
        flags = new_flags;
    }

  protected:
    // Clock to manage time
    const sf::Time& delta;
    std::string title = "Basic Menu";
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;
};