#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <math.h>

class ReadingInfo
{
    public:
        sf::Vector2i position;
        double tempValue;
        long creationTime; /// Time in ms

        ReadingInfo(sf::Vector2i p, double t)
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

        void addValue(sf::Vector2i inPos, double inTemp)
        {
            list.emplace_back(inPos, inTemp);
        }

        void fillWindow(cv::Mat_<double> &matRef, cv::Rect region)
        {
            std::cout << "--------------------------------" << std::endl;
            double rowOffset = ((double)region.height)/((double)matRef.rows);
            double colOffset = ((double)region.width)/((double)matRef.cols);
            for(int row = 0; row < matRef.rows; row++)
            {
                for(int col = 0; col < matRef.cols; col++)
                {
                    double sum = 0;
                    int num = 0;
                    for (ReadingInfo r : list)
                    {
                        cv::Rect_<double> cell(region.x + colOffset*col, region.y +
                            rowOffset*row, colOffset, rowOffset);
                        cv::Rect_<double> reading(r.position.x-10, r.position.y-10,
                            20, 20);
                        if((cell & reading).area() != 0) 
                        {
                            sum += r.tempValue;
                            num++;
                        }
                    }
                    sum = (num == 0 ? 0 : sum/num);
                    std::cout << "Row: " << row << " Col: "  << col
                    << " Avg: " <<  sum << std::endl;
                    matRef(row,col) = sum;
                }
            }
        }

        void copyMatToTexture(const cv::Mat &mat, sf::Texture &tex, 
                                                        bool toGray) 
        {
            if(mat.total() == 0) { return; }
            
            sf::Image tempImage;
            std::vector<unsigned char> imgData;
            imgData.resize(mat.total()*4);
            
            cv::Mat continuousRBGA(mat.size(), CV_8UC4, &imgData[0]);

            cv::cvtColor(mat, continuousRBGA,
                        (toGray ? CV_BGR2GRAY : CV_BGR2RGBA), 4); 
            
            tempImage.create(mat.cols, mat.rows, &imgData[0]);
            
            tex.loadFromImage(tempImage);
        }
};

void calcReading(SuperSampler &ss, sf::Vector2i pos)
{
    double sum = 0;
    int num = 0;
    for (ReadingInfo r : ss.list)
    {
        if ((abs(pos.x - r.position.x) < 10) && (abs(pos.y - r.position.y) < 10))
        {
            sum += r.tempValue;
            num++;
        }
    }
    sum = (num == 0 ? 0 : sum/num);
    std::cout << sum << std::endl;
}

void recordPoint(SuperSampler &ss, sf::Vector2i pos)
{
    int random = rand() % 20 + 20;
    ss.addValue(pos, (double)random);
}

sf::Color hsv(int hue, float sat, float val)
{
    hue %= 360;
    while(hue<0) hue += 360;

    if(sat<0.f) sat = 0.f;
    if(sat>1.f) sat = 1.f;

    if(val<0.f) val = 0.f;
    if(val>1.f) val = 1.f;

    int h = hue/60;
    float f = float(hue)/60-h;
    float p = val*(1.f-sat);
    float q = val*(1.f-sat*f);
    float t = val*(1.f-sat*(1-f));

    switch(h)
    {
        default:
        case 0:
        case 6: return sf::Color(val*255, t*255, p*255);
        case 1: return sf::Color(q*255, val*255, p*255);
        case 2: return sf::Color(p*255, val*255, t*255);
        case 3: return sf::Color(p*255, q*255, val*255);
        case 4: return sf::Color(t*255, p*255, val*255);
        case 5: return sf::Color(val*255, p*255, q*255);
    }
}

void render()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "SuperSampler");
    window.setFramerateLimit(60);

    sf::RenderWindow window2(sf::VideoMode(640,480), "Smoothened");
    window2.setFramerateLimit(60);

    bool isDrag = false;
    sf::Vector2i dragStart;
    cv::Mat_<double> matrix(100, 100);

    sf::Mouse mouse;
    sf::Texture tex;

    SuperSampler ss;
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
                        recordPoint(ss, mouse.getPosition(window));
                    }
                    break;
                }
                case sf::Event::MouseButtonPressed:
                {
                    isDrag = true;
                    dragStart = mouse.getPosition(window);
                    break;
                }
                case sf::Event::MouseButtonReleased:
                {
                    /// This is bugged if u go bot-right -> top left
                    ss.fillWindow(matrix, cv::Rect(dragStart.x, 
                        dragStart.y, mouse.getPosition(window).x-dragStart.x,
                        mouse.getPosition(window).y-dragStart.y));

                    cv::Mat colMat(matrix.rows, matrix.cols, CV_8UC4);

                    for(int x = 0; x < colMat.cols; x++) 
                    {
                        for(int y = 0; y < colMat.rows; y++) 
                        {
                            double temp = matrix(y,x);
                            double hue = (temp-20.0)/20.0*300.0;
                            sf::Color col = hsv(hue, 1, 1);

                            colMat.at<cv::Vec4b>(y, x) = cv::Vec4b(col.r, col.g, col.b, col.a);
                        }
                    }

                    ss.copyMatToTexture(colMat, tex, false);
                    isDrag = false;
                    break;
                }
                case sf::Event::MouseMoved:
                {
                    //calcReading(ss, mouse.getPosition(window));
                    break;
                }
            }
        }

        window.clear();
        for (ReadingInfo r : ss.list)
        {
            sf::RectangleShape rect; 
            rect.setPosition (r.position.x-10, r.position.y-10);
            double hue = (r.tempValue-20.0)/20.0*300.0;
            sf::Color color = hsv(hue, 1, 1);
            rect.setFillColor(color);
            rect.setSize(sf::Vector2f(20, 20));
            window.draw(rect);
        }
        if (isDrag)
        {
            sf::RectangleShape rect;
            rect.setPosition(dragStart.x, dragStart.y);
            rect.setOutlineColor(sf::Color::Red);
            rect.setOutlineThickness(5.0);
            rect.setFillColor(sf::Color::Transparent);
            sf::Vector2f size = sf::Vector2f(mouse.getPosition(window).x - dragStart.x,
                        mouse.getPosition(window).y - dragStart.y);
            rect.setSize(size);
            window.draw(rect);
        }
        window.display();

        while (window2.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window2.close(); 
                    break;
                }
            }
        }
        
        window2.clear();

        sf::Sprite sprite;
        sprite.setTexture(tex);
        sprite.setPosition(0, 0);
        sf::Vector2u texSize = tex.getSize();
        sprite.setScale(sf::Vector2f(640.0/(double)texSize.x, 480.0/(double)texSize.y));

        window2.draw(sprite);
        window2.display();


        sf::sleep(sf::milliseconds(1));
    }
}

int main()
{
    srand(time(0));
    render();
}
