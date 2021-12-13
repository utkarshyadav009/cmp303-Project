#include "Level.h"
#include <box2d/box2d.h>
Level::Level(sf::RenderWindow* hwnd, Input* in, int w, int h)
{
	window = hwnd;
	input = in;
	Swidth = w;
	Sheight = h;
	//connecting to server and getting playerId 
	std::string name;
	std::string IP;
	cout << "Enter IP: ";
	std::cin >> IP;
	ip = IP;
	cout << "\nConnected to " << ip.toString() << "\n";
	while (client.connect(ip, 2000) != sf::Socket::Done) {
		cout << "Failed to connect, trying again...\n";
	}
	cout << "connected to server\n";
	cout << "Enter Player Name: ";
	std::cin >> name;
	packetSend << name;
	sendtoServer();
	recievefromServer();
	packetRecieved >> playerID;

	cout << "\nyou are player: " << playerID << endl;
	playerID = playerID - 1;

	client.setBlocking(false);
	//playerID = 0;
	//loading font for fps 
	if (!font.loadFromFile("font/MarioKart.ttf"))
	{
		std::cout << "Error Loading the font" << std::endl;
	}
	//lap text
	laps_Text.setFont(font);
	laps_Text.setCharacterSize(34);
	laps_Text.setFillColor(sf::Color::White);
	laps_Text.setPosition(0.0f, 0.0f);
	laps_Text.setString("laps       -3");

	//result 
	result.setFont(font);
	result.setCharacterSize(64);
	result.setFillColor(sf::Color::White);
	result.setPosition(Swidth / 2, Sheight / 2);

	//lap count gets updated everytime a lap is done 
	laps_Count.setFont(font);
	laps_Count.setCharacterSize(34);
	laps_Count.setFillColor(sf::Color::White);
	laps_Count.setPosition(0.0f, 0.0f);

	//coin count gets updated everytime player collects a coin 
	coins_count.setFont(font);
	coins_count.setCharacterSize(34);
	coins_count.setFillColor(sf::Color::White);
	coins_count.setPosition(0.0f, 0.0f);

	//mushroom count gets updated everytime player reaches 10 coins 
	mushroom_count.setFont(font);
	mushroom_count.setCharacterSize(34);
	mushroom_count.setFillColor(sf::Color::White);
	mushroom_count.setPosition(0.0f, 0.0f);

	//loading map
	mapText.loadFromFile("images/race/rainbowroad.png");
	mapText.setSmooth(true);
	map.setTexture(mapText);
	map.scale(2, 2);

	//loading coin sprite  
	coinText.loadFromFile("images/race/coin.png");
	coinText.setSmooth(true);
	coinSprite.setTexture(coinText);
	coinSprite.scale(2, 2);

	//loading mushroom sprite
	mushroomText.loadFromFile("images/race/mushroom1.png");
	mushroomText.setSmooth(true);
	mushroomSprite.setTexture(mushroomText);
	mushroomSprite.scale(1.5, 1.5);

	

	cameraView.setCenter(1024/2, 768/2);
	cameraView.setSize(1024, 768);

	InitPlayers();

	InitWalls();
	b2Init();
	initCheckpoints();
	InitCoins();


}


Level::~Level()
{

}

void Level::sendtoServer()
{
	client.send(packetSend);
	packetSend.clear();
}
void Level::recievefromServer()
{
	packetRecieved.clear();
	if (client.receive(packetRecieved) == sf::Socket::Done) {
		recieved = true;
	}
	else
		recieved = false;
}


// handle user input
void Level::handleInput(float dt)
{
	if (!isFocused) return;

	if (input->isKeyDown(sf::Keyboard::Escape))
	{
		exit(0);
	}

	if (players[playerID].currentSpeed > players[playerID].maxSpeed) {
		float diff = players[playerID].currentSpeed - players[playerID].maxSpeed;
		players[playerID].currentSpeed -= diff;
	}

	//players[0].player_->ApplyForceToCenter({ 10.f, 0.f }, true);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		players[playerID].moving = true;
		players[playerID].car.rotate(-players[playerID].rotateAmount * dt);

	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		players[playerID].moving = true;
		players[playerID].car.rotate(players[playerID].rotateAmount * dt);

	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		players[playerID].moving = true;
		if (players[playerID].currentSpeed < players[playerID].maxSpeed)
		{
			players[playerID].currentSpeed += players[playerID].acceleration * dt;
		}

		//rotate direction vector
		sf::Vector2f oldVec = players[playerID].movementVec;
		sf::Transform t;
		t.rotate(players[playerID].car.getRotation());
		players[playerID].movementVec = t.transformPoint(players[playerID].forwardVec);
		//calc the dot product so any rotation reduces the
		//current speed - gives the impression of wheel spin
		//when accelerating after a tight turn
		players[playerID].currentSpeed *= dot(oldVec, players[playerID].movementVec);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		players[playerID].moving = true;
		//currentSpeed -= deceleration * dt;
		if (players[playerID].currentSpeed < players[playerID].maxSpeed)
		{
			players[playerID].currentSpeed += players[playerID].acceleration * dt;

		}

		//rotate direction vector
		sf::Vector2f oldVec = players[playerID].movementVec;
		sf::Transform t;
		t.rotate(players[playerID].car.getRotation());
		players[playerID].movementVec = t.transformPoint(-players[playerID].forwardVec);
		//calc the dot product so any rotation reduces the
		//current speed - gives the impression of wheel spin
		//when accelerating after a tight turn
		players[playerID].currentSpeed *= dot(oldVec, players[playerID].movementVec);
		

	}
	else
	{
		players[playerID].currentSpeed -= players[playerID].deceleration * dt;
		if (players[playerID].currentSpeed < 0)
		{
			players[playerID].currentSpeed = 0;
			players[playerID].moving = false;

		}

	}

	static bool wasPressed = false;
	bool isPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		players[playerID].currentSpeed -= (players[playerID].deceleration * dt * 3);
		if (players[playerID].currentSpeed < 0)
		{
			players[playerID].currentSpeed = 0;
			players[playerID].moving = false;

		}
	}
	if (isPressed && !wasPressed)
	{
		if (players[playerID].boost)
		{		
			players[playerID].currentSpeed *= 2.05;
			players[playerID].maxSpeed = 1500;		
		}
	}

	wasPressed = isPressed;

	if (players[playerID].maxSpeed == 1500) {
		timer += dt;

		if (timer > 1.f)
		{
			//cout << "cut out the boosters" << endl;
			//players[playerID].boostTime = 0;
			if (players[playerID].currentSpeed > 800)
			{
				//cout << "decelerate this goddamn car" << endl;
			}
			players[playerID].maxSpeed = 800;
			players[playerID].mushroomCount--;
			if (players[playerID].mushroomCount < 0)
			{
				players[playerID].mushroomCount = 0;
				players[playerID].boost = false;

			}
			timer = 0;
		}
	}

} 

sf::Vector2f lerp(sf::Vector2f old, sf::Vector2f next, float timer) {
	return (1 - timer) * old + timer * next;
}

float length(const sf::Vector2f &v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}


// Update game objects
void Level::update(float dt, int frame_rate)
{
	sendTime += dt;
	float timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	world_->Step(timeStep, velocityIterations, positionIterations);

		
	players[playerID].car.move(players[playerID].movementVec * players[playerID].currentSpeed * dt);
	players[playerID].player_->SetTransform(b2Vec2(players[playerID].car.getPosition().x, players[playerID].car.getPosition().y), players[playerID].car.getRotation());
	players[playerID].car_collisionBox.setPosition(players[playerID].car.getPosition());
	players[playerID].car_collisionBox.setRotation(players[playerID].car.getRotation());
	players[playerID].camerapos = players[playerID].car.getPosition() - (sf::Vector2f(Swidth / 2, Sheight / 2));
	cameraView.setCenter(players[playerID].camerapos + sf::Vector2f(500.0, 200.f));
	
	laps_Count.setString(std::to_string(players[playerID].lapCount));
	laps_Text.setPosition(cameraView.getCenter()-sf::Vector2f(Swidth/2,Sheight/2));
	laps_Count.setPosition(cameraView.getCenter() - sf::Vector2f(Swidth/2.4, Sheight/2));
	coins_count.setString(std::to_string(players[playerID].coinCount / 10));
	if (players[playerID].coinCount / 10 >= 10)
	{
		players[playerID].coinCount = 10;
		players[playerID].mushroomCount++;
		players[playerID].boost = true;
		if (players[playerID].mushroomCount == 0)
		{
			players[playerID].boost = false;
		}
	}
	coins_count.setPosition(cameraView.getCenter() - sf::Vector2f(Swidth / 2.2, Sheight / 9.4));
	coinSprite.setPosition(cameraView.getCenter() - sf::Vector2f(Swidth / 2, Sheight / 10));

	mushroom_count.setString(std::to_string(players[playerID].mushroomCount));
	mushroom_count.setPosition(cameraView.getCenter() - sf::Vector2f(Swidth / 2.29, Sheight/600));
	mushroomSprite.setPosition(cameraView.getCenter() - sf::Vector2f(Swidth / 2, Sheight / 800));
	tickTimer += dt;

	//cout << tickTimer << endl;

	if (tickTimer>0.05){
		packetSend << playerID;
		packetSend << players[playerID].car.getPosition().x;
		packetSend << players[playerID].car.getPosition().y;
		packetSend << sendTime;
		packetSend << players[playerID].car.getRotation();
		packetSend << players[playerID].lapCount;
		sendtoServer();
		tickTimer = 0;
	}

	recievefromServer();
		
	if (recieved) {
		int id, lapCount;
		float x, y, rotation,time;
		packetRecieved >> id >> x >> y >> time >> rotation >> lapCount;
		sf::Vector2f next{ x, y };
		Data NetMs;
		NetMs.id = id;
		NetMs.player_position = next;
		NetMs.time = time;
		Data LocalMs;
		LocalMs.id = id;
		LocalMs.player_position = players[id].car.getPosition();
		LocalMs.time = getTime();
		players[id].prediction.NetworkPositionData(NetMs);
		players[id].prediction.LocalPositionData(LocalMs);
		if (players[id].prediction.NetworkDataHistory.size() == players[id].prediction.quadratic_message_number && players[id].prediction.LocalDataHistory.size() == players[id].prediction.quadratic_message_number)
		{

			sf::Vector2f m0_local(players[id].prediction.LocalDataHistory.at(0).player_position);
			sf::Vector2f m1_local(players[id].prediction.LocalDataHistory.at(1).player_position);
			sf::Vector2f m2_local(players[id].prediction.LocalDataHistory.at(2).player_position);

			sf::Vector2f m0_network(players[id].prediction.NetworkDataHistory.at(0).player_position);
			sf::Vector2f m1_network(players[id].prediction.NetworkDataHistory.at(1).player_position);
			sf::Vector2f m2_network(players[id].prediction.NetworkDataHistory.at(2).player_position);

			float m0_time = players[id].prediction.LocalDataHistory.at(0).time;
			float m1_time = players[id].prediction.LocalDataHistory.at(1).time;
			float m2_time = players[id].prediction.LocalDataHistory.at(2).time;

			float currentTime = getTime();

			sf::Vector2f newPos = players[id].prediction.quadraticInterpolation(m0_local, m1_local, m2_local, m0_network, m1_network, m2_network, m0_time, m1_time, m2_time, currentTime);
			players[id].car.setPosition(newPos);

			LocalMs.id = id;
			LocalMs.player_position = newPos;
			LocalMs.time = getTime();

		}
		players[id].car.setRotation(rotation);
		players[id].lapCount = lapCount;
		if (!players[id].isFinished)
		{
			if (players[id].lapCount == 3)
			{
				rank++;
				players[id].standings = rank;
				cout << "Player " << id + 1 << " came " << players[id].standings << " in the race" << endl;
				players[id].isFinished = true;
			}
		}
	}

		
	if (displayresult)
	{
		result.setPosition((cameraView.getCenter() - sf::Vector2f(Swidth / 2.29, Sheight / 600)));
	}
	collision(dt);
}
void Level::collision(float dt)
{
	// collision detection
	// get the head of the contact list
	b2Contact* contact = world_->GetContactList();
	// get contact count
	int contact_count = world_->GetContactCount();

	for (int contact_num = 0; contact_num < contact_count; ++contact_num)
	{
		if (contact->IsTouching())
		{
			

			auto fA = contact->GetFixtureA();
			auto fB = contact->GetFixtureB();

			auto bA = fA->GetBody();
			auto bB = fB->GetBody();

			b2BodyUserData budA = bA->GetUserData();
			b2BodyUserData budB = bB->GetUserData();

			//cout << budA.pointer <<":" << budB.pointer << std::endl;
			
			//if player collides with the wall
			if ((budA.pointer == 0 && budB.pointer == playerID+1) || (budA.pointer == playerID+1 && budB.pointer == 0))
			{
				
				if (budA.pointer == 1)
				{

					players[playerID].car.move(-(players[playerID].movementVec * players[playerID].currentSpeed * dt));
				}
				if (budB.pointer == 1)
				{
					players[playerID].car.move(-(players[playerID].movementVec * players[playerID].currentSpeed * dt));

				}

			}
			//coin collection
			if ((budA.pointer == playerID + 1 && budB.pointer == 99) || (budA.pointer == 99 && budB.pointer == playerID + 1))
			{

				players[playerID].coinCount++;
				break;
			}


			//checkpoints detection to monitor number of laps 
			for (int i = 0; i < 10; i++)
			{
				if ((budA.pointer == playerID + 1 && budB.pointer == (100 + i)) || (budA.pointer == (100 + i) && budB.pointer == playerID + 1))
				{
					if (players[playerID].checkpointsCount == i)
						{
							players[playerID].checkpointsCount++;
							break;
						}
					
					if (players[playerID].checkpointsCount == 10)
					{
						players[playerID].checkpointsCount++;
						players[playerID].lapCount++;
						if (!players[playerID].isFinished) {
							if (players[playerID].lapCount == 3)
							{
								rank++;
								players[playerID].standings = rank;
								std::string Sresult;
								Sresult.append("           you came  ");
								Sresult.append(std::to_string(players[playerID].standings));
								Sresult.append("  in the race");
								result.setString(Sresult);
								displayresult = true;
								cout << "You came " << players[playerID].standings << " in the race" << endl;
								players[playerID].isFinished = true;
							}
						}
					
						players[playerID].checkpointsCount = 0;
						
					}
					break;
				}
			}
		}
	}

}
// Render level
void Level::render()
{
	beginDraw();
	window->draw(map);

	for (int i = 0; i < 4; i++)
	{
		window->draw(players[i].car);		
	}
	window->draw(laps_Text);
	window->draw(laps_Count);

	window->draw(coinSprite);
	window->draw(coins_count);

	window->draw(mushroomSprite);
	window->draw(mushroom_count);
	
	for (int i = 0; i < 10; i++)
	{//window->draw(checkpoints[i]);
	}
	window->setView(cameraView);

	if (displayresult)
	{
		window->draw(result);
	}

	endDraw();
}


float Level::dot(sf::Vector2f first, sf::Vector2f second)
{
	float result;
	result = (first.x * second.x) + (first.y * second.y);
	return result;
}

void Level::InitPlayers()
{
	//box2d World initialisation
	b2Vec2 gravity(0.0f, 1.f);
	world_ = new b2World(gravity);

	///////////PLAYERS ONE INITIALISATION///////////
	players[0].carText.loadFromFile("images/race/mareo.png");
	players[0].carText.setSmooth(true);
	players[0].car.setTexture(players[0].carText);
	players[0].car.setPosition(128, 917);
	players[0].car_collisionBox.setSize(sf::Vector2f(players[0].carText.getSize().x, players[0].carText.getSize().y));
	players[0].car_collisionBox.setOutlineColor(sf::Color::Blue);
	players[0].car_collisionBox.setFillColor(sf::Color::Transparent);
	players[0].car_collisionBox.setOutlineThickness(3.0f);
	players[0].car_collisionBox.setPosition(players[0].car.getPosition());

	//box 2d initialisation
	b2BodyUserData playerUserData[4];
	playerUserData[0].pointer = 1;

	b2BodyDef playerDef[4];
	playerDef[0].type = b2_dynamicBody;
	playerDef[0].position = b2Vec2(players[0].car.getPosition().x, players[0].car.getPosition().y);
	playerDef[0].userData = playerUserData[0];
	players[0].player_ = world_->CreateBody(&playerDef[0]);

	b2PolygonShape playerBox[4];
	playerBox[0].SetAsBox((players[0].carText.getSize().x) / 2, (players[0].carText.getSize().y) / 2);

	b2FixtureDef fixtureDef[4];
	fixtureDef[0].shape = &playerBox[0];
	fixtureDef[0].density = 1.0f;
	fixtureDef[0].friction = 0.0f;
	 players[0].player_->CreateFixture(&fixtureDef[0]);



	///////////PLAYERS TWO INITIALISATION///////////
	players[1].carText.loadFromFile("images/race/luigi.png");
	players[1].carText.setSmooth(true);
	players[1].car.setTexture(players[1].carText);
	players[1].car.setPosition(63, 917);
	players[1].car_collisionBox.setSize(sf::Vector2f(players[1].carText.getSize().x, players[1].carText.getSize().y));
	players[1].car_collisionBox.setOutlineColor(sf::Color::Blue);
	players[1].car_collisionBox.setFillColor(sf::Color::Transparent);
	players[1].car_collisionBox.setOutlineThickness(3.0f);
	players[1].car_collisionBox.setPosition(players[1].car.getPosition());

	//box 2d initialisation
	playerUserData[1].pointer = 2;

	playerDef[1].type = b2_dynamicBody;
	playerDef[1].position = b2Vec2(players[1].car.getPosition().x, players[1].car.getPosition().y);
	playerDef[1].userData = playerUserData[1];
	players[1].player_ = world_->CreateBody(&playerDef[1]);

	playerBox[1].SetAsBox((players[1].carText.getSize().x) / 2, (players[1].carText.getSize().y) / 2);

	fixtureDef[1].shape = &playerBox[1];
	fixtureDef[1].density = 1.0f;
	fixtureDef[1].friction = 0.0f;
	 players[1].player_->CreateFixture(&fixtureDef[1]);


	 ///////////PLAYERS THREE INITIALISATION///////////
	 players[2].carText.loadFromFile("images/race/wario.png");
	 players[2].carText.setSmooth(true);
	 players[2].car.setTexture(players[2].carText);
	 players[2].car.setPosition(63, 917+ players[2].carText.getSize().y);
	 players[2].car_collisionBox.setSize(sf::Vector2f(players[2].carText.getSize().x, players[2].carText.getSize().y));
	 players[2].car_collisionBox.setOutlineColor(sf::Color::Blue);
	 players[2].car_collisionBox.setFillColor(sf::Color::Transparent);
	 players[2].car_collisionBox.setOutlineThickness(3.0f);
	 players[2].car_collisionBox.setPosition(players[2].car.getPosition());

	 //box 2d initialisation
	 playerUserData[2].pointer = 3;

	 playerDef[2].type = b2_dynamicBody;
	 playerDef[2].position = b2Vec2(players[2].car.getPosition().x, players[2].car.getPosition().y);
	 playerDef[2].userData = playerUserData[2];
	 players[2].player_ = world_->CreateBody(&playerDef[2]);

	 playerBox[2].SetAsBox((players[2].carText.getSize().x) / 2, (players[2].carText.getSize().y) / 2);

	 fixtureDef[2].shape = &playerBox[2];
	 fixtureDef[2].density = 1.0f;
	 fixtureDef[2].friction = 0.0f;
	 players[2].player_->CreateFixture(&fixtureDef[2]);


	 ///////////PLAYERS FOUR INITIALISATION///////////
	 players[3].carText.loadFromFile("images/race/bowser.png");
	 players[3].carText.setSmooth(true);
	 players[3].car.setTexture(players[3].carText);
	 players[3].car.setPosition(128, 917+ players[3].carText.getSize().y);
	 players[3].car_collisionBox.setSize(sf::Vector2f(players[3].carText.getSize().x, players[3].carText.getSize().y));
	 players[3].car_collisionBox.setOutlineColor(sf::Color::Blue);
	 players[3].car_collisionBox.setFillColor(sf::Color::Transparent);
	 players[3].car_collisionBox.setOutlineThickness(3.0f);
	 players[3].car_collisionBox.setPosition(players[3].car.getPosition());

	 //box 2d initialisation
	 playerUserData[3].pointer = 4;

	 playerDef[3].type = b2_dynamicBody;
	 playerDef[3].position = b2Vec2(players[3].car.getPosition().x, players[3].car.getPosition().y);
	 playerDef[3].userData = playerUserData[3];
	 players[3].player_ = world_->CreateBody(&playerDef[3]);

	 playerBox[3].SetAsBox((players[3].carText.getSize().x) / 2, (players[3].carText.getSize().y) / 2);

	 fixtureDef[3].shape = &playerBox[3];
	 fixtureDef[3].density = 1.0f;
	 fixtureDef[3].friction = 0.0f;
	 players[3].player_->CreateFixture(&fixtureDef[3]);
}
void Level::InitWalls()
{
	//Top wall
	walls[0].setSize(sf::Vector2f((mapText.getSize().x) * 2, 1));
	walls[0].setOutlineColor(sf::Color::Red);
	walls[0].setFillColor(sf::Color::Red);
	walls[0].setOutlineThickness(1.f);
	walls[0].setPosition(0.0, -100.0);

	//Bottom Wall
	walls[1].setSize(sf::Vector2f((mapText.getSize().x) * 2, 1));
	walls[1].setOutlineColor(sf::Color::Red);
	walls[1].setFillColor(sf::Color::Red);
	walls[1].setOutlineThickness(1.f);
	walls[1].setPosition(0.0, mapText.getSize().y * 2 + 100);

	//Left Wall
	walls[2].setSize(sf::Vector2f(1, (mapText.getSize().y) * 2));
	walls[2].setOutlineColor(sf::Color::Red);
	walls[2].setFillColor(sf::Color::Red);
	walls[2].setOutlineThickness(1.f);
	walls[2].setPosition(-100, 0.0);

	//Right Wall
	walls[3].setSize(sf::Vector2f(1, (mapText.getSize().y) * 2));
	walls[3].setOutlineColor(sf::Color::Red);
	walls[3].setFillColor(sf::Color::Red);
	walls[3].setOutlineThickness(1.f);
	walls[3].setPosition(mapText.getSize().x * 2 + 100, 0.0);

	//cout<<walls[0].getSize().x << ":" << walls[2].getSize().y << std::endl;
}
void Level::b2Init()
{

	/////Walls/////
	b2BodyDef wallsBodyDef[4];
	b2PolygonShape wallBox[4];
	b2BodyUserData wallsUserData;
	wallsUserData.pointer = 0;

	b2FixtureDef wallsFixtureDef;
	wallsFixtureDef.isSensor = true;
	for (int i = 0; i < 4; i++)
	{
		wallsBodyDef[i].type = b2_staticBody;
		wallsBodyDef[i].position = b2Vec2(walls[i].getPosition().x, walls[i].getPosition().y);
		wallsBodyDef[i].userData = wallsUserData;
		walls_[i] = world_->CreateBody(&wallsBodyDef[i]);
		wallBox[i].SetAsBox(((walls[i].getSize().x)), ((walls[i].getSize().y)));
		wallsFixtureDef.shape = &wallBox[i];
		walls_[i]->CreateFixture(&wallsFixtureDef);
	}
	//checkPoints
	b2BodyUserData checkpointUserData[10];
	for (int i = 0; i < 10; i++)
	{
		checkpointUserData[i].pointer = 100 + i;
	}

	b2PolygonShape checkpointShape[10];
	b2BodyDef checkpointBodyDef[10];
	b2FixtureDef checkpointFixtureDef[10];

	//first checkpoint
	checkpointBodyDef[0].type = b2_staticBody;
	checkpointBodyDef[0].userData = checkpointUserData[0];
	checkpointBodyDef[0].position = b2Vec2(0, 847);
	checkpoints_[0] = world_->CreateBody(&checkpointBodyDef[0]);
	checkpointShape[0].SetAsBox(300, 16);
	checkpointFixtureDef[0].shape = &checkpointShape[0];
	checkpointFixtureDef[0].isSensor = true;
	checkpoints_[0]->CreateFixture(&checkpointFixtureDef[0]);

	//second checkpoint
	checkpointBodyDef[1].type = b2_staticBody;
	checkpointBodyDef[1].userData = checkpointUserData[1];
	checkpointBodyDef[1].position = b2Vec2(713, 0);
	checkpoints_[1] = world_->CreateBody(&checkpointBodyDef[1]);
	checkpointShape[1].SetAsBox(40, 300);
	checkpointFixtureDef[1].shape = &checkpointShape[1];
	checkpointFixtureDef[1].isSensor = true;
	checkpoints_[1]->CreateFixture(&checkpointFixtureDef[1]);

	//third checkpoint
	checkpointBodyDef[2].type = b2_staticBody;
	checkpointBodyDef[2].userData = checkpointUserData[2];
	checkpointBodyDef[2].position = b2Vec2(1168, 461);
	checkpoints_[2] = world_->CreateBody(&checkpointBodyDef[2]);
	checkpointShape[2].SetAsBox(720, 32);
	checkpointFixtureDef[2].shape = &checkpointShape[2];
	checkpointFixtureDef[2].isSensor = true;
	checkpoints_[2]->CreateFixture(&checkpointFixtureDef[2]);
	//fourth checkpoint
	checkpointBodyDef[3].type = b2_staticBody;
	checkpointBodyDef[3].userData = checkpointUserData[3];
	checkpointBodyDef[3].position = b2Vec2(970, 672);
	checkpoints_[3] = world_->CreateBody(&checkpointBodyDef[3]);
	checkpointShape[3].SetAsBox(32, 350);
	checkpointFixtureDef[3].shape = &checkpointShape[3];
	checkpointFixtureDef[3].isSensor = true;
	checkpoints_[3]->CreateFixture(&checkpointFixtureDef[3]);
	//fifth checkpoint 
	checkpointBodyDef[4].type = b2_staticBody;
	checkpointBodyDef[4].userData = checkpointUserData[4];
	checkpointBodyDef[4].position = b2Vec2(422, 1068);
	checkpoints_[4] = world_->CreateBody(&checkpointBodyDef[4]);
	checkpointShape[4].SetAsBox(438, 34);
	checkpointFixtureDef[4].shape = &checkpointShape[4];
	checkpointFixtureDef[4].isSensor = true;
	checkpoints_[4]->CreateFixture(&checkpointFixtureDef[4]);
	//sixth checkpoint
	checkpointBodyDef[5].type = b2_staticBody;
	checkpointBodyDef[5].userData = checkpointUserData[5];
	checkpointBodyDef[5].position = b2Vec2(1211, 1100);
	checkpoints_[5] = world_->CreateBody(&checkpointBodyDef[5]);
	checkpointShape[5].SetAsBox(32, 460);
	checkpointFixtureDef[5].shape = &checkpointShape[5];
	checkpointFixtureDef[5].isSensor = true;
	checkpoints_[5]->CreateFixture(&checkpointFixtureDef[5]);
	//seventh checkpoint
	checkpointBodyDef[6].type = b2_staticBody;
	checkpointBodyDef[6].userData = checkpointUserData[6];
	checkpointBodyDef[6].position = b2Vec2(1692, 1582);
	checkpoints_[6] = world_->CreateBody(&checkpointBodyDef[6]);
	checkpointShape[6].SetAsBox(350, 32);
	checkpointFixtureDef[6].shape = &checkpointShape[6];
	checkpointFixtureDef[6].isSensor = true;
	checkpoints_[6]->CreateFixture(&checkpointFixtureDef[6]);
	//eighth checkpoint 
	checkpointBodyDef[7].type = b2_staticBody;
	checkpointBodyDef[7].userData = checkpointUserData[7];
	checkpointBodyDef[7].position = b2Vec2(1020, 1665);
	checkpoints_[7] = world_->CreateBody(&checkpointBodyDef[7]);
	checkpointShape[7].SetAsBox(46, 380);
	checkpointFixtureDef[7].shape = &checkpointShape[7];
	checkpointFixtureDef[7].isSensor = true;
	checkpoints_[7]->CreateFixture(&checkpointFixtureDef[7]);
	//ninth checkpoint
	checkpointBodyDef[8].type = b2_staticBody;
	checkpointBodyDef[8].userData = checkpointUserData[8];
	checkpointBodyDef[8].position = b2Vec2(0, 1566);
	checkpoints_[8] = world_->CreateBody(&checkpointBodyDef[8]);
	checkpointShape[8].SetAsBox(366, 30);
	checkpointFixtureDef[8].shape = &checkpointShape[8];
	checkpointFixtureDef[8].isSensor = true;
	checkpoints_[8]->CreateFixture(&checkpointFixtureDef[8]);
	//tenth checkpoint
	checkpointBodyDef[9].type = b2_staticBody;
	checkpointBodyDef[9].userData = checkpointUserData[9];
	checkpointBodyDef[9].position = b2Vec2(0, 890);
	checkpoints_[9] = world_->CreateBody(&checkpointBodyDef[9]);
	checkpointShape[9].SetAsBox(366, 30);
	checkpointFixtureDef[9].shape = &checkpointShape[9];
	checkpointFixtureDef[9].isSensor = true;
	checkpoints_[9]->CreateFixture(&checkpointFixtureDef[9]);


	//coins 
	b2BodyUserData coinUserData;
	coinUserData.pointer = 99;

	b2PolygonShape coinShape;
	coinShape.SetAsBox(14, 14);

	b2FixtureDef coinFixtureDef;
	coinFixtureDef.shape = &coinShape;
	coinFixtureDef.isSensor = true;


	b2BodyDef coinBodyDef[27];
	int j = 0;

	for (int i = 0; i < 9; i++)
	{
		coinBodyDef[j].position = b2Vec2(coinPos[i].x, coinPos[i].y);
		coinBodyDef[j + 1].position = b2Vec2(coinPos[i].x + coinOffset[i].x, coinPos[i].y + coinOffset[i].y);
		coinBodyDef[j + 2].position = b2Vec2(coinPos[i].x + (coinOffset[i].x * 2), coinPos[i].y + (coinOffset[i].y * 2));
		j += 3;
	}
	for (int i = 0; i < 27; i++)
	{
		coinBodyDef[i].type = b2_staticBody;
		coinBodyDef[i].userData = coinUserData;
		coins_[i] = world_->CreateBody(&coinBodyDef[i]);
		coins_[i]->CreateFixture(&coinFixtureDef);
	}

}
void Level::initCheckpoints()
{
	//first checpoint
	checkpoints[0].setSize(sf::Vector2f(300, 16));
	checkpoints[0].setOutlineColor(sf::Color::Black);
	checkpoints[0].setFillColor(sf::Color::Red);
	checkpoints[0].setOutlineThickness(1.0f);
	checkpoints[0].setPosition(checkpoints_[0]->GetPosition().x, checkpoints_[0]->GetPosition().y);

	//second checkpoint
	checkpoints[1].setSize(sf::Vector2f(40, 300));
	checkpoints[1].setOutlineColor(sf::Color::Black);
	checkpoints[1].setFillColor(sf::Color::Red);
	checkpoints[1].setOutlineThickness(1.0f);
	checkpoints[1].setPosition(checkpoints_[1]->GetPosition().x, checkpoints_[1]->GetPosition().y);

	//third checkpoint
	checkpoints[2].setSize(sf::Vector2f(720, 32));
	checkpoints[2].setOutlineColor(sf::Color::Black);
	checkpoints[2].setFillColor(sf::Color::Red);
	checkpoints[2].setOutlineThickness(1.0f);
	checkpoints[2].setPosition(checkpoints_[2]->GetPosition().x, checkpoints_[2]->GetPosition().y);

	//fourth checkpoint 
	checkpoints[3].setSize(sf::Vector2f(32, 350));
	checkpoints[3].setOutlineColor(sf::Color::Black);
	checkpoints[3].setFillColor(sf::Color::Red);
	checkpoints[3].setOutlineThickness(1.0f);
	checkpoints[3].setPosition(checkpoints_[3]->GetPosition().x, checkpoints_[3]->GetPosition().y);

	//fifth checkpoint 
	checkpoints[4].setSize(sf::Vector2f(438, 34));
	checkpoints[4].setOutlineColor(sf::Color::Black);
	checkpoints[4].setFillColor(sf::Color::Red);
	checkpoints[4].setOutlineThickness(1.0f);
	checkpoints[4].setPosition(checkpoints_[4]->GetPosition().x, checkpoints_[4]->GetPosition().y);

	//sixth checkpoint
	checkpoints[5].setSize(sf::Vector2f(32, 460));
	checkpoints[5].setOutlineColor(sf::Color::Black);
	checkpoints[5].setFillColor(sf::Color::Red);
	checkpoints[5].setOutlineThickness(1.0f);
	checkpoints[5].setPosition(checkpoints_[5]->GetPosition().x, checkpoints_[5]->GetPosition().y);

	//seventh checkpoint 
	checkpoints[6].setSize(sf::Vector2f(350, 32));
	checkpoints[6].setOutlineColor(sf::Color::Black);
	checkpoints[6].setFillColor(sf::Color::Red);
	checkpoints[6].setOutlineThickness(1.0f);
	checkpoints[6].setPosition(checkpoints_[6]->GetPosition().x, checkpoints_[6]->GetPosition().y);

	//eigth checkpoint
	checkpoints[7].setSize(sf::Vector2f(46, 380));
	checkpoints[7].setOutlineColor(sf::Color::Black);
	checkpoints[7].setFillColor(sf::Color::Red);
	checkpoints[7].setOutlineThickness(1.0f);
	checkpoints[7].setPosition(checkpoints_[7]->GetPosition().x, checkpoints_[7]->GetPosition().y);

	//ninth checkpoint
	checkpoints[8].setSize(sf::Vector2f(366, 30));
	checkpoints[8].setOutlineColor(sf::Color::Black);
	checkpoints[8].setFillColor(sf::Color::Red);
	checkpoints[8].setOutlineThickness(1.0f);
	checkpoints[8].setPosition(checkpoints_[8]->GetPosition().x, checkpoints_[8]->GetPosition().y);

	//tenth checkpoint
	checkpoints[9].setSize(sf::Vector2f(366, 30));
	checkpoints[9].setOutlineColor(sf::Color::Black);
	checkpoints[9].setFillColor(sf::Color::Red);
	checkpoints[9].setOutlineThickness(1.0f);
	checkpoints[9].setPosition(checkpoints_[9]->GetPosition().x, checkpoints_[9]->GetPosition().y);




}
void Level::InitCoins()
{
	for (int i = 0; i < 27; i++)
	{
		coins[i].setSize(sf::Vector2f(14, 14));
		coins[i].setOutlineColor(sf::Color::Black);
		coins[i].setFillColor(sf::Color::Red);
		coins[i].setOutlineThickness(1.0f);
		coins[i].setPosition(coins_[i]->GetPosition().x, coins_[i]->GetPosition().y);
	}
}
// Begins rendering to the back buffer. Background colour set to light blue.
void Level::beginDraw()
{
	window->clear(sf::Color(0, 0, 0));
}

// Ends rendering to the back buffer, and swaps buffer to the screen.
void Level::endDraw()
{
	window->display();
}



