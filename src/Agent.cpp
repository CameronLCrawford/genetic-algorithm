#include "Agent.h"

#define PI 3.1415926536

// Sigmoid activation function
float sigmoid(float x)
{
	return 1 / (1 + exp(-x));
}

// Convert from degrees to radians
float radians(float theta)
{
	return (PI / 180) * theta;
}

// Find intersection point between two line segments from their start and end coordinates
intersectionPoint checkIntersection(sf::Vector2f line1Start, sf::Vector2f line1End, sf::Vector2f line2Start, sf::Vector2f line2End)
{
	float deltaLine1X = line1Start.x - line1End.x;
	float deltaLine2X = line2Start.x - line2End.x;
	float deltaLine1Y = line1Start.y - line1End.y;
	float deltaLine2Y = line2Start.y - line2End.y;
	float denominator = deltaLine1X * deltaLine2Y - deltaLine1Y * deltaLine2X;
	// Handle parallel lines
	if (denominator == 0)
	{
		return { 0, 0 };
	}
	// Proportion along line1 point of intersection is
	float lambda = ((line1Start.x - line2Start.x) * deltaLine2Y - (line1Start.y - line2Start.y) * deltaLine2X) / denominator;
	// Proportion along line2 point of intersection is
	float mu = -(deltaLine1X * (line1Start.y - line2Start.y) - deltaLine1Y * (line1Start.x - line2Start.x)) / denominator;
	return { lambda, mu };
}

Agent::Agent(std::vector<Matrix> weights, sf::Vector2f startPosition, int startRotation, std::vector<sf::VertexArray> mapData)
	: weights(weights), mapData(mapData)
{
	// Build ship by adding coordinates of vertices
	ship.setPointCount(4);
	ship.setPoint(0, { 0 , 0 });
	ship.setPoint(1, { -8, 8 });
	ship.setPoint(2, { 16, 0 });
	ship.setPoint(3, { -8, -8 });
	ship.setPosition(startPosition);
	ship.setRotation(startRotation);
	ship.setFillColor({ 0,0,0,0 });
	ship.setOutlineColor(sf::Color::Black);
	ship.setOutlineThickness(1);
}

Agent::~Agent()
{
}

void Agent::draw(sf::RenderWindow &window)
{
	if (failed)
	{
		return;
	}
	window.draw(ship);
}

void Agent::update()
{
	fitness += 1;
	// Get input to neural network
	std::vector<float> inputDistances = { 1, 1, 1 };
	float rotation = radians(ship.getRotation());
	for (int currentRay = 0; currentRay < 3; currentRay++)
	{
		sf::Vector2f rayStart = ship.getPosition();
		sf::Vector2f rayEnd;
		sf::Vector2f rayDirection;
		/* 
		Sets the start and end positions of each ray. There are three rays, one
		which looks directly to the left of the agent, one to the right, and one
		straight ahead 
		*/
		switch (currentRay)
		{
		case 0:
			rayStart.x += cos(rotation) * -8.0f - sin(rotation) * -8.0f;
			rayStart.y += sin(rotation) * -8.0f + cos(rotation) * -8.0f;
			rayDirection.x = sin(rotation);
			rayDirection.y = -cos(rotation);
			break;
		case 1:
			rayStart.x += cos(rotation) * -8.0f - sin(rotation) * 8.0f;
			rayStart.y += sin(rotation) * -8.0f + cos(rotation) * 8.0f;
			rayDirection.x = -sin(rotation);
			rayDirection.y = cos(rotation);
			break;
		case 2:
			rayStart.x += cos(rotation) * 16.0f;
			rayStart.y += sin(rotation) * 16.0f;
			rayDirection.x = cos(rotation);
			rayDirection.y = sin(rotation);
		}
		rayEnd = rayStart + rayDirection;
		for (sf::VertexArray mapElement : mapData)
		{
			sf::Vector2f point1;
			sf::Vector2f point2;
			for (int i = 0; i < mapElement.getVertexCount() - 1; i++)
			{
				// Vertices of line segment of map wall
				point1 = mapElement[i].position;
				point2 = mapElement[i + 1].position;

				intersectionPoint intersection = checkIntersection(point1, point2, rayStart, rayEnd);
				float lambda = intersection.lambda;
				float mu = intersection.mu;
				// Handles parallel lines 
				if (lambda == 0 && mu == 0)
				{
					continue;
				}
				// Checks there is an intersection
				else if (0 < lambda && lambda < 1 && mu > 0)
				{
					float distance = mu / 100;
					// Checks if closest line segment to vertex
					if (distance < inputDistances[currentRay])
					{
						inputDistances[currentRay] = distance;
					}
				}
			}
		}
	}
	// Feed the input through the neural network to obtain the output decision 
	Matrix networkOutput(1, 3, inputDistances);
	for (Matrix currentWeights : weights)
	{
		networkOutput = networkOutput * currentWeights;
		for (int node = 0; node < networkOutput.getDimensions().columns; node++)
		{
			float nodePreActivation = networkOutput(0, node);
			float nodeValue = sigmoid(nodePreActivation);
			networkOutput.setData(0, node, nodeValue);
		}
	}
	// Rotate and move the agent according to the output decision 
	if (networkOutput(0, 0) >= networkOutput(0, 1))
	{ //Turn left
		ship.rotate(-2.0f);
		ship.setFillColor(sf::Color::Red);
	}
	else
	{ //Turn right
		ship.rotate(2.0f);
		ship.setFillColor(sf::Color::Blue);
	}
	ship.move({ 2.0f * cos(radians(ship.getRotation())), 2.0f * sin(radians(ship.getRotation())) });
}

// Tests if agent has collided with wall
bool Agent::checkFail()
{
	if (failed)
	{
		return true;
	}
	float rotation = radians(ship.getRotation());
	sf::Vector2f testLineStart = ship.getPosition();
	testLineStart.x += cos(rotation) * 16.0f;
	testLineStart.y += sin(rotation) * 16.0f;
	// For each line of the bounding box of the agent (which is a triangle)
	for (int testLine = 0; testLine < 2; testLine++)
	{
		sf::Vector2f testLineEnd = ship.getPosition();
		switch (testLine)
		{
		case 0:
			testLineEnd.x += cos(rotation) * -8.0f - sin(rotation) * -8.0f;
			testLineEnd.y += sin(rotation) * -8.0f + cos(rotation) * -8.0f;
			break;
		case 1:
			testLineEnd.x += cos(rotation) * -8.0f - sin(rotation) * 8.0f;
			testLineEnd.y += sin(rotation) * -8.0f + cos(rotation) * 8.0f;
			break;
		}
		// Finds intersection with each bounding line and each line in the map
		for (sf::VertexArray mapElement : mapData)
		{
			sf::Vector2f point1;
			sf::Vector2f point2;
			for (int i = 0; i < mapElement.getVertexCount() - 1; i++)
			{
				point1 = mapElement[i].position;
				point2 = mapElement[i + 1].position;
				intersectionPoint intersection = checkIntersection(point1, point2, testLineStart, testLineEnd);
				float lambda = intersection.lambda;
				float mu = intersection.mu;
				if (lambda == 0 && mu == 0)
				{
					continue;
				}
				if (0 <= lambda && lambda <= 1 && 0 <= mu && mu <= 1)
				{
					failed = true;
					return true;
				}
			}
		}
	}
	return false;
}

// Updates fitness each tine a checkpoint is passed
bool Agent::updateFitness(sf::Vector2f checkPointStart, sf::Vector2f checkPointEnd, int checkPointIndex)
{
	// Body is a line segment running though the agent
	sf::Vector2f bodyStart = ship.getPosition();
	sf::Vector2f bodyEnd = ship.getPosition();
	bodyEnd.x += cos(radians(ship.getRotation())) * 16.0f;
	bodyEnd.y += sin(radians(ship.getRotation())) * 16.0f;

	// Find intersection between body and checkpoint
	intersectionPoint intersection = checkIntersection(bodyStart, bodyEnd, checkPointStart, checkPointEnd);
	float lambda = intersection.lambda;
	float mu = intersection.mu;
	if (lambda == 0 && mu == 0)
	{
		return false;
	}
	if (0 < lambda && lambda < 1 && 0 < mu && mu < 1)
	{
		if (checkPointIndex == lastCheckPoint)
		{
			failed = true;
			return false;
		}
		// Increase fitness if passed checkpoint only if not already passed it
		if (!passingCheckPoint)
		{
			fitness += 100.0f;
		}
		passingCheckPoint = true;
		currentCheckPoint = checkPointIndex;
		return true;
	}
	if (passingCheckPoint && checkPointIndex == currentCheckPoint)
	{
		lastCheckPoint = checkPointIndex;
		passingCheckPoint = false;
	}
	return false;
}

// Getter
bool Agent::isFailed()
{
	return failed;
}

// Mutate fitness by random amount to add more random selection
void Agent::mutateFitness()
{
	fitness *= ((float)rand() / RAND_MAX / 5) + 0.9;
}

// Random weights will be changed by a small amount
void Agent::softMutate(int mutationRate)
{
	for (int currentWeights = 0; currentWeights < weights.size(); currentWeights++)
	{
		int rows = weights[currentWeights].getDimensions().rows;
		int columns = weights[currentWeights].getDimensions().columns;
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < columns; column++)
			{
				/*
				The mutation rate is a percentage which indicates what percentage of weights (on average)
				will be mutated by a small value
				*/
				if (rand() % 100 < mutationRate)
				{
					float weightDelta = ((float)rand() / RAND_MAX - 0.5) / 10;
					weights[currentWeights].setData(row, column, weights[currentWeights](row, column) + weightDelta);
				}
			}
		}
	}
}


// Random weights will be rewritten by new random weight value
void Agent::hardMutate(int mutationRate)
{
	for (int currentWeights = 0; currentWeights < weights.size(); currentWeights++)
	{
		int rows = weights[currentWeights].getDimensions().rows;
		int columns = weights[currentWeights].getDimensions().columns;
		for (int row = 0; row < rows; row++)
		{
			for (int column = 0; column < columns; column++)
			{
				/*
				The mutation rate is a percentage which indicates what percentage of weights (on average)
				will be mutated to a random value
				*/
				if (rand() % 100 < mutationRate)
				{
					float newWeight = ((float)rand() / (RAND_MAX)) * 2 - 1;
					weights[currentWeights].setData(row, column, newWeight);
				}
			}
		}
	}
}

// Getter
int Agent::getFitness()
{
	return fitness;
}

// Getter
std::vector<Matrix> Agent::getNetwork()
{
	return weights;
}