#pragma once

#include <vector>

#ifdef _WIN64
#include <SFML/Graphics.hpp>

class GraphDrawer
{
	const float sRadius = 5.0f;
	const std::vector<sf::Color> sColors = {sf::Color::Cyan, sf::Color::Blue, sf::Color::Green, sf::Color::Magenta, sf::Color::Red, sf::Color::Yellow};
	int mWindowHeight = 720;
	int mWindowWidth = 1280;

public:
	void Draw();
	void SetPoints(const std::vector<std::pair<float, float>>& points);
	void RescalePoints();
	void SetRoutes(const std::vector<std::vector<int>>& routes);
	void SetDepot(size_t index);

private:
	const sf::Color& GetLineColor(size_t index);

	std::vector<std::pair<float, float>> mPoints;

	std::vector<sf::CircleShape> mCircles;
	std::vector<sf::VertexArray> mRoutes;
};

#endif
