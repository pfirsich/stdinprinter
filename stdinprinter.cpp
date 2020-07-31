#include <array>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <string_view>
#include <vector>

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

const char* getControlString(char ch)
{
    static constexpr std::array<const char*, 32> lut { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ",
        "ACK", "BEL", "BS", "TAB", "LF", "VT", "FF", "CR", "SO", "SI", "DLE", "DC1", "DC2", "DC3",
        "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US" };

    assert(std::iscntrl(ch));
    if (ch > 0 && ch < 32)
        return lut[ch];
    if (ch == 127)
        return "DEL";
    assert(false && "Unhandled control character");
}

std::string hexString(const void* data, size_t size)
{
    static constexpr char hexDigits[] = "0123456789ABCDEF";

    std::string out;
    out.reserve(size * 3);
    for (size_t i = 0; i < size; ++i) {
        const auto c = reinterpret_cast<const uint8_t*>(data)[i];
        out.push_back(hexDigits[c >> 4]);
        out.push_back(hexDigits[c & 15]);
        if (i < size - 1)
            out.push_back(' ');
    }
    return out;
}

termios termiosBackup;

void restoreTermios()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &termiosBackup);
}

int main()
{
    tcgetattr(STDIN_FILENO, &termiosBackup);
    atexit(restoreTermios);

    termios ios = termiosBackup;

    ios.c_iflag &= ~(BRKINT); // No SIGINT when for break condition
    ios.c_iflag &= ~(ICRNL); // Don't map 13 to 10
    ios.c_iflag &= ~(INPCK); // No input parity check
    ios.c_iflag &= ~(ISTRIP); // Strip the eight
    ios.c_iflag &= ~(IXON); // Disable Software Flow Control (Ctrl-S and Ctrl-Q)

    ios.c_oflag &= ~(OPOST); // No output processing (move cursor for newlines, etc.)

    ios.c_cflag |= (CS8); // Set character size to 8 bits

    ios.c_lflag &= ~(ECHO); // Don't echo input characters
    ios.c_lflag &= ~(ICANON); // Disable canonical mode (get each char, not lines)
    ios.c_lflag &= ~(IEXTEN); // Disable Ctrl-V
    ios.c_lflag &= ~(ISIG); // Don't get SIGINT on Ctrl-C or SIGTSTP on Ctrl-Z

    ios.c_cc[VMIN] = 0; // minimum numbers to be read
    ios.c_cc[VTIME] = 1; // read() timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &ios);

    printf("Press Ctrl-Q to exit.\r\n");

    while (true) {
        std::vector<char> buf;
        char ch;
        while (read(STDIN_FILENO, &ch, 1) == 1)
            buf.push_back(ch);

        if (!buf.empty()) {
            for (const auto c : buf) {
                if (std::iscntrl(c)) {
                    printf("\x1b[2m%s\x1b[0m", getControlString(c)); // faint
                } else {
                    putchar(c);
                }
            }
            printf(" (%s)\r\n", hexString(&buf[0], buf.size()).c_str());
        }

        if (buf.size() == 1 && buf[0] == 0x11) {
            printf("Quit.\r\n");
            break;
        }
    }
    return 0;
}
