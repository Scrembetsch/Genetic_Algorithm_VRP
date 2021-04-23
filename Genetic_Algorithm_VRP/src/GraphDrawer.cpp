#include "GraphDrawer.h"

#include "Util.h"

#ifdef _WIN64
// Receive all cities (vector of points with x,y pair)
void GraphDrawer::SetPoints(const std::vector<std::pair<float, float>>& points)
{
	mPoints = points;
	mCircles = std::vector<sf::CircleShape>(points.size());
	for (size_t i = 0; i < points.size(); i++)
	{
		mCircles[i].setRadius(sRadius);
		mCircles[i].setFillColor(sf::Color::White);
		mCircles[i].setPosition(points[i].first - sRadius, points[i].second - sRadius);
	}
}

// Rescale points to fit/fill window
void GraphDrawer::RescalePoints()
{
	std::pair<float, float> min = std::make_pair(99999, 999999);
	std::pair<float, float> max = std::make_pair(0, 0);
	for (const auto& point : mPoints)
	{
		if (point.first < min.first)
		{
			min.first = point.first;
		}
		if (point.second < min.second)
		{
			min.second = point.second;
		}
		if (point.first > max.first)
		{
			max.first = point.first;
		}
		if (point.second > max.second)
		{
			max.second = point.second;
		}
	}

	for (auto& point : mPoints)
	{
		point.first = ((point.first - min.first) / (max.first - min.first)) * (mWindowWidth - 50) + 25;
		point.second = ((point.second - min.second) / (max.second - min.second)) * (mWindowHeight - 50) + 25;
	}

	for (size_t i = 0; i < mPoints.size(); i++)
	{
		mCircles[i].setPosition(mPoints[i].first - sRadius, mPoints[i].second - sRadius);
	}
}

// Receive route of every vehicle
void GraphDrawer::SetRoutes(const std::vector<std::vector<int>>& routes)
{
	mRoutes = std::vector<sf::VertexArray>(routes.size());
	for (size_t i = 0; i < routes.size(); i++)
	{
		mRoutes[i] = sf::VertexArray(sf::LineStrip, routes[i].size());
		for (size_t j = 0; j < routes[i].size(); j++)
		{
			mRoutes[i][j].position = sf::Vector2f(mPoints[routes[i][j]].first, mPoints[routes[i][j]].second);
			mRoutes[i][j].color = GetLineColor(i);
		}
	}
}


// Sets depot point
void GraphDrawer::SetDepot(size_t index)
{
	mCircles[index].setFillColor(sf::Color::Yellow);
	float radius = sRadius * 3;
	mCircles[index].setRadius(radius);
	mCircles[index].setPosition(mPoints[index].first - radius, mPoints[index].second - radius);
}

// Help function to iterate through SFML colors
const sf::Color& GraphDrawer::GetLineColor(size_t index)
{
	return sColors[Util::Clamp(index, size_t(0U), sColors.size() - 1U)];
}

// Draw all cities and roads
void GraphDrawer::Draw()
{
	sf::RenderWindow window(sf::VideoMode(mWindowWidth, mWindowHeight), "VRP");

	while (window.isOpen())
	{
		sf::Event event;

		for (const auto& route : mRoutes)
		{
			window.draw(route);
		}

		for (const auto& circle : mCircles)
		{
			window.draw(circle);
		}

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}
		window.display();
	}
}
#endif
