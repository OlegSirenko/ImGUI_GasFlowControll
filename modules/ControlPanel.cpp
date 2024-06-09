//
// Created by tehnokrat on 4/10/24.
//

#include "ControlPanel.h"


void ControlPanel::Render(bool& connection_emitted, std::vector<std::string>& logs) {
    if (ImGui::Begin("Control panel")) // begin window
    {
        ImGui::SeparatorText("Server options");
        ImGui::BeginDisabled(true);
        ImGui::InputInt("PORT", &port_preset, 1, 10);
        ImGui::SliderInt("Num of connections", &num_connections,0, 10);
        ImGui::EndDisabled();
        ImGui::SeparatorText("Logs section");
        if(ImGui::BeginChild("Logs")){
            for (const std::string& log : logs){
                ImGui::TextUnformatted(log.c_str());
            }
            // Auto scroll to the bottom when a new log is added
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild(); // end child window
    }
    ImGui::End(); // Control Window
}