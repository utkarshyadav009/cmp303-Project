//Application based on Paul Robertson's (p.robertson@abertay.ac.uk) CMP105 SFML Framework


#include <iostream>
#include "Level.h"
#include<SFML/Network.hpp>
#include<string>
#include<chrono>
int width = 1024;
int height = 768;
using std::cout;
using std::cin;
using std::endl;

int main()
{
	//Create the window
	sf::RenderWindow window(sf::VideoMode(960,540), "CMP303"); 

	window.setFramerateLimit(60);
	// Initialise input and level objects.
	Input input;
	Level level(&window, &input,width,height);
	
	// Initialise objects for delta time
	sf::Clock clock;
	float deltaTime;
	int frame_rate;
	// Game Loop
	while (window.isOpen())
	{
		// Calculate delta time. How much time has passed 
		// since it was last calculated (in seconds) and restart the clock.
		deltaTime = clock.restart().asSeconds();

		// Handle window events.
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
				window.setView(sf::View(sf::FloatRect(0.f, 0.f, (float)event.size.width, (float)event.size.height)));
				break;
			case sf::Event::KeyPressed:
				// update input class
				input.setKeyDown(event.key.code);
				break;
			case sf::Event::KeyReleased:
				//update input class
				input.setKeyUp(event.key.code);
				break;
			case sf::Event::MouseMoved:
				//update input class
				input.setMousePosition(event.mouseMove.x, event.mouseMove.y);
				break;
			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					//update input class
					input.setMouseLDown(true);
				}
				else if (event.mouseButton.button == sf::Mouse::Right)
				{
					input.setMouseRDown(true);
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					//update input class
					input.setMouseLDown(false);
				}
				else if (event.mouseButton.button == sf::Mouse::Right)
				{
					input.setMouseRDown(false);
				}
				break;
			case sf::Event::GainedFocus:
				level.setFocused(true);
				break;
			case sf::Event::LostFocus:
				level.setFocused(false);
				break;
			default:
				// don't handle other events
				break;
			}
		}

		//calculate FPS
		sf::Time time = clock.getElapsedTime();
		frame_rate  = 1.0f / time.asSeconds();
		// Call standard game loop functions (input, update and render)
		level.handleInput(deltaTime);
		level.update(deltaTime,frame_rate);
		level.render();
		

	}

}

