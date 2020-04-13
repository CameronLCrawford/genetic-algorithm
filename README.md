# genetic-algorithm
A small investigation into genetic algorithms and neuroevolution.

# Overview
This project implements a genetic algorithm to evolve car-like agents to be able to race around a simple track. It uses SFML for the graphics (can be found at https://www.sfml-dev.org/) and is written solely in C++. The only other file necessary is the map.txt file which encodes the layout of the map. The program is just on a constant loop until the window is closed so the agents just continuously evolve from generation to generation until the window is terminated.

# Agents
The agents consist of a sprite which is drawn to the screen and a network of weights which represents their genome and is how they respond to input. The neural network is fed three inputs, has a singular hidden layer of size 5, and has two output nodes. The network is fully connected and the architecture doesn't change throughout the course of the program. The three inputs come from three "sightlines" which tell the agent how far they are from a wall. If the sightlines are divided by 100 and if the distance to the nearest wall is greater than 100 then it is just 1. This means that the three input values are always between 0 and 1. The three sightlines are located on the two sides (pointing directly away from the agent) and in front of the agent. The two outputs are indications of which direction the agent wants to turn. If the first one is greater it turns left and if the second is greater it turns right. When the agents are turning left they are coloured red and when they are turning right they are blue. When an agents collides with a wall it is failed for that generation. The fitness for an agent is based off how long it is alive and how many checkpoints it passes.

# Genetic Algorithn
After every agent for that generation is failed, the next generation is generated. Before the selection process begins, the fitness for each agent is mutated by being multiplied by a random value between 0.9 and 1.1. The agents are then sorted by fitness and the top 20% become parents. These are randomly put into couples and each produce 10 offspring which are hard mutated with a mutation rate of 10%. A hard mutation is where the weights of the network that are mutated (which would be 10% on average in this case) are changed to a completely random value. The top half of parents are selected as the elite. The elite are automatically added to the next generation as well as a soft mutated copy (with a mutation rate of 5%) for each elite agent. A soft mutation is where the weights that are mutated are changed by a small delta which can be positive or negative.

# Map file
The map file is read in the following way:
1. Starting position x-coordinate
2. Starting position y-coordinate
3. Starting angle for each agent
4. The number of map elements. A map element is a set of vertices that specify some shape in the map scene. The lines between these vertices are what the agents "see" and collide with
5. The number of vertices for the current element
6. The x-coordinate of the current vertex
7. The y-coordinate of the current vertex
8. After all of the map elements have been read in the next number is the number of check point vertices
9. The x-coordinate of the current check point vertex
10. The y-coordinate of the current check point vertex
11. EOF
Check points are comprised of two vertices and are simply a line segment between the two points. When an agent's body intersects this line, the agent gets a large boost in fitness. All check points start off as red at the beginning of a generation but once it has been reached by any agent for that generation the check point is coloured green.

# Diversity
In order to measure how quickly the population converged on a particular solution (which isn't necessarily a good solution) I created a metric called the diversity of the population. This metric is used to give an indication of how different the genomes of the agents in the population are. To estimate this, I defined the diversity function for two agents as the sum of the absolute differences for each weight. I then take a random sample of the population each generation and calculate a rough estimate of what the diversity is when compared to the previous generations. This is by no means an exact science and is not meant to be interpreted value by value but instead as the change between generations and what this says about the mutation rate and the effectiveness of the crossover algorithm. From investigation I found that the use of a genetic crossover to generate the new population instead of solely relying on mutation helps to stabilise the diversity and leads to a far better population performance in the long run. 
