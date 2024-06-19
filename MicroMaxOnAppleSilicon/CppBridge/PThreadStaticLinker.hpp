#ifndef PTHREAD_STATIC_LINKER_HPP
#define PTHREAD_STATIC_LINKER_HPP

#include <string>
#include "StaticLinker.hpp"

class PThreadStaticLinkerImpl;

class PThreadStaticLinker : public StaticLinker
{
    public:
        PThreadStaticLinker(const std::string &engine);
        virtual ~PThreadStaticLinker() { close(); }

        // Inherited from StaticLinking
        virtual void create();

        // Inherited from StaticLinking
        virtual void close();

        // Inherited from StaticLinking
        virtual std::string read();

        // Inherited from StaticLinking
        virtual void write(const std::string& line);

    private:
        PThreadStaticLinkerImpl *_pimpl;
};

#endif
