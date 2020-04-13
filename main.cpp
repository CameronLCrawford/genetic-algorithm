#include <SFML/Graphics.hpp>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>
#include "Agent.h"

/*
Used in the std::sort to sort the agents in reverse order (hence the "backwards"
inequality sign)
*/
bool compareFitness(std::shared_ptr<Agent> agent1, std::shared_ptr<Agent> agent2)
{
	return agent1->getFitness() > agent2->getFitness();
}

/*
This will perform a genetic "crossover" on the weights of two parent agents.
For each layer in the parents' networks, a crossover point is randomly chosen
and the new weights consist of the weights from parent 1 before the crossover
point and the weights from parent 2 after the crossover point
*/
std::vector<Matrix> crossParents(std::shared_ptr<Agent> parent1, std::shared_ptr<Agent> parent2)
{
	std::vector <Matrix> newNetwork;
	std::vector<Matrix> network1 = parent1->getNetwork();
	std::vector<Matrix> network2 = parent2->getNetwork();
	for (int currentLayer = 0; currentLayer < network1.size(); currentLayer++)
	{
		std::vector<float> newLayer;
		Dimensions dimensions = network1[currentLayer].getDimensions();
		int rows = dimensions.rows;
		int columns = dimensions.columns;
		int numberWeights = rows * columns;
		int crossoverPoint = rand() % numberWeights;
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < columns; column++)
			{
				if (row * columns + column < crossoverPoint)
				{
					newLayer.push_back(network1[currentLayer](row, column));
				}
				else
				{
					newLayer.push_back(network2[currentLayer](row, column));
				}
			}
		}
		newNetwork.push_back(Matrix(rows, columns, newLayer));
	}
	return newNetwork;
}

/*
This implements a metric for measuring the genetic diversity between two agents
which take the sum of differences for each weight
*/
double geneticDiversity(std::shared_ptr<Agent> agent1, std::shared_ptr<Agent> agent2)
{
	double diversity = 0.0;
	std::vector<Matrix> network1 = agent1->getNetwork();
	std::vector<Matrix> network2 = agent2->getNetwork();
	for (int currentLayer = 0; currentLayer < network1.size(); currentLayer++)
	{
		Dimensions dimensions = network1[currentLayer].getDimensions();
		int rows = dimensions.rows;
		int columns = dimensions.columns;
		int numberWeights = rows * columns;
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < columns; column++)
			{
				float weightDifference = network1[currentLayer](row, column) - network2[currentLayer](row, column);
				diversity += abs(weightDifference);
			}
		}
	}
	return diversity;
}

int main()
{
	/*
	Some seeds lead to agents creating a very strong solution in the first few generations
	but some lead to the agents taking a few more generations. For testing, I had to vary
	the seed to find the "worst" intial population and then evaluate how the algorithm performed
	*/
	srand(0);
	/*
	The first portion of the program is reading in from the map file
	*/
	std::vector<sf::VertexArray> mapElements;
	sf::VertexArray checkPoints(sf::Lines);
	sf::Vector2f startingPosition;
	int startingAngle;
	int numberAgents;
	std::ifstream mapFile("map.txt");
	mapFile >> startingPosition.x >> startingPosition.y >> startingAngle >> numberAgents;
	int numberMapElements;
	mapFile >> numberMapElements;
	for (int currentMapElement = 0; currentMapElement < numberMapElements; currentMapElement++)
	{
		sf::VertexArray newMapElement(sf::LineStrip);
		int numberVertices;
		mapFile >> numberVertices;
		for (int currentVertex = 0; currentVertex < numberVertices; currentVertex++)
		{
			sf::Vertex newVertex;
			sf::Vector2f coordinates;
			newVertex.color = sf::Color::Black;
			mapFile >> coordinates.x;
			mapFile >> coordinates.y;
			newVertex.position = coordinates;
			newMapElement.append(newVertex);
		}
		mapElements.push_back(newMapElement);
	}
	int numberCheckPoints;
	mapFile >> numberCheckPoints;
	for (int i = 0; i < numberCheckPoints; i++)
	{
		sf::Vertex newVertex;
		sf::Vector2f coordinates;
		newVertex.color = sf::Color::Red;
		mapFile >> coordinates.x;
		mapFile >> coordinates.y;
		newVertex.position = coordinates;
		checkPoints.append(newVertex);
	}

	/*
	The next stage is the initialisation of variables used in the program
	*/
	sf::RenderWindow window(sf::VideoMode(1000, 600), "Genetic Algorithm");
	int numberFailed = 0;
	sf::Event event;
	int currentGeneration = 1;
	window.setFramerateLimit(200);
	std::vector<std::shared_ptr<Agent>> agents;
	const char numberLayers = 3;
	char networkArchitecture[numberLayers] = { 3,5,2 };

	/*
	Initialising the network for each agent
	*/
	for (int i = 0; i < numberAgents; i++)
	{
		std::vector<Matrix> weights;
		for (int currentWeights = 0; currentWeights < numberLayers - 1; currentWeights++)
		{
			std::vector<float> weightData = {};
			char rows = networkArchitecture[currentWeights];
			char columns = networkArchitecture[currentWeights + 1];
			int currentWeightsSize = rows * columns;
			for (int randomWeight = 0; randomWeight < currentWeightsSize; randomWeight++)
			{
				weightData.push_back(((float)rand() / (RAND_MAX)) * 2 - 1);
			}
			weights.push_back(Matrix(rows, columns, weightData));
		}
		agents.push_back(std::make_shared<Agent>(weights, startingPosition, startingAngle, mapElements));
	}

	/*
	Now for the main game loop
	*/
	while (window.isOpen())
	{
		/*
		This just handles events and draws the necessary elements to the window
		*/
		window.clear(sf::Color(200, 200, 200));
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			}
		}
		for (sf::VertexArray mapElement : mapElements)
		{
			window.draw(mapElement);
		}

		/*
		Updates and draws each agents onto the window
		*/
		for (int currentAgent = 0; currentAgent < numberAgents; currentAgent++)
		{
			if (agents[currentAgent]->isFailed()) continue;
			agents[currentAgent]->update();
			agents[currentAgent]->draw(window);
			for (int currentCheckPoint = 0; currentCheckPoint < numberCheckPoints; currentCheckPoint += 2)
			{
				sf::Vector2f checkPointStart = checkPoints[currentCheckPoint].position;
				sf::Vector2f checkPointEnd = checkPoints[currentCheckPoint + 1].position;
				if (agents[currentAgent]->updateFitness(checkPointStart, checkPointEnd, currentCheckPoint / 2) 
					&& checkPoints[currentCheckPoint].color == sf::Color::Red)
				{
					checkPoints[currentCheckPoint].color = sf::Color::Green;
					checkPoints[currentCheckPoint + 1].color = sf::Color::Green;
				}
			}
			if (agents[currentAgent]->checkFail()) numberFailed += 1;
		}

		window.draw(checkPoints);
		window.display();

		/*
		This is the genetic algorithm implementation. The fitness is always mutated by being multiplied
		by a random value between 0.9 and 1.1. The agents are then sorted in reverse order so the 
		best performing agents are at the beginning of the vector. It takes the top 20% of agents 
		to become parents (so there will be N/10 "couples"). This will each create 8 children each 
		and these offspring will have a 10% hard mutation rate. A hard mutation means that the weights
		that are muated are mutated to a random value. It also takes the top 10% of parents and uses 
		the idea of elitism to send them through to the next generation. The final 10% comes from
		mutated versions of the elite parents with a 5% soft mutation rate. A soft mutation is where
		each of the weights that are mutated are altered by a small delta. This algorithm ensures a 
		few key things:

		1.	The high number (80%) of agents that have been genetically spliced and mutated at such a
			high rate ensures that genetic diversity is kept high and the agents don't converge on a
			solution prematurely. Through testing I found that the diversity metric I made up works
			as an excellent indicator of how the algorithm will perform over hundreds of generations.
			If the genetic diversity drops by over an order of magnitude in the first 20 generations
			then it is very unlikely that the agents will mutate sufficiently and this is indicitive
			of a poor algorithm.
		
		2.	The agents will not regress. This is important for this task and elitism is how this is
			solved but too great a bearing on elitism can lead to the diversity dropping. This is why
			10% of the agents are carried through as I found this is a decent compromise to allow for
			a stable diversity but also the population being carried in the right direction
		
		3.	There will often be incremental progress from generation to generation even if there isn't
			a breakthrough. This is created through the mutated top 10% as these are very similar to the
			most successful agents but have slight variations which could cause a minor improvement.
		*/
		if (numberFailed == agents.size())
		{
			std::vector<std::shared_ptr<Agent>> parents;
			std::vector<std::shared_ptr<Agent>> survivedAgents;
			numberFailed = 0;
			double diversity = 0;
			/*
			Arbitratily picks some random agents to give an indication of diversity
			*/
			for (int i = 0; i < 1000; i++)
			{
				diversity += geneticDiversity(agents[rand() % numberAgents], agents[rand() % numberAgents]);
			}
			int maxFitness = 0;
			int averageFitness = 0;
			for (int currentAgent = 0; currentAgent < numberAgents; currentAgent++)
			{
				int currentFitness = agents[currentAgent]->getFitness();
				averageFitness += currentFitness;
				if (currentFitness > maxFitness)
				{
					maxFitness = currentFitness;
				}
				agents[currentAgent]->mutateFitness();
			}
			averageFitness /= numberAgents;
			std::sort(agents.begin(), agents.end(), compareFitness);
			diversity /= 1000;
			std::cout << "Generation: " << currentGeneration++ << "; Greatest Fitness: "
				<< maxFitness << "; Average Fitness: " << averageFitness 
				<< "; Diversity: " << diversity << '\n';
			for (int currentAgent = 0; currentAgent < numberAgents / 5; currentAgent++)
			{
				parents.push_back(agents[currentAgent]);
				if (currentAgent < numberAgents / 10)
				{
					std::vector<Matrix> weights = agents[currentAgent]->getNetwork();
					survivedAgents.push_back(std::make_shared<Agent>(weights, startingPosition, startingAngle, mapElements));
					std::shared_ptr<Agent> mutatedParent = std::make_shared<Agent>(weights, startingPosition, startingAngle, mapElements);
					mutatedParent->softMutate(5);
					survivedAgents.push_back(mutatedParent);
				}
			}
			/* 
			Creates random couples 
			*/
			std::random_shuffle(parents.begin(), parents.end());
			for (int currentAgent = 0; currentAgent < numberAgents / 5; currentAgent += 2)
			{
				std::shared_ptr<Agent> parent1 = parents[currentAgent];
				std::shared_ptr<Agent> parent2 = parents[currentAgent + 1];
				for (int i = 0; i < 8; i++)
				{
					std::vector<Matrix> crossoverNetwork = crossParents(parent1, parent2);
					std::shared_ptr<Agent> child = std::make_shared<Agent>(crossoverNetwork, startingPosition, startingAngle, mapElements);
					child->hardMutate(10);
					survivedAgents.push_back(child);
				}
			}
			agents = std::move(survivedAgents);
			for (int currentCheckPoint = 0; currentCheckPoint < numberCheckPoints; currentCheckPoint += 2)
			{
				checkPoints[currentCheckPoint].color = sf::Color::Red;
				checkPoints[currentCheckPoint + 1].color = sf::Color::Red;
			}
		}
	}
	return 0;
}