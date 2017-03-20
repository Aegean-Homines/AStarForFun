#include "AStarSolver.h"

#define FILENAME "Map.json"

void main(){
	AStarSolver solver(FILENAME);

	unsigned x, y, x2, y2;

	while(std::cin >> x, std::cin >> y, std::cin >> x2, std::cin >> y2) {
		std::list<Position> path;
		if (solver.FindPath(Position(x, y), Position(x2, y2), path)) {
			std::cout << "Path found" << std::endl;
		}
		else {
			std::cout << "No path :(" << std::endl;
		}
	}

}