#pragma once

//header file based on https://github.com/matzar/SFML-Networking for quadratic predictions 
#include<SFML/Graphics.hpp>
#include <deque>
struct Data {
	/** The object number of this Player within the game world. */
	int id;

	/** The coordinates of this Player within the game world. */
	sf::Vector2f player_position;
	
	/** The time at which this message was sent.
		(Not the time at which it was received!) */
	float time;
};

class QuadraticPrediction
{
public:
	QuadraticPrediction();
	~QuadraticPrediction();

	// number of messages to keep track of
	const unsigned int quadratic_message_number = 3;

	std::deque<Data> NetworkDataHistory;
	std::deque<Data> LocalDataHistory;

	void NetworkPositionData(const Data& receivedMessage);
	void LocalPositionData(const Data& localMessage);

	sf::Vector2f quadraticInterpolation(sf::Vector2f& m0_local,
		sf::Vector2f& m1_local,
		sf::Vector2f& m2_local,
		sf::Vector2f& m0_network,
		sf::Vector2f& m1_network,
		sf::Vector2f& m2_network,
		float& m0_time,
		float& m1_time,
		float& m2_time,
		float& time);

protected:

	sf::Vector2f LocalPath(sf::Vector2f& m0_local_position,
		sf::Vector2f& m1_local_position,
		sf::Vector2f& m2_local_position,
		float& m0_time,
		float& m1_time,
		float& m2_time,
		float& time);

	sf::Vector2f NetworkPath(sf::Vector2f& m0_network_position,
		sf::Vector2f& m1_network_position,
		sf::Vector2f& m2_network_position,
		float& m0_time,
		float& m1_time,
		float& m2_time,
		float& time);

	sf::Vector2f lerp(sf::Vector2f old, sf::Vector2f next, float timer) {
		return (1 - timer) * old + timer * next;
	}

};
