#include <algorithm>
#include <iostream>

#include "opentimelineio/serialization.h"
#include "opentimelineio/deserialization.h"
#include "opentimelineio/errorStatus.h"

int
main(int argc, char* argv[])
{
    const std::vector<std::string> arg_vec(argv+1, argv+argc);

    if (
            std::find(arg_vec.begin(), arg_vec.end(), "-h") != arg_vec.end()
            or std::find(arg_vec.begin(), arg_vec.end(), "--help") != arg_vec.end())
    {
        std::cerr << "usage: " + std::string(argv[0]) + " [-h] filepath" << std::endl;
        std::cerr <<std::endl;
        std::cerr << "Print the contents of the OpenTimelineIO file to stdout." << std::endl;

        std::cerr << std::endl;
        return (0);
    }

    const int num_args = arg_vec.size();
    if (num_args != 1)
    {
        std::cerr << "usage: " + std::string(argv[0]) + " [-h] filepath" << std::endl;
        std::cerr << std::string(argv[0]) + ": error: invalid number of arguments" << std::endl;

        // what python's argparse seems to return in this case
        return (2);
    }

    
    linb::any resulting_timeline;
    opentimelineio::v1_0::ErrorStatus err;
    if (opentimelineio::v1_0::deserialize_json_from_file(arg_vec[0], &resulting_timeline, &err))
    {
        std::cerr << "Something went wrong deserializing '" << arg_vec[0] << "'" << std::endl;
        std::cerr << err.details << std::endl;
        std::cerr << err.full_description << std::endl;

        return (1);
    }

    opentimelineio::v1_0::serialize_json_to_file(resulting_timeline, "/dev/stdout", &err);
}
