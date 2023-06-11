#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    };
}

// terminal 一般处于 canonical model 也叫 cooked mode
// 在这个模式中，用户输入的内容只有在按下 enter 键之后，才会发送给运行的程序
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    };
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    // IXON disable Ctrl-S and Ctrl-Q
    // ICRNL fix Ctrl-M 默认会将 13 转成 10 ，所以输入 Ctrl-M 看到的是 10 ，而不是 13
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // OPOST turn off all output processing
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    // ECHO turn off echoing 输入的内容不会在屏幕中显示出来
    // ICANON turn off canonical mode
    // ISIG turn off signal 输入 Ctrl-C 的时候不会转为信号发给程序
    // IEXTEN disable Ctrl-V 试了下，没什么特殊效果（按教程所说，有些系统有这个功能）
    // lflag = local flags
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    // VMIN 表示在 read 返回前需要读取几个字节的输入
    raw.c_cc[VMIN] = 0;
    // VTIME 表示 read 最大等待时间，单位为 0.1s
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    };
}

/*** init ***/

int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        };
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') {
            break;
        }
    }
    return 0;
}
