#ifndef LINKER_HPP
#define LINKER_HPP

#include <string>

class Linker
{
    public:
        // Createa a link
        virtual void create() = 0;

        // Close the existing link
        virtual void close() = 0;

        // Read a line from the existing link
        virtual std::string read() = 0;

        // Write a line to the existing link
        virtual void write(const std::string& line) = 0;
};

#endif
