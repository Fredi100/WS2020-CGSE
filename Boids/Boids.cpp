#include <iostream>

#include "GraphicsService.h" // Graphics Implementation


int main() {

    GraphicsService& gService = GraphicsService::getInstance();

    gService.initialize();
    gService.run();


    exit(EXIT_SUCCESS);
}
