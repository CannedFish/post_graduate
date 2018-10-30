#ifndef MAPITERATOR_H
#define MAPITERATOR_H

#include "common.h"

template<typename J, typename Strategy, typename Map>
class MapIterator : public std::iterator<std::forward_iterator_tag, J>
{
public:
    MapIterator(const MapIterator &other) {
        junction = other.junction;
        endJunc = other.endJunc;
        strategyObj = other.strategyObj;
    }
    MapIterator(Map *map, J *junc, J *end = NULL)
        : junction(junc), endJunc(end), strategyObj(new Strategy(map, junc, end)) {}
    ~MapIterator() {
        junction = NULL;
        endJunc = NULL;
        delete strategyObj;
        strategyObj = NULL;
    }

    MapIterator & operator =(const MapIterator &other);
    MapIterator & operator ++();
    MapIterator operator ++(int);
    bool operator ==(const MapIterator &other);
    bool operator !=(const MapIterator &other);
    J & operator *();
    J * operator ->();
    bool isDone() {return (junction == endJunc || junction == NULL);}

    void getRoute(std::list<std::string> &ret) {
        if(junction == endJunc) {
            strategyObj->getRoute(ret);
        } else {
            printf("Fail to get a route!!\n");
            ret.clear();
        }
    }

private:
    MapIterator() : junction(NULL), strategyObj(NULL) {}

    J *junction, *endJunc;
    Strategy * strategyObj;
};

template<typename J, typename Strategy, typename Map> inline
MapIterator<J, Strategy, Map> & MapIterator<J, Strategy, Map>::operator =(const MapIterator<J, Strategy, Map> &other) {
    if(this == &other) return *this;
    junction = other.junction;
    endJunc = other.endJunc;
    strategyObj = other.strategyObj;
    return *this;
}

template<typename J, typename Strategy, typename Map> inline
MapIterator<J, Strategy, Map> & MapIterator<J, Strategy, Map>::operator ++() {
    //printf("Get next junction\n");
    junction = (*strategyObj)();
    return *this;
}

template<typename J, typename Strategy, typename Map> inline
MapIterator<J, Strategy, Map> MapIterator<J, Strategy, Map>::operator ++(int x) {
    MapIterator<J, Strategy, Map> tmp(*this);
    operator ++();
    return tmp;
}

template<typename J, typename Strategy, typename Map> inline
bool MapIterator<J, Strategy, Map>::operator ==(const MapIterator<J, Strategy, Map> &other) {
    return junction == other.junction;
}

template<typename J, typename Strategy, typename Map> inline
bool MapIterator<J, Strategy, Map>::operator !=(const MapIterator<J, Strategy, Map> &other) {
    return junction != other.junction;
}

template<typename J, typename Strategy, typename Map> inline
J & MapIterator<J, Strategy, Map>::operator *() {
    return *junction;
}

template<typename J, typename Strategy, typename Map> inline
J * MapIterator<J, Strategy, Map>::operator ->() {
    return junction;
}

#endif // MAPITERATOR_H
