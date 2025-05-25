#pragma once

#include "basic_menu.hpp"
#include "imfilebrowser.h"

class FileBrowser : public BasicMenu {
  public:
    FileBrowser(const sf::Time& delta) : BasicMenu(delta) {
        setTitle("File Browser");
        setFlags(ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);
    }

    FileBrowser() {}

    void process() override;

    void setTitle(const std::string& new_title) override {
        title = new_title;
        fileBrowser.SetTitle(new_title);
    }

    void setFilters(const std::vector<std::string>& filters) {
        fileBrowser.SetTypeFilters(filters);
    }

    bool fileAvailable = false;

    std::string getSelectedFile() {
        fileAvailable = false;
        return selectedFile;
    }

  private:
    ImGui::FileBrowser fileBrowser;
    std::string selectedFile = "";
};