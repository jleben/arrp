#pragma once

// expect:
// APP_HEADER = application header
// APP_NAMESPACE = application namespace

#define local_include_str(x) #x
#define local_include(x) local_include_str(x)

#define nonlocal_include(x) <x>

#include nonlocal_include(APP_HEADER)
#include <arrp.hpp>

#include <cstddef>
#include <vector>

#include <H5Cpp.h>

//#include "../../utility/counting_iterator.hpp"

namespace arrp {
namespace hdf5 {

using std::size_t;
using std::vector;

template<typename T>
struct batch_of { using type = T; };

template <typename E>
struct batch_of<arrp::stream_type<E>> { using type = E; };

using output_type = APP_NAMESPACE::traits::output_type;
using batch_type = batch_of<output_type>::type;

template <typename T>
struct hdf5_type;
template<> struct hdf5_type<int>
{
    static const H5::PredType & file_type() { return H5::PredType::STD_I32BE; }
    static const H5::PredType & native_type() { return H5::PredType::NATIVE_INT; }
};
template<> struct hdf5_type<float>
{
    static const H5::PredType & file_type() { return H5::PredType::IEEE_F32BE; }
    static const H5::PredType & native_type() { return H5::PredType::NATIVE_FLOAT; }
};
template<> struct hdf5_type<double>
{
    static const H5::PredType & file_type() { return H5::PredType::IEEE_F64BE; }
    static const H5::PredType & native_type() { return H5::PredType::NATIVE_DOUBLE; }
};

struct writer
{
    using scalar_type = ::arrp::data_traits<output_type>::scalar_type;

    writer(const string & file_path, int length):
        m_length(length),
        m_file(file_path.c_str(), H5F_ACC_TRUNC)
    {
        ::arrp::data_traits<output_type>::size(m_size);

        m_has_time = m_size[0] == -1;

        if (m_has_time)
            m_size[0] = length;

        vector<hsize_t> dims;
        for (auto & s : m_size)
            dims.push_back(s);

        H5::DataSpace dataspace(dims.size(), dims.data());

        m_dataset = m_file.createDataSet("data", hdf5_type<scalar_type>::file_type(), dataspace);

        {
            vector<hsize_t> batch_start(dims.size(), 0);

            vector<hsize_t> batch_size = dims;
            if (m_has_time)
                batch_size[0] = 1;

            m_batch_src_space = H5::DataSpace(dims.size(), batch_size.data());

            m_batch_dst_space = m_dataset.getSpace();
            m_batch_dst_space.selectHyperslab(H5S_SELECT_SET,
                                              batch_size.data(),
                                              batch_start.data());

            m_batch_dst_offset = vector<hssize_t>(dims.size(), 0);
        }
    }

    void output(batch_type & data)
    {
        if (is_done())
            return;

        m_batch_dst_offset[0] = m_index;
        m_batch_dst_space.offsetSimple(m_batch_dst_offset.data());

        m_dataset.write(data, hdf5_type<scalar_type>::native_type(),
                        m_batch_src_space, m_batch_dst_space);

        if (m_has_time)
        {
            m_index += 1;
        }
        else
        {
            m_index += m_size[0];
        }
    }

    bool is_done()
    {
        return m_index >= m_size[0];
    }

private:
    int m_length;
    bool m_has_time = false;
    vector<int> m_size;
    H5::H5File m_file;
    H5::DataSet m_dataset;
    H5::DataSpace m_batch_src_space;
    H5::DataSpace m_batch_dst_space;
    vector<hssize_t> m_batch_dst_offset;
    int m_index = 0;
};

}
}

