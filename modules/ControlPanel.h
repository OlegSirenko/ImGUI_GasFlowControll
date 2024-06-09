//
// Created by OlegSirenko on 4/10/24.
//

#ifndef IMGUI_GASFLOWCONTROLL_CONTROLPANEL_H
#define IMGUI_GASFLOWCONTROLL_CONTROLPANEL_H

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"
#include "implot.h"
#include <iostream>
#include <vector>
#include <chrono>

class ControlPanel {
public:
    ControlPanel(int& window_width, int& window_height, int& window_position_x, int& window_position_y):
            window_width(window_width), window_height(window_height),
            window_position_x(window_position_x), window_position_y(window_position_y){}

    void Render(bool&, std::vector<std::string>&);
private:
    int& window_width;
    int& window_height;
    int& window_position_x;
    int& window_position_y;

    std::string connection_button_label = "Start server";
    std::string autotune_button_label = "Enable";

public:
    float slider_kp = 0.025;
    float slider_ki = 0.064;
    float slider_kd = 0.28;
    float slider_error = 0.0004;
    std::string ip_preset = "255.255.255.255";
    int port_preset = 12000;
    int num_connections = 0;
};


#endif //IMGUI_GASFLOWCONTROLL_CONTROLPANEL_H