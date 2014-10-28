#include <sys/ioctl.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <sys/time.h>
#include <cstring>

#include "ConsoleHelper.hpp"


namespace
{
    // dummy signal handler
    void alarmSignalHandler(int signalNumber)
    {
        // ignore the signal
        signal(signalNumber, SIG_IGN);
    }
}



ConsoleHelper::ConsoleHelper() : m_isBuffered(false)
{

}


/**
 * This method gets the number of characters specified in the parameter maximumNumberOfCharacters
 * from the user and puts them in the parameter result. The number of remaining characters is
 * displayed nicely infront of the string while typing.
 *
 * @param result A string into which the user input is saved
 * @param maximumNumberOfCharacters The maximum length of the string
 *
 * @return The method returns false, if the input got canceled by the user (escape was pressed) otherwise true
 */

bool ConsoleHelper::getLimitedInput(std::string& result, unsigned int maximumNumberOfCharacters)
{
    enableUnbufferedConsole();

    int characterCount = maximumNumberOfCharacters;

    // save the current cursors position, so we know where we started
    std::cout << "\e7";

    // display the remaining amount of characters
    const int numberOfDigits = getNumberOfDigits(maximumNumberOfCharacters);
    std::cout << "[" << std::setw(numberOfDigits) << characterCount << "] : ";

    while (true) {
        char c = getchar();

        if (c == 10) {
            // if the enter key is pressed and the string is not empty stop the input
            if (result.length() > 0)
                break;
        }
        else if (c == 27) {
            /*
             * Alright, the following needs explaining. The problem here is that the ANSI escape sequences
             * (for example when you press the arrow keys) start with an escape character followed by (usually)
             * two more characters. Now to keep them from showing in the console and so we can react to them we have to call
             * getchar() two more times.
             * But what happens if the user presses the escape key? Only one escape character followed by nothing else is sent.
             * This means we would be stuck here waiting until the user presses the next key, which then would never appear on screen.
             *
             * Sooo to work around all that we set a timer, which raises an ALARM signal after 10 milliseconds. That
             * signal interrupts the blocking getchar(). If getchar() is not blocking (i.e. there is another character ready,
             * meaning we are in a sequence) it will probably return in under 10 milliseconds. The timer is then deleted by setting it to zero.
             *
             * Yes, this is a very dirty hack!
             */
            char character = '\0';

            // register our dummy signal handler for the ALARM signal
            struct sigaction signalAction;
            std::memset(&signalAction, 0, sizeof(signalAction));

            signalAction.sa_handler = &alarmSignalHandler ;
            sigaction(SIGALRM, &signalAction, NULL);

            // initialize a timer for 10 ms
            struct itimerval timer ;
            std::memset(&timer, 0, sizeof(timer));
            timer.it_value.tv_usec = 10000;

            // set the timer to the "real timer", which raises SIGALRM on expiration
            setitimer(ITIMER_REAL, &timer, NULL);

            // get the next character (if there is one)
            character = getchar();

            // stop the timer by setting it to 0
            timer.it_value.tv_usec = 0;
            setitimer(ITIMER_REAL, &timer, NULL);

            // so is this an ANSI escape code?
            if (character == 91) {
                char codeChar = getchar();

                // left arrow key
                if (codeChar == 68) {
                    std::cout << "\e[D"; // move cursor left
                }
                // right arrow key
                else if (codeChar == 67) {
                    std::cout << "\e[C"; // move cursor right
                }

                continue;
            }

            // if not it's the escape key, clear the message and end the input
            result.clear();
            std::cout << "\e8"; // restore the cursors position (to where it was when we first started typing)
            std::cout << "\e[0J"; // clear screen from the cursor down
            std::cout << std::endl;

            restoreConsoleSettings();
            return false;
        }
        else if (c == 127) {
            // a backspace character removes the last character from the string
            if (result.length() > 0) {
                result.erase(result.end() - 1);
                characterCount++;
            }
        }
        else {
            // all other characters are added to the back of the message string
            if (result.length() < maximumNumberOfCharacters) {
                result.push_back(c);
                characterCount--;
            }
        }

        std::cout << "\e8"; // restore the cursors position (to where it was when we first started typing)
        std::cout << "\e[0J"; // clear screen from the cursor down


        std::cout << "[" << std::setw(numberOfDigits) << characterCount << "] : "; // display how many characters are left
        std::cout << result; // add the message
    }

    restoreConsoleSettings();

    std::cout << std::endl;

    return true;
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

        m_isBuffered = true;
    }
}


void ConsoleHelper::restoreConsoleSettings()
{
    if (m_isBuffered) {
        /* Restore old terminal i/o settings */
        tcsetattr(0, TCSANOW, &m_oldConsolSettings);

        m_isBuffered = false;
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
