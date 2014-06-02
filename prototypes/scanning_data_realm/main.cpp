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

void generateImage()
{
    /// Iterate through all the list or pixels, looking for points within its range.

}

void recordPoint()
{

}

void render()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "SuperSampler");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window.close(); 
                    break;
                }
                case sf::Event::KeyPressed:
                {
                    if(event.key.code == sf::Keyboard::Escape)
                    { 
                        window.close(); 
                    }
                    else if (event.key.code == sf::Keyboard::Space)
                    {
                        generateImage();
                    }
                    break;
                }
                case sf::Event::MouseButtonPressed:
                {
                    recordPoint();
                    break;
                }
            }
        }

        window.clear();
        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{
    render();
}
