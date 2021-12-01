#pragma once
#include <cstdint>
#include <string>

class BitBoard {
private:
	uint64_t a, b;
	
public:
	explicit BitBoard(uint64_t a, uint64_t b);
	operator bool() const;
	static BitBoard empty();
	static BitBoard full();
	static BitBoard row(unsigned r);
	static BitBoard column(unsigned c);
	//static BitBoard cube(unsigned r, unsigned c);

	bool at(unsigned r, unsigned c) const;

	BitBoard operator|(const BitBoard other) const;
	BitBoard operator&(const BitBoard other) const;
	BitBoard operator-(const BitBoard other) const;

	BitBoard shiftRight() const;
	BitBoard shiftDown() const;

	int count() const;

	std::string str() const;
};
