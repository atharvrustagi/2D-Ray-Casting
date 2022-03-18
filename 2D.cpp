#include <math.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#define PI 3.141592653589793238463

sf::RenderWindow win(sf::VideoMode(1280, 720), "Ray Casting");
const sf::Vector2i win_size = sf::Vector2i(win.getSize()), grid_size = {16, 9};
// size of each block in pixels
const float block_size = win_size.x / grid_size.x;
int dir = 0;
sf::RectangleShape block({block_size, block_size});
sf::CircleShape point(3, 4);

bool load_cursor (sf::Cursor& cursor)    {
    sf::Image img;
    img.loadFromFile("assets/light_r.png");
    return cursor.loadFromPixels(img.getPixelsPtr(), img.getSize(), {img.getSize().x / 2, img.getSize().y / 2});
}

const char grid[50][50] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,1,1,1,1,0,0,1,1,1,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,1,0,1,0,0,0,1},
    {1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,0,0,1,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

sf::Color colors[] = {
    sf::Color::Black,
    sf::Color::White,
    sf::Color::Red,
    sf::Color::Green
};

class RayCaster {
  public:
    RayCaster () {
        max_dist_2 = max_dist * max_dist;
    }
    
    /// Casts a ray from the given positon in the direction of the given angle (in degrees)
    /// \param pos starting position \param angle direction of projection
    /// \returns Coordinates of the end point of the casted ray
    sf::Vector2f cast_ray (sf::Vector2f pos, int angle) {
        // initial position
        sf::Vector2f org = pos;

        // angle to radians
        const float radians = to_radians(angle);

        // tan(theta)
        const float tangent = tan(radians);

        // check in which quadrant will the ray move
        const char quad = angle / 90;
        
        // position of the block in the grid
        int bx = floor(pos.x / block_size), by = floor(pos.y / block_size), addx = (quad == 0 || quad == 3 ? 1 : -1), addy = (quad > 1 ? -1 : 1);
        
        // round off x and y to block_size multiples
        float x = block_size * bx, y = block_size * by;

        // for adding
        float dx = addx * block_size, dy = addy * block_size;

        while (bx >= 0 && bx < grid_size.x && by >= 0 && by < grid_size.y && !grid[by][bx])   {
            sf::Vector2f v = (this->*fptr[quad]) (pos, x, y, tangent);
            
            // check if vx is valid
            if (x <= pos.x + v.x && pos.x + v.x <= x + block_size)    {
                pos.x += v.x;
                pos.y = y + (quad < 2 ? dy : 0);
                y += dy;
                by += addy;
            }
            // else vy is valid
            else    {
                pos.x = x + ((quad == 0 || quad == 3) ? dx : 0);
                pos.y += v.y;
                x += dx;
                bx += addx;
            }

            // check if maximum length of the ray has been exceeded
            if ((org.x - pos.x) * (org.x - pos.x) + (org.y - pos.y) * (org.y - pos.y) > max_dist_2) {
                pos.x = org.x + max_dist * cos(radians);
                pos.y = org.y + max_dist * sin(radians);
                return pos;
            }
        }
        return pos;
    }

    /// Casts multiple rays (according to the FOV) from the given position in the given direction
    /// \param pos starting position \param angle direction of projection
    /// \returns Coordinates of the end points of the casted rays
    sf::VertexArray cast_field (sf::Vector2f pos, int angle)   {
        // Number of points -> fov
        sf::VertexArray points(sf::TriangleFan, fov + 1);
        points[0].position = pos;
        points[0].color = colors[3];
        for (int i = 1, d = -fov / 2, mx = -d + 1; d < mx; ++i, ++d) {
            points[i].position = cast_ray(pos, (angle + d + 360) % 360);
            points[i].color = colors[3];
        }
        return points;
    }

    // set the ray_caster's maximum length of projected ray
    void setMaxDist (float maxD) {
        max_dist = maxD;
        max_dist_2 = maxD * maxD;
    }

    // set the ray_caster's field of view (in degrees)
    void setFOV (int _fov)   {
        fov = _fov;
        fov -= !(fov & 1);
    }

  private:
    float max_dist = 1e9, max_dist_2;             // maximum length of ray in pixels
    int fov = 135;                                  // field of view (in degrees)

    // Array of utility functions
    sf::Vector2f (RayCaster::* fptr[4]) (const sf::Vector2f &, float, float, float) const = {
        &RayCaster::quad0_dist, &RayCaster::quad1_dist, &RayCaster::quad2_dist, &RayCaster::quad3_dist
    };

    // Utility functions to calculate intersection coordinates with ray, different for each quadrant
    sf::Vector2f quad0_dist (const sf::Vector2f &pos, float x, float y, float tangent) const {
        return {(y + block_size - pos.y) / tangent, (x + block_size - pos.x) * tangent};
    }

    sf::Vector2f quad1_dist (const sf::Vector2f &pos, float x, float y, float tangent) const {
        return {(y + block_size - pos.y) / tangent, (x - pos.x) * tangent};
    }

    sf::Vector2f quad2_dist (const sf::Vector2f &pos, float x, float y, float tangent) const {
        return {(y - pos.y) / tangent, (x - pos.x) * tangent};
    }

    sf::Vector2f quad3_dist (const sf::Vector2f &pos, float x, float y, float tangent) const {
        return {(y - pos.y) / tangent, (x + block_size - pos.x) * tangent};
    }

    constexpr float to_radians(int deg) const {
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

void draw_line(sf::Vector2f &p1, sf::Vector2f &p2, const sf::Color &c)    {
    sf::VertexArray line(sf::Lines, 2);
    line[0].position = p1;
    line[1].position = p2;
    line[1].color = c;
    line[0].color = c;
    win.draw(line);
}

void render ()   {
    draw_grid();
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(win));
    sf::VertexArray field = ray_caster.cast_field(mousePos, dir);
    win.draw(field);
    for (int i=1, n = field.getVertexCount(); i < n; ++i)   {
        draw_line(mousePos, field[i].position, colors[2]);
        point.setPosition(field[i].position);
        point.setFillColor(colors[2]);
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
    // sleep time between each frame
    sf::Time sleep_duration = sf::milliseconds(10);
    // cursor settings
    sf::Cursor light_cursor;
    if (load_cursor(light_cursor))
        win.setMouseCursor(light_cursor);
    // initialise some values and shapes
    init();
    sf::Event event;
    // start the clock
    sf::Clock clock;
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
