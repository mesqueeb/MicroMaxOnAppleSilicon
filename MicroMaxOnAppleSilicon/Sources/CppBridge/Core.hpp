#ifndef CORE_HPP
#define CORE_HPP

#include <vector>
#include <string>
#include "Types.hpp"
#include "VirtualMove.hpp"
#include "VirtualSquare.hpp"

namespace Framework
{
	class Core
	{
		public:
			virtual ~Core() {}
    
			// Initialise the core
			virtual void open() = 0;

			// Close the core and resources
			virtual void close() = 0;

			// Start the core from default state
			virtual void start() = 0;

			// Start the core from a given state
			virtual void start(const std::string& state) = 0;

			// Add a piece to the given square.
			virtual bool add(const VirtualSquare &square, const VirtualPiece &piece) = 0;

			// Generate all legal moves in the current position
            virtual void getMoves(std::vector<VirtualMove>&, bool extended = false) const {}

			// Generate all legal moves from a specified source in the current position
			virtual void getMoves(const VirtualSquare &,
                                  std::vector<VirtualMove> &,
                                  bool) const = 0;

			// Report the current status for the core
			virtual Status getStatus() const = 0;

			// Return true if the move is legal, otherwise false.
			virtual bool canPlay(const VirtualSquare &,
                                 const VirtualSquare &,
                                 const VirtualPiece  &,
                                 VirtualMove *move = nullptr) const = 0;

			// Play a move from src to dst
			virtual bool play(const VirtualSquare &,
                              const VirtualSquare &,
                              const VirtualPiece  &promote = NilPiece,
                              VirtualMove *move = nullptr) = 0;

			virtual void *getInternal() const = 0;

            // Returns the number of pieces on the board
            virtual unsigned countPieces() const = 0;

#ifdef SMALLCHESS_FRAMEWORK
            virtual std::shared_ptr<Core> copy() const = 0;
#endif

            virtual void captured(unsigned &wq,
                                  unsigned &wr,
                                  unsigned &wb,
                                  unsigned &wn,
                                  unsigned &wp,
                                  unsigned &bq,
                                  unsigned &br,
                                  unsigned &bb,
                                  unsigned &bn,
                                  unsigned &bp) const = 0;
        
			virtual void update(const std::string &) = 0;

			virtual VirtualPiece getPiece(const VirtualSquare &) const = 0;
   
			virtual VirtualSquare find(VirtualPiece piece) = 0;
	};
}

#endif
