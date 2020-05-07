#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace arrp {
namespace generic_io {

using std::string;
using std::istream;
using std::ostream;
using std::shared_ptr;
using std::unordered_map;
using std::vector;

template <typename T>
class AbstractChannel
{
public:
    virtual ~AbstractChannel() {}
    virtual void transfer(T* location, size_t count) = 0;
};


template <typename T>
struct promoted_int { typedef T type; };

template <>
struct promoted_int<int8_t> { typedef int type; };

template <>
struct promoted_int<uint8_t> { typedef int type; };


template <typename T>
class TextInputStream : public AbstractChannel<T>
{
public:
    TextInputStream(istream *d): d_stream(d)
    {
        d->exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    virtual void transfer(T* location, size_t count) override
    {
        for(size_t i = 0; i < count; ++i)
        {
            typename promoted_int<T>::type value;
            (*d_stream) >> value;
            *(location + i) = value;
        }
    }

private:
    istream * d_stream = nullptr;
};

template <typename T>
class TextOutputStream : public AbstractChannel<T>
{
public:
    TextOutputStream(ostream *d): d_stream(d)
    {
        d->exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    virtual void transfer(T* location, size_t count) override
    {
        for(size_t i = 0; i < count; ++i)
        {
            typename promoted_int<T>::type value = *(location + i);
            (*d_stream) << value << std::endl;
        }
    }

private:
    ostream * d_stream;
};

template <typename T>
class BinaryInputStream : public AbstractChannel<T>
{
public:
    BinaryInputStream(istream *d): d_stream(d)
    {
        d->exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    virtual void transfer(T* location, size_t count) override
    {
        //fprintf(stderr, "Input: %lu\n", count);
        d_stream->read((char*)(location), count * sizeof(T));
    }

protected:
    istream * d_stream;
};

template <typename T>
class BinaryOutputStream : public AbstractChannel<T>
{
public:
    BinaryOutputStream(ostream *d): d_stream(d)
    {
        d->exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    virtual void transfer(T* location, size_t count) override
    {
        //fprintf(stderr, "Output: %lu\n", count);
        d_stream->write((char*)(location), count * sizeof(T));
    }

protected:
    ostream * d_stream;
};

template <typename T>
class BufferedBinaryInputStream : public BinaryInputStream<T>
{
public:
    BufferedBinaryInputStream(istream *d, int buffer_size):
        BinaryInputStream<T>(d),
        d_buffer_size(buffer_size)
    {
        d_buffer = new T[buffer_size];
        d->exceptions(std::ifstream::badbit);
    }

    ~BufferedBinaryInputStream()
    {
        delete[] d_buffer;
    }

    virtual void transfer(T* destination, size_t count) override
    {
        using namespace std;

        int dst_index = 0;

        for (; d_buffer_index < d_buffer_count and dst_index < count;
             ++d_buffer_index, ++dst_index)
        {
            destination[dst_index] = d_buffer[d_buffer_index];
        }

        if (dst_index < count)
        {
            //cerr << "Reading... " << endl;
            d_buffer_index = 0;
            this->d_stream->read((char*)(d_buffer), d_buffer_size * sizeof(T));
            d_buffer_count = this->d_stream->gcount() / sizeof(T);
            //cerr << "Read " << d_buffer_count << " items." << endl;
            if (count - dst_index > d_buffer_count)
                throw std::ios_base::failure("Extracted fewer characters than required.");
        }

        for (; d_buffer_index < d_buffer_count and dst_index < count;
             ++d_buffer_index, ++dst_index)
        {
            destination[dst_index] = d_buffer[d_buffer_index];
        }
    }

private:
    T * d_buffer;
    int d_buffer_size = 0;
    int d_buffer_index = 0;
    int d_buffer_count = 0;
};

template <typename T>
class BufferedBinaryOutputStream : public BinaryOutputStream<T>
{
public:
    BufferedBinaryOutputStream(ostream *d, int buffer_size):
        BinaryOutputStream<T>(d),
        d_buffer_size(buffer_size)
    {
        d_buffer = new T[buffer_size];
        d->exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    ~BufferedBinaryOutputStream()
    {
        this->d_stream->exceptions(std::ios_base::iostate());

        if (d_buffer_index > 0)
        {
            write(d_buffer_index);
        }

        delete[] d_buffer;
    }

    virtual void transfer(T* source, size_t count) override
    {
        int src_index = 0;

        for (; d_buffer_index < d_buffer_size and src_index < count;
             ++d_buffer_index, ++src_index)
        {
            d_buffer[d_buffer_index] = source[src_index];
        }

        if (d_buffer_index == d_buffer_size)
        {
            d_buffer_index = 0;
            write(d_buffer_size);
        }

        for (; d_buffer_index < d_buffer_size and src_index < count;
             ++d_buffer_index, ++src_index)
        {
            d_buffer[d_buffer_index] = source[src_index];
        }
    }

private:
    void write(size_t count)
    {
        this->d_stream->write((char*)(d_buffer), count * sizeof(T));
    }

    T * d_buffer;
    int d_buffer_size = 0;
    int d_buffer_index = 0;
};


struct ChannelConfig
{
    string value;
    string type;
    string format;
    int max_buffer_size = 1024;
};

struct ActualChannelConfig
{
    string value;
    string type;
    string format;
    int block_size = 0;
};

class AbstractChannelManager
{
public:
    virtual ~AbstractChannelManager() {}
    virtual void setup(ChannelConfig &) = 0;
    virtual std::ios* stream() = 0;
    virtual bool is_stream() const = 0;
    virtual bool is_input() const = 0;
    virtual ActualChannelConfig configuration() const = 0;
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
    struct Properties
    {
        bool is_input;
        bool is_stream;
        int transfer_size;
    };

    ChannelManager(shared_ptr<AbstractChannel<T>>& c, const Properties & properties):
        channel(c),
        d_properties(properties)
    {
        using namespace std;
#if 0
        cerr << "Channel: "
             << " input: " << properties.is_input
             << " stream: " << properties.is_stream
             << " chunk: " << properties.transfer_size
             << endl;
#endif
    }

    ~ChannelManager()
    {
        // Make sure channel is destroyed before io streams
        channel = nullptr;

        if (owns_stream)
        {
            delete in_stream;
            delete out_stream;
        }
    }

    bool is_stream() const override
    {
        return d_properties.is_stream;
    }

    bool is_input() const override
    {
        return d_properties.is_input;
    }

    std::ios* stream() override
    {
        if (d_properties.is_input)
            return in_stream;
        else
            return out_stream;
    }

    ActualChannelConfig configuration() const override
    {
        return d_configuration;
    }

    virtual void setup(ChannelConfig & config) override
    {
        using namespace std;

        d_configuration.type = config.type;
        d_configuration.value = config.value;
        d_configuration.format = config.format;
        d_configuration.block_size = d_properties.transfer_size;

        if (config.type == "value")
        {
            if (d_properties.is_input)
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
            if (d_properties.is_input)
                in_stream = &cin;
            else
                out_stream = &cout;
            // FIXME: Do we need std::ios_base::binary for cin and cout?
        }
        else if (config.type == "file")
        {
            owns_stream = true;

            std::ios_base::openmode mode = d_properties.is_input ? std::ios_base::in : std::ios_base::out;
            if (config.format == "raw")
                mode |= std::ios_base::binary;

            if (d_properties.is_input)
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
            bool should_buffer = config.max_buffer_size >= d_properties.transfer_size * 2;

            if (d_properties.is_input)
            {
                if (should_buffer)
                    channel = make_shared<BufferedBinaryInputStream<T>>(in_stream, config.max_buffer_size);
                else
                    channel = make_shared<BinaryInputStream<T>>(in_stream);
            }
            else
            {
                if (should_buffer)
                    channel = make_shared<BufferedBinaryOutputStream<T>>(out_stream, config.max_buffer_size);
                else
                    channel = make_shared<BinaryOutputStream<T>>(out_stream);
            }

            if (should_buffer)
            {
                d_configuration.block_size = config.max_buffer_size;
            }
        }
        else if (config.format == "text")
        {
            if (d_properties.is_input)
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
    const Properties d_properties;
    ActualChannelConfig d_configuration;
    string value;
    istream * in_stream = nullptr;
    ostream * out_stream = nullptr;
    bool owns_stream = false;
};

using ChannelManagerMap = unordered_map<string, shared_ptr<AbstractChannelManager>>;

}
}
