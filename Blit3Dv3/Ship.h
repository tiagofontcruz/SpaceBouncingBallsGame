#pragma once
#include<Blit3D.h>
#include "Ball.h"
#include <random>

class Shot
{
public:
	float angle;
	glm::vec2 velocity, position;
	Sprite* sprite = NULL;
	float timeToLive = 1.0f; //how long shot stay in the screen
	virtual void Draw();
	virtual bool Update(float seconds); //return false if shot dead (timeToLive <= 0)
};

class Ship
{
public:
	Sprite* shotSprite = NULL;
	Sprite* shieldSprite = NULL;
	std::vector<Sprite*> spriteList;
	glm::vec2 velocity, position;
	float angle = 0; //the angle of the ship
	float shotTimer = 0.1f; //counts the time for shots
	float radius = 27.f; //radius for ship collision
	float radius2 = radius * radius; //radius for collisions between ship and balls
	int lives = 3;
	int frameTime = 0;
	float thrustTimer = 0;
	bool thrusting = false; //inertia
	bool turningLeft = false; //moviment to the left
	bool turningRight = false; //moviment to the right
	float shieldTimer = 0; //if shieldTimer > 0 then shield's up
	float blinkTimer = 0.f; //timer for shield to blink
	bool blink = false; //enable or disable shield blinker

	void Draw();
	void Update(float seconds);
	bool Shoot(std::vector<Shot>& shotList);
};

//it generates random seeds/rng (Ship.cpp)
void InitializeRNG();

//asteroid's collision area
float DistanceSquared(glm::vec2 position1, glm::vec2 position2);

bool CollideBallWithShot(Ball*& a, Shot& s);

bool CollideWithBalls(Ball*& a, Ship* s);

extern std::vector<Sprite*> explosionSpriteList;

class Explosion
{
public:
	int frameNum = 0; //which frame will be displayed
	float frameSpeed = 1.f / 10.f; //how long
	float frameTimer = 0.f;
	float scale = 3.f; //explosions scale
	glm::vec2 position;

	void Draw();
	bool Update(float seconds); // return false if explosion will be removed
	Explosion(glm::vec2 location, float size);
};