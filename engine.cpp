#include "engine.h"
#include <cassert>
#include <bitset>
#include <algorithm>
#include <iostream>

namespace {
	const uint64_t ROW_0 = 0x1FFULL;
	const uint64_t TOP_LEFT_CUBE = 0x7ULL | (0x7ULL << 9) | (0x7ULL << 18);
	const uint64_t ALL_ALLOWED_BITS_IN_A = 0x3FFFFFFFFFFFFFULL;
	const uint64_t ALL_ALLOWED_BITS_IN_B = 0x7FFFFFFULL;
	const uint64_t RIGHT_MOST_COLUMN_B = (1ULL << 8) | (1ULL << 17) | (1ULL << 26);
	const uint64_t RIGHT_MOST_COLUMN_A = RIGHT_MOST_COLUMN_B
		 | (1ULL << 35) | (1ULL << 44) | (1ULL << 53);
	const uint64_t ROW_5 = 0x1FFULL << (5 * 9);
}

// === BIT BOARD

BitBoard::BitBoard(uint64_t a, uint64_t b) : a(a), b(b) {}

bool BitBoard::operator==(BitBoard other) const {
	return a == other.a && b == other.b;
}

bool BitBoard::operator<(BitBoard other) const {
	return a < other.a || (a == other.a && b < other.b);
}

bool BitBoard::at(unsigned r, unsigned c) const {
	assert(r < 9);
	assert(c < 9);
	if (r <= 5) {
		return (1ULL << (r * 9 + c)) & a;
	}
	else {
		return (1ULL << ((r - 6) * 9 + c)) & b;
	}
}

BitBoard BitBoard::operator|(const BitBoard other) const {
	return BitBoard(a | other.a, b | other.b);
}

BitBoard BitBoard::operator&(const BitBoard other) const {
	return BitBoard(a & other.a, b & other.b);
}

BitBoard BitBoard::operator-(const BitBoard other) const {
	return BitBoard(a &~other.a, b &~other.b);
}

BitBoard BitBoard::empty() {
	return BitBoard(0, 0);
}

BitBoard BitBoard::full() {
	return BitBoard(ALL_ALLOWED_BITS_IN_A, ALL_ALLOWED_BITS_IN_B);
}

BitBoard BitBoard::row(unsigned r) {
	assert(r < 9);
	if (r <= 5) {
		return BitBoard(ROW_0 << (r * 9), 0);
	}
	else {
		return BitBoard(0, ROW_0 << ((r - 6)*9));
	}
}

BitBoard BitBoard::column(unsigned c) {
	assert(c < 9);
	int to_shift = 8 - c;
	return BitBoard(RIGHT_MOST_COLUMN_A >> to_shift, RIGHT_MOST_COLUMN_B >> to_shift);
}

BitBoard BitBoard::cube(unsigned r, unsigned c) {
	assert(c < 3);
	assert(r < 3);
	if (r < 2) {
		return BitBoard(TOP_LEFT_CUBE << (3*c + 27*r), 0);
	}
	else {
		return BitBoard(0, TOP_LEFT_CUBE << 3 * c);
	}
}


BitBoard BitBoard::shiftRight() const {
	return BitBoard((a &~ RIGHT_MOST_COLUMN_A) << 1, (b&~RIGHT_MOST_COLUMN_B) << 1);
}

BitBoard BitBoard::shiftDown() const {
	return BitBoard((a << 9) & ALL_ALLOWED_BITS_IN_A, 
		((b << 9) | (a & ROW_5) >> 45) & ALL_ALLOWED_BITS_IN_B);
}


int BitBoard::count() const {
	return (int)std::bitset<64>(a).count() + (int)std::bitset<64>(b).count();
}

BitBoard::operator bool() const {
	return a | b;
}

std::string BitBoard::str() const {
	std::string result;
	for (int r = 0; r < 9; ++r) {
		for (int c = 0; c < 9; ++c) {
			result += at(r, c) ? '#' : '.';
		}
		result += "\n";
	}
	return result;
}

// ====== Piece
Piece::Piece(uint64_t a): bb(BitBoard(a, 0)) {}
Piece::Piece() : bb(BitBoard::empty()){};
BitBoard Piece::getBitBoard() const {
	return bb;
}


PieceIterator::PieceIterator(uint8_t i): i(i)  {}

PieceIterator PieceIteratorGenerator::begin() const {
	return PieceIterator(0);
}


namespace {

	const uint64_t A = 1ULL << 0;
	const uint64_t B = 1ULL << 1;
	const uint64_t C = 1ULL << 2;
	const uint64_t D = 1ULL << 3;
	const uint64_t E = 1ULL << 4;
	const uint64_t F = 1ULL << 9;
	const uint64_t G = 1ULL << 10;
	const uint64_t H = 1ULL << 11;
	const uint64_t I = 1ULL << 18;
	const uint64_t J = 1ULL << 19;
	const uint64_t K = 1ULL << 20;
	const uint64_t L = 1ULL << 27;
	const uint64_t M = 1ULL << 36;
	/*
	A B C D E
	F G H
	I J K
	L
	M */
	const uint64_t PIECES[] = {
		// 1 square
		A,

		// 2 squares
		A | B,
		A | F,
		A | G,
		B | F,

		// 3 squares
		A | B | C,
		A | F | I,
		A | G | K,
		C | G | I,
		A | B | F,
		A | F | G,
		B | F | G,
		A | B | G,

		// 4 squares
		A | B | C | D,
		A | F | I | L,
		A | B | F | G,

		// L Shape
		A | F | I | J,
		C | F | G | H,
		A | B | G | J,
		A | B | C | F,

		// Flipped L
		A | B | F | I,
		A | B | C | H,
		B | G | I | J,
		A | F | G | H,


		//  X
		// XXX
		B | F | G | H,
		A | F | G | I,
		B | F | G | J,
		A | B | C | G,

		// XX
		//  XX
		A | B | G | H,
		B | F | G | I,
		A | F | G | J,
		B | C | F | G,

		// 5 squares
		A | B | C | D | E, // #####
		A | F | I | L | M,

		A | F | G | H | I, // #
		A | B | C | G | J, // ###
		B | G | I | J | K, // #
		C | F | G | H | K,

		A | B | C | F | H, // ##
		A | B | G | I | J, // #
		A | B | F | I | J, // ##
		A | C | F | G | H,

		A | B | C | F | I, // #
		A | B | C | H | K, // #
		C | H | I | J | K, // ###
		A | F | I | J | K,

		B | F | G | H | J, // + sign
	};
}

Piece Piece::getRandom() {
	return Piece(PIECES[rand() % 47]);
}

bool Piece::operator<(Piece other) const {
	return bb < other.bb;
}

PieceIterator PieceIteratorGenerator::end() const {
	return PieceIterator(sizeof PIECES / sizeof *PIECES);
}

Piece PieceIterator::operator*() const {
	return Piece(PIECES[i]);
}


PieceIteratorGenerator Piece::getAll() {
	return PieceIteratorGenerator();
}

void PieceIterator::operator++() {
	i++;
}

bool PieceIterator::operator!=(PieceIterator other) const {
	return i != other.i;
}

// ====== Game State
GameState::GameState(BitBoard bb) : bb(bb) {}
BitBoard GameState::getBitBoard() const {
	return bb;
}
NextGameStateIteratorGenerator GameState::nextStates(Piece piece) const {
	return NextGameStateIteratorGenerator(*this, piece);
}

double GameState::simpleEval() const {
	double result = bb.count();

	for (int i = 0; i < 9; i++) {
		if (bb & BitBoard::column(i)) {
			result += 1;
		}
		if (bb & BitBoard::row(i)) {
			result += 1;
		}
	}

	for (int r = 0; r < 3; ++r) {
		for (int c = 0; c < 3; ++c) {
			if (bb & BitBoard::cube(r, c)) {
				result += 1;
			}
		}
	}

	return result;
}


NextGameStateIterator::NextGameStateIterator(GameState state, Piece piece):
	original(state), left(piece.getBitBoard()), next(piece.getBitBoard()) {
	if (!canPlace()) {
		operator++();
	}
}

GameState NextGameStateIterator::operator*() const {
	const auto after_add = original.getBitBoard() | next;
	auto result = after_add;
	for (int i = 0; i < 9; i++) {
		const auto col = BitBoard::column(i);
		if ((after_add & col) == col) {
			result = result - col;
		}
		const auto row = BitBoard::row(i);
		if ((after_add & row) == row) {
			result = result - row;
		}
	}

	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 3; c++) {
			const auto cube = BitBoard::cube(r, c);
			if ((after_add & cube) == cube) {
				result = result - cube;
			}
		}
	}
	
	return GameState(result);
}

bool NextGameStateIterator::operator!=(NextGameStateIterator other) const {
	return other.next != next;
}

void NextGameStateIterator::operator++() {
	do {
		// We've reached the right edge.
		if (next & BitBoard::column(8)) {

			// We've reached the bottom edge too.
			if (left & BitBoard::row(8)) {
				next = BitBoard::empty();
				break;
			}

			left = left.shiftDown();
			next = left;
		}
		else {
			next = next.shiftRight();
		}
	} while (!canPlace());
}


bool NextGameStateIterator::canPlace() const {
	return !(next & original.getBitBoard());
}

NextGameStateIteratorGenerator::NextGameStateIteratorGenerator(
	GameState state, Piece piece):
	state(state), piece(piece) {
}

NextGameStateIterator NextGameStateIteratorGenerator::begin() const {
	return NextGameStateIterator(state, piece);
}


NextGameStateIterator NextGameStateIteratorGenerator::end() const {
	return NextGameStateIterator(state, Piece(BitBoard::empty()));
}

// ====== AI
GameState AI::makeMove(GameState game, Piece p1, Piece p2, Piece p3) {
	Piece pieces[3] = { p1, p2, p3 };
	std::sort(pieces, pieces+3);

	double bestScore = 9999999999;
	auto bestNext = GameState(BitBoard::full());

	// If we can clear with 2 pieces or fewer, then we must try permutations.
	bool canClearWith2Pieces = false;
	for (const auto after_p1 : game.nextStates(pieces[0])) {
		if (after_p1.getBitBoard().count() < game.getBitBoard().count()) {
			canClearWith2Pieces = true;
		}
		for (const auto after_p2 : after_p1.nextStates(pieces[1])) {
			if (after_p2.getBitBoard().count() < after_p1.getBitBoard().count()) {
				canClearWith2Pieces = true;
			}
		}
	}

	do {
		for (const auto after_p1 : game.nextStates(pieces[0])) {
			for (const auto after_p2 : after_p1.nextStates(pieces[1])) {
				for (const auto after_p3 : after_p2.nextStates(pieces[2])) {
					double total_after_p3 = 0;
					for (const auto p4 : Piece::getAll()) {
						double best_after_p4 = 99999999;
						for (const auto after_p4 : after_p3.nextStates(p4)) {
							best_after_p4 = std::min(best_after_p4,
								(double)after_p4.simpleEval());
						}
						total_after_p3 += best_after_p4;
						if (total_after_p3 > bestScore) {
							// It already looks shitty.
							break;
						}
					}
					if (total_after_p3 < bestScore) {
						bestScore = total_after_p3;
						bestNext = after_p3;
					}
				}
			}
		}
	} while (canClearWith2Pieces && std::next_permutation(pieces, pieces+3));

	return bestNext;
}