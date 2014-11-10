#include "common.h"
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <sstream>
#include <string>

std::string tobase64(unsigned char *data, int dataSize)
{
    using namespace boost::archive::iterators;

    std::stringstream os;
    std::copy(
        base64_from_binary<transform_width<unsigned char*, 6, 8>>(data),
        base64_from_binary<transform_width<unsigned char*, 6, 8>>(data + dataSize),
        ostream_iterator<char>(os)
    );
    return os.str();
}
