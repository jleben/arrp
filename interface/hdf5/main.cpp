#include "writer.hpp"
#include "../../compiler/arg_parser.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;
using namespace arrp::hdf5;
using namespace H5;

using app_type = APP_NAMESPACE::program<writer>;

int main(int argc, char *argv[])
{
    arrp::hdf5::writer * writer;

    string file_path("output.h5");
    int length = 0;
    // FIXME: This is just a temporary safety measure:
    int max_period_count = 100000;

    stream::compiler::arguments args;
    args.add_option({"out", "o", "", "Output file."},
                    new stream::compiler::string_option(&file_path));
    args.add_option({"length", "l", "", "Output stream length."},
                    new stream::compiler::int_option(&length));

    try {
        args.parse(argc-1, argv+1);
    } catch (stream::compiler::arguments::error & e) {
        cerr << "Command line error: " << e.msg() << endl;
        return 1;
    }

    cout << "Output file: " << file_path << endl;
    cout << "Output stream length: " << length << endl;

    try {
        writer = new arrp::hdf5::writer(file_path, length);
    } catch (H5::Exception & e) {
        cerr << "Failed to create writer: " << e.getCDetailMsg() << endl;
        return 1;
    }

    app_type * app = new app_type;
    app->io = writer;

    try {
        app->prelude();

        int period = 0;
        while(!writer->is_done())
        {
            if (period > max_period_count)
            {
                ostringstream msg;
                msg << "Timed out at " << period << " periods." << endl;
                throw std::runtime_error(msg.str());
            }

            app->period();
            ++period;
        }
    } catch (H5::Exception & e) {
        cerr << "HDF5 Error: " << e.getCDetailMsg() << endl;
    } catch (std::exception & e) {
        cerr << "Error: " << e.what() << endl;
    }

    delete app;
    delete writer;

    return 0;
}

