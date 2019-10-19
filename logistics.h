#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <limits>
#include <set>
#include <stack>

using namespace std;

struct Edge {
    int id;
    string endCity;
    int capacity;
    int current;
};

//this script receives two vectors: start cities and end cities
//for this version we will assume there is only one start and end city,
//represented by the string of index 0 for each vector (ex: start.at(0).first)
unordered_map<int, double> organizeLogistics(vector<pair<string,double>> start, vector <pair<string,double>> end){
    ifstream inFile;
    inFile.open("worldmap.txt");
    int id, capacity, cost;
    string startCity, endCity;
    unordered_map<int, double> answer;
    unordered_map<string, vector<Edge>> adjacencyList;
    //min heap idea from https://www.geeksforgeeks.org/implement-min-heap-using-stl/
    priority_queue <int, vector<int>, greater<int> > flows; 
    set<int> flowIDs;
    //represents the strings whose neighbors are at full capacity
    set<string> fullCapacity;
    
    int count=0;
    bool atStartCity = false;

    while(inFile >> id >> startCity >> endCity >> capacity >> cost){
        
        Edge neighbor = {.id=id, .endCity = endCity, .capacity=capacity, .current=0};
        
        //prevent edges that loop back to the start
        if(endCity!=start.at(0).first){
            //if this startCity is already in the map, add it to the vector of possible neighbors
            //otherwise, make a new key-value pair
            unordered_map<string, vector<Edge>>::iterator it = adjacencyList.find(startCity);
            if(it!=adjacencyList.end()){
                it->second.push_back(neighbor);
            }
            else {
                vector<Edge> vectorOfNeighbors;
                vectorOfNeighbors.push_back(neighbor);
                adjacencyList.insert({startCity, vectorOfNeighbors});
            }
        }
    }
    //now the adjancey list is ready and we can start looking for maximum flow
    
    int current = 0;
    bool theEnd = false;
    
    //access the vector associated with 
    //the starting string we got from the arguments
    vector<Edge> neighbors = adjacencyList[start.at(0).first];
    string currentCity = start.at(0).first;
    vector<Edge> startNeighbors = neighbors;
    string finalCity = end.at(0).first;

    int maxFlowLeft, flowLeft, currentID, flowToAdd;
    int validNeighbors = 0;
    unordered_map<int, double>::iterator idIt;
    bool roomForFlow = false, addingFlow;
    
    int fullEdgeCount = 0;
    bool neighborInAnswer = false;
    bool roomFromStart = true;
    int fullCount;

    //stop the loop when end city is at max flow capacity
    while(roomFromStart){
        
        flows = priority_queue <int, vector<int>, greater<int> >();
        flowIDs.clear();
        
        addingFlow = true;
        
        //stop looking for flow when you can't move anymore
        //(if there are no neighbors or if all neighbors are full)
        //then go back to start city
        if(neighbors.size()==0){
            fullCapacity.insert(currentCity);
            currentCity = start.at(0).first;
            neighbors = adjacencyList[currentCity];
        }
        
        //startCity's full if all her endCities are also full
        fullCount=0;
        for(int i=0; i<neighbors.size(); i++){
            if(fullCapacity.count(neighbors.at(i).endCity))
                fullCount++;
        }
        if(fullCount==neighbors.size()){
            fullCapacity.insert(currentCity);
            currentCity = start.at(0).first;
            neighbors = adjacencyList[currentCity];
        }
        
        
        //if fullEdges.count(currentCity), all the cities neighbors are at full capacity,
        //so we don't wanna look there
        while(neighbors.size()>0 && currentCity!=finalCity && 
            !fullCapacity.count(currentCity) && count<20 && addingFlow){
            count++;
        
            //assume the greatest flow possible is negative,
            //forcing the program to select a larger flow
            //if it has valid neighbors
            maxFlowLeft=-1;

            //find the maximum flow extending from the current city
            //this will optimize the algorithm's runtime
            //if(!fullCapacity.count(currentCity))
            validNeighbors = 0;
            for(int i=0; i<neighbors.size(); i++) {
                
                
                current = 0;
                idIt = answer.find(neighbors.at(i).id);
                if(idIt!=answer.end()){
                    neighborInAnswer = true;
                    current = answer[neighbors.at(i).id];
                }
                
                flowLeft = neighbors.at(i).capacity-current;
                roomForFlow = (!neighborInAnswer || 
                    answer[neighbors.at(i).id]< neighbors.at(i).capacity);
            
                //prevent a loop by seeing if this ID has already been
                //checked
                if(flowIDs.count(neighbors.at(i).id)){
                    fullCapacity.insert(neighbors.at(i).endCity);
                    
                }
                
                //don't consider an edge if it's at full capacity
                else if (!fullCapacity.count(neighbors.at(i).endCity) && roomForFlow){
                    
                    validNeighbors++;
                    //if it's the final node we definitly want it!
                    if(neighbors.at(i).endCity==finalCity){
                        currentID = neighbors.at(i).id;
                        maxFlowLeft = flowLeft;
                        i = neighbors.size();
                        currentCity = finalCity;
                    }
                    //if it's big it's also ok
                    else if(flowLeft>maxFlowLeft){
                        maxFlowLeft = flowLeft;
                        currentCity = neighbors.at(i).endCity;
                        currentID = neighbors.at(i).id;
                        
                    }
                }
            }//max flow found
            

            //if all neighbors were at full capacity
            //maxFlowLeft would be -1, so go back to the start
            if(maxFlowLeft==-1){
                fullCapacity.insert(currentCity);
                flows=priority_queue<int, vector<int>, greater<int>>();
                flowIDs.clear();
                currentCity = start.at(0).first;
                neighbors = adjacencyList[currentCity];
                addingFlow = false;
            }
            //otherwise, fill in all the flow data 
            else {
                //save the current and ID of this edge 
                flowIDs.insert(currentID);
                flows.push(maxFlowLeft);
                neighbors = adjacencyList[currentCity];
            }
        }//go back to looking for flow, if you can
        
        if(addingFlow && currentCity==finalCity){
            //access the smallest current
            flowToAdd = flows.top();
            //update in final answer AND in original map
            set<int>::iterator it = flowIDs.begin();
            for(auto it=flowIDs.begin(); it!=flowIDs.end(); it++){
                answer[*it] += flowToAdd;
            }
            flows = priority_queue <int, vector<int>, greater<int>>();
            flowIDs.clear();
            
            //keep looking! from start this time
            currentCity = start.at(0).first;
            neighbors = adjacencyList[currentCity];
        }
        
        //extra check
        if(currentCity==finalCity){
            currentCity = start.at(0).first;
            neighbors = adjacencyList[currentCity];
        }
            
        //check for full capacity
        fullCount = 0;
        for(int i=0; i<startNeighbors.size(); i++){
            if(fullCapacity.count(startNeighbors.at(i).endCity)){
                fullCount++;
            }
        }
        if(fullCount==startNeighbors.size())
            roomFromStart = false;
        if(fullCapacity.count(start.at(0).first)){
            roomFromStart = false;
        }
    }
    
    return answer;
}
