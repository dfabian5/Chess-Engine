////////////////////////////////////////////////////////////////////////////////
//
// FILE:        piece.h
// DESCRIPTION: contains Piece function implementations
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include "piece.h"
#include "board.h"

////////////////////////////////////////////////////////////////////////////////
//
// PIECE functions
////////////////////////////////////////
// prints out all moves
void Piece::print_moves() const
{
	for (int i = 0; i < moves_.size(); ++i)
	{
		cout << i << ": ";
		if (moves_[i].first == -1) // king side castle
			cout << "O-O ";
		else if (moves_[i].first == -2) // queen side castle
			cout << "O-O-O ";
		else if (moves_[i].first == -3) // pawn jump
			cout << "jump ";
		else if (moves_[i].first == -4) // en passant
			cout << "en passant " << char(97 + moves_[i].second) << " pawn ";
		else
			cout << char(97 + moves_[i].second) << moves_[i].first + 1 << ' ';
		cout << endl;
	}
}

////////////////////////////////////////
// finds all possible moves of piece
vector<Position> Piece::get_possible_moves(const PieceList &pieces) const
{
	switch (rep_)
	{
	case KING_REP:
		return king_get_possible_moves(pieces);
		break;
	case QUEEN_REP:
		return queen_get_possible_moves(pieces);
		break;
	case KNIGHT_REP:
		return knight_get_possible_moves(pieces);
		break;
	case BISHOP_REP:
		return bishop_get_possible_moves(pieces);
		break;
	case ROOK_REP:
		return rook_get_possible_moves(pieces);
		break;
	case PAWN_REP:
		return pawn_get_possible_moves(pieces);
		break;
	default:
		return vector<Position>();
	}
}

////////////////////////////////////////
// finds all moves that wont put king in check
void Piece::get_moves(Board &board, const vector<Position> &possibleMoves)
{
	if (possibleMoves.empty())
		remove_check_moves(board, position_, get_possible_moves(board.get_pieces()), moves_);
	else
		remove_check_moves(board, position_, possibleMoves, moves_);
}

////////////////////////////////////////
// overloaded assignment
Piece &Piece::operator=(const Piece &rhs)
{
	position_ = rhs.position_;
	moves_ = rhs.moves_; 
	color_ = rhs.color_;
	rep_ = rhs.rep_;
	points_ = rhs.points_;
	hasMoved_ = rhs.hasMoved_;

	return *this;
}

////////////////////////////////////////
// for piece equality we only care about position
bool Piece::operator==(const Piece &rhs) const
{
	return position_ == rhs.position_;
}

////////////////////////////////////////////////////////////////////////////////
//
// PIECE helper functions
////////////////////////////////////////
// finds all moves piece has avalible in current board state
vector<Position> Piece::king_get_possible_moves(const PieceList &pieces) const
{
	// check all basic king moves
	vector<Position> possibleMoves;
	for (int i = position_.first - 1; i <= position_.first + 1; ++i)
		for (int j = position_.second - 1; j <= position_.second + 1; ++j)
		{
			// check if position is occupied
			auto it = cfind(pieces, make_pair(i, j));

			if (in_bounds(i, j) && // open move
				it == pieces.end())
				possibleMoves.push_back(make_pair(i, j));
			else if (in_bounds(i, j) && // check attack
					 it->get_color() != color_ &&
					 it->get_rep() != EMPTY_REP)
				possibleMoves.push_back(make_pair(i, j));
		}

	// check for king side castle
	if (!hasMoved_ &&
		cfind(pieces, make_pair(position_.first, position_.second + 1)) == pieces.end() &&
		cfind(pieces, make_pair(position_.first, position_.second + 2)) == pieces.end() &&
		cfind(pieces, make_pair(position_.first, position_.second + 3)) != pieces.end() &&
		!cfind(pieces, make_pair(position_.first, position_.second + 3))->has_moved()) // rook hasnt moved
		possibleMoves.push_back(make_pair(-1, -1));

	// check for queen side castle
	if (!hasMoved_ &&
		cfind(pieces, make_pair(position_.first, position_.second - 1)) == pieces.end() &&
		cfind(pieces, make_pair(position_.first, position_.second - 2)) == pieces.end() &&
		cfind(pieces, make_pair(position_.first, position_.second - 3)) == pieces.end() &&
		cfind(pieces, make_pair(position_.first, position_.second - 4)) != pieces.end() &&
		!cfind(pieces, make_pair(position_.first, position_.second - 4))->has_moved()) // rook hasnt moved
		possibleMoves.push_back(make_pair(-2, -2));

	return possibleMoves;
}

////////////////////////////////////////
// finds all moves piece has avalible in current board state
vector<Position> Piece::queen_get_possible_moves(const PieceList &pieces) const
{
	// lambda for adding moves, returns false to break loop
	vector<Position> possibleMoves;
	auto check_move = [&](const int &i, const int &j) {

		auto it = cfind(pieces, make_pair(i, j));

		if (in_bounds(i, j) && // open move
			it == pieces.end())
		{
			possibleMoves.push_back(make_pair(i, j));
			return true;
		}
		else if (in_bounds(i, j) && // check attack
				 it->get_color() != color_ &&
				 it->get_rep() != EMPTY_REP)
		{
			possibleMoves.push_back(make_pair(i, j));
			return false;
		}
		else if (in_bounds(i, j) && // pawn en passant move
				 it->get_rep() == EMPTY_REP)
		{
			possibleMoves.push_back(make_pair(i, j));
			return true;
		}

		return false; // out of bounds
	};

	// check upward moves
	for (int i = position_.first + 1; i < SIZE; ++i)
		if (!check_move(i, position_.second)) break;

	// check downward
	for (int i = position_.first - 1; i >= 0; --i)
		if (!check_move(i, position_.second)) break;

	// check right moves
	for (int i = position_.second + 1; i < SIZE; ++i)
		if (!check_move(position_.first, i)) break;

	// check left moves
	for (int i = position_.second - 1; i >= 0; --i)
		if (!check_move(position_.first, i)) break;

	// check up/right diagnal
	for (int i = position_.first + 1, j = position_.second + 1; i < SIZE && j < SIZE; ++i, ++j)
		if (!check_move(i, j)) break;

	// check up/left diagnal
	for (int i = position_.first + 1, j = position_.second - 1; i < SIZE && j >= 0; ++i, --j)
		if (!check_move(i, j)) break;

	// check down/right diagnal
	for (int i = position_.first - 1, j = position_.second + 1; i >= 0 && j < SIZE; --i, ++j)
		if (!check_move(i, j)) break;

	// check down/left diagnal
	for (int i = position_.first - 1, j = position_.second - 1; i >= 0 && j >= 0; --i, --j)
		if (!check_move(i, j)) break;

	return possibleMoves;
}

////////////////////////////////////////
// finds all moves piece has avalible in current board state
vector<Position> Piece::knight_get_possible_moves(const PieceList &pieces) const
{
	// lambda for adding moves
	vector<Position> possibleMoves;
	auto check_move = [&](const int &i, const int &j) {

		auto it = cfind(pieces, make_pair(i, j));

		if (in_bounds(i, j) && // open move
			it == pieces.end())
			possibleMoves.push_back(make_pair(i, j));
		else if (in_bounds(i, j) && // check attack
				 it->get_color() != color_)
			possibleMoves.push_back(make_pair(i, j));
		else if (in_bounds(i, j) && // en passant
				 it->get_color() == color_ &&
				 it->get_rep() == EMPTY_REP) 
			possibleMoves.push_back(make_pair(i, j));
	};

	// check up/right moves
	check_move(position_.first + 2, position_.second + 1);
	check_move(position_.first + 1, position_.second + 2);

	// check up/left moves
	check_move(position_.first + 2, position_.second - 1);
	check_move(position_.first + 1, position_.second - 2);

	// check down/right moves
	check_move(position_.first - 2, position_.second + 1);
	check_move(position_.first - 1, position_.second + 2);

	// check down/left moves
	check_move(position_.first - 2, position_.second - 1);
	check_move(position_.first - 1, position_.second - 2);

	return possibleMoves;
}

////////////////////////////////////////
// finds all moves piece has avalible in current board state
vector<Position> Piece::bishop_get_possible_moves(const PieceList &pieces) const
{
	// lambda for adding moves, returns false to break loop
	vector<Position> possibleMoves;
	auto check_move = [&](const int &i, const int &j) {

		auto it = cfind(pieces, make_pair(i, j));

		if (in_bounds(i, j) && // open move
			it == pieces.end())
		{
			possibleMoves.push_back(make_pair(i, j));
			return true;
		}
		else if (in_bounds(i, j) && // check attack
				 it->get_color() != color_ &&
				 it->get_rep() != EMPTY_REP)
		{
			possibleMoves.push_back(make_pair(i, j));
			return false;
		}
		else if (in_bounds(i, j) && // pawn en passant move
				 it->get_rep() == EMPTY_REP)
		{
			possibleMoves.push_back(make_pair(i, j));
			return true;
		}

		return false; // out of bounds
	};

	// check up/right diagnal
	for (int i = position_.first + 1, j = position_.second + 1; i < SIZE && j < SIZE; ++i, ++j)
		if (!check_move(i, j)) break;

	// check up/left diagnal
	for (int i = position_.first + 1, j = position_.second - 1; i < SIZE && j >= 0; ++i, --j)
		if (!check_move(i, j)) break;

	// check down/right diagnal
	for (int i = position_.first - 1, j = position_.second + 1; i >= 0 && j < SIZE; --i, ++j)
		if (!check_move(i, j)) break;

	// check down/left diagnal
	for (int i = position_.first - 1, j = position_.second - 1; i >= 0 && j >= 0; --i, --j)
		if (!check_move(i, j)) break;

	return possibleMoves;
}

////////////////////////////////////////
// finds all moves piece has avalible in current board state
vector<Position> Piece::rook_get_possible_moves(const PieceList &pieces) const
{
	// lambda for adding moves, returns false to break loop
	vector<Position> possibleMoves;
	auto check_move = [&](const int &i, const int &j) {

		auto it = cfind(pieces, make_pair(i, j));

		if (in_bounds(i, j) && // open move
			it == pieces.end())
		{
			possibleMoves.push_back(make_pair(i, j));
			return true;
		}
		else if (in_bounds(i, j) && // check attack
				 it->get_color() != color_ &&
				 it->get_rep() != EMPTY_REP)
		{
			possibleMoves.push_back(make_pair(i, j));
			return false;
		}
		else if (in_bounds(i, j) && // pawn en passant move
				 it->get_rep() == EMPTY_REP)
		{
			possibleMoves.push_back(make_pair(i, j));
			return true;
		}

		return false; // out of bounds
	};

	// check upward moves
	for (int i = position_.first + 1; i < SIZE; ++i)
		if (!check_move(i, position_.second)) break;

	// check downward
	for (int i = position_.first - 1; i >= 0; --i)
		if (!check_move(i, position_.second)) break;

	// check right moves
	for (int i = position_.second + 1; i < SIZE; ++i)
		if (!check_move(position_.first, i)) break;

	// check left moves
	for (int i = position_.second - 1; i >= 0; --i)
		if (!check_move(position_.first, i)) break;

	return possibleMoves;
}

////////////////////////////////////////
// finds all moves piece has avalible in current board state
vector<Position> Piece::pawn_get_possible_moves(const PieceList &pieces) const
{
	// lambda for adding moves
	vector<Position> possibleMoves;
	auto check_move = [&](const int &i, const int &j, const bool &attack) {

		auto it = cfind(pieces, make_pair(i, j));

		// open move
		if (in_bounds(i, j) &&
			!attack &&
			it == pieces.end())
			possibleMoves.push_back(make_pair(i, j));

		// check attacks and en passant
		if (attack &&
			it == pieces.end())
			return;
		else if (in_bounds(i, j) &&
				 attack &&
				 it->get_color() != color_ &&
				 it->get_rep() == EMPTY_REP)
			possibleMoves.push_back(make_pair(-4, j)); // en passant
		else if (in_bounds(i, j) &&
				 attack &&
				 it->get_color() != color_ &&
				 it->get_color() != Color::Empty)
			possibleMoves.push_back(make_pair(i, j));
	};

	if (color_ == Color::Black)
	{
		check_move(position_.first - 1, position_.second, false);

		// check for jump
		if (in_bounds(position_.first - 2, position_.second) &&
			!hasMoved_ &&
			cfind(pieces, make_pair(position_.first - 1, position_.second)) == pieces.end() &&
			cfind(pieces, make_pair(position_.first - 2, position_.second)) == pieces.end())
			possibleMoves.push_back(make_pair(-3, position_.second));

		// check attacks
		check_move(position_.first - 1, position_.second + 1, true);
		check_move(position_.first - 1, position_.second - 1, true);
	}
	else
	{
		check_move(position_.first + 1, position_.second, false);

		// check for jump
		if (in_bounds(position_.first + 2, position_.second) &&
			!hasMoved_ &&
			cfind(pieces, make_pair(position_.first + 1, position_.second)) == pieces.end() &&
			cfind(pieces, make_pair(position_.first + 2, position_.second)) == pieces.end())
			possibleMoves.push_back(make_pair(-3, position_.second));

		// check attacks
		check_move(position_.first + 1, position_.second + 1, true);
		check_move(position_.first + 1, position_.second - 1, true);
	}

	return possibleMoves;
}

////////////////////////////////////////////////////////////////////////////////
//
// HELPER functions
////////////////////////////////////////
// find function overload for PieceList to avoid creating many pointers
PieceList::iterator find(PieceList &pieces, const Position &pos)
{
	// create one object to be used for searching
	static Piece search(Color::Empty, EMPTY_REP, EMPTY_POINTS, Position(), false);
	search.set_position(pos);
	return find(pieces.begin(), pieces.end(), search);
}

////////////////////////////////////////
// find function overload for PieceList to avoid creating many pointers
PieceList::const_iterator cfind(const PieceList &pieces, const Position &pos)
{
	// create one object to be used for searching
	static Piece search2(Color::Empty, EMPTY_REP, EMPTY_POINTS, Position(), false);
	search2.set_position(pos);
	return find(pieces.cbegin(), pieces.cend(), search2);
}

////////////////////////////////////////
// get weighted point value of tile
double get_tile_value(const Position &pos)
{
	// central tiles
	if ((pos.first == 3 || pos.first == 4) &&
		(pos.second == 3 || pos.second == 4))
		return 1.1;
	// next ring from center
	else if (((pos.first == 2 || pos.first == 5) &&
			  (pos.second >= 2 && pos.second <= 5)) ||
			 ((pos.first >= 2 && pos.first <= 5) &&
			  (pos.second == 2 || pos.second == 5)))
		return 1.05;
	// next ring
	else if (((pos.first == 1 || pos.first == 6) &&
			  (pos.second >= 1 && pos.second <= 6)) ||
			 ((pos.first >= 1 && pos.first <= 6) &&
			  (pos.second == 1 || pos.second == 6)))
		return 1.025;

	return 1;
}

////////////////////////////////////////
// checks if position is on board
bool in_bounds(const Position &pos)
{
	return !(pos.first < 0 || pos.first >= SIZE || pos.second < 0 || pos.second >= SIZE);
}

////////////////////////////////////////
// overloaded in_bounds func
bool in_bounds(const int &i, const int &j)
{
	return !(i < 0 || i >= SIZE || j < 0 || j >= SIZE);
}

////////////////////////////////////////
// removes moves that put the king in check
void remove_check_moves(Board &board, const Position &pos,
						const vector<Position> &possibleMoves, vector<Position> &moves)
{
	// find piece color
	Color color = cfind(board.get_pieces(), pos)->get_color();

	// clear current move set
	moves.clear();

	for (const Position &p : possibleMoves)
	{
		// create a copy of current board
		Board updated(board);

		if ((p.first == -1 || p.first == -2) &&
			updated.player_in_check(color)) // remove castling if in check
			continue;

		updated.make_move(pos, p);

		// if move doesn't put player in check then add it to the final move list
		if (!updated.player_in_check(color))
			moves.push_back(p);
	}
}

////////////////////////////////////////
// get distance between two positions, used in player_in_check func
double distance(const Position &a, const Position &b)
{
	return sqrt(pow(a.first - b.first, 2.0) + pow(a.second - b.second, 2.0));
}