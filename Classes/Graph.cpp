//
//  Graph.cpp
//  Koumachika
//
//  Created by Toni on 5/30/16.
//
//

#include "Prefix.h"

#include "Graph.hpp"

namespace graph{

const bool debug = false;

//The heuristic is simply the Euclidean distance to the end point.
float heuristicDistance(position a, position b)
{
    float x = abs(a.first - b.first);
    float y = abs(a.second - b.second);
    
    return sqrt(x*x + y*y);
}

vector<position> gridAStar(const boost::dynamic_bitset<>& obstacleMask, position start, position end, IntVec2 dimensions)
{
    if(obstacleMask.size() != dimensions.first*dimensions.second){
        throw "gridSearch: obstacleMask does not match given dimensions!";
    }

    //Best cost for all positions that have been visited
    unordered_map<position, float, PositionHash, PositionEqual> bestCostVisited;
    unordered_map<position, position, PositionHash, PositionEqual> visitedFrom;
    
    //Reversed to make it a min queue.
    //The comparison is defined here as a lambda so it can capture the end point.
    //Cost f() = g() + h()
    //f - node cost
    //g - exact cost from start
    //h - heuristic cost to goal
    auto cmp = [=](node a, node b) -> bool{
        float f1 = a.costFromStart + a.remainingHeuristic;
        float f2 = b.costFromStart + b.remainingHeuristic;
//        
        return f1 > f2;

//        return a.costFromStart > b.costFromStart;
    };

    priority_queue<node, vector<node>, decltype(cmp)> frontier(cmp);

    //Use bounds checking to avoid leaving the map.
    auto checkVisitAdjacent = [&](node& current, int dx, int dy)-> void{
        bool outOfBounds = false;
    
        if(dx < 0 && current.p.first <= 0) outOfBounds = true;
        if(dx > 0 && current.p.first >= dimensions.first-1) outOfBounds = true;
        
        if(dy < 0 && current.p.second <= 0) outOfBounds = true;
        if(dy > 0 && current.p.second >= dimensions.second-1) outOfBounds = true;
        
        position adj(current.p.first + dx, current.p.second + dy);
        int rawIndex = adj.second*dimensions.first + adj.first;
        
        if(outOfBounds){
    
            if(debug)
                log2(
                    "checkVisitAdjacent skipped %d,%d for out of bounds.",
                    adj.first,
                    adj.second
                );
        
            return;
        }
        
        if(obstacleMask[rawIndex]){
        
            if(debug)
                log2(
                    "checkVisitAdjacent skipped %d,%d for obstacle.",
                    adj.first,
                    adj.second
                );

        
            return;
        }
        
        //If dialogal movement, check for collision with tiles in both primary directions.
        if(dx != 0 && dy != 0)
        {
            int hIndex = current.p.second*dimensions.first + (current.p.first + dx);
            int vIndex = (current.p.second+dy)*dimensions.first + current.p.first;
            
            if(obstacleMask[hIndex] || obstacleMask[vIndex]){
                
                if(debug)
                    log2(
                        "checkVisitAdjacent skipped %d,%d for indirect obstacle.",
                        adj.first,
                        adj.second
                    );
                
                return;
            }
        }
        
        float actualCostFromStart = (dx != 0 && dy != 0 ? boost::math::constants::root_two<float>() : 1.0) + current.costFromStart;
        
        float heuristicCost = heuristicDistance(adj, end);
        
        node adjNode(adj,current.p, actualCostFromStart, heuristicCost);
        
        if(debug)
            log4(
                "Adjacent node %d,%d: f: %f, g: %f added.",
                adjNode.p.first,
                adjNode.p.second,
                adjNode.costFromStart,
                adjNode.remainingHeuristic
            );
        
        frontier.push(adjNode);
    };
    
    auto buildPath =[&]() -> vector<position> {
        vector<position> path;
        
        position current = end;
        
        while(current != start){
            path.push_back(current);
            current = visitedFrom[current];
        }
        
        reverse(path.begin(), path.end());

		if (debug) {
			log0("Generating final path:");
			for(auto p: path) {
				log2("%d,%d", p.first, p.second);
			}
		}
    
        return path;
    };
    
    auto visitAdjacents = [&](node& current)-> void{
        checkVisitAdjacent(current, -1, -1);
        checkVisitAdjacent(current, 0, -1);
        checkVisitAdjacent(current, 1, -1);
        
        checkVisitAdjacent(current, -1, 0);
        checkVisitAdjacent(current, 1, 0);
        
        checkVisitAdjacent(current, -1, 1);
        checkVisitAdjacent(current, 0, 1);
        checkVisitAdjacent(current, 1, 1);
    };
    
    frontier.push(node(start, start, 0.0, heuristicDistance(start, end)));
    
    while(!frontier.empty())
    {
        node current = frontier.top();
        frontier.pop();
             
        if(debug)
            log4("popped node %d,%d f: %f, g: %f",
                current.p.first,
                current.p.second,
                current.costFromStart,
                current.remainingHeuristic
            );
        
        if(current.p == end){
            //Solved.
            bestCostVisited[current.p] = current.costFromStart;
            visitedFrom[current.p] = current.visitedFrom;
        
            if(debug)
                log0("solved");
            
            return buildPath();
        }
        
        //First or closer-to-optimal visit
        if(bestCostVisited.find(current.p) == bestCostVisited.end() ||
           bestCostVisited[current.p] > current.costFromStart
        ){
            bestCostVisited[current.p] = current.costFromStart;
            visitedFrom[current.p] = current.visitedFrom;

            if(debug)
                log1("Cost set at %f.", current.costFromStart);

            visitAdjacents(current);
        }

        
    }
    //If we exit the loop,
    //All nodes have been traversed and the end is unreachable.
    return vector<position>();
}

}
