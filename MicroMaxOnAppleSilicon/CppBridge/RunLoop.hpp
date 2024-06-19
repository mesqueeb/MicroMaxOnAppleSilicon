#ifndef RUN_LOOP_HPP
#define RUN_LOOP_HPP

class RunLoop
{
    public:
        virtual void run()  = 0;
        virtual void exit() = 0;
};

#endif
