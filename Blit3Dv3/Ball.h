#pragma once
#include "Blit3D.h"

class Ball
{
public:
	Sprite* sprite;
	glm::vec2 position;
	glm::vec2 velocity;
	float radius;
	float radius2;
	float mass;
	float gravity;

	Ball(std::string filename, int width, int height, glm::vec2 Velocity, glm::vec2 Position, float Radius, float Mass, float Gravity);

	void Draw();
	void Update(float seconds);
};

bool CollideWithWindows(Ball* ball, float width, float heigth);