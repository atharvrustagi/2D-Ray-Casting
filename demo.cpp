#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>


sf::Color light_c (sf::Color(255, 255, 0)),
          bg_c    (sf::Color(25, 25, 25));
sf::RenderWindow win(sf::VideoMode(500, 500), "Ray Casting");
std::vector<sf::ConvexShape> objects;
const sf::Vector2f ws(win.getSize());
float line_thickness = 3.f;

bool load_cursor(sf::Cursor &cursor)    {
    sf::Image img;
    img.loadFromFile("assets/light_r.png");
    return cursor.loadFromPixels(img.getPixelsPtr(), img.getSize(), {img.getSize().x / 2, img.getSize().y / 2});
}

sf::Vector2f point_map[] = {
    {ws.x, 0},
    {ws.x, ws.y},
    {0, ws.y},
    {0, 0}
};

// possible optimisations:
//     make booleans for c and m if they are negative, and checking can be done without divisions
// shape of the shadow to be casted
sf::ConvexShape shadow_cast(const sf::Vector2f &p1, const sf::Vector2f &p2, const sf::Vector2f &cursor)   {
    sf::ConvexShape shape(6);
    shape.setFillColor(bg_c);
    float d1, d2, m, c;
    enum {U, R, D, L};
    short edge[2], e = 0;
    // find intersection of line with the screen borders
    auto intersection = [&]()  {
        sf::Vector2f point;
        // possible left border
        if (d1 < 0) {
            // upper border
            if (d2 < 0 && (-c / m) >= 0)    {
                point.x = (-c / m);
                point.y = 0;
                edge[e++] = U;
            }
            // lower border
            else if (d2 >= 0 && (ws.y - c) / m >= 0)  {
                point.x = (ws.y - c) / m;
                point.y = ws.y;
                edge[e++] = D;
            }
            // left border
            else    {
                point.x = 0;
                point.y = c;
                edge[e++] = L;
            }
        }
        // possible right border
        else    {
            // upper border
            if (d2 < 0 && (-c / m) <= ws.x)    {
                point.x = (-c / m);
                point.y = 0;
                edge[e++] = U;
            }
            // lower border
            else if (d2 >= 0 && (ws.y - c) / m <= ws.x)  {
                point.x = (ws.y - c) / m;
                point.y = ws.y;
                edge[e++] = D;
            }
            // right border
            else    {
                point.x = ws.x;
                point.y = ws.x * m + c;
                edge[e++] = R;
            }
        }
        return point;
    };

    // for point p1
    d2 = p1.y - cursor.y;
    d1 = p1.x - cursor.x;
    // slope and constant of the line
    m = d2 / (1e-7 + d1);
    c = p1.y - m * p1.x;
    sf::Vector2f insec1 = intersection();

    // for point p2
    d2 = p2.y - cursor.y;
    d1 = p2.x - cursor.x;
    // slope and constant of the line
    m = d2 / (1e-7 + d1);
    c = p2.y - m * p2.x;
    sf::Vector2f insec2 = intersection();

    // set the points
    shape.setPoint(0, p1);
    shape.setPoint(1, insec1);
    shape.setPoint(4, insec2);
    shape.setPoint(5, p2);
    
    // calculate points 2 and 3 through border info of insec1 and insec2
    if (edge[0] != edge[1]) {
        short e0 = edge[0], e1 = edge[1];
        if (e0 > e1) std::swap(e0, e1);
        // adjacent edges, both vertices 2 and 3 will be equal
        if (e0 + 1 == e1)   {
            e = (e0 + e1) >> 1;
            shape.setPoint(2, point_map[e]);
            shape.setPoint(3, point_map[e]);
        }
        // adjacent special case
        else if (e0 == U && e1 == L)  {
            shape.setPoint(2, point_map[3]);
            shape.setPoint(3, point_map[3]);
        }
        // opposite edges
        else    {
            // find the center of the line
            float cx = (p1.x + p2.x) / 2, cy = (p1.y + p2.y) / 2;
            // if left and right edge
            if (e0 & 1)    {
                // cast shadow up
                if (cursor.y > cy) {
                    shape.setPoint(2, point_map[(edge[0] == 1 ? 0 : 3)]);
                    shape.setPoint(3, point_map[(edge[1] == 1 ? 0 : 3)]);
                }
                // cast shadow down
                else    {
                    shape.setPoint(2, point_map[(edge[0] == 1 ? 1 : 2)]);
                    shape.setPoint(3, point_map[(edge[1] == 1 ? 1 : 2)]);
                }
            }
            // up and down edge
            else    {
                // cast shadow left
                if (cursor.x > cx) {
                    shape.setPoint(2, point_map[(edge[0] ? 2 : 3)]);
                    shape.setPoint(3, point_map[(edge[1] ? 2 : 3)]);
                }
                // cast shadow right
                else    {
                    shape.setPoint(2, point_map[(edge[0] ? 1 : 0)]);
                    shape.setPoint(3, point_map[(edge[1] ? 1 : 0)]);
                }
            }
        }
    }
    // on the same edge
    else    {
        shape.setPoint(2, shape.getPoint(4));
        shape.setPoint(3, shape.getPoint(1));
    }
    return shape;
}

// ray casting
void ray_cast(sf::RenderWindow &win, const sf::Vector2f &cursor_position)    {
    for (sf::ConvexShape &obj: objects)
        win.draw(obj);
    // initially everything is lit up
    win.clear(light_c);
    // for every line in obstacles, draw shadows
    for (sf::ConvexShape &obj: objects) {
        int num_vertices = obj.getPointCount();
        // if only 2 vertices are there, only 1 iteration is required
        if (num_vertices < 3)   {
            sf::ConvexShape shadow = shadow_cast(obj.getPoint(0), obj.getPoint(1), cursor_position);
            win.draw(shadow);
        }
        else    {
            // cast shadows for all adjacent vertex lines
            for (int i=1; i<num_vertices; ++i)  {
                sf::ConvexShape shadow = shadow_cast(obj.getPoint(i-1), obj.getPoint(i), cursor_position);
                win.draw(shadow);
            }
            sf::ConvexShape shadow = shadow_cast(obj.getPoint(0), obj.getPoint(num_vertices-1), cursor_position);
            win.draw(shadow);
        }
    }
}

void add_object()   {
    objects.push_back(sf::ConvexShape(4));
    objects.back().setPoint(0, {100, 100});
    objects.back().setPoint(1, {200, 100});
    objects.back().setPoint(2, {200, 102});
    objects.back().setPoint(3, {100, 102});
    objects.back().setFillColor(sf::Color::Black);
    objects.back().setOutlineColor(sf::Color::White);    

    // objects.push_back(sf::VertexArray(sf::Lines, 2));
    // objects.back()[0].position = {200, 200};
    // objects.back()[1].position = {300, 300};
    // objects.back()[0].color = sf::Color::White;
    // objects.back()[1].color = sf::Color::White;

    objects.push_back(sf::ConvexShape(4));
    objects.back().setPoint(0, {200, 200});
    objects.back().setPoint(1, {300, 200});
    objects.back().setPoint(2, {300, 300});
    objects.back().setPoint(3, {200, 300});
    objects.back().setFillColor(sf::Color::Black);
    objects.back().setOutlineColor(sf::Color::White);    
}


int main()  {
    // printf("%f %f\n", ws.x, ws.y);
    sf::Event event;
    // start the clock
    sf::Clock clock;
    // sleep time between each frame
    sf::Time sleep_duration = sf::milliseconds(10);
    // cursor settings
    sf::Cursor cursor;
    if (load_cursor(cursor))
        win.setMouseCursor(cursor);
    add_object();
    // main loop
    while (win.isOpen())    {
        while (win.pollEvent(event))
            // check if the window has been closed
            if (event.type == sf::Event::Closed)
                win.close();
        ray_cast(win, sf::Vector2f(sf::Mouse::getPosition(win)));
        // printf("%d %d\n", sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
        win.display();
        sf::sleep(sleep_duration);
    }
    return 0;
}

