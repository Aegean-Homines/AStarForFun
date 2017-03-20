#include "AStarSolver.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <algorithm>

#define SQRTOF2 1.41421356237f

using namespace rapidjson;

// For comparison for priority queue insertion
// failed, this is for the optimal implementation though
class Comparator {
	bool operator()(Node const * lhs, Node const * rhs) {
		return lhs > rhs;
	}
};

void AStarSolver::GenerateNodeMap()
{
	// I'm sure there is a better way of doing this, I always suck at grid reading :(
	// So if you refactor this, please let me know of the better way

	// Inner "safe" area
	for (unsigned i = 1; i < height - 1; ++i) {
		for (unsigned j = 1; j < width - 1; ++j) {
			AddNeighbors(nodeMap[i][j], -1, -1, 1, 1);
		}
	}

	// Borders
	for (unsigned i = 1; i < height - 1; ++i) {
		AddNeighbors(nodeMap[i][0], -1, 0, 1, 1); // Left
		AddNeighbors(nodeMap[i][width - 1], -1, -1, 1, 0); // Right
	}

	for (unsigned j = 1; j < width - 1; ++j) {
		AddNeighbors(nodeMap[0][j], 0, -1, 1, 1); // Left
		AddNeighbors(nodeMap[height - 1][j], -1, -1, 0, 1); // Right
	}

	// Corners
	AddNeighbors(nodeMap[0][0], 0, 0, 1, 1); // Top left
	AddNeighbors(nodeMap[0][width - 1], 0, -1, 1, 0); // Top right
	AddNeighbors(nodeMap[height - 1][0], -1, 0, 0, 1); // Bottom left
	AddNeighbors(nodeMap[height - 1][width - 1], -1, -1, 0, 0); //Bottom right
}

void AStarSolver::ClearNodeMap ( ) {
	// Basically re-initialize the entire map but on allocated data
	for (unsigned i = 0; i < height; ++i) {
		for (unsigned j = 0; j < width; ++j) {
			nodeMap[i][j]->state = UNDISCOVERED;
			nodeMap[i][j]->givenCost = std::numeric_limits<float>::max();
			nodeMap[i][j]->totalCost = std::numeric_limits<float>::max();
			nodeMap[i][j]->parent = nullptr;

		}
	}
}

void AStarSolver::AddNeighbors ( Node* node, short deltaXMin, short deltaYMin, short deltaXMax, short deltaYMax ) {
	unsigned x = node->pos.first;
	unsigned y = node->pos.second;

	if(mapJson[x][y] != 0) //This is not an empty tile
		return;
	

	// add neighbors
	for (short deltaRow = deltaXMin; deltaRow <= deltaXMax; ++deltaRow) {
		short neighborRow = x + deltaRow;
		for (short deltaColumn = deltaYMin; deltaColumn <= deltaYMax; ++deltaColumn) {
			short neighborColumn = y + deltaColumn;

			// remove self
			if (neighborRow == x && neighborColumn == y)
				continue;

			// neighbor not empty tile
			if(mapJson[neighborRow][neighborColumn] != 0)
				continue;

			nodeMap[x][y]->neighbors.push_back(nodeMap[x + deltaRow][y + deltaColumn]);
		}
	}
}

float AStarSolver::CalculateHeuristic(Node * source, Node * target) const {
	unsigned diffX = abs(source->pos.first - target->pos.first);
	unsigned diffY = abs(source->pos.second - target->pos.second);

	return 1.0f * (diffX + diffY) + (SQRTOF2 - 2 * 1.0f) * std::min(diffX, diffY);
}

float AStarSolver::CalculateDistance ( Node* source, Node* target ) const {
	// Square of distance should also work. If not, then add sqrt to it. 
	// No sqrt = faster <3
	return pow(target->pos.second - source->pos.second, 2) + pow(target->pos.first - source->pos.first, 2);
}


void InsertToOpenList(Node * node, std::list<Node*>& openList)
{
	node->state = OPEN_LIST;
	openList.push_back(node);
}

AStarSolver::AStarSolver ( std::string const& mapFileName ) {
	std::stringstream content;
	std::ifstream file(mapFileName);

	content << file.rdbuf();
	std::string jsonInText = content.str();

	Document document;
	document.Parse(jsonInText.c_str());

	// Get width - height from the doc
	width = document["width"].GetUint();
	height = document["height"].GetUint();

	// Get the 2D array from the doc
	const Value& mapData = document["tileData"];
	
	// Init rows
	mapJson.resize(mapData.Size());
	mapChar.resize(mapData.Size());
	nodeMap.resize(mapData.Size());
	for (SizeType i = 0; i < height; ++i) {
		for (unsigned j = 0; j < width; ++j) {
			// Tile data
			unsigned tileVal = mapData[i][j].GetUint();

			// Push it to JSON map
			mapJson[i].push_back(tileVal);

			// Push char equivalent to char map
			if (0 == tileVal)
				mapChar[i].push_back('e');
			else
				mapChar[i].push_back('w');

			// Finally create the node data
			nodeMap[i].push_back(new Node(std::make_pair(i, j)));
		}
	}

	PrintMap(mapJson, "Map data stored in JSON");
	PrintMap(mapChar, "Char data parsed from JSON");


	// Generating nodeMap data
	GenerateNodeMap();
}

AStarSolver::~AStarSolver()
{
	for (unsigned i = 0; i < nodeMap.size(); ++i) {
		for (unsigned j = 0; j < nodeMap.size(); ++j) {
			delete nodeMap[i][j];
		}
	}
}

bool AStarSolver::FindPath( Position start, Position target, std::list<Position>& path )
{
	Node* startingNode = nodeMap[start.first][start.second];
	Node* targetNode = nodeMap[target.first][target.second];

	if (mapJson[target.first][target.second] != 0) //target isn't an empty tile, no path finding
		return false;

	ClearNodeMap();

	std::list<Node *> closedList;
	std::list<Node *> openList;

	startingNode->givenCost = 0;
	startingNode->totalCost = CalculateHeuristic(startingNode, targetNode);
	InsertToOpenList(startingNode, openList);

	while(!openList.empty()) {
		Node* currentNode = openList.front();
		openList.pop_front();

		if(currentNode == targetNode) {
			while(currentNode->parent) {
				path.push_front(currentNode->pos);
				currentNode = currentNode->parent;
			}
			path.push_front(start);

#if _DEBUG
			MapChar finalMap = mapChar;
			for(auto & pos : path) {
				finalMap[pos.first][pos.second] = '*';
			}

			PrintMap(finalMap, "PathMap");
#endif

			return true;


		}

		for(Node* neighbor : currentNode->neighbors) {
			if (neighbor->state == CLOSED_LIST) // If we're already finished it
				continue;

			float actualCost = currentNode->givenCost + CalculateDistance(currentNode, neighbor);


			if(neighbor->state == UNDISCOVERED) { //new node
				InsertToOpenList(neighbor, openList);
			}
			else if (actualCost >= neighbor->givenCost) {
				continue;
			}

			// we already discovered this node but this path is better than the previous one
			neighbor->parent = currentNode;
			neighbor->givenCost = actualCost;
			neighbor->totalCost = actualCost + CalculateHeuristic(neighbor, targetNode);
		}
		currentNode->state = CLOSED_LIST;
	}

	return false;
}



