//
//  Graph.hpp
//  Koumachika
//
//  Created by Toni on 5/30/16.
//
//

#ifndef Graph_hpp
#define Graph_hpp

namespace graph{

typedef pair<int,int> position;

struct PositionHash{
    size_t operator()(const position& p) const{
        return hash<int>()(p.first) ^ hash<int>()(p.second);
    }
};

struct PositionEqual{
    bool operator()(const position& l, const position& r) const{
        return l.first == r.first && l.second == r.second;
    }
};

struct node
{
    position p;
    position visitedFrom;
    
    float costFromStart;
    float remainingHeuristic;
    
    inline node(position p, position from, float costFromStart, float remainingHeuristic) : p(p), visitedFrom(from),
        costFromStart(costFromStart),
        remainingHeuristic(remainingHeuristic)
        {}
};

vector<position> gridAStar(const boost::dynamic_bitset<>& obstacleMask, position start, position end, IntVec2 dimensions);

}

#endif /* Graph_hpp */
