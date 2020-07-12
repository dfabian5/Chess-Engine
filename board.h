#ifndef BOARD_H
#define BOARD_H

////////////////////////////////////////////////////////////////////////////////
//
// FILE:        board.h
// DESCRIPTION: contains board class declarations as well as any helper functions
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include "piece.h"
#include <string>
#include <fstream>

using std::string;
using std::ofstream; using std::ifstream;

////////////////////////////////////////////////////////////////////////////////
//
// NODE
// note: return type for min_max function
struct Node {
	Node(const double &value, const Position &cur, const Position &des) :
		value_(value), current_(cur), desired_(des) {}

	// for algorithms
	bool operator<(const Node &rhs) const { return value_ < rhs.value_; }

	double value_;
	Position current_;
	Position desired_;
};

////////////////////////////////////////////////////////////////////////////////
//
// BOARD
class Board {
public:
	// constructors
	Board();
	Board(const Board &board);

	// methods
	PieceList get_pieces() const { return pieces_; }
	void print() const;
	void save_game(const string &game = "game.txt") const;
	void load_game(const string &game = "game.txt");
	bool player_in_check(const Color &color) const;
	void make_move(const Position &currentPos, const Position &desiredPos);
	int end_game(const Color &color) const;
	void update_move_set();
	double favor() const; // positive = favor of white, negative = favor of black, 0 = neutral
	Node min_max_call(const Board &board, const Color &maximizingColor, const int &depth);
	double min_max(const Board &board, int depth, double alpha, double beta, Color maximizingColor);
	void play(const Color &cpu = Color::Empty, const int &depth = 3);

	// friends
	friend Piece;
	friend void remove_check_moves(const Board &board, const Position &pos,
								   const vector<Position> &possibleMoves,
								   vector<Position> &moves);

protected:
	PieceList pieces_; // list of pieces on board, position not listed then its empty
	Position kingPos_[2]; // king positions for quick access
	double totalGridPoints_; // for reuse in favor function
	int turn_; // turn number
};

#endif // BOARD_H