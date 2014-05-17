
#include <cstdlib>

inline int irand(int min, int max)
{
    return min+((int)((double)(max-min)*((double)rand()/(double)RAND_MAX)));
}

inline double drand(double min, double max)
{
    return min+(max-min)*((double)rand()/(double)RAND_MAX);
}

template<typename T>
inline T clamp(T value, T min, T max)
{
    return (
            value < min ? min
        :   value > max ? max
        :   value
    );
}
