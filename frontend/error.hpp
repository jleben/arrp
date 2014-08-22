#ifndef STREAM_LANG_ERROR_INCLUDED
#define STREAM_LANG_ERROR_INCLUDED

#include <exception>
#include <sstream>
#include <string>

// FIXME: Enable on later versions of MS compiler:
#ifdef _MSC_VER
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace stream {

using std::string;

struct abort_error {};

struct error : public std::exception
{
public:
    error(const string & what):
        m_msg(what)
    {}

    virtual const char *what() const NOEXCEPT
    {
        return m_msg.c_str();
    }

private:
    string m_msg;
};

struct source_error : public error
{
    source_error(const string & what, int line):
        error(msg(what, line))
    {}

private:
    static string msg(const string & what, int line)
    {
        std::ostringstream text;
        text << "[line " << line << "] " << what;
        return text.str();
    }
};

}

#endif // STREAM_LANG_ERROR_INCLUDED
