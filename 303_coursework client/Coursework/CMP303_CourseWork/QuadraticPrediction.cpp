#include "QuadraticPrediction.h"

QuadraticPrediction::QuadraticPrediction()
{
}


QuadraticPrediction::~QuadraticPrediction()
{
}


void QuadraticPrediction::NetworkPositionData(const Data& receivedMessage)
{
	if (NetworkDataHistory.size() >= quadratic_message_number) NetworkDataHistory.pop_back();
	NetworkDataHistory.push_front(receivedMessage);
}

void QuadraticPrediction::LocalPositionData(const Data& localMessage)
{
	if (LocalDataHistory.size() >= quadratic_message_number) LocalDataHistory.pop_back();
	LocalDataHistory.push_front(localMessage);
}

sf::Vector2f QuadraticPrediction::quadraticInterpolation(sf::Vector2f& m0_local, sf::Vector2f& m1_local, sf::Vector2f& m2_local, sf::Vector2f& m0_network, sf::Vector2f& m1_network, sf::Vector2f& m2_network, float& m0_time, float& m1_time, float& m2_time, float& time)
{
	float tm = time;

	sf::Vector2f local_path = LocalPath(
		m0_local,
		m1_local,
		m2_local,
		m0_time,
		m1_time,
		m2_time,
		tm);
	sf::Vector2f network_path = NetworkPath(m0_network,
		m1_network,
		m2_network,
		m0_time,
		m1_time,
		m2_time,
		tm);

	//lerp path works better with 100ms lag
	sf::Vector2f lerp_position = lerp(local_path, network_path, 0.1);
	// add lerped to the history of the local posistions
	
	return lerp_position;
}

sf::Vector2f QuadraticPrediction::LocalPath(sf::Vector2f& m0_local_position, sf::Vector2f& m1_local_position, sf::Vector2f& m2_local_position, float& m0_time, float& m1_time, float& m2_time, float& time)
{
	// quadratic model
	float x_average_velocity_1, y_average_velocity_1,
		x_average_velocity_2, y_average_velocity_2,
		a_x, a_y,
		x_, y_;

	// average velocity = (recieved_position - last_position) / (recieved_time - last_time)
	x_average_velocity_1 = (m0_local_position.x - m1_local_position.x) / (m0_time - m1_time);
	y_average_velocity_1 = (m0_local_position.y - m1_local_position.y) / (m0_time - m1_time);

	x_average_velocity_2 = (m1_local_position.x - m2_local_position.x) / (m1_time - m2_time);
	y_average_velocity_2 = (m1_local_position.y - m2_local_position.y) / (m1_time - m2_time);

	a_x = (x_average_velocity_2 - x_average_velocity_1) / (m2_time - m0_time);
	a_y = (y_average_velocity_2 - y_average_velocity_1) / (m2_time - m0_time);

	// s = s0 + v0t + ½at2
	x_ = m2_local_position.x + (x_average_velocity_2 * (time - m2_time)) + ((0.5 * a_x) * powf((time - m2_time), 2));
	y_ = m2_local_position.y + (y_average_velocity_2 * (time - m2_time)) + ((0.5 * a_y) * powf((time - m2_time), 2));

	sf::Vector2f local_player_pos(x_, y_);
	return local_player_pos;
}

sf::Vector2f QuadraticPrediction::NetworkPath(sf::Vector2f& m0_network_position, sf::Vector2f& m1_network_position, sf::Vector2f& m2_network_position, float& m0_time, float& m1_time, float& m2_time, float& time)
{

	float x_average_velocity_1, y_average_velocity_1,
		x_average_velocity_2, y_average_velocity_2,
		a_x, a_y,
		x_, y_;

	// average velocity = (recieved_position - last_position) / (recieved_time - last_time)
	x_average_velocity_1 = (m0_network_position.x - m1_network_position.x) / (m0_time - m1_time);
	y_average_velocity_1 = (m0_network_position.y - m1_network_position.y) / (m0_time - m1_time);

	x_average_velocity_2 = (m1_network_position.x - m2_network_position.x) / (m1_time - m2_time);
	y_average_velocity_2 = (m1_network_position.y - m2_network_position.y) / (m1_time - m2_time);

	a_x = (x_average_velocity_2 - x_average_velocity_1) / (m2_time - m0_time);
	a_y = (y_average_velocity_2 - y_average_velocity_1) / (m2_time - m0_time);

	// s = s0 + v0t + ½at2
	x_ = m2_network_position.x + (x_average_velocity_2 * (time - m2_time)) + ((0.5 * a_x) * powf((time - m2_time), 2));
	y_ = m2_network_position.y + (y_average_velocity_2 * (time - m2_time)) + ((0.5 * a_y) * powf((time - m2_time), 2));

	// porn ahah

	sf::Vector2f network_player_pos(x_, y_);
	return network_player_pos;
}


