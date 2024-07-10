#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <string>

// This violates the concepts of layers, but there isn't any other simple solutions
#include "Types.hpp"

namespace Framework
{
	class Protocol
	{
		public:
			virtual ~Protocol() {}
    
			// Open connection to the protocol
			virtual void open() = 0;

            // Start from the default state
            virtual void start() = 0;

			// Start from a given state
            virtual void start(const std::string &state) = 0;

			// Request a move from the other player
			virtual void think(unsigned duration) = 0;
    
			// Whether it's currently analyzing
			virtual bool isAnalyzing() const = 0;

			// Whether it's currently waiting for a move
			virtual bool isThinking() const = 0;

			// Analyse the current state for specified seconds
			virtual void analyze() = 0;
    
			// Stop the current operation
			virtual void stop() = 0;

			// Return true if the other player is ready
			virtual bool isReady() = 0;

			// Close the protocol and its underlying connection.
			virtual void close() = 0;

			// Update the board to state
			virtual void update(const std::string& state) = 0;

			// Update the board by a movement
			virtual void update(unsigned srcFile, unsigned srcRank, unsigned dstFile, unsigned dstRank, unsigned promote) = 0;
	};
}

#endif
