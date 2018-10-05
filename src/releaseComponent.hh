#pragma once

#include <vector>
#include <iostream>

class releaseComponent {
public:
    releaseComponent(std::vector<std::string> preCommands, std::vector<std::string>	postCommands,
        std::vector<std::string> components, std::string path);

    ~releaseComponent();

    std::string releasePath;
    std::vector<std::string> preCommands;
    std::vector<std::string> components;
    std::vector<std::string> postCommands;
};
