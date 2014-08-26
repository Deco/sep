#include <SFML/Graphics.hpp>
#include <cmath>
#include <cassert>
#include <iostream>

#define clamp(v, n, m) (std::min(std::max((v), (n)), (m)))

struct BezierControlPoint;

enum SegmentBehaviour
{
    CONSTANT,
    LINEAR,
    BEZIER
};
struct SegmentAnchor
{
    double x;
    double y;
    SegmentBehaviour kind;
    std::vector<BezierControlPoint> bezierControlPointList;
};

struct BezierControlPoint {
    double x, y; // relative to segment anchor
};

std::vector<SegmentAnchor> segmentList;


float graphSize = 800.0;

int selectedAnchorI = 0;
bool isDragging = false;
bool isDraggingControlPoint = false;
int dragAnchorI;
int dragControlPointI;
sf::Vector2f mouseDragStartCoords;
sf::Vector2f dragObjectStartCoords;

double getDist(sf::Vector2f a, sf::Vector2f b) { return sqrt(pow(a.x-b.x, 2)+pow(a.y-b.y, 2)); }

template<typename T>
int reorder(std::vector<T> &list, int i) {
    while(i > 0 && list[i].x < list[i-1].x) {
        std::swap(list[i], list[i-1]);
        i--;
    }
    while(i < list.size()-1 && list[i].x > list[i+1].x) {
        std::swap(list[i], list[i+1]);
        i++;
    }
    return i;
}

void mouseDown(sf::Event e, sf::Vector2f mousePos)
{
    auto mouseCoords = mousePos/graphSize;
    
    int closestSegmentI = 0;
    int closestControlPointI = 0;
    bool isClosestAControlPoint = false;
    double closestDist = 9999999.0;
    sf::Vector2f closestPos;
    for(int segmentAnchorI = 0; segmentAnchorI < segmentList.size(); segmentAnchorI++) {
        SegmentAnchor &segmentAnchor = segmentList[segmentAnchorI];
        
        auto currAnchorPos = sf::Vector2f(segmentAnchor.x, segmentAnchor.y);
        double dist = getDist(mouseCoords, currAnchorPos);;
        if(dist < closestDist) {
            closestDist = dist;
            closestSegmentI = segmentAnchorI;
            isClosestAControlPoint = false;
            closestPos = currAnchorPos;
        }
        if(segmentAnchor.kind == SegmentBehaviour::BEZIER) {
            SegmentAnchor &nextSegmentAnchor = segmentList[segmentAnchorI+1];
            auto nextAnchorPos = sf::Vector2f(nextSegmentAnchor.x, nextSegmentAnchor.y);
            
            for(int controlPointI = 0; controlPointI < segmentAnchor.bezierControlPointList.size(); controlPointI++) {
                BezierControlPoint &controlPoint = segmentAnchor.bezierControlPointList[controlPointI];
                auto controlPointPos = sf::Vector2f(
                    currAnchorPos.x+(nextAnchorPos.x-currAnchorPos.x)*controlPoint.x,
                    currAnchorPos.y+(nextAnchorPos.y-currAnchorPos.y)*controlPoint.y
                );
                double dist = getDist(mouseCoords, controlPointPos);
                if(dist < closestDist) {
                    closestDist = dist;
                    closestSegmentI = segmentAnchorI;
                    closestControlPointI = controlPointI;
                    isClosestAControlPoint = true;
                    closestPos = sf::Vector2f(controlPoint.x, controlPoint.y);
                }
            }
        }
    }
    isDragging = true;
    isDraggingControlPoint = isClosestAControlPoint;
    dragAnchorI = closestSegmentI;
    dragControlPointI = closestControlPointI;
    mouseDragStartCoords = mouseCoords;
    dragObjectStartCoords = closestPos;
    selectedAnchorI = dragAnchorI;
}

void mouseMoved(sf::Event e, sf::Vector2f mousePos)
{
    auto mouseCoords = mousePos/graphSize;
    if(isDragging) {
        if(isDraggingControlPoint) {
            SegmentAnchor &draggedAnchor = segmentList[dragAnchorI];
            SegmentAnchor &nextAnchor = segmentList[dragAnchorI+1];
            BezierControlPoint &draggedControlPoint = draggedAnchor.bezierControlPointList[dragControlPointI];
            auto mouseDiffInSegmentSpace = sf::Vector2f(
                (mouseCoords.x-mouseDragStartCoords.x)/(nextAnchor.x-draggedAnchor.x),
                (mouseCoords.y-mouseDragStartCoords.y)/(nextAnchor.y-draggedAnchor.y)
            );
            draggedControlPoint.x = clamp(dragObjectStartCoords.x+mouseDiffInSegmentSpace.x, 0.0f, 1.0f);
            draggedControlPoint.y = clamp(dragObjectStartCoords.y+mouseDiffInSegmentSpace.y, 0.0f, 1.0f);
            dragControlPointI = reorder(draggedAnchor.bezierControlPointList, dragControlPointI);
        } else {
            SegmentAnchor &draggedAnchor = segmentList[dragAnchorI];
            if(dragAnchorI > 0 && dragAnchorI < segmentList.size()-1) {
                draggedAnchor.x = dragObjectStartCoords.x+(mouseCoords.x-mouseDragStartCoords.x);
            }
            draggedAnchor.y = dragObjectStartCoords.y+(mouseCoords.y-mouseDragStartCoords.y);
            dragAnchorI = reorder(segmentList, dragAnchorI);
            selectedAnchorI = dragAnchorI;
        }
    }
}

void mouseUp(sf::Event e, sf::Vector2f mousePos)
{
    isDragging = false;
}

void handleInput(sf::Event e)
{
    SegmentAnchor &selectedAnchor = segmentList[selectedAnchorI];
    switch (e.key.code) {
        case sf::Keyboard::Space: {
            SegmentAnchor newAnchor;
            newAnchor.x = selectedAnchor.x+0.05;
            newAnchor.y = 0.5;
            newAnchor.kind = SegmentBehaviour::CONSTANT;
            //newAnchor.bezierControlPointList
            
            segmentList.push_back(newAnchor);
            
            int newAncorI = reorder(segmentList, segmentList.size()-1);
            selectedAnchorI = newAncorI;
            break;
        }
        
        case sf::Keyboard::A: {
            selectedAnchorI--;
            if (selectedAnchorI < 0) { selectedAnchorI = segmentList.size() - 1; }
            break;
        }
        case sf::Keyboard::D: {
            selectedAnchorI++;
            if (selectedAnchorI >= segmentList.size()) { selectedAnchorI = 0; }
            break;
        }
        case sf::Keyboard::S: {
            if(selectedAnchor.kind == SegmentBehaviour::BEZIER) {
                BezierControlPoint controlPoint;
                controlPoint.x = 0.5; controlPoint.y = 0.5;
                selectedAnchor.bezierControlPointList.push_back(controlPoint);
                int newControlPointI = selectedAnchor.bezierControlPointList.size()-1;
                newControlPointI = reorder(selectedAnchor.bezierControlPointList, newControlPointI);
            }
            break;
        }
        
        case sf::Keyboard::Num1: {
            selectedAnchor.kind = SegmentBehaviour::CONSTANT;
            break;
        }
        case sf::Keyboard::Num2: {
            selectedAnchor.kind = SegmentBehaviour::LINEAR;
            break;
        }
        case sf::Keyboard::Num3: {
            selectedAnchor.kind = SegmentBehaviour::BEZIER;
            break;
        }
        
        case sf::Keyboard::Left: {
            selectedAnchor.x = std::max(selectedAnchor.x-0.01, 0.00);
            selectedAnchorI = reorder(segmentList, selectedAnchorI);
            break;
        }
        case sf::Keyboard::Right: {
            if(selectedAnchorI == 0) {
                break;
            }
            selectedAnchor.x = std::min(selectedAnchor.x+0.01, 1.00);
            selectedAnchorI = reorder(segmentList, selectedAnchorI);
            break;
        }
        case sf::Keyboard::Up: {
            selectedAnchor.y -= 0.01;
            break;
        }
        case sf::Keyboard::Down: {
            selectedAnchor.y += 0.01;
            break;
        }
    }
}

double factorial(double k) { return (k <= 1) ? 1 : k * factorial(k-1); }

double bernstain(double i, double n, double t)
{
    assert(n >= i);
    return factorial(n) / (factorial(i) * factorial(n-i)) * pow(t, i) * pow(1.0-t, n-i);
}

sf::Vector2f bezier(double t, const std::vector<BezierControlPoint> &bezierControlPointList)
{
    sf::Vector2f result;
    int count = 2+bezierControlPointList.size();
    for(int curvePointI = 0; curvePointI < count; curvePointI++) {
        sf::Vector2f curvePoint;
        if(curvePointI == 0) {
            curvePoint = {0.0, 0.0};
        } else if(curvePointI == count-1) {
            curvePoint = {1.0, 1.0};
        } else {
            int controlPointI = curvePointI-1;
            auto &controlPoint = bezierControlPointList[controlPointI];
            curvePoint = sf::Vector2f(controlPoint.x, controlPoint.y);
        }
        result +=  curvePoint*(float)bernstain(curvePointI, count-1, t);
    }
    return result;
}


void loop()
{
    sf::RenderWindow window(sf::VideoMode(1280, 960), "Colour Diddler");
    window.setFramerateLimit(30);
    
    sf::Font font;
    font.loadFromFile("arial.ttf");
    
    sf::Text hue;
    hue.setColor(sf::Color(0,255,0));
    hue.setPosition(0,240);
    hue.setFont(font);
    hue.setString("HUE");
    hue.setCharacterSize(24);
    
    std::string str;
    
    while (window.isOpen())
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
                handleInput(event);
            else if (event.type == sf::Event::MouseButtonPressed)
                mouseDown(event, sf::Vector2f(mousePos.x, mousePos.y));
            else if (event.type == sf::Event::MouseMoved)
                mouseMoved(event, sf::Vector2f(mousePos.x, mousePos.y));
            else if (event.type == sf::Event::MouseButtonReleased)
                mouseUp(event, sf::Vector2f(mousePos.x, mousePos.y));
        }
        
        window.clear();
        
        sf::VertexArray drawingVertexList(sf::LinesStrip, 0);
        
        for(int segmentI = 0; segmentI < segmentList.size(); segmentI++) {
            auto &currAnchor = segmentList[segmentI];
            auto &nextAnchor = segmentList[segmentI+1];
            
            auto currAnchorPos = sf::Vector2f(currAnchor.x, currAnchor.y);
            auto nextAnchorPos = sf::Vector2f(nextAnchor.x, nextAnchor.y);
            
            sf::RectangleShape rect(sf::Vector2f(5, 5));
            rect.setPosition(currAnchorPos*graphSize-sf::Vector2f(2.5, 2.5));
            rect.setFillColor(segmentI == selectedAnchorI ? sf::Color(0,255,0) : sf::Color(255,0,0));
            window.draw(rect);
            
            drawingVertexList.append(currAnchorPos*graphSize);
            
            if(segmentI == segmentList.size()-1) { continue; }
            
            switch(currAnchor.kind) {
                case SegmentBehaviour::CONSTANT: {
                    auto finalPos = sf::Vector2f(nextAnchorPos.x, currAnchorPos.y);
                    drawingVertexList.append(finalPos*graphSize);
                    break;
                }
                case SegmentBehaviour::LINEAR: {
                    /*auto finalPos = sf::Vector2f(nextAnchorPos.x, nextAnchorPos.y);
                    drawingVertexList.append(finalPos*graphSize);*/
                    break;
                }
                case SegmentBehaviour::BEZIER: {
                    for(auto &cp : currAnchor.bezierControlPointList) {
                        auto cpPos = sf::Vector2f(
                            currAnchorPos.x+(nextAnchorPos.x-currAnchorPos.x)*cp.x,
                            currAnchorPos.y+(nextAnchorPos.y-currAnchorPos.y)*cp.y
                        );
                        sf::RectangleShape rect(sf::Vector2f(3, 3));
                        rect.setPosition(cpPos*graphSize-sf::Vector2f(1.5, 1.5));
                        rect.setFillColor(sf::Color(0, 0, 255));
                        window.draw(rect);
                    }
                    int curveVertexCount = 50;
                    for(int curveVertexI = 0; curveVertexI < curveVertexCount; curveVertexI++) {
                        double t = (double)curveVertexI/(double)curveVertexCount;
                        sf::Vector2f curveLocalPos = bezier(t, currAnchor.bezierControlPointList);
                        sf::Vector2f curvePos = sf::Vector2f(
                            currAnchorPos.x+(nextAnchorPos.x-currAnchorPos.x)*curveLocalPos.x,
                            currAnchorPos.y+(nextAnchorPos.y-currAnchorPos.y)*curveLocalPos.y
                        );
                        drawingVertexList.append(curvePos*graphSize);
                    }
                }
            }
        }
        window.draw(drawingVertexList);
        
        window.display();
        sf::sleep(sf::milliseconds(1));
    }
}

/*
for (int i = 0; i < selectedAnchorI.size(); i++) {
    ControlPoint c = selectedAnchorI[i];
    sf::RectangleShape rect(sf::Vector2f(5,5));
    rect.setPosition(c.position);
    rect.setFillColor(sf::Color(255,0,0));

    switch (c.type)
    {
        case 1:
            vertices.append(sf::Vertex(sf::Vector2f(selectedAnchorI[i+1].position.x, c.position.y)));
            vertices.append(sf::Vertex(selectedAnchorI[i+1].position));
            break;
        case 2:
            vertices.append(sf::Vertex(selectedAnchorI[i+1].position));
            break;
        case 3:
            std::vector<sf::Vector2f> v;
            int j = 0;
            while (selectedAnchorI[i+j].type == 3)
            {
                v.emplace_back(selectedAnchorI[i+j].position);
                j++;
            }
            i += j;
            
            int lineToDrawCount = 25;
            for(int lineToDrawI = 0; lineToDrawI < lineToDrawCount; lineToDrawI++) {
                float t = lineToDrawI/(lineToDrawCount-1);
                vertices.append(bezier(t, v));
            }
            
            break;
    }
    if (i == curSelected)
    {
        rect.setFillColor(sf::Color(0,255,0));
    }
    window.draw(rect);

}*/

int main()
{
    {
        SegmentAnchor sa;
        sa.x = 0.0;
        sa.y = 1.0;
        sa.kind = SegmentBehaviour::LINEAR;
        segmentList.push_back(sa);
    }
    /*{
        SegmentAnchor sa;
        sa.x = 0.4;
        sa.y = 0.8;
        sa.kind = SegmentBehaviour::BEZIER;
        
        BezierControlPoint cp1;
        cp1.x = 0.2; cp1.y = 0.2;
        sa.bezierControlPointList.push_back(cp1);
        
        BezierControlPoint cp2;
        cp2.x = 0.5; cp2.y = 0.8;
        sa.bezierControlPointList.push_back(cp2);
        
        BezierControlPoint cp3;
        cp3.x = 0.7; cp3.y = 0.9;
        sa.bezierControlPointList.push_back(cp3);
        
        segmentList.push_back(sa);
    }*/
    {
        SegmentAnchor sa;
        sa.x = 1.0;
        sa.y = 0.0;
        sa.kind = SegmentBehaviour::LINEAR;
        segmentList.push_back(sa);
    }
    
    loop();
    
    return 1;
}


