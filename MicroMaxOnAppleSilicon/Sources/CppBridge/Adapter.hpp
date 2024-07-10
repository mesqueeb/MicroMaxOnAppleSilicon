#ifndef ADAPTER_HPP
#define ADAPTER_HPP

#include <string>

namespace Framework
{
	class Adapter
	{
		public:
			virtual ~Adapter() {}
    
			// Establish connection to the other party
			virtual void open() = 0;

			// Close the exisiting connection
			virtual void close() = 0;

			// Read a line from the other party
			virtual std::string read() = 0;

			// Write a line to the other party
			virtual void write(const std::string& line) = 0;
	};
}

#endif
