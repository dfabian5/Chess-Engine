#ifndef PIECE_H
#define PIECE_H

////////////////////////////////////////////////////////////////////////////////
//
// FILE:        piece.h
// DESCRIPTION: contains piece class declarations and helper functions
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include <iostream>
#include <vector>
#include <utility>
#include <list>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <new>

using std::cout; using std::endl; using std::cin;
using std::vector;
using std::pair; using std::make_pair;
using std::list;
using std::unordered_set;
using std::max; using std::min; using std::find;

// board size
const int SIZE = 8;

// chess piece letter representation
const char KING_REP = 'K', QUEEN_REP = 'Q', KNIGHT_REP = 'N', BISHOP_REP = 'B',
	ROOK_REP = 'R', PAWN_REP = 'P', EMPTY_REP = 'O';

// piece points
const double KING_POINTS = 0.0, QUEEN_POINTS = 8.0, KNIGHT_POINTS = 3.0, BISHOP_POINTS = 3.0,
	ROOK_POINTS = 5.0, PAWN_POINTS = 1.0, EMPTY_POINTS = 0.0;

// piece color enum, black and white MUST be listed first for use indexing arrays
enum class Color { Black, White, Empty };

// forward declarations
class Piece;
class PieceHash;
class Board;

////////////////////////////////////////////////////////////////////////////////
//
// POSITION
// note: first = row, second = col
typedef pair<int, int> Position;

////////////////////////////////////////////////////////////////////////////////
//
// PIECE LIST
typedef list<Piece> PieceList;

////////////////////////////////////////////////////////////////////////////////
//
// PIECE
// note: king side castle is given by position(-1, -1)
//		 while queen side castle is (-2, -2)
//	     pawn jump is given by position (-3, col)
//		 en passent is given by position (-4, col)
class Piece {
public:
	// constructors
	Piece(const Color &color = Color::Empty, const char &rep = EMPTY_REP, 
		  const double &points = 0, const Position &p = make_pair(0, 0), 
		  const bool &hasMoved = false) :
		color_(color), rep_(rep), points_(points), position_(p), hasMoved_(hasMoved) {}
	Piece(const Piece &p) = default;

	// methods
	Position get_position () const { return position_; }
	char get_rep          () const { return rep_; }
	Color get_color       () const { return color_; }
	double get_points     () const { return points_; }
	bool has_moved        () const { return hasMoved_; }
	void set_position  (const Position &p)    { position_ = p; }
	void set_color     (const Color &color)   { color_ = color; }
	void set_has_moved (const bool &hasMoved) { hasMoved_ = hasMoved; }
	void print_moves                    () const;
	vector<Position> get_possible_moves (const PieceList &pieces) const; // creates list of possible moves
	void get_moves                      (Board &board, const vector<Position> &possibleMoves = vector<Position>()); // create list of moves that wont put king in check
	vector<Position> move_list          () const { return moves_; }

	// operators
	Piece &operator=(const Piece &rhs);
	bool operator==(const Piece &rhs) const; // only based on position

	// piece creation for ease of use
	static Piece king   (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, KING_REP, KING_POINTS, p, hasMoved); }
	static Piece queen  (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, QUEEN_REP, QUEEN_POINTS, p, hasMoved); }
	static Piece knight (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, KNIGHT_REP, KNIGHT_POINTS, p, hasMoved); }
	static Piece bishop (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, BISHOP_REP, BISHOP_POINTS, p, hasMoved); }
	static Piece rook   (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, ROOK_REP, ROOK_POINTS, p, hasMoved); }
	static Piece pawn   (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, PAWN_REP, PAWN_POINTS, p, hasMoved); }
	static Piece empty  (const Color &c, const Position &p, const bool &hasMoved = false) { return Piece(c, EMPTY_REP, EMPTY_POINTS, p, hasMoved); }

protected:
	// helpers
	vector<Position> king_get_possible_moves   (const PieceList &pieces) const;
	vector<Position> queen_get_possible_moves  (const PieceList &pieces) const;
	vector<Position> knight_get_possible_moves (const PieceList &pieces) const;
	vector<Position> bishop_get_possible_moves (const PieceList &pieces) const;
	vector<Position> rook_get_possible_moves   (const PieceList &pieces) const;
	vector<Position> pawn_get_possible_moves   (const PieceList &pieces) const;

	// data
	Position position_;
	vector<Position> moves_; // list of possible moves
	Color color_;
	char rep_; // representation of piece
	double points_; // point value of piece
	bool hasMoved_;

};

////////////////////////////////////////////////////////////////////////////////
//
// HELPER functions
////////////////////////////////////////
// find function overload for PieceList to avoid creating many pointers
PieceList::iterator find(PieceList &pieces, const Position &pos);

////////////////////////////////////////
// find function overload for PieceList to avoid creating many pointers
PieceList::const_iterator cfind(const PieceList &pieces, const Position &pos);

////////////////////////////////////////
// get weighted point value of tile
double get_tile_value(const Position &pos);

////////////////////////////////////////
// checks if position is on board
bool in_bounds(const Position &pos);

////////////////////////////////////////
// overloaded in_bounds func
bool in_bounds(const int &i, const int &j);

////////////////////////////////////////
// removes moves that put the king in check
void remove_check_moves(Board &board, const Position &pos,
						const vector<Position> &possibleMoves, vector<Position> &moves);

////////////////////////////////////////
// get distance between two positions, used in player_in_check func
double distance(const Position &a, const Position &b);

#endif // PIECE_H