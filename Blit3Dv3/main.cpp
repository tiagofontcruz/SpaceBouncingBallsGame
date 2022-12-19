//Hyper Space game by Tiago

#include <stdlib.h>
#include <crtdbg.h>
#include <random>
#include "Blit3D.h"
#include "Ball.h"
#include "Ship.h"
#include "AudioEngine.h"

std::mt19937 rng;

//GLOBAL DATA
double elapsedTime = 0;
float timeSlice = 1.f / 60.f;
bool shoot = false;
bool engineResumed = false;
glm::vec2 gravity;

//objects
Blit3D* blit3D = NULL;
Ship* ship = NULL;
AngelcodeFont* neon80s = NULL;
AudioEngine* audioE = NULL;
AkGameObjectID mainGameID = 1;
AkPlayingID laserFireID, explosionID, thrustID, musicID;

//sprites
Sprite* titleSprite = NULL;
Sprite* backgroundSprite = NULL;
Sprite* lifeSprite = NULL;

//vectors
std::vector<Ball*> ballList;
std::vector<Shot> shotList;
std::vector<Sprite*> explosionSpriteList;
std::vector<Explosion> explosionList;

//constants
enum class GameState { PLAYING, PAUSE, GAMEOVER, TITLE , GAMEWIN};
GameState gameState = GameState::TITLE;

//random distributions for the ball's positions
std::uniform_real_distribution<float> pos(0.f, 300.f);
std::uniform_real_distribution<float> pos1(0.f, 500.f);
std::uniform_real_distribution<float> pos2(0.f, 700.f);

void MakeLevel() {
	//turn on invulnerability - activate the player's shield
	ship->shieldTimer = 3.f;
	ship->lives = 3;

	//move the ship back to the center of the screen
	ship->position = glm::vec2(1920.f / 2, 80.f);
	ship->angle = 90;
	ship->velocity = glm::vec2(0.f, 0.f);

	//delete the balls
	for (auto& B : ballList) delete B;

	//cleanup old shots, explosions and balls list
	ballList.clear();
	shotList.clear();
	explosionList.clear();

	//instanciate objects
	//OUT A RANDOM NUMBER TO THE X VELOCITY
	Ball* B = new Ball("Media\\balls.png", 76, 76, glm::vec2(100, 300), glm::vec2(pos(rng), pos(rng)), 76 / 2, 100, -100.f);
	ballList.push_back(B);
	Ball* C = new Ball("Media\\balls1.png", 76, 76, glm::vec2(100, 300), glm::vec2(pos1(rng), pos1(rng)), 76 / 2, 100, -100.f);
	ballList.push_back(C);												
	Ball* D = new Ball("Media\\balls2.png", 76, 76, glm::vec2(100, 300), glm::vec2(pos2(rng), pos2(rng)), 76 / 2, 100, -100.f);
	ballList.push_back(D);
}

void Init()
{
	InitializeRNG(); //initializing seed/rng

	//turn cursor off
	blit3D->ShowCursor(false);

	//load our title screen image: the arguments are upper-left corner x, y, width to copy, height to copy, and file name.
	titleSprite = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\title.png");

	//load our background image: the arguments are upper-left corner x, y, width to copy, height to copy, and file name.
	backgroundSprite = blit3D->MakeSprite(0, 0, 1920, 1080, "Media\\back.png");

	//load an Angelcode binary32 font file.
	neon80s = blit3D->MakeAngelcodeFontFromBinary32("Media\\neon80s.bin");

	//create a ship
	ship = new Ship;

	//load a sprite off of a spritesheet
	for (int i = 0; i < 4; ++i)
		ship->spriteList.push_back(blit3D->MakeSprite(i * 72, 0, 72, 88, "Media\\Player_Ship2c.png"));

	ship->position = glm::vec2(1920.f / 2, 80.f);

	//load our small ship life counter
	lifeSprite = blit3D->MakeSprite(0, 0, 88, 88, "Media\\Player_Ship_small.png");

	//load the shield graphic
	ship->shieldSprite = blit3D->MakeSprite(0, 0, 60, 60, "Media\\shield.png");

	//load the shot graphic
	ship->shotSprite = blit3D->MakeSprite(0, 0, 8, 8, "Media\\shot.png");

	//load explosion's spritesheet
	for (int i = 0; i < 8; ++i)
	{
		explosionSpriteList.push_back(blit3D->MakeSprite(1 + i * 63, 0, 63, 63, "Media\\explosion.png"));
	}

	//create audio engine
	audioE = new AudioEngine;
	audioE->Init();
	audioE->SetBasePath("Media\\");

	//load banks
	audioE->LoadBank("Init.bnk");
	audioE->LoadBank("Main.bnk");

	//register our game objects
	audioE->RegisterGameObject(mainGameID);

	//start playing the engine sound
	//We can play events by name:
	musicID = audioE->PlayEvent("Music", mainGameID);

	thrustID = audioE->PlayEvent("Thrust", mainGameID);
	audioE->PauseEvent("Thrust", mainGameID, thrustID);

	explosionID = audioE->PlayEvent("Explosion", mainGameID);
	audioE->PauseEvent("Explosion", mainGameID, explosionID);

	laserFireID = audioE->PlayEvent("LaserFire", mainGameID);
	audioE->PauseEvent("LaserFire", mainGameID, laserFireID);
}

void DeInit(void)
{
	//delete the object to deallocate memory
	//delete de ship
	if (ship != NULL) delete ship;
	//delete the audios
	if (audioE != NULL) delete audioE;	
	//delete the balls
	for (auto& B : ballList) delete B;
}

void Update(double seconds)
{
	//checks and resets the deltaTime
	float deltaTime = static_cast<float>(seconds);
	if (deltaTime > 0.15f) {
		deltaTime = 0.15f;
	}

	//only update time to a maximun amount - prevents big jumps in 
	//the simulation if the computer "hiccups"
	if (seconds < 0.15)
		elapsedTime += seconds;
	else elapsedTime += 0.15;

	//must always update audio in our game loop
	audioE->ProcessAudio();

	//alters the game's status
	switch (gameState)
	{
		case GameState::TITLE:
		break;
		case GameState::PLAYING:
		{
			//create the balls
			for (auto& B : ballList) {
				B->Update(deltaTime);
			}

			//create the collision with the screen
			for (auto& B : ballList) {
				CollideWithWindows(B, blit3D->screenWidth, blit3D->screenHeight);
			}

			//handle ship thrusting noise
			if (ship->thrusting)
			{
				if (!engineResumed)
				{	//unpause the engine event
					audioE->ResumeEvent("Thrust", mainGameID, thrustID);
					engineResumed = true;
				}
			}
			else
			{
				if (engineResumed)
				{
					//pause the engine event
					audioE->PauseEvent("Thrust", mainGameID, thrustID);
					engineResumed = false;
				}
			}

			//update by a full timeslice when it's time
			while (elapsedTime >= timeSlice)
			{
				elapsedTime -= timeSlice;
				ship->Update(timeSlice);

				//update the Wwise parameter for "Stereo",
				//which is mapped to pan it left/right
				AKRESULT result = AK::SoundEngine::SetRTPCValue(L"StereoX", (AkRtpcValue)(ship->position.x), mainGameID);
				assert(result == AK_Success);
				result = AK::SoundEngine::SetRTPCValue(L"StereoY", (AkRtpcValue)(ship->position.x), mainGameID);
				assert(result == AK_Success);

				if (shoot) {
					if (ship->Shoot(shotList))
					{						
						laserFireID = audioE->PlayEvent("LaserFire", mainGameID);
					}
				}

				//iterate backwards through the shotlist for the next loop
				for (int i = shotList.size() - 1; i >= 0; --i)
				{
					//shot Update() returns false when the bullet should be killed off
					if (!shotList[i].Update(timeSlice)) {
						shotList.erase(shotList.begin() + i);
					}
				}

				//explosion update
				for (int i = explosionList.size() - 1; i >= 0; --i)
				{
					if (!explosionList[i].Update(timeSlice)) {
						explosionList.erase(explosionList.begin() + i);
					}
				}

				for (auto& B : ballList) B->Update(timeSlice);

			//verifies the collision between the shots and balls
			for (int aIndex = ballList.size() - 1; aIndex >= 0; --aIndex)
			{
				for (int sIndex = shotList.size() - 1; sIndex >= 0; --sIndex)
				{
					if (ballList[aIndex] != NULL) {
						//verifies the collision
						if (CollideBallWithShot(ballList[aIndex], shotList[sIndex]))
						{
							//remove the shot
							shotList.erase(shotList.begin() + sIndex);					

							//delete ball after third gravity change
							if (ballList[aIndex]->gravity == -800.f) {
								Explosion E(ballList[aIndex]->position, 3.f);
								explosionList.push_back(E);
								delete ballList[aIndex];
								ballList.erase(ballList.begin() + aIndex);
							}
							//change the ball's gravity and velocity (second stage)
							else if (ballList[aIndex]->gravity == -400.f)
							{
								ballList[aIndex]->gravity = -800.f;
								ballList[aIndex]->velocity = glm::vec2(1000, 1000);
							}
							//change the ball's gravity and velocity (third stage)
							else if (ballList[aIndex]->gravity == -100.f)
							{
								ballList[aIndex]->gravity = -400.f;
								ballList[aIndex]->velocity = glm::vec2(600, 600);
							}
							
							explosionID = audioE->PlayEvent("Explosion", mainGameID);	
							break;
						}//end the collisions IF
					}
				}//end of collisions inner loop
			}//end of collision loop

			
			if (ballList.empty())
			{				
				gameState = GameState::GAMEWIN;
				break;
			}

			for (auto A : ballList)
			{
				//check for collision with the ship
				if (CollideWithBalls(A, ship))
				{
					//take away a life
					ship->lives--;
					//check for game over
					if (ship->lives < 1)
					{
						gameState = GameState::GAMEOVER;
						break;
					}

					//turn on invulnerability - activate the player's shield
					ship->shieldTimer = 5.f;

					//make an explosion
					Explosion e(ship->position, 3.f);
					explosionList.push_back(e);

					//move the ship to the center of the screen
					ship->position = glm::vec2(1920.f / 2, 80.f);
					ship->angle = 90;
					ship->velocity = glm::vec2(0.f, 0.f);
				}
			}
		}
	}//end PLAYING case	
	break;
	case GameState::PAUSE:
	case GameState::GAMEWIN:
		break;
	case GameState::GAMEOVER:

	default:
		break;
	}//end gameState switch
}

void Draw(void)
{
	float textTitleScreen;
	std::string titleScreen;

	//wipe the drawing surface clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (gameState)
	{
	case GameState::TITLE:
		titleSprite->Blit(1920.f / 2, 1080.f / 2);
		titleScreen = "PRESS ENTER";
		textTitleScreen = neon80s->WidthText(titleScreen);
		neon80s->BlitText(1920.f / 2.f - textTitleScreen / 2.f, 1080.f / 4.f, titleScreen);

		break;
	case GameState::PAUSE:
	{
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);

		for (auto& B : ballList) B->Draw();

		//draw the ship
		ship->Draw();

		//draw the shots
		for (auto& S : shotList) S.Draw();

		//draw explosions
		for (auto& E : explosionList) E.Draw();

		//draw our life HUD
		for (int i = 0; i < ship->lives; ++i)
		{
			lifeSprite->Blit(100 + i * 32, 1080 - 50);
		}
	}
	break;
	case GameState::GAMEOVER:
	{
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);

		//messages when game is over
		std::string gameOver = "GAME OVER";
		float textGameOver = neon80s->WidthText(gameOver);
		neon80s->BlitText(1920.f / 2.f - textGameOver / 2.f, 1080.f / 1.2f, gameOver);

		std::string pressG = "Press G to start a new game";
		float textPressG = neon80s->WidthText(pressG);
		neon80s->BlitText(1920.f / 2.f - textPressG / 2.f, 1080.f / 1.2f - 100.f, pressG);
	}//end GAMEOVER case
	break;
	case GameState::GAMEWIN:
	{
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);

		//messages when game is over
		std::string gameWin = "YOU WIN!";
		float textGameWin = neon80s->WidthText(gameWin);
		neon80s->BlitText(1920.f / 2.f - textGameWin / 2.f, 1080.f / 1.2f, gameWin);

		std::string pressG = "Press G to start a new game";
		float textPressG = neon80s->WidthText(pressG);
		neon80s->BlitText(1920.f / 2.f - textPressG / 2.f, 1080.f / 1.2f - 100.f, pressG);
	}//end GAMEWIN case
	break;
	case GameState::PLAYING:
	{
		//draw the background in the middle of the screen
		//the arguments to Blit(0 are the x, y pixel coords to draw the center of the sprite at, 
		//starting as 0,0 in the bottom-left corner.
		backgroundSprite->Blit(1920.f / 2, 1080.f / 2);

		//draw the balls
		for (auto& B : ballList) {
			B->Draw();
		}

		for (auto& C : ballList) {
			C->Draw();
		}

		for (auto& D : ballList) {
			D->Draw();
		}

		//draw the ship
		ship->Draw();
		//draw the shots
		for (auto& S : shotList) S.Draw();

		//Draw the explosions
		for (auto& E : explosionList) E.Draw();

		//draw our life HUD
		for (int i = 0; i < ship->lives; ++i)
		{
			lifeSprite->Blit(100 + i * 84, 1080 - 50);
		}
	}//end PLAYING case
	break;
	}//end gameState switch	
}//end Draw

//the key codes/actions/mods for DoInput are from GLFW: check its documentation for their values
void DoInput(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		blit3D->Quit(); //start the shutdown sequence

	switch (gameState)
	{
	case GameState::TITLE:
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {						
			MakeLevel();			
			gameState = GameState::PLAYING;
		}
		break;
	case GameState::GAMEOVER:
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {			
			gameState = GameState::TITLE;
		}
		break;
	case GameState::GAMEWIN:
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {			
			gameState = GameState::TITLE;
		}
		break;
	case GameState::PAUSE:
		
	case GameState::PLAYING:
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			ship->turningLeft = true;
			ship->thrusting = true;
		}

		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			ship->turningLeft = false;
			ship->thrusting = false;
		}

		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			ship->turningRight = true;
			ship->thrusting = true;
		}

		if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			ship->turningRight = false;
			ship->thrusting = false;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			shoot = true;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			shoot = false;
		}
		break;
	default:
		break;
	}//end gameState switch
}

int main(int argc, char* argv[])
{
	//set X to the memory allocation number in order to force a break on the allocation:
	//useful for debugging memory leaks, as long as your memory allocations are deterministic.
	//_crtBreakAlloc = X;

	blit3D = new Blit3D(Blit3DWindowModel::DECORATEDWINDOW, 1920, 1080);

	//set our callback funcs
	blit3D->SetInit(Init);
	blit3D->SetDeInit(DeInit);
	blit3D->SetUpdate(Update);
	blit3D->SetDraw(Draw);
	blit3D->SetDoInput(DoInput);

	//Run() blocks until the window is closed
	blit3D->Run(Blit3DThreadModel::SINGLETHREADED);
	if (blit3D) delete blit3D;
}