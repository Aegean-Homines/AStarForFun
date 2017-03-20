#pragma once

#include <vector>
#include <list>
#include <iostream>
#include <limits>

typedef std::vector<std::vector<char>> MapChar;
typedef std::vector<std::vector<unsigned>>  MapJSON;
typedef std::pair<unsigned short, unsigned short> Position;

enum NodeState {
	UNDISCOVERED,
	OPEN_LIST,
	CLOSED_LIST
};

struct Node{
	Position pos;
	float givenCost = std::numeric_limits<float>::max();
	float totalCost = std::numeric_limits<float>::max(); // for ordering
	Node * parent = nullptr;
	NodeState state = UNDISCOVERED;
	std::vector<Node *> neighbors;

	Node(Position const & newPos) : 
	pos(newPos){}

	bool operator== (Node const * rhs) const {
		return (rhs->pos.first == pos.first && rhs->pos.second == pos.second);
	}

	bool operator<(Node const * rhs) const{
		return (totalCost < rhs->totalCost);
	}
};

// 2D array of nodes
typedef std::vector<std::vector<Node *>> NodeMap;

class AStarSolver
{
private:
	unsigned width;
	unsigned height;
	MapJSON mapJson;
	MapChar mapChar;
	NodeMap nodeMap;

	// Helper functions
	void GenerateNodeMap();
	void ClearMap();
	void AddNeighbors( Node* node, short deltaXMin, short deltaYMin, short deltaXMax, short deltaYMax );

	float CalculateHeuristic(Node* source, Node* target) const;
	float CalculateDistance(Node* source, Node* target) const;
public:
	AStarSolver( std::string const & mapFileName);
	~AStarSolver();

	bool FindPath( Position start, Position target, std::list<Position>& path );
	template<typename TMap, typename TMapDef>
	void PrintMap(TMap const & map, TMapDef const & definiton);
};

template<typename TMap, typename TMapDef>
inline void AStarSolver::PrintMap(TMap const & map, TMapDef const & definiton)
{
	std::cout << "MapData for " << definiton << ": " << std::endl;
	for (unsigned i = 0; i < map.size(); ++i) {
		for (unsigned j = 0; j < map.size(); ++j) {
			std::cout << map[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "***********************************" << std::endl;
}
