#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <future>
#include <functional>
#include <vector>

#include <curses.h>
#define ctrl(x) ((x) & 0x1f)

#define TIME 15

std::atomic<bool> exit_thread_flag{false};
std::atomic_int total_time{TIME};
bool tab_switch = false;

class FileReader
{
private:
    std::string text;
    std::vector<std::string> randomText;

public:
    FileReader(std::string _file)
    {
        std::ifstream ifs(_file);
        std::string temp;
        while (std::getline(ifs, temp))
        {
            randomText.push_back(temp);
        }
        ifs.close();
    }
    std::string getText()
    {
        return text;
    }
    void loadRandomText()
    {
        srand((int)time(0));
        int randNum = rand() % randomText.size();
        text = randomText[randNum];
    }
};

void main_win(int index, FileReader &f, int &posRow, int &posCol, std::string &output, std::string &state, int *correct_char, int *wrong_char)
{
    move(0, 0);
    while (index < f.getText().size() && !exit_thread_flag)
    {
        posCol = index % COLS;
        posRow = index / COLS;
        bool flag = true;

        attron(COLOR_PAIR(4));
        mvaddch(posRow, posCol, f.getText()[index]);
        attroff(COLOR_PAIR(4));

        int ch = getch();
        switch (ch)
        {
        case KEY_RESIZE:
        {
            clear();
            flag = false;
            posCol = index % COLS;
            posRow = index / COLS;
            int writtenIndex = 0;
            for (; output[writtenIndex] != '\0'; ++writtenIndex)
            {
                int color_pair = 1;
                if (state[writtenIndex] == 'c')
                    color_pair = 2;
                if (state[writtenIndex] == 'i')
                    color_pair = 3;
                attron(COLOR_PAIR(color_pair));
                if (output[writtenIndex] != ' ')
                    mvaddch(writtenIndex / COLS, writtenIndex % COLS, output[writtenIndex]);
                else
                    mvaddch(writtenIndex / COLS, writtenIndex % COLS, f.getText()[writtenIndex]);
                attroff(COLOR_PAIR(color_pair));
            }
            for (; writtenIndex < f.getText().size(); ++writtenIndex)
            {
                attron(COLOR_PAIR(1));
                mvaddch(writtenIndex / COLS, writtenIndex % COLS, f.getText()[writtenIndex]);
                attroff(COLOR_PAIR(1));
            }
            refresh();
            break;
        }
        case KEY_BACKSPACE:
        {
            attron(COLOR_PAIR(1));
            mvaddch(posRow, posCol, f.getText()[index]);
            attroff(COLOR_PAIR(1));
            if (index > 0)
                --index;
            posCol = index % COLS;
            posRow = index / COLS;
            attron(COLOR_PAIR(1));
            mvaddch(posRow, posCol, f.getText()[index]);
            attroff(COLOR_PAIR(1));
        }
        case KEY_BEG: case KEY_BREAK: case KEY_ENTER: case KEY_CLOSE: case KEY_COMMAND: case KEY_SCOMMAND: case KEY_HOME: case KEY_SHOME: case KEY_END: case KEY_SEND: case KEY_NEXT: case KEY_FIND: case KEY_HELP: case KEY_SHELP: case KEY_MARK: case KEY_IC: case KEY_IL: case KEY_DC: case KEY_DL: case KEY_PPAGE: case KEY_NPAGE: case KEY_UP: case KEY_LEFT: case KEY_SLEFT: case KEY_DOWN: case KEY_PREVIOUS: case KEY_SPREVIOUS: case KEY_RIGHT: case KEY_SRIGHT: case KEY_EXIT: case KEY_SEXIT: case ctrl('a'): case ctrl('b'): case ctrl('d'): case ctrl('e'): case ctrl('f'): case ctrl('g'): case ctrl('h'): case ctrl('j'): case ctrl('k'): case ctrl('l'): case ctrl('m'): case ctrl('n'): case ctrl('o'): case ctrl('p'): case ctrl('q'): case ctrl('r'): case ctrl('s'): case ctrl('t'): case ctrl('u'): case ctrl('v'): case ctrl('w'): case ctrl('x'): case ctrl('y'): case ctrl('z'): case ctrl('`'): case ctrl('='): case ctrl('['): case ctrl('\\'): case KEY_F(1): case KEY_F(2): case KEY_F(3): case KEY_F(4): case KEY_F(5): case KEY_F(6): case KEY_F(7): case KEY_F(8): case KEY_F(9): case KEY_F(10): case KEY_F(11): case KEY_F(12):
            flag = false;
            break;
        case KEY_CATAB: case KEY_BTAB: case KEY_CTAB: case KEY_STAB: case ctrl('i'):
        {
            tab_switch = true;
            exit_thread_flag = true;
            return;
        }
        default:
        {
            output[index] = ch;
            if (ch == f.getText()[index])
            {
                state[index] = 'c';
                attron(COLOR_PAIR(2));
                mvaddch(posRow, posCol, ch);
                attroff(COLOR_PAIR(2));
                ++(*correct_char);
            }
            else
            {
                state[index] = 'i';
                attron(COLOR_PAIR(3));
                if (ch != ' ')
                    mvaddch(posRow, posCol, ch);
                else
                    mvaddch(posRow, posCol, f.getText()[index]);
                attroff(COLOR_PAIR(3));
                ++(*wrong_char);
            }
            refresh();
            break;
        }
        }

        if (flag)
            ++index;
    }
    exit_thread_flag = true;
}

int result(FileReader &f, std::string &state, int *posRow, int *correct, int *wrong)
{
    float accuracy = *correct + *wrong ? (float)((*correct * 100) / (*correct + *wrong)) : 0.0f;
    int uncorr = std::count(state.begin(), state.end(), 'i');
    float netWPM = accuracy ? (((float)f.getText().size() / 5) - uncorr) / ((float)(TIME - total_time) / 60) : 0.0f;
    if (netWPM < 0.0f)
        netWPM = 0.0f;
    char netWPMc[40];
    sprintf(netWPMc, "%.2f", netWPM);
    std::string netWPMs = netWPMc;
    std::string result = "Correct characters: " + std::to_string(*correct) + " | Wrong characters: " + std::to_string(*wrong) + " | Accuracy: " + std::to_string((int)accuracy) + "% | WPM: " + netWPMs;
    int linesreq = (result.size() / COLS) + 1;
    WINDOW *w = newwin(linesreq + 1, COLS, LINES - linesreq - 1, 0);
    wclear(w);
    mvwprintw(w, 0, 0, "%s", result.c_str());
    mvwprintw(w, linesreq, 0, "Retry? (Y/N): ");
    wrefresh(w);
    int ch = 0;
    do
    {
        ch = wgetch(w);
        if (ch == 'y' || ch == 'n')
            break;
    } while (true);
    delwin(w);
    return ch;
}

void display_timer()
{
    WINDOW *w = newwin(1, COLS, LINES - 1, 0);

    wclear(w);
    while (total_time > 0 && !exit_thread_flag)
    {
        int tt = total_time;
        wrefresh(w);
        wclear(w);
        wprintw(w, "%d seconds", tt);
        wrefresh(w);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        --total_time;
    }

    if (total_time == 0)
    {
        wclear(w);
        wprintw(w, "Press any key to display results");
        wrefresh(w);
    }
    delwin(w);
    exit_thread_flag = true;
}

int main()
{
    FileReader f("body.txt");
    initscr();
    start_color();
    use_default_colors();
    curs_set(0);
    init_pair(1, COLOR_YELLOW, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_RED, -1);
    init_pair(4, -1, COLOR_YELLOW);
    cbreak();
    noecho();
    nonl();
    keypad(stdscr, true);
    int index = 0, correct_char = 0, wrong_char = 0, posRow = 0, posCol = 0;
    int retry = 0;

    do
    {
        exit_thread_flag = false;
        f.loadRandomText();
        std::string state(f.getText().size(), 'n');
        std::string output(f.getText().size(), '\0');
        clear();
        attron(COLOR_PAIR(1));
        printw("%s", f.getText().c_str());
        attroff(COLOR_PAIR(1));
        refresh();

        index = 0, correct_char = 0, wrong_char = 0, posRow = 0, posCol = 0, total_time = TIME;
        retry = 0;

        std::thread t3(main_win, index, std::ref(f), std::ref(posRow), std::ref(posCol), std::ref(output), std::ref(state), &correct_char, &wrong_char);
        std::thread t2(display_timer);

        t3.join();
        t2.join();

        if (!tab_switch)
            retry = result(std::ref(f), std::ref(state), &posRow, &correct_char, &wrong_char);
        else
            tab_switch = false;

        clear();
        refresh();
    } while (retry != 'n');

    curs_set(1);
    endwin();

    return 0;
}
