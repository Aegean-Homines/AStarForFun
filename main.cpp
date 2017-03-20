#include "AStarSolver.h"

#define FILENAME "Map.json"

void main(){
	AStarSolver solver(FILENAME);

	int x1, y1, x2, y2;
	while(true) {

		std::cout << "Enter start x y and target x y" << std::endl;
		std::cin >> x1;
		std::cin >> y1;
		std::cin >> x2;
		std::cin >> y2;

		std::list<Position> path;
		if (solver.FindPath(Position(x1, y1), Position(x2, y2), path)) {
			std::cout << "Path found" << std::endl;
		}
		else {
			std::cout << "No path :(" << std::endl;
		}
	}
	std::list<Position> path;
	if(solver.FindPath(Position(0, 0), Position(8, 3), path)) {
		std::cout << "Path found" << std::endl;
	}else {
		std::cout << "No path :(" << std::endl;
	}
}