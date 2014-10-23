#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdio>

#include "ConsoleHelper.hpp"

ConsoleHelper::ConsoleHelper() : m_isBuffered(false)
{

}


std::string ConsoleHelper::getLimitedInput(unsigned int maximumNumberOfCharacters)
{
    enableUnbufferedConsole();

    int characterCount = maximumNumberOfCharacters;
    const int numberOfDigits = getNumberOfDigits(maximumNumberOfCharacters);
    std::cout << "[" << std::setw(numberOfDigits) << characterCount << "] : ";
    std::string messageString;

    while (true) {
        char c = getchar();

        if (c == 10) {
            // if the enter key is pressed and the string is not empty stop the input
            if (messageString.length() > 0)
                break;
        }
        else if (c == 27) {
            // is this an ANSI escape code?
            char temp = getchar(); // this is not good! if only escape pressed (no sequence) we are stuck here
            if (temp == 91) {
                char codeChar = getchar();

                // left arrow key
                if (codeChar == 68) {
                    std::cout << "\33[D"; // move cursor left
                }
                // right arrow key
                else if (codeChar == 67) {
                    std::cout << "\33[C"; // move cursor right
                }
            }

            // if not it's the escape key and we ignore it
            continue;
        }
        else if (c == 127) {
            // a backspace character removes the last character from the string
            if (messageString.length() > 0) {
                messageString.erase(messageString.end() - 1);
                characterCount++;
            }
        }
        else {
            // all other characters are added to the back of the message string
            if (messageString.length() < maximumNumberOfCharacters) {
                messageString.push_back(c);
                characterCount--;
            }
        }

        // check if we are spanning over multiple lines
        int linesToClear = std::floor((messageString.length() + 6) / getConsoleWidth());

        // clear appropriately
        if(linesToClear > 0) {
            std::cout << "\33[" << linesToClear << "A"; // move up the right number of lines
            std::cout << "\r"; // move to the beginning of the line
            std::cout << "\33[0J"; // clear screen from the cursor down
        }
        else {
            std::cout << "\33[2K"; // delete the current line
            std::cout << "\r"; // move to the beginning of the line
        }


        std::cout << "[" << std::setw(numberOfDigits) << characterCount << "] : "; // display how many characters are left
        std::cout << messageString; // add the message
    }

    restoreConsoleSettings();

    std::cout << std::endl;

    return messageString;
}


unsigned int ConsoleHelper::getConsoleWidth()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}


void ConsoleHelper::enableUnbufferedConsole()
{
    if (!m_isBuffered) {
        tcgetattr(0, &m_oldConsolSettings); /* grab old terminal i/o settings and save them in a member variable*/

        struct termios newConsoleSettings;

        newConsoleSettings = m_oldConsolSettings; /* make new settings same as old settings */
        newConsoleSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
        newConsoleSettings.c_lflag &= ~ECHO; /* turn off echo mode */
        tcsetattr(0, TCSANOW, &newConsoleSettings); /* use these new terminal i/o settings now */
    }
}


void ConsoleHelper::restoreConsoleSettings()
{
    if (m_isBuffered) {
        /* Restore old terminal i/o settings */
        tcsetattr(0, TCSANOW, &m_oldConsolSettings);
    }
}


/**
 *  Get the number of digits a Integer has. '-' counts as a digit.
 */
int ConsoleHelper::getNumberOfDigits(int number)
{
    int digits = 0;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}
