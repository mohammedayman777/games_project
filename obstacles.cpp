#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>
#include <functional>
#include "obstacles.h"

using namespace std;

obstacles_Board::obstacles_Board() : Board(6, 6) {
    // Initialize all cells
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            board[i][j] = blank_symbol;
        }
    }
}

bool obstacles_Board::update_board(Move<char>* move) {
    int x = move->get_x();
    int y = move->get_y();
    char mark = move->get_symbol();

    // Validate move and apply if valid
    if (!(x < 0 || x >= rows || y < 0 || y >= columns) &&
        (board[x][y] == blank_symbol || mark == 0)) {
        if (mark == 0) {
            n_moves--;
            board[x][y] = blank_symbol;
        }
        else {
            n_moves++;
            board[x][y] = toupper(mark);
        }

        if (n_moves != 0 && n_moves % 2 == 0) {
            int l = 0;
            while (l < 2) {
                int xrand = rand() % 6;
                int yrand = rand() % 6;
                if (board[xrand][yrand] == blank_symbol) {
                    board[xrand][yrand] = '#';
                    l++;
                }
            }
        }
        return true;
    }
    return false;
}

bool obstacles_Board::is_win(Player<char>* player) {
    const char sym = player->get_symbol();

    auto check = [&](int x, int y, int dx, int dy) {
        for (int k = 0; k < 4; k++) {
            int xx = x + dx * k;
            int yy = y + dy * k;
            if (xx < 0 || xx >= 6 || yy < 0 || yy >= 6)
                return false;
            if (board[xx][yy] != sym)
                return false;
        }
        return true;
        };

    for (int i = 0; i < 6; i++)
        for (int j = 0; j < 6; j++) {
            if (check(i, j, 1, 0)) return true;  // vertical
            if (check(i, j, 0, 1)) return true;  // horizontal
            if (check(i, j, 1, 1)) return true;  // diag ↘
            if (check(i, j, 1, -1)) return true; // diag ↙
        }
    return false;
}

bool obstacles_Board::is_draw(Player<char>* player) {
    return (n_moves == 18 && !is_win(player));
}

bool obstacles_Board::game_is_over(Player<char>* player) {
    return is_win(player) || is_draw(player);
}

obstacles_UI::obstacles_UI() : UI<char>("Welcome to Obstacles Tic Tac Toe", 3) {}

Player<char>* obstacles_UI::create_player(string& name, char symbol, PlayerType type) {
    cout << "Creating " << (type == PlayerType::HUMAN ? "human" : "computer")
        << " player: " << name << " (" << symbol << ")\n";
    return new Player<char>(name, symbol, type);
}

Move<char>* obstacles_UI::get_move(Player<char>* player) {
    int x, y;
    Board<char>* b = player->get_board_ptr();

    if (player->get_type() == PlayerType::HUMAN) {
        cout << player->get_name() << " ,Please enter your move x and y (0 to 5): ";
        cin >> x >> y;

        while (x < 0 || x >= 6 || y < 0 || y >= 6 || b->get_cell(x, y) != blank_symbol) {
            cout << "Invalid move, try again: ";
            cin >> x >> y;
        }
    }
    else {
        // Computer AI
        vector<vector<char>> board_matrix = b->get_board_matrix();
        char symbol = player->get_symbol();
        char opp_symbol = (symbol == 'X') ? 'O' : 'X';

        // Check line
        auto check_line_on_board = [](vector<vector<char>>& brd, int x, int y, int dx, int dy, int length, char sym) -> bool {
            for (int k = 0; k < length; k++) {
                int xx = x + dx * k;
                int yy = y + dy * k;
                if (xx < 0 || xx >= 6 || yy < 0 || yy >= 6 || brd[xx][yy] != sym)
                    return false;
            }
            return true;
            };

        // Check win
        auto check_win_on_board = [&](vector<vector<char>>& brd, char sym) -> bool {
            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
                    if (check_line_on_board(brd, i, j, 1, 0, 4, sym)) return true;
                    if (check_line_on_board(brd, i, j, 0, 1, 4, sym)) return true;
                    if (check_line_on_board(brd, i, j, 1, 1, 4, sym)) return true;
                    if (check_line_on_board(brd, i, j, 1, -1, 4, sym)) return true;
                }
            }
            return false;
            };

        // evaluation
        auto evaluate = [&](vector<vector<char>>& brd) -> int {
            if (check_win_on_board(brd, symbol)) return 10000;
            if (check_win_on_board(brd, opp_symbol)) return -10000;

            int score = 0;
            const int directions[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
                    for (int d = 0; d < 4; d++) {
                        int dx = directions[d][0];
                        int dy = directions[d][1];

                        // Line of 3
                        if (check_line_on_board(brd, i, j, dx, dy, 3, symbol)) score += 50;
                        if (check_line_on_board(brd, i, j, dx, dy, 3, opp_symbol)) score -= 60;

                        // Line of 2
                        if (check_line_on_board(brd, i, j, dx, dy, 2, symbol)) score += 10;
                        if (check_line_on_board(brd, i, j, dx, dy, 2, opp_symbol)) score -= 15;
                    }
                }
            }

            // Center 
            if (brd[2][2] == symbol || brd[2][3] == symbol ||
                brd[3][2] == symbol || brd[3][3] == symbol) score += 15;

            return score;
            };

        // Minimax
        function<int(vector<vector<char>>&, int, int, int, bool)> minimax =
            [&](vector<vector<char>>& brd, int depth, int alpha, int beta, bool is_max) -> int {

            if (check_win_on_board(brd, symbol)) return 10000 - depth;
            if (check_win_on_board(brd, opp_symbol)) return -10000 + depth;
            if (depth >= 3) return evaluate(brd);

            int best_score = is_max ? -99999 : 99999;

            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
                    if (brd[i][j] == blank_symbol) {
                        brd[i][j] = is_max ? symbol : opp_symbol;
                        int score = minimax(brd, depth + 1, alpha, beta, !is_max);
                        brd[i][j] = blank_symbol;

                        if (is_max) {
                            best_score = max(best_score, score);
                            alpha = max(alpha, score);
                        }
                        else {
                            best_score = min(best_score, score);
                            beta = min(beta, score);
                        }

                        if (beta <= alpha) break;
                    }
                }
                if (beta <= alpha) break;
            }

            return best_score;
            };

        int best_score = -99999;
        int best_x = -1, best_y = -1;
        bool found = false;

        // Check immediate win
        for (int i = 0; i < 6 && !found; i++) {
            for (int j = 0; j < 6 && !found; j++) {
                if (board_matrix[i][j] == blank_symbol) {
                    board_matrix[i][j] = symbol;
                    if (check_win_on_board(board_matrix, symbol)) {
                        best_x = i;
                        best_y = j;
                        best_score = 10000;
                        found = true;
                    }
                    board_matrix[i][j] = blank_symbol;
                }
            }
        }

        // Check immediate block (if no win found)
        if (!found) {
            for (int i = 0; i < 6 && !found; i++) {
                for (int j = 0; j < 6 && !found; j++) {
                    if (board_matrix[i][j] == blank_symbol) {
                        board_matrix[i][j] = opp_symbol;
                        if (check_win_on_board(board_matrix, opp_symbol)) {
                            best_x = i;
                            best_y = j;
                            best_score = 5000;
                            found = true;
                        }
                        board_matrix[i][j] = blank_symbol;
                    }
                }
            }
        }

        // Use minimax (if no immediate win/block)
        if (!found) {
            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < 6; j++) {
                    if (board_matrix[i][j] == blank_symbol) {
                        board_matrix[i][j] = symbol;
                        int score = minimax(board_matrix, 0, -99999, 99999, false);
                        board_matrix[i][j] = blank_symbol;

                        if (score > best_score) {
                            best_score = score;
                            best_x = i;
                            best_y = j;
                        }
                    }
                }
            }
        }

        // random move
        if (best_x == -1 || best_y == -1) {
            do {
                best_x = rand() % 6;
                best_y = rand() % 6;
            } while (b->get_cell(best_x, best_y) != blank_symbol);
        }

        x = best_x;
        y = best_y;
        cout << "Computer plays (" << x << ", " << y << ") " << symbol
            << " (score: " << best_score << ")" << endl;
    }

    return new Move<char>(x, y, player->get_symbol());
}
