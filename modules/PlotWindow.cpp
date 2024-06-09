//
// Created by tehnokrat on 4/10/24.
//

#include <cmath>
#include "PlotWindow.h"

int PlotWindow::instance_count = 0;



// DEPRECATED
void PlotWindow::Render(bool connection_emitted, long times_delta, double current_data, double pid_output, std::string window_name) {
    // Add the current time and framerate to your data
    if(connection_emitted){
        times.push_back(times_delta);
        framerates.push_back(current_data);
        pid_outs.push_back(pid_output);
    }

    ImGui::SetNextWindowSize(size_of_plot);
    window_name = "client_" + std::to_string(id);
    if(ImGui::Begin(window_name.c_str())){
        if(ImPlot::BeginPlot("Data from Sensor") ){
            ImPlot::SetupAxes("Time, ms", "Data from sensor", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotLine("Sensor input", times.data(), framerates.data(), framerates.size());
            ImPlot::PlotLine("PID output", times.data(), pid_outs.data(), pid_outs.size());
        }
        ImPlot::EndPlot();
    }
    ImGui::Separator();
    if(ImGui::BeginChild("Data")){
        std::string output = "Framerate " + std::to_string(current_data) + " At " + std::to_string(times_delta / 1000) + " seconds from started application";
        ImGui::Text("%s", output.c_str());
    }
    ImGui::EndChild();

    ImGui::SeparatorText("PID control");
    ImGui::SliderFloat("Kp", &slider_kp, -1, 1);
    ImGui::SliderFloat("Ki", &slider_ki, -1, 1);
    ImGui::SliderFloat("Kd", &slider_kd, -1, 1);
    ImGui::SliderFloat("max average error", &slider_error, 0.0005, 0.00001, "%.5f", ImGuiSliderFlags_NoRoundToFormat);
    ImGui::End();
}



void PlotWindow::Render(long times_delta, double current_data, std::string window_name) {
    times.push_back(times_delta);
    framerates.push_back(current_data);
    pid_outs.push_back(pid.GetSteerValue());


    window_name = "client_" + std::to_string(id);
    if(ImGui::Begin(window_name.c_str())){
        if(ImPlot::BeginPlot("Data from Sensor") ){
            ImPlot::SetupAxes("Time, ms", "Data from sensor", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::PlotLine("Sensor input", times.data(), framerates.data(), framerates.size());
            if(pid_enable)
                ImPlot::PlotLine("PID output", times.data(), pid_outs.data(), pid_outs.size());
        }
        ImPlot::EndPlot();
    }
    ImGui::Separator();
    if(ImGui::BeginChild("Data", ImVec2(640, 20))){
        std::string output = "Accepted data from client: " + std::to_string(current_data) + " At " + std::to_string(times_delta / 1000) + " seconds from started application";
        ImGui::Text("%s", output.c_str());
    }
    ImGui::EndChild();

    if(ImGui::BeginChild("PID Control child widget")){
        ImGui::SeparatorText("PID control");

        if(ImGui::Button(enable_pid_button_label.c_str())){
            enable_pid_button_label = pid_enable ? "Enable PID configuration" : "Disable PID control" ;
            pid_enable = !pid_enable;
        }

        if(pid_enable){
            ImGui::SameLine();
            enable_autotune_button_label = autotune_enabled ? "Disable autotune" : "Enable autotune";
            if(ImGui::Button(enable_autotune_button_label.c_str())){
                autotune_enabled = !autotune_enabled;
                std::cout<<"Button autotune clocked: "<<autotune_enabled<<std::endl;
            }

            update_pid(1, current_data);

            ImGui::SliderFloat("Kp", &slider_kp, -1, 1, "%.5f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::SliderFloat("Ki", &slider_ki, -1, 1, "%.5f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::SliderFloat("Kd", &slider_kd, -1, 1, "%.5f", ImGuiSliderFlags_NoRoundToFormat);
            ImGui::SliderFloat("max average error", &slider_error, 0.05, 0.00001, "%.5f", ImGuiSliderFlags_NoRoundToFormat);
        }

    }
    ImGui::EndChild();

    ImGui::End();
}

void PlotWindow::update_pid(double set_point, double input_data) {
    double error = set_point - input_data;
    pid.UpdateError(error);
    //std::cout<<"KD: "<<pid.Kd<<" KP: "<<pid.Kp<<" KI: "<<pid.Ki<<std::endl;
    recent_errors.push_back(error);
    sum_errors += error;
    if (recent_errors.size() > max_errors_size) {
        sum_errors -= recent_errors.front();
        recent_errors.pop_front();
    }
    double average_error = sum_errors / recent_errors.size();
    if (autotune_enabled) {
        std::string output_autotune = "Started autotuning " +
                                      std::to_string(pid.Kp ) + " " +
                                      std::to_string(pid.Ki) + " " +
                                      std::to_string(pid.Kd );
        std::cout<<output_autotune<<std::endl;
        std::cout << "Before checking error: " << "average_error = "
                  << average_error << ", slider_error = "
                  << slider_error << ", autotune_enabled = "
                  << autotune_enabled << std::endl;
        pid.AutoTuneController(average_error);
        if(std::fabs(average_error) < slider_error){
            autotune_enabled = false;
            output_autotune = "Autotuning ended with " +
                              std::to_string(pid.Kp ) + " " +
                              std::to_string(pid.Ki) + " " +
                              std::to_string(pid.Kd ) +
                              "\n Average Error == " + std::to_string(average_error);
            std::cout<<output_autotune<<std::endl;
            slider_kp = pid.Kp;
            slider_ki = pid.Ki;
            slider_kd = pid.Kd;
        }
        std::cout << "After checking error: " << "average_error = "
                  << average_error << ", slider_error = "
                  << slider_error << ", autotune_enabled = "
                  << autotune_enabled << std::endl;
        std::cout<<"ENDED"<<std::endl;
    }
    else{
        pid.Kp = slider_kp;
        pid.Ki = slider_ki;
        pid.Kd = slider_kd;
    }
}

