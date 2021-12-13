#pragma once

#include <SFML/Graphics.hpp>
#include<SFML/Network.hpp>
#include "Framework/Input.h"
#include <string>
#include <iostream>
#include<box2d/box2d.h>
#include<thread>
#include<vector>
#include <chrono>
#include"QuadraticPrediction.h"
using std::cout;
using std::endl;





// a structure that contains all the basic variable that the player would need to play the game 
// making four instances of this and initialising the variables according to who connects first 
// 3 out of four of the variables are going to be updated according to the data recieved by the server
// the last variable is going to be the main player and hes gon send data to the server
struct Players
{

	sf::Texture carText; 
	sf::Sprite car; 
	sf::RectangleShape car_collisionBox;
	b2Body* player_;

	//lap count to see whos won the race 
	int checkpointsCount = 0;
	int lapCount = 0;

	//coin and mushroom counting 
	int coinCount = 0;
	int mushroomCount = 0;

	///////car movement////////
	sf::Vector2f movementVec; //normal vector based on current direction
	const sf::Vector2f forwardVec = sf::Vector2f(0.f, 1.f); //normal vec pointing forward
	float currentSpeed = 0.f;
	//consts are units per second as each value is multiplied by frame time
	const float acceleration = 200.f;
	const float deceleration = 150.f;
	 float maxSpeed = 800.f;
	const float rotateAmount = 245.f;

	//boost mechanic after you've collected ten coins which makes one mushroom
	bool boost = false;

	sf::Vector2f camerapos = sf::Vector2f(0.f,0.f);
 	bool moving = false;
	sf::Vector2f nextPos, oldPos;
	float timer;
	int standings = 0;
	bool isFinished = false;

	QuadraticPrediction prediction;

};
class Level{
public:
	Level(sf::RenderWindow* hwnd, Input* in,int w,int h);
	~Level();
	void handleInput(float dt);
	void update(float dt,int frame_rate);
	void render();
	float dot(sf::Vector2f,sf::Vector2f);
	void collision(float);
	void InitWalls();
	void b2Init();
	void initCheckpoints();
	void InitCoins();
	void sendtoServer();
	void InitPlayers();
	void recievefromServer();
	void setFocused(bool focused) { isFocused = focused; }
	float getTime() { return sendTime; };

private:
	// Default functions for rendering to the screen.
	void beginDraw();
	void endDraw();
	
	// Default variables for level class.
	sf::RenderWindow* window;
	Input* input;
	int Swidth, Sheight;


	// array of the struct players for 2 players so that according to the player ID the right car is assigned
	Players players[4];

	/////////////////SFML variables/////////////////
	int rank = 0;
	//
	int playerID;
	//Networking varibales 
	sf::IpAddress ip = sf::IpAddress::getLocalAddress();
	//sf::IpAddress ip;
	sf::TcpSocket client;
	sf::Packet packetSend, packetRecieved;
	std::thread recieveThread;
	//Text Variables 
	sf::Text laps_Text;
	sf::Text laps_Count;
	sf::Text coins_count;
	sf::Text mushroom_count;
	sf::Text result;
	sf::Font font;

	//for camera view so that it follows the player 
	sf::View cameraView;
	sf::Vector2f camerPos;
	
	//coincount texture 
	sf::Texture coinText;
	sf::Sprite coinSprite;

	//mushroom texture
	sf::Texture mushroomText;
	sf::Sprite mushroomSprite;

	//map texture 
	sf::Texture mapText;
	sf::Sprite map;
	
	//player car texture 
	sf::Texture carText; //this one
	sf::Sprite car; //this one

	//walls so that player doesn't go out of bounds 
	sf::RectangleShape walls[4];

	//debugging car collision hitbox 
	sf::RectangleShape car_collisionBox; 

	//race checkpoints 
	sf::RectangleShape checkpoints[10];

	//coin shapes
	sf::RectangleShape coins[27];

	//////////Box2D//////////

	b2World* world_;
	b2Body* player_; 
	b2Body* coins_[27];
	b2Body* walls_[4];
	b2Body* checkpoints_[10];

	float timer = 0;
	float sendTime = 0;
	float tickTimer = 0;
	// static coin position for easier initialisation of collision bodies 
	b2Vec2 coinPos[9] = {b2Vec2(94,766),
						 b2Vec2(334,46),
						 b2Vec2(1230,46),
						 b2Vec2(1454,702),
						 b2Vec2(574,974),
						 b2Vec2(1182,1262),
						 b2Vec2(1886,1646),
						 b2Vec2(910,1870),
						 b2Vec2(94,1678)};

	// off set as coins are place in sets of 3 so you only have to run a single for loop to initialise positions of the collision bodies of the coins 
	b2Vec2 coinOffset[9] = { b2Vec2(32,0),
							 b2Vec2(0,32),
							 b2Vec2(0,32),
							 b2Vec2(32,0),
							 b2Vec2(32,0),
							 b2Vec2(0,32),
							 b2Vec2(32,0),
							 b2Vec2(0,32),
							 b2Vec2(32,0) };

	bool isFocused = true;
	bool displayresult = false;
	bool recieved = false;	

	
};
