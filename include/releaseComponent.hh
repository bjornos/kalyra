#pragma once

#include <vector>
#include <iostream>

class releaseComponent {
public:
    releaseComponent(std::vector<std::string> preCommands, std::vector<std::string>	postCommands,
        std::vector<std::string> components);
    ~releaseComponent();

    std::string getFileName(std::string& pathName);
    std::vector<std::string>& getPreCommands();
    std::vector<std::string>& getComponents();
    std::vector<std::string>& getPostCommands();

private:
    std::vector<std::string> preCommands;
    std::vector<std::string> components;
    std::vector<std::string> postCommands;
};
