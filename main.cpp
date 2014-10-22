#include <iostream>

#include "ConsoleHelper.hpp"


int main() {

    ConsoleHelper consoleHelper;

    std::cout << "Please input your name. (Press enter to confirm)" << std::endl;
    std::string name = consoleHelper.getLimitedInput(10);
    std::cout << "Hello " << name << "!" << std::endl;

    std::cout << "Please input your message. (Press enter to send)" << std::endl;
    std::string messageString = consoleHelper.getLimitedInput(150);

    std::cout << messageString.length() << std::endl;

    return 0;
}
