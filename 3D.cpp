#include <math.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#define PI 3.141592653589793238463


char grid[50][50] = {
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

sf::RenderWindow win(sf::VideoMode(1280, 720), "Ray Casting");
const sf::Vector2i win_size = sf::Vector2i(win.getSize()), grid_size = {16, 9};
// size of each block in pixels
const float block_size = win_size.x / grid_size.x;
float box_width, scaling_factor = 5e4;
int dir = 0;
sf::Vector2f cur_pos = {300, 300};
sf::RectangleShape rect;


sf::Color colors[] = {
    sf::Color::Black,
    sf::Color::White,
    sf::Color::Red,
    sf::Color::Green,
    sf::Color ({0, 200, 0})
};

class RayCaster {
  public:
    RayCaster () {
        max_dist_2 = max_dist * max_dist;
    }

    // get the ray_caster's field of view (in degrees)
    int getFOV ()   {
        return fov;
    }

    /// @returns distance of the end point of casted ray. -ve if it hit a horizontal wall
    float distance (sf::Vector2f pos, int angle) {
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

        // boolean return value
        float wall = 0;

        while (bx >= 0 && bx < grid_size.x && by >= 0 && by < grid_size.y && !grid[by][bx])   {
            sf::Vector2f v = (this->*fptr[quad]) (pos, x, y, tangent);
            
            // check if vx is valid
            if (x <= pos.x + v.x && pos.x + v.x <= x + block_size)    {
                pos.x += v.x;
                pos.y = y + (quad < 2 ? dy : 0);
                y += dy;
                by += addy;
                wall = 1;
            }
            // else vy is valid
            else    {
                pos.x = x + ((quad == 0 || quad == 3) ? dx : 0);
                pos.y += v.y;
                x += dx;
                bx += addx;
                wall = 0;
            }
        }
        return (wall * 2 - 1) * sqrtf((org.x - pos.x) * (org.x - pos.x) + (org.y - pos.y) * (org.y - pos.y));
    }

    /// @returns a vector of distances of all the casted rays
    std::vector<float> distance_field (sf::Vector2f pos, int angle) {
        // Number of points -> fov
        std::vector<float> distances;
        for (int d = -fov / 2, mx = -d + 1; d < mx; ++d) {
            distances.push_back(distance(pos, (angle + d + 360) % 360) * cosf(to_radians((d + 360) % 360)));
        }
        return distances;        
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
    
    constexpr float to_radians(int deg) const {
        return PI * deg / 180;
    }

  private:
    float max_dist = 1e9, max_dist_2;               // maximum length of ray in pixels
    int fov = 105;                                  // field of view (in degrees)

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

} ray_caster;



void init ()    {
    box_width = win_size.x / ray_caster.getFOV();

}

void render ()  {
    // get the points and the distances
    std::vector<float> distances = ray_caster.distance_field(cur_pos, dir);
    
    float posx = 0;
    // draw boxes according to the frames
    for (float box_length: distances) {
        bool wall = box_length < 0;
        box_length = scaling_factor / abs(box_length);
        rect.setSize({box_width, box_length});
        rect.setOrigin({0, box_length / 2});
        rect.setPosition({posx, win_size.y / 2});
        rect.setFillColor(colors[3 + wall]);
        win.draw(rect);
        posx += box_width;
    }    
}

int main()  {
    // sleep time between each frame
    sf::Time sleep_duration = sf::milliseconds(10);
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))   {
            cur_pos.x += 2.f * cosf(ray_caster.to_radians(dir));
            cur_pos.y += 2.f * sinf(ray_caster.to_radians(dir));
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))   {
            cur_pos.x -= 2.f * cosf(ray_caster.to_radians(dir));
            cur_pos.y -= 2.f * sinf(ray_caster.to_radians(dir));
        }
        win.clear();
        render();
        win.display();
        sf::sleep(sleep_duration);
        // std::cin >> scaling_factor;
    }
    return 0;
}