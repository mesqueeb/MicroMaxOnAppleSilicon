#ifndef B_BOOK_HPP
#define B_BOOK_HPP

#include <fstream>
#include <string>
#include <vector>

#include "BMove.h"
#include "BRkiss.h"
#include "BPosition.h"

namespace BBoard
{
    /*
     * A Polyglot book is a series of "entries" of 16 bytes. All integers are
     * stored highest byte first (regardless of size). The entries are ordered
     * according to key. Lowest key first.
     */
    struct BookEntry
    {
        uint64_t key;
        uint16_t move;
        uint16_t count;
        uint32_t learn;
    };

    class BBBook
    {
        public:
            BBBook();
            ~BBBook();
 
            void open(const std::string& fileName);
            void close();

            Move get_move(const Position& pos, bool findBestMove);
        
            // Query for all book moves for the given position
            void query(const Position& pos, std::vector<std::pair<Move, float> > &moves);

        private:
            template<typename T> void get_number(T& n);

            BookEntry read_entry(int idx);
            int find_entry(uint64_t key);

            std::ifstream bookFile;
            int bookSize;
            RKISS RKiss;
    };
}

#endif
