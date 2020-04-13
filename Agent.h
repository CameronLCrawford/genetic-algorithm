#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Matrix.h"

struct intersectionPoint
{
	float lambda;
	float mu;
};

class Agent
{
public:
	Agent(std::vector<Matrix>, sf::Vector2f, int, std::vector<sf::VertexArray>);
	~Agent();

	void draw(sf::RenderWindow&);

	void update();

	bool checkFail();

	bool updateFitness(sf::Vector2f, sf::Vector2f, int);

	bool isFailed();

	void mutateFitness();

	void softMutate(int);

	void hardMutate(int);

	int getFitness();

	std::vector<Matrix> getNetwork();

private:
	sf::ConvexShape ship;
	std::vector<Matrix> weights;
	std::vector<sf::VertexArray> mapData;

	bool failed = false;
	float fitness = 1.0f;
	bool passingCheckPoint = false;
	int lastCheckPoint = 1;
	int currentCheckPoint = 0;
};