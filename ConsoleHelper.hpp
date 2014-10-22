#ifndef CONSOLEHELPER_INCLUDE
#define CONSOLEHELPER_INCLUDE

#include <string>
#include <termios.h>



class ConsoleHelper {

public:

    ConsoleHelper();
    std::string getLimitedInput(int maximumNumberOfCharacters);
    unsigned int getConsoleWidth();

private:
    void enableUnbufferedConsole();
    void restoreConsoleSettings();
    int getNumberOfDigits(int number);


    bool m_isBuffered;
    struct termios m_oldConsolSettings;
};


#endif // CONSOLEHELPER_INCLUDE
