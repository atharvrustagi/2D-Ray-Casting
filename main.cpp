#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
#define PI 3.141592653589793238463
#define PI2 PI * 2

sf::RenderWindow win(sf::VideoMode(1280, 720), "Ray Casting");
const sf::Vector2i win_size = sf::Vector2i(win.getSize()), grid_size = {16, 9};
// size of each block in pixels
const float block_size = win_size.x / grid_size.x;
int dir = 89;
sf::RectangleShape block({block_size, block_size});
sf::CircleShape point(10, 8);

bool load_cursor (sf::Cursor& cursor)    {
    sf::Image img;
    img.loadFromFile("assets/light_r.png");
    return cursor.loadFromPixels(img.getPixelsPtr(), img.getSize(), {img.getSize().x / 2, img.getSize().y / 2});
}

const char grid[9][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0},
    {0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0},
    {0,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// enum color_codes {
//     black,
//     white,
//     red
// };
sf::Color colors[] = {
    sf::Color::Black,
    sf::Color::White,
    sf::Color::Red,
    sf::Color::Green
};

class RayCaster {
    
  public:
    RayCaster (float maxD = 200.f)   {       
        max_dist = maxD;
    }

    /// Casts a ray from the given positon in the direction of the given angle (in degrees)
    /// \return Coordinates of the end point of the casted ray
    sf::Vector2f cast (sf::Vector2f pos, int dir)   {
        // to radians
        float radians = to_radians(dir);

        // tan(theta)
        const double tangent = tan(radians);

        // function to calculate intersections
        sf::Vector2f (RayCaster::* fptr) (const sf::Vector2f &, float, float, float);

        // check in which quadrant will the ray move
        char quad;
        if      (dir < 90)   {
            quad = 0;
            fptr = &RayCaster::quad0_dist;
        }
        else if (dir < 180)  {
            quad = 1;
            fptr = &RayCaster::quad1_dist;
        }
        else if (dir < 270)  {
            quad = 2;
            fptr = &RayCaster::quad2_dist;
        }
        else                 {
            quad = 3;
            fptr = &RayCaster::quad3_dist;
        }

        int bx = floor(pos.x / block_size), by = floor(pos.y / block_size), addx = (quad == 0 || quad == 3 ? 1 : -1), addy = (quad > 1 ? -1 : 1);
        // char edge = -1;     // last processed edge

        // round off x and y to block_size multiples
        float x = block_size * bx, y = block_size * by;

        // for adding
        float dx = block_size, dy = block_size;
        // do iterations
        while (bx >= 0 && bx < grid_size.x && by >= 0 && by < grid_size.y && !grid[by][bx])   {
            auto [vx, vy] = (this->*fptr) (pos, x, y, tangent);
            
            // check if vx is valid
            // if ((quad == 0 || quad == 3) && x <= pos.x + vx && pos.x + vx <= x + dx || (quad == 1 || quad == 2) && x + dx <= pos.x + vx && pos.x + vx <= x)    {
            if (x <= pos.x + vx && pos.x + vx <= x + dx)    {
                pos.x += vx;
                pos.y = y + dy;
                y += dy;
                by += addy;
            }
            // else vy is valid
            else    {
                pos.x = x + dx;
                pos.y += vy;
                x += dx;
                bx += addx;
            }
        }
        // adjustments according to quadrants
        // if (quad == 1)  {
        //     pos.x += block_size;
        // }
        // else if (quad == 2) {
        //     pos.x += block_size;
        //     pos.y += block_size;
        // }
        // else if (quad == 3 && edge == 0) {
        //     pos.y += block_size;
        // }

        // printf("%f %f\n", pos);
        return pos; //{max(0.f, x), max(0.f, y)};
    }
    
  private:
    float max_dist; // maximum length of ray in pixels

    // Utility functions to calculate intersection coordinates with ray, different for each quadrant
    sf::Vector2f quad0_dist (const sf::Vector2f &pos, float x, float y, float tangent)  {
        return {(y + block_size - pos.y) / tangent, (x + block_size - pos.x) * tangent};
    }

    sf::Vector2f quad1_dist (const sf::Vector2f &pos, float x, float y, float tangent)  {
        return {(y + block_size - pos.y) / tangent, (x - pos.x) * tangent};
    }

    sf::Vector2f quad2_dist (const sf::Vector2f &pos, float x, float y, float tangent)  {
        return {(y - pos.y) / tangent, (x - pos.x) * tangent};
    }

    sf::Vector2f quad3_dist (const sf::Vector2f &pos, float x, float y, float tangent)  {
        return {(y - pos.y) / tangent, (x + block_size - pos.x) * tangent};
    }

    float to_radians(int deg)   {
        return PI * deg / 180;
    }
} ray_caster;

void draw_grid ()    {
    for (int x = 0; x < grid_size.x; ++x)   {
        for (int y = 0; y < grid_size.y; ++y)   {
            block.setFillColor(colors[grid[y][x]]);
            block.setOutlineColor(colors[1]);
            block.setPosition({block_size * x, block_size * y});
            win.draw(block);
        }
    }
}

void draw_line(sf::Vector2f &p1, sf::Vector2f &p2)    {
    sf::VertexArray line(sf::Lines, 2);
    line[0].position = p1;
    line[1].position = p2;
    line[0].color = colors[3];
    line[1].color = colors[3];
    win.draw(line);
}

void render ()   {
    draw_grid();
    sf::Vector2f pos, mousePos = sf::Vector2f(sf::Mouse::getPosition(win));
    // pos = r.cast_ray(mousePos, 0);
    // point.setPosition(pos);
    // draw_line(pos, mousePos);
    // win.draw(point);

    // pos = r.cast_ray(mousePos, PI / 2);
    // point.setPosition(pos);
    // draw_line(pos, mousePos);
    // win.draw(point);

    pos = ray_caster.cast(mousePos, dir);
    point.setPosition(pos);
    draw_line(pos, mousePos);
    win.draw(point);

    // for (int ang = 1; ang < 90; ++ang) {
    //     pos = r.cast_ray(mousePos, ang + dir);
    //     point.setPosition(pos);
    //     draw_line(pos, mousePos);
    //     win.draw(point);
    // }
}

void init () {
    point.setFillColor(colors[2]);
    sf::FloatRect f = point.getLocalBounds();
    point.setOrigin({(f.top + f.height) / 2, (f.left + f.width) / 2});
    block.setOutlineThickness(2);
}

int main()  {
    // printf("%f %f\n", ws.x, ws.y);
    sf::Event event;
    // start the clock
    sf::Clock clock;
    // sleep time between each frame
    sf::Time sleep_duration = sf::milliseconds(10);
    // cursor settings
    sf::Cursor light_cursor;
    if (load_cursor(light_cursor))
        win.setMouseCursor(light_cursor);
    init();
    // main loop
    while (win.isOpen())    {
        while (win.pollEvent(event))    {
            // check if the window has been closed
            if (event.type == sf::Event::Closed)
                win.close();
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))    {
                dir -= 1;
                dir = (dir + 3600000) % 360;
                // if (dir < 0) dir += PI * 2;
                // else if (dir >= PI * 2) dir -= PI * 2;
                printf("%d ", dir);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))   {
                dir += 1;
                dir = (dir + 3600000) % 360;
                // if (dir < 0) dir += PI * 2;
                // else if (dir >= PI * 2) dir -= PI * 2;
                printf("%d ", dir);
            }
        }
        win.clear();
        render();
        win.display();
        sf::sleep(sleep_duration);
    }
    return 0;
}

