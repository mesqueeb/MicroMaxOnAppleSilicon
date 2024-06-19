#ifndef STATIC_LINKER_HPP
#define STATIC_LINKER_HPP

#include "Linker.hpp"

class StaticLinker : public Linker
{
    public:
        virtual void create() = 0;

        virtual void close() = 0;

        virtual std::string read() = 0;

        virtual void write(const std::string &) = 0;
};

#endif
