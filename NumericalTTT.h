// NumericalTTT.h
#pragma once
#ifndef NUMERICALTTT_CLASSES_H
#define NUMERICALTTT_CLASSES_H

#include "BoardGame_Classes.h"
using namespace std;

class NumericalTTT_Board : public Board<int> {
private:
    bool used[10] = { false };
    int blank_symbol = 0;
    int n_moves = 0;
public:
    NumericalTTT_Board(int rows = 3, int cols = 3);
    bool update_board(Move<int>* move) override;
    bool is_win(Player<int>* player) override;
    bool is_lose(Player<int>* player) override { return false; } 
    bool is_draw(Player<int>* player) override;
    bool game_is_over(Player<int>* player) override;
};

class NumericalTTT_UI : public UI<int> {
public:
    NumericalTTT_UI();
    Player<int>** setup_players() override;
    Player<int>* create_player(string& name, int symbol, PlayerType type) override;
    Move<int>* get_move(Player<int>* player) override;
};

#endif
