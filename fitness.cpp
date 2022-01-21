// blockudoku-solver.cpp : Defines the entry point for the application.
//

#include "engine.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <atomic>
#include <thread>
#include <numeric>
#include <cstdlib>

int getNumTurnsSample(EvalWeights weights) {
	auto game = GameState(BitBoard::empty());
	int score = 0;

	while(true) {
		score += 1;
		const auto piece_set = PieceSet::getRandom();
		const auto next = AI::makeMoveSimple(weights, game, piece_set);
		if (next.isOver()){
			break;
		}
		game = next;
	}
	return score;
}

// Call this to test changes to the evaluation function.
double simpleEvalFitnessTest(EvalWeights weights, int numGames) {
	std::atomic<int> games_done(0);
	std::vector<double> scores;
	std::vector<std::thread> workers;
    for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
        workers.push_back(std::thread([&]()
        {
			while (games_done++ < numGames){
				const auto score = getNumTurnsSample(weights);
				scores.push_back(score);
				std::cout << scores.size() << '/' << numGames << ' ' << score << std::endl;
			}
        }));
    }
	std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
        t.join();
    });

	std::sort(scores.begin(), scores.end());
	double result = scores[numGames / 2];
	std::cout << "p50: " << result << std::endl;
	std::cout << "avg: " << (std::accumulate(scores.begin(), scores.end(), 0.0) / numGames) << std::endl;

	return result;
}

int main(int argc, char *argv[]) {
	srand((unsigned)time(NULL));
    const int num_iterations = std::atoi(argv[1]);
    simpleEvalFitnessTest(EvalWeights::getDefault(), num_iterations);

	return 0;
}
