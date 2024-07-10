#ifndef VALIDATION_HPP
#define VALIDATION_HPP

#include <string>

struct Validation
{
    Validation() : canWhiteMove(true), canBlackMove(true), canWKCastle(true), canWQCastle(true), canBKCastle(true), canBQCastle(true) {}

    std::size_t wc, bc;
    std::size_t wk, wq, wr, wb, wn, wp;
    std::size_t bk, bq, br, bb, bn, bp;
    
	// Indicate the validity of the position
	bool isValid;

	std::string error, state;
    
	bool canWhiteMove, canBlackMove;
	bool canWKCastle, canWQCastle, canBKCastle, canBQCastle;
};

#endif
