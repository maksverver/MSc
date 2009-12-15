#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

extern "C" double time_now();

class Timer
{
public:
    Timer() : last_(time_now()) { };
    double elapsed() { return time_now() - last_; }

private:
    double last_;
};

#endif /* nde fTIMER_H_INCLUDED */
