#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>
#define PI 3.141592653589793238463
#define PI2 PI * 2

sf::RenderWindow win(sf::VideoMode(1280, 720), "Ray Casting");
const sf::Vector2i win_size = sf::Vector2i(win.getSize()), grid_size = {16, 9};
// size of each block in pixels
const float block_size = win_size.x / grid_size.x;
int dir = 0;
sf::RectangleShape block({block_size, block_size});
sf::CircleShape point(8, 4);

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
    sf::Vector2f cast (sf::Vector2f pos, int angle)   {
        // tan(theta)
        const float tangent = tan(to_radians(angle));

        // check in which quadrant will the ray move
        const char quad = angle / 90;
        
        int bx = floor(pos.x / block_size), by = floor(pos.y / block_size), addx = (quad == 0 || quad == 3 ? 1 : -1), addy = (quad > 1 ? -1 : 1);
        
        // round off x and y to block_size multiples
        float x = block_size * bx, y = block_size * by;

        // for adding
        float dx = addx * block_size, dy = addy * block_size;

        while (bx >= 0 && bx < grid_size.x && by >= 0 && by < grid_size.y && !grid[by][bx])   {
            auto [vx, vy] = (this->*fptr[quad]) (pos, x, y, tangent);
            
            // check if vx is valid
            if (x <= pos.x + vx && pos.x + vx <= x + block_size)    {
                pos.x += vx;
                pos.y = y + (quad < 2 ? dy : 0);
                y += dy;
                by += addy;
            }
            // else vy is valid
            else    {
                pos.x = x + ((quad == 0 || quad == 3) ? dx : 0);
                pos.y += vy;
                x += dx;
                bx += addx;
            }
        }
        return pos;
    }

    sf::Vector2f quad_wise(sf::Vector2f pos, int dir)   {
        // to radians
        float radians = to_radians(dir);

        // tan(theta)
        const float tangent = tan(radians);

        // check in which quadrant will the ray move
        char quad;
        if      (dir < 90)   quad = 0;
        else if (dir < 180)  quad = 1;
        else if (dir < 270)  quad = 2;
        else                 quad = 3;
        
        int bx = pos.x / block_size, by = pos.y / block_size, addx = (quad == 0 || quad == 3 ? 1 : -1), addy = (quad > 1 ? -1 : 1);
        
        // round off x and y to block_size multiples
        float x = block_size * bx, y = block_size * by;

        // for adding
        float dx = addx * block_size, dy = addy * block_size;

        if (quad == 0)  {
            while (bx >= 0 && bx < grid_size.x && by >= 0 && by < grid_size.y && !grid[by][bx])   {
                float vx = (y + block_size - pos.y) / tangent, vy = (x + block_size - pos.x) * tangent;
                
                // check if vx is valid
                // if ((quad == 0 || quad == 3) && x <= pos.x + vx && pos.x + vx <= x + dx || (quad == 1 || quad == 2) && x + dx <= pos.x + vx && pos.x + vx <= x)    {
                if (x <= pos.x + vx && pos.x + vx <= x + dx)    {
                    y += dy;
                    pos.x += vx;
                    pos.y = y;
                    by += addy;
                }
                // else vy is valid
                else    {
                    x += dx;
                    pos.x = x;
                    pos.y += vy;
                    bx += addx;
                }
            }
        }
        else if (quad == 1) {
            while (bx >= 0 && by < grid_size.y && !grid[by][bx])   {
                float vx = (y + block_size - pos.y) / tangent, vy = (x - pos.x) * tangent;
                
                // check if vx is valid
                if (x <= pos.x + vx)    {
                    y += dy;
                    pos.x += vx;
                    pos.y = y;
                    by += addy;
                }
                // else vy is valid
                else    {
                    pos.x = x;
                    pos.y += vy;
                    x += dx;
                    bx += addx;
                }
            }
        }
        else if (quad == 2) {
            while (bx >= 0 && by >= 0 && !grid[by][bx])   {
                float vx = (y - pos.y) / tangent, vy = (x - pos.x) * tangent;
                
                // check if vx is valid
                if (x <= pos.x + vx)    {
                    pos.x += vx;
                    pos.y = y;
                    y += dy;
                    by += addy;
                }
                // else vy is valid
                else    {
                    pos.x = x;
                    pos.y += vy;
                    x += dx;
                    bx += addx;
                }
            }
        }
        else    {
            while (bx < grid_size.x && by >= 0 && !grid[by][bx])   {
                float vx = (y - pos.y) / tangent, vy = (x + block_size - pos.x) * tangent;
                
                // check if vx is valid
                if (pos.x + vx <= x + block_size)    {
                    pos.x += vx;
                    pos.y = y;
                    y += dy;
                    by += addy;
                }
                // else vy is valid
                else    {
                    x += dx;
                    pos.x = x;
                    pos.y += vy;
                    bx += addx;
                }
            }
        }

        return pos;
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

    // Array of utility functions
    sf::Vector2f (RayCaster::* fptr[4]) (const sf::Vector2f &, float, float, float) = {
        &RayCaster::quad0_dist, &RayCaster::quad1_dist, &RayCaster::quad2_dist, &RayCaster::quad3_dist
    };

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

    for (int ang = 0; ang < 360; ang += 2) {
        pos = ray_caster.cast(mousePos, ang);
        point.setPosition(pos);
        draw_line(pos, mousePos);
        win.draw(point);
    }
}

void init () {
    point.setFillColor(colors[2]);
    sf::FloatRect f = point.getLocalBounds();
    point.setOrigin({(f.top + f.height) / 2, (f.left + f.width) / 2});
    block.setOutlineThickness(2);
}

int main()  {
    // start the clock
    sf::Clock clock;
    // sleep time between each frame
    sf::Time sleep_duration = sf::milliseconds(10);
    // cursor settings
    sf::Cursor light_cursor;
    if (load_cursor(light_cursor))
        win.setMouseCursor(light_cursor);
    // initialise some values and shapes
    init();
    sf::Event event;
    // main loop
    while (win.isOpen())    {
        while (win.pollEvent(event))
            // check if the window has been closed
            if (event.type == sf::Event::Closed)
                win.close();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))    {
            dir -= 1;
            if (dir < 0) dir += 360;
            // printf("%d ", dir);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))   {
            dir += 1;
            if (dir >= 360) dir -= 360;
            // printf("%d ", dir);
        }
        win.clear();
        render();
        win.display();
        sf::sleep(sleep_duration);
    }
    return 0;
}
