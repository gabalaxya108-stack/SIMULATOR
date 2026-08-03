#include <iostream>
#include <string>

#include "WebAppService.h"

int main(int argc, char** argv) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port. Using 8080 instead." << std::endl;
        }
    }

    std::cout << "Starting COA web simulator..." << std::endl;
    WebAppService::runServer(port);
    return 0;
}
