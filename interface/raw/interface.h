#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>

namespace arrp {
namespace generic_io {

using std::string;
using std::istream;
using std::ostream;
using std::shared_ptr;
using std::unordered_map;

template <typename T>
class AbstractChannel
{
public:
    virtual ~AbstractChannel() {}
    virtual void transfer(T* location, size_t count) = 0;
    virtual void transfer(T & location) = 0;
};

template <typename T>
class TextInputStream : public AbstractChannel<T>
{
public:
    TextInputStream(istream *d): d_stream(d) {}

    virtual void transfer(T & location)
    {
        (*d_stream) >> location;
    }

    virtual void transfer(T* location, size_t count)
    {
        for(size_t i = 0; i < count; ++i)
        {
            (*d_stream) >> *(location + i);
        }
    }

private:
    istream * d_stream = nullptr;
};

template <typename T>
class BinaryInputStream : public AbstractChannel<T>
{
public:
    BinaryInputStream(istream *d): d_stream(d) {}

    virtual void transfer(T & location)
    {
        d_stream->read((char*)(&location), sizeof(T));
    }

    virtual void transfer(T* location, size_t count)
    {
        d_stream->read((char*)(&location), count * sizeof(T));
    }

private:
    istream * d_stream;
};

template <typename T>
class TextOutputStream : public AbstractChannel<T>
{
public:
    TextOutputStream(ostream *d): d_stream(d) {}

    virtual void transfer(T & location)
    {
        (*d_stream) << location << std::endl;
    }

    virtual void transfer(T* location, size_t count)
    {
        for(size_t i = 0; i < count; ++i)
        {
            (*d_stream) << *(location + i) << std::endl;
        }
    }

private:
    ostream * d_stream;
};

template <typename T>
class BinaryOutputStream : public AbstractChannel<T>
{
public:
    BinaryOutputStream(ostream *d): d_stream(d) {}

    virtual void transfer(T & location)
    {
        d_stream->write((char*)(&location), sizeof(T));
    }

    virtual void transfer(T* location, size_t count)
    {
        d_stream->write((char*)(&location), count * sizeof(T));
    }

private:
    ostream * d_stream;
};

struct ChannelConfig
{
    string value;
    string type;
    string format;
};

class AbstractChannelManager
{
public:
    virtual ~AbstractChannelManager() {}
    virtual void setup(ChannelConfig &) = 0;
    virtual std::ios* stream() = 0;
    virtual bool is_stream() const = 0;
    virtual bool is_input() const = 0;
};

static string infer_channel_type(string value)
{
    if (value.empty())
        return string();

    if (value[0] == '-' or (value[0] >= 48 and value[1] <= 57))
    {
        return "value";
    }
    if (value == "pipe")
    {
        return "pipe";
    }
    return "file";
}

template <typename T>
class ChannelManager : public AbstractChannelManager
{
public:
    ChannelManager(shared_ptr<AbstractChannel<T>>& c, bool is_input, bool is_stream):
        channel(c),
        d_is_input(is_input),
        d_is_stream(is_stream)
    {}

    ~ChannelManager()
    {
        if (owns_stream)
        {
            delete in_stream;
            delete out_stream;
        }
    }

    bool is_stream() const override
    {
        return d_is_stream;
    }

    bool is_input() const override
    {
        return d_is_input;
    }

    std::ios* stream() override
    {
        if (d_is_input)
            return in_stream;
        else
            return out_stream;
    }

    virtual void setup(ChannelConfig & config)
    {
        using namespace std;

        if (config.type == "value")
        {
            if (d_is_input)
            {
                owns_stream = true;
                value = config.value;
                // FIXME: This discards last element in 'value'
                // if it is not followed by whitespace.
                in_stream = new istringstream(value);
                channel = make_shared<TextInputStream<T>>(in_stream);
            }
            else
            {
                throw std::runtime_error("Invalid output channel type: value.");
            }
            return;
        }

        if (config.type == "pipe")
        {
            if (d_is_input)
                in_stream = &cin;
            else
                out_stream = &cout;
            // FIXME: Do we need std::ios_base::binary for cin and cout?
        }
        else if (config.type == "file")
        {
            owns_stream = true;

            std::ios_base::openmode mode = d_is_input ? std::ios_base::in : std::ios_base::out;
            if (config.format == "raw")
                mode |= std::ios_base::binary;

            if (d_is_input)
            {
                auto file = new ifstream(config.value, mode);
                if (!file->is_open())
                    throw std::runtime_error("Failed to open file: " + config.value);
                in_stream = file;
            }
            else
            {
                auto file = new ofstream(config.value, mode);
                if (!file->is_open())
                    throw std::runtime_error("Failed to open file: " + config.value);
                out_stream = file;
            }
        }
        else
        {
            throw std::runtime_error("Invalid channel type: " + config.type);
        }

        if (config.format == "raw")
        {
            if (d_is_input)
                channel = make_shared<BinaryInputStream<T>>(in_stream);
            else
                channel = make_shared<BinaryOutputStream<T>>(out_stream);
        }
        else if (config.format == "text")
        {
            if (d_is_input)
                channel = make_shared<TextInputStream<T>>(in_stream);
            else
                channel = make_shared<TextOutputStream<T>>(out_stream);
        }
        else
        {
            throw std::runtime_error("Invalid channel format: " + config.format);
        }
    }

    shared_ptr<AbstractChannel<T>>& channel;
    const bool d_is_input = false;
    const bool d_is_stream = false;
    string value;
    istream * in_stream = nullptr;
    ostream * out_stream = nullptr;
    bool owns_stream = false;
};

using ChannelManagerMap = unordered_map<string, shared_ptr<AbstractChannelManager>>;

}
}
