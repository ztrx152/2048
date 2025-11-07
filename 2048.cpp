#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

class Game2048 {
private:
    vector<vector<int>> board;
    int score;
    bool moved;
    bool gameOver;
    bool win;

    // Функции для работы с терминалом
    void enableRawMode() {
        termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    void disableRawMode() {
        termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag |= (ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
            perror("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
            perror("tcsetattr ~ICANON");
        return buf;
    }

public:
    Game2048() : board(4, vector<int>(4, 0)), score(0), moved(false), gameOver(false), win(false) {
        srand(time(0));
        addRandomTile();
        addRandomTile();
    }

    void printBoard() {
        system("clear");
        cout << "Score: " << score << "\n\n";
        
        for (int i = 0; i < 4; i++) {
            cout << "+------+------+------+------+" << endl;
            cout << "|";
            for (int j = 0; j < 4; j++) {
                if (board[i][j] == 0) {
                    cout << "      |";
                } else {
                    printf(" %4d |", board[i][j]);
                }
            }
            cout << endl;
        }
        cout << "+------+------+------+------+" << endl;
        
        if (win) {
            cout << "\n🎉 You won! Press 'q' to quit or continue playing.\n";
        }
        if (gameOver) {
            cout << "\n💀 Game Over! Press 'q' to quit or 'r' to restart.\n";
        }
        cout << "\nControls: WASD or arrow keys to move, q to quit, r to restart\n";
    }

    void addRandomTile() {
        vector<pair<int, int>> emptyCells;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (board[i][j] == 0) {
                    emptyCells.push_back({i, j});
                }
            }
        }

        if (!emptyCells.empty()) {
            int index = rand() % emptyCells.size();
            int value = (rand() % 10 == 0) ? 4 : 2; // 10% chance for 4
            board[emptyCells[index].first][emptyCells[index].second] = value;
        }
    }

    bool canMove() {
        // Проверка пустых клеток
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (board[i][j] == 0) return true;
            }
        }

        // Проверка возможных слияний
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == board[i][j + 1]) return true;
            }
        }

        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < 3; i++) {
                if (board[i][j] == board[i + 1][j]) return true;
            }
        }

        return false;
    }

    void moveLeft() {
        moved = false;
        for (int i = 0; i < 4; i++) {
            // Сжатие
            vector<int> newRow;
            for (int j = 0; j < 4; j++) {
                if (board[i][j] != 0) {
                    newRow.push_back(board[i][j]);
                }
            }

            // Слияние
            for (int j = 0; j < (int)newRow.size() - 1; j++) {
                if (newRow[j] == newRow[j + 1]) {
                    newRow[j] *= 2;
                    score += newRow[j];
                    if (newRow[j] == 2048) win = true;
                    newRow.erase(newRow.begin() + j + 1);
                    moved = true;
                }
            }

            // Заполнение нулями
            while (newRow.size() < 4) {
                newRow.push_back(0);
            }

            // Проверка изменения
            for (int j = 0; j < 4; j++) {
                if (board[i][j] != newRow[j]) moved = true;
                board[i][j] = newRow[j];
            }
        }
    }

    void moveRight() {
        moved = false;
        for (int i = 0; i < 4; i++) {
            vector<int> newRow;
            for (int j = 3; j >= 0; j--) {
                if (board[i][j] != 0) {
                    newRow.push_back(board[i][j]);
                }
            }

            for (int j = 0; j < (int)newRow.size() - 1; j++) {
                if (newRow[j] == newRow[j + 1]) {
                    newRow[j] *= 2;
                    score += newRow[j];
                    if (newRow[j] == 2048) win = true;
                    newRow.erase(newRow.begin() + j + 1);
                    moved = true;
                }
            }

            while (newRow.size() < 4) {
                newRow.push_back(0);
            }

            for (int j = 0; j < 4; j++) {
                if (board[i][3 - j] != newRow[j]) moved = true;
                board[i][3 - j] = newRow[j];
            }
        }
    }

    void moveUp() {
        moved = false;
        for (int j = 0; j < 4; j++) {
            vector<int> newCol;
            for (int i = 0; i < 4; i++) {
                if (board[i][j] != 0) {
                    newCol.push_back(board[i][j]);
                }
            }

            for (int i = 0; i < (int)newCol.size() - 1; i++) {
                if (newCol[i] == newCol[i + 1]) {
                    newCol[i] *= 2;
                    score += newCol[i];
                    if (newCol[i] == 2048) win = true;
                    newCol.erase(newCol.begin() + i + 1);
                    moved = true;
                }
            }

            while (newCol.size() < 4) {
                newCol.push_back(0);
            }

            for (int i = 0; i < 4; i++) {
                if (board[i][j] != newCol[i]) moved = true;
                board[i][j] = newCol[i];
            }
        }
    }

    void moveDown() {
        moved = false;
        for (int j = 0; j < 4; j++) {
            vector<int> newCol;
            for (int i = 3; i >= 0; i--) {
                if (board[i][j] != 0) {
                    newCol.push_back(board[i][j]);
                }
            }

            for (int i = 0; i < (int)newCol.size() - 1; i++) {
                if (newCol[i] == newCol[i + 1]) {
                    newCol[i] *= 2;
                    score += newCol[i];
                    if (newCol[i] == 2048) win = true;
                    newCol.erase(newCol.begin() + i + 1);
                    moved = true;
                }
            }

            while (newCol.size() < 4) {
                newCol.push_back(0);
            }

            for (int i = 0; i < 4; i++) {
                if (board[3 - i][j] != newCol[i]) moved = true;
                board[3 - i][j] = newCol[i];
            }
        }
    }

    void restart() {
        board = vector<vector<int>>(4, vector<int>(4, 0));
        score = 0;
        moved = false;
        gameOver = false;
        win = false;
        addRandomTile();
        addRandomTile();
    }

    void run() {
        enableRawMode();
        
        while (true) {
            printBoard();
            
            if (!canMove()) {
                gameOver = true;
                printBoard();
            }

            char c = getch();
            
            if (c == 'q' || c == 'Q') {
                break;
            } else if (c == 'r' || c == 'R') {
                restart();
                continue;
            }

            if (gameOver && c != 'r' && c != 'R') {
                continue;
            }

            moved = false;
            
            if (c == 'w' || c == 'W' || c == 'A' - 64) { // W или стрелка вверх
                moveUp();
            } else if (c == 's' || c == 'S' || c == 'B' - 64) { // S или стрелка вниз
                moveDown();
            } else if (c == 'a' || c == 'A' || c == 'D' - 64) { // A или стрелка влево
                moveLeft();
            } else if (c == 'd' || c == 'D' || c == 'C' - 64) { // D или стрелка вправо
                moveRight();
            }

            if (moved) {
                addRandomTile();
            }
        }
        
        disableRawMode();
        cout << "\nThanks for playing! Final score: " << score << endl;
    }
};

int main() {
    Game2048 game;
    game.run();
    return 0;
}
