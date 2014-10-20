
//#include "boost/date_time/posix_time/posix_time.hpp"

#ifndef COMMON_H
#define COMMON_H

/* alias moment
    Author: Declan White
    Description: TODO
    Changelog:
        [2014-09-04 DWW] Created.
*/
using byte = uint8_t;

/* alias moment
    Author: Declan White
    Description: TODO
    Changelog:
        [2014-09-02 DWW] Created.
*/
using moment = boost::posix_time::ptime;

/* alias duration
    Author: Declan White
    Description: TODO
    Changelog:
        [2014-09-02 DWW] Created.
*/
using duration = boost::posix_time::time_duration;


#endif//COMMON_H
