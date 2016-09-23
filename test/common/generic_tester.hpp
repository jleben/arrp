#pragma once

template <typename P>
class generic_tester
{
public:
    generic_tester(P * program)
    {
        m_program = program;
    }

    template <typename IO>
    generic_tester(IO * io)
    {
        m_program = new P;
        m_program->io = io;
    }

    void run(int count)
    {
        m_program->prelude();
        while(count--)
            m_program->period();
    }

private:
    P * m_program;
};
