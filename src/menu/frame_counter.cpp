#include "frame_counter.hpp"

#include <iostream>

void FrameCounter::process() {
    ImGui::Begin(this->title.c_str(), nullptr, this->flags);
    ImGui::Text("FPS: %.1f", 1000.0f / this->delta.asMilliseconds());
    ImGui::End();
}