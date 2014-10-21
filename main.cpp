#include <iostream>
#include <iomanip>
#include <cmath>
#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

unsigned int getConsoleWidth();


int main() {

    std::cout << "Please input your message. (Press enter to send)" << std::endl;

    struct termios oldConsolSettings, newConsoleSettings;

    tcgetattr(0, &oldConsolSettings); /* grab old terminal i/o settings */
    newConsoleSettings = oldConsolSettings; /* make new settings same as old settings */
    newConsoleSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
    newConsoleSettings.c_lflag &= ~ECHO; /* turn off echo mode */
    tcsetattr(0, TCSANOW, &newConsoleSettings); /* use these new terminal i/o settings now */

    int characterCount = 150;
    std::cout << "[" << std::setw(3) << characterCount << "] : ";
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
            if (getchar() == 91) {
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
            if (messageString.length() < 150) {
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


        std::cout << "[" << std::setw(3) << characterCount << "] : "; // display how many characters are left
        std::cout << messageString; // add the message
    }

    std::cout << std::endl << messageString.length() << std::endl;

    /* Restore old terminal i/o settings */
    tcsetattr(0, TCSANOW, &oldConsolSettings);

    return 0;
}

unsigned int getConsoleWidth()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}
