/************************************************/
/* main.cc, (c) Rene Puchinger                  */
/************************************************/

#include <cstdlib>
#include <iostream>
#include "Application.h"
#include "Exception.h"

int main(int argc, char** argv) {
    try {
        Application* app = new Application(argc, argv);
        app->run();
        delete app;
    } catch (Exception e) {
        std::cout << "ERROR: " << e.get_message() << std::endl;
    }
    return 0;
}
