#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

class ReadingInfo
{
    public:
        cv::Vec2d position;
        double tempValue;
        long creationTime; /// Time in ms

        ReadingInfo(cv::Vec2d p, double t)
        {
            position = p;
            tempValue = t;
            creationTime = time(0) * 1000;
        }
};

class SuperSampler
{
    public:
        std::vector<ReadingInfo> list;

        void onNewValue(cv::Vec2d inPos, double inTemp)
        {
            list.emplace_back(inPos, inTemp);
        }
};

void render()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "SuperSampler");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{
    SuperSampler ss;
    cv::Vec2d pos;
    render();
}
