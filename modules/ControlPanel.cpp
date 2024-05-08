//
// Created by tehnokrat on 4/10/24.
//

#include "ControlPanel.h"


void ControlPanel::Render(bool& connection_emitted, bool& autotune_enable, std::vector<std::string>& logs) {


    if (ImGui::Begin("Control panel")) // begin window
    {
        ImGui::SeparatorText("Connection to socket");
        if (ImGui::Button(connection_button_label.c_str())) // Buttons return true when clicked.
        {
            connection_button_label = (connection_button_label == "Connect") ? "Disconnect" : "Connect";
            logs.emplace_back(!connection_emitted ? "Connection emitted..." : "Connection lost");
            connection_emitted = !connection_emitted;
        }

        ImGui::InputText("IP address", const_cast<char *>((ip_preset).c_str()), ip_preset.size()+1);

        ImGui::InputInt("PORT", &port_preset, 1, 10);
        ImGui::SeparatorText("PID regulator configuration");
        if(ImGui::Button(autotune_button_label.c_str())){
            autotune_button_label = autotune_enable ? "Disable" : "Enable";
            autotune_enable = !autotune_enable;
        }
        ImGui::Separator();
        ImGui::SliderFloat("Kp", &slider_kp, -1, 1);
        ImGui::SliderFloat("Ki", &slider_ki, -1, 1);
        ImGui::SliderFloat("Kd", &slider_kd, -1, 1);
        ImGui::SliderFloat("max average error", &slider_error, 0.0005, 0.00001, "%.5f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::SeparatorText("Logs section");
        if(ImGui::BeginChild("Logs")){
            for (const std::string& log : logs){
                ImGui::TextUnformatted(log.c_str());
            }
            // Auto scroll to the bottom when a new log is added
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild(); // end child window
        }
    }
    ImGui::End(); // Control Window
}