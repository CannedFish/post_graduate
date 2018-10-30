#ifndef EVALUATIONFUNCTIONS_H
#define EVALUATIONFUNCTIONS_H

#include "common.h"
#include "basicmapdatatype.h"

namespace EvFunc {
    class GFunc {
    public:
        virtual ~GFunc() {}
        virtual double operator ()(double origin, const BASICTYPE::RoadInfo &info) = 0;
    };

    class SimpleDistanceG : public GFunc {
    public:
        double operator ()(double origin, const BASICTYPE::RoadInfo &info) {
            //printf("Distance Shortest\n");
            return origin + info.length;
        }
    };

    class TimeShortestG : public GFunc {
    public:
        double operator ()(double origin, const BASICTYPE::RoadInfo &info) {
            //printf("Time Shortest\n");
            return origin + info.length / info.avgSpeed;
        }
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template<typename J>
    class SimpleDistanceEv {
    private:
        const static double CO_DISTANCE = 0.8;
    public:
        double operator ()(J* from, J *target, const BASICTYPE::RoadInfo &info) {
            BASICTYPE::Point tmpA, tmpB;
            tmpA = from->getCoor();
            tmpB = target->getCoor();
            /*Debug Info
            printf("%s: (%s, %s), (%s, %s)\n\t%lf\t%lf\n", __func__
                   , tmpA.first.c_str(), tmpA.second.c_str()
                   , tmpB.first.c_str(), tmpB.second.c_str()
                   , fabs(atof(const_cast<const char *>(tmpA.first.c_str()))
                          - atof(const_cast<const char *>(tmpB.first.c_str())))
                   , fabs(atof(const_cast<const char *>(tmpA.second.c_str()))
                          - atof(const_cast<const char *>(tmpB.second.c_str()))));
            */
            //return the Manhattan Distance
            return (fabs(atof(const_cast<const char *>(tmpA.first.c_str()))
                         - atof(const_cast<const char *>(tmpB.first.c_str())))
                    + fabs(atof(const_cast<const char *>(tmpA.second.c_str()))
                           - atof(const_cast<const char *>(tmpB.second.c_str()))))
                    * CO_DISTANCE;
        }
    };

    template<typename J>
    class TimeEv {
    private:
        const static double CO_SPEED = 1.5;
    public:
        double operator ()(J* from, J *target, const BASICTYPE::RoadInfo &info) {
            BASICTYPE::Point tmpA, tmpB;
            tmpA = from->getCoor();
            tmpB = target->getCoor();
            //return the Manhattan Distance
            return ((fabs(atof(const_cast<const char *>(tmpA.first.c_str()))
                         - atof(const_cast<const char *>(tmpB.first.c_str())))
                    + fabs(atof(const_cast<const char *>(tmpA.second.c_str()))
                           - atof(const_cast<const char *>(tmpB.second.c_str()))))
                    / (info.avgSpeed * CO_SPEED));
        }
    };
}

#endif // EVALUATIONFUNCTIONS_H
