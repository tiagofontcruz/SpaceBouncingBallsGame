#include "Ball.h"

extern Blit3D* blit3D;

Ball::Ball(std::string filename, int width, int height, glm::vec2 Velocity, glm::vec2 Position, float Radius, float Mass, float Gravity)
{
	sprite = blit3D->MakeSprite(0,0, height, width, filename);
	position = Position;
	velocity = Velocity;
	radius = Radius;
	radius2 = radius * radius;
	mass = Mass;
	gravity = Gravity;
}

void Ball::Draw() {
	sprite->Blit(position.x, position.y);
}
void Ball::Update(float seconds) {	
	position += velocity * seconds; //update position
	velocity.y += gravity * seconds;
}

bool CollideWithWindows(Ball* ball, float width, float heigth) {
	bool collided = false;
	//check for collision with:
	//LEFT SIDE
	if (ball->position.x < ball->radius) 
	{
		//collision in this side
		collided = true;
		//reset the position inside the windows
		ball->position.x = ball->radius;
		//changing the velocity fot the bounce off the edge
		ball->velocity.x *= -1.f;
	}

	//RIGHT SIDE
	if (ball->position.x > width - ball->radius)
	{
		//collision in this side
		collided = true;
		//reset the position inside the windows
		ball->position.x = width - ball->radius;
		//changing the velocity fot the bounce off the edge
		ball->velocity.x *= -1.f;
	}

	//BOTTOM SIDE
	if (ball->position.y < ball->radius)
	{
		//collision in this side
		collided = true;
		//reset the position inside the windows
		ball->position.y = ball->radius;
		//changing the velocity fot the bounce off the edge
		ball->velocity.y *= -1.f;
	}

	//TOP SIDE
	if (ball->position.y > heigth - ball->radius)
	{
		//collision in this side
		collided = true;
		//reset the position inside the windows
		ball->position.y = heigth - ball->radius;
		//changing the velocity fot the bounce off the edge
		ball->velocity.y *= -1.f;
	}
	return collided;
}