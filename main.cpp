#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>

sf::Color light_c (sf::Color(255, 255, 0)),
          bg_c    (sf::Color(25, 25, 25));
sf::RenderWindow win(sf::VideoMode(500, 500), "Ray Casting");
std::vector<sf::VertexArray> objects;
const sf::Vector2f ws(win.getSize());

sf::Vector2f point_map[12][2] = {

};

bool load_cursor(sf::Cursor &cursor)    {
    sf::Image img;
    img.loadFromFile("assets/light_r.png");
    return cursor.loadFromPixels(img.getPixelsPtr(), img.getSize(), {img.getSize().x / 2, img.getSize().y / 2});
}

/*
possible optimisations:
    make booleans for c and m if they are negative, and checking can be done without divisions
*/
// shape of the shadow to be casted
sf::ConvexShape shadow_cast(const sf::Vector2f &p1, const sf::Vector2f &p2, const sf::Vector2f &cursor)   {
    sf::ConvexShape shape(6);
    shape.setFillColor(bg_c);
    float d1, d2, m, c;
    // find intersection of line with the screen borders
    auto intersection = [&]()  {
        sf::Vector2f point;
        // possible left border
        if (d1 < 0) {
            // upper border
            if (d2 < 0 && (-c / m) >= 0)    {
                point.x = (-c / m);
                point.y = 0;
            }
            // lower border
            else if (d2 >= 0 && (ws.y - c) / m >= 0)  {
                point.x = (ws.y - c) / m;
                point.y = ws.y;
            }
            // left border
            else    {
                point.x = 0;
                point.y = c;
            }
        }
        // possible right border
        else    {
            // upper border
            if (d2 < 0 && (-c / m) <= ws.x)    {
                point.x = (-c / m);
                point.y = 0;
            }
            // lower border
            else if (d2 >= 0 && (ws.y - c) / m <= ws.x)  {
                point.x = (ws.y - c) / m;
                point.y = ws.y;
            }
            // right border
            else    {
                point.x = ws.x;
                point.y = ws.x * m + c;
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
    shape.setPoint(0, insec1);
    shape.setPoint(1, insec2);
    // calculate points 2 and 3 through border info of insec1 and insec2
    // 12 possible cases
    
    
    shape.setPoint(4, p2);
    shape.setPoint(5, p1);

    // corner point
    

    return shape;
}



// ray casting
void ray_cast(sf::RenderWindow &win, const sf::Vector2f &cursor_position)    {
    // initially everything is lit up
    win.clear(light_c);
    // for every line in obstacles, draw shadows
    for (sf::VertexArray &obj: objects) {
        int num_vertices = obj.getVertexCount();
        // if only 2 vertices are there, only 1 iteration is required
        if (num_vertices < 3)   {
            sf::ConvexShape shadow = shadow_cast(obj[0].position, obj[1].position, cursor_position);
            win.draw(shadow);
        }
    }
    for (sf::VertexArray &obj: objects)
        win.draw(obj);
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
    objects.push_back(sf::VertexArray(sf::Lines, 2));
    objects[0][0].position = {100, 100};
    objects[0][1].position = {200, 100};
    objects[0][0].color = sf::Color::Black;
    objects[0][1].color = sf::Color::Black;
    // sf::PrimitiveType p = objects[0].getPrimitiveType();
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


/*
class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:

    ParticleSystem(unsigned int count) :
    m_particles(count),
    m_vertices(sf::Points, count),
    m_lifetime(sf::seconds(3)),
    m_emitter(0, 0)
    {
    }

    void setEmitter(sf::Vector2f position)
    {
        m_emitter = position;
    }

    void update(sf::Time elapsed)
    {
        for (std::size_t i = 0; i < m_particles.size(); ++i)
        {
            // update the particle lifetime
            Particle& p = m_particles[i];
            p.lifetime -= elapsed;

            // if the particle is dead, respawn it
            if (p.lifetime <= sf::Time::Zero)
                resetParticle(i);

            // update the position of the corresponding vertex
            m_vertices[i].position += p.velocity * elapsed.asSeconds();

            // update the alpha (transparency) of the particle according to its lifetime
            float ratio = p.lifetime.asSeconds() / m_lifetime.asSeconds();
            m_vertices[i].color.a = static_cast<sf::Uint8>(ratio * 255);
        }
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // our particles don't use a texture
        states.texture = NULL;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

private:

    struct Particle
    {
        sf::Vector2f velocity;
        sf::Time lifetime;
    };

    void resetParticle(std::size_t index)
    {
        // give a random velocity and lifetime to the particle
        float angle = (std::rand() % 360) * 3.14f / 180.f;
        float speed = (std::rand() % 50) + 50.f;
        m_particles[index].velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        m_particles[index].lifetime = sf::milliseconds((std::rand() % 2000) + 1000);

        // reset the position of the corresponding vertex
        m_vertices[index].position = m_emitter;
    }

    std::vector<Particle> m_particles;
    sf::VertexArray m_vertices;
    sf::Time m_lifetime;
    sf::Vector2f m_emitter;
};
And a little demo that uses it:

int main()
{
    // create the window
    sf::RenderWindow window(sf::VideoMode(512, 256), "Particles");

    // create the particle system
    ParticleSystem particles(1000);

    // create a clock to track the elapsed time
    sf::Clock clock;

    // run the main loop
    while (window.isOpen())
    {
        // handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        // make the particle system emitter follow the mouse
        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        particles.setEmitter(window.mapPixelToCoords(mouse));

        // update it
        sf::Time elapsed = clock.restart();
        particles.update(elapsed);

        // draw it
        window.clear();
        window.draw(particles);
        window.display();
    }

    return 0;
}

*/