#ifndef ADAPTER_RECEIVER_HPP
#define ADAPTER_RECEIVER_HPP

#include <string>

class AdapterReceiver
{
    public:
        virtual void process(const std::string& message) = 0;
};

#endif