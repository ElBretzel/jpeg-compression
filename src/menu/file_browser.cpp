#include "file_browser.hpp"

#include <iostream>

void FileBrowser::process() {
    ImGui::Begin(title.c_str(), nullptr, flags);

    ImGui::Button("Open File Browser");
    if (ImGui::IsItemClicked()) {
        fileBrowser.Open();
    }
    ImGui::End();

    fileBrowser.Display();

    if (fileBrowser.HasSelected()) {
        selectedFile = fileBrowser.GetSelected().c_str();
        fileAvailable = true;
        fileBrowser.ClearSelected();
    }
}