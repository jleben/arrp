#include "filesystem.hpp"
#include "subprocess.hpp"

#include <random>
#include <iostream>

using namespace std;

namespace arrp {
namespace filesystem {

temporary_dir::temporary_dir() {}

const string & temporary_dir::name()
{
    if (!d_name.empty())
        return d_name;

    string base = "/tmp/arrp";
    subprocess::run("mkdir -p " + base);

    random_device rd;
    minstd_rand gen(rd());
    uniform_int_distribution<> letter(65, 90);

    string full_name;
    string name;
    bool ok = false;
    int tries = 5;
    while(!ok and tries--)
    {
        string name;
        int count = 6;
        while(count--)
        {
            char c(letter(gen));
            name.push_back(c);
        }

        full_name = base + "/" + name;

        ok = true;
        try { subprocess::run("mkdir " + full_name); }
        catch (subprocess::error &) { ok = false; }
    }

    if (!ok)
    {
        throw stream::error("Failed to create temporary directory.");
    }

    d_name = full_name;

    return d_name;
}

temporary_dir::~temporary_dir()
{
    if (d_name.empty())
        return;

    try
    {
        subprocess::run("rm -r " + d_name);
    }
    catch(subprocess::error &)
    {
        cerr << "Warning: Failed to remove temporary directory: " << d_name << endl;
    }
}

}
}
