#pragma once

#include <SFML/Graphics.hpp>
#include <cstdlib> 
#include "tilemap.h"


//PLANNING!!!

/*
- STARS - flashing
- COMETS - prodecurally generated?
- FLOATING ROCKS - they just drift
- PLANETS - static, ez
- A BLACK HOLE? - static, ez
- MOON - static, ez
---- volcano and horizonland, MAYBE.

STRUCTS FOR 5 GUARANTEEDS + ONE MOON
STARS: position, circleshape, brightness
COMETS: trail, pos, shape, cd
BLACK HOLES: pos | Maybe 1 or 2 on the screen.
FLOATING ROCKS: speed, shape,
PLANETS: color, circleshape, size, pos

UPDATE FUNCTION to update pos, brightness...of shapes
DRAW to draw everything, will do away w/ stars and whatnot.


TO DO STILL:
- black hole particle fx
- general stuff
*/

struct star {
    sf::Vector2f pos;
    sf::CircleShape star;
    int brightness;
    int speed;
    int phase;
};

struct comet {
    sf::Vector2f pos;
    sf::Vector2f velocity;
    std::vector<sf::Vector2f> trail;
    bool cometinsky = false;
    int cd = 20;
    sf::Clock comet_cooldown;
};

struct bhparticle {
    float speed_ang;
    float speed_rad;
    float dist;
    float angle;
    sf::Color color;
};

struct blackhole_bkgd {
    sf::Vector2f pos;
    sf::CircleShape bh;
    sf::CircleShape ring;
    std::vector<bhparticle> particlelist;
    sf::VertexArray particle_drawer {sf::PrimitiveType::Points};
};

struct planet {
    sf::Color color;
    sf::CircleShape planet;
    sf::VertexArray ring_back;
    sf::VertexArray ring_front;
    float ring_angle;
    bool has_ring;
    std::vector<sf::CircleShape> details;
};

struct rock {
    sf::ConvexShape rock;
    sf::Vector2f pos;
    sf::Vector2f drift;
    float rotation;
};

class sky {
public:
    std::vector<star> starlist;
    std::vector<rock> rocklist;
    std::vector<planet> planetlist;
    std::vector<blackhole_bkgd> blackholelist;
    std::vector<comet> cometlist;
    planet moon;
    sf::Clock bkgd_clock;
    sf::RectangleShape skyblock;

    sky::sky() {
        skyblock = sf::RectangleShape(sf::Vector2f(width, height));
        skyblock.setFillColor(sf::Color(0, 0, 35));
        skyblock.setPosition(sf::Vector2f(0, 0));
        makestars(stars);
        makeplanets(planets);
        makecomet();
        makerocks(6);
        makeblackholes(2);
        makemoon();
    }

    sf::VertexArray makeplanetrings(float radius, float thickness, float startangle, float endangle, sf::Color color) {
        sf::VertexArray ring_arc(sf::PrimitiveType::TriangleStrip);

        float innerradius = radius - (thickness / 2);
        float outerradius = radius + (thickness / 2);

        float startradians = startangle * (3.14159265358979 / 180);
        float endradians = endangle * (3.14159265358979 / 180);
        float dists = (endradians - startradians) / 25;

        for (int i = 0; i < 27; i++) {
            float angle = startradians + (i * dists);
            float cos = std::cos(angle);
            float sin = std::sin(angle);

            ring_arc.append(sf::Vertex{sf::Vector2f(outerradius*cos, outerradius*sin), color});
            ring_arc.append(sf::Vertex{sf::Vector2f(innerradius*cos, innerradius*sin), color});
        }
        return ring_arc;
    }

    void makestars (int stars) {
        for (int i = 0; i < stars; i++) {
            star star;
            float radius = 1.f + (std::rand() % 2);
            star.star.setRadius(radius);
            star.star.setPosition(sf::Vector2f((std::rand() % width)*1.2, (std::rand() % height)*1.2));
            if (starlist.size() > 0) {
                for (auto& pos: starlist) {
                    while (pos.star.getGlobalBounds().findIntersection(star.star.getGlobalBounds())) {
                        star.star.setPosition(sf::Vector2f((std::rand() % width)*1.2, (std::rand() % 520)*1.2));
                        if (!pos.star.getGlobalBounds().findIntersection(star.star.getGlobalBounds())) {
                            break;
                        }
                    }
                }
            } 
            star.brightness = 215 + (std::rand() % 40);
            star.star.setFillColor(sf::Color(star.brightness, star.brightness, star.brightness));
            star.phase = std::rand() % 500 / 100.f;
            star.speed = (1 + std::rand() % 100) / 100.f;
            starlist.push_back(star);
        };
    }

    void makeplanets (int planets) {
        for (int i = 0; i < planets; i++) {
            planet planet;
            float radius = 5.f + (std::rand() % 20);
            planet.planet.setRadius(radius);
            planet.planet.setOrigin(sf::Vector2f(radius, radius));
            planet.planet.setPosition(sf::Vector2f((std::rand() % width) * 1.1 + 20, (std::rand() % 520) + 20));
            if (planetlist.size() > 0) {
                for (auto& pos: planetlist) {
                    while (pos.planet.getGlobalBounds().findIntersection(planet.planet.getGlobalBounds())) {
                        planet.planet.setPosition(sf::Vector2f(std::rand() % width + 20, (std::rand() % 520) + 20));
                        if (!pos.planet.getGlobalBounds().findIntersection(planet.planet.getGlobalBounds())) {
                            break;
                        }
                    }
                }
            }
             
            planet.planet.setFillColor(sf::Color(std::rand() % 50 + 95, std::rand() % 50 + 95, std::rand() % 50 + 95));
            planet.planet.setOutlineColor(sf::Color(planet.planet.getFillColor().r - 50, planet.planet.getFillColor().g - 50, planet.planet.getFillColor().b - 50));
            planet.planet.setOutlineThickness(1);

            if (std::rand() % 5 > 1) {
                planet.ring_back = makeplanetrings(radius*1.4, std::rand() % 2 + 2, 180, 360, sf::Color(170, 100, 80));
                planet.ring_front = makeplanetrings(radius*1.4, std::rand() % 2 + 2, 0, 180, sf::Color(170, 100, 80));
                planet.ring_angle = std::rand() % 40 - 20;
            }

            for (int i = 0; i < std::rand() % 4 + 2; i++) {
                sf::CircleShape crater;
                crater.setRadius(planet.planet.getRadius()/(5 + std::rand() % 5));
                crater.setOrigin(sf::Vector2f(crater.getRadius(), crater.getRadius()));
                float crateroffset = std::rand() % static_cast<int>(radius * 0.7);
                float crateroffsetangle = std::rand() % 360 * (3.14159265358979/180);
                crater.setPosition(sf::Vector2f(std::cos(crateroffsetangle)*crateroffset, std::sin(crateroffsetangle)*crateroffset));
                if (planet.details.size() > 0) {
                    for (auto& pos: planet.details) {
                        while (pos.getGlobalBounds().findIntersection(crater.getGlobalBounds())) {
                            if (!pos.getGlobalBounds().findIntersection(crater.getGlobalBounds())) break;
                            crateroffset = std::rand() % static_cast<int>(radius * 0.7);
                            crateroffsetangle = std::rand() % 360 * (3.14159265358979/180);
                            crater.setPosition(sf::Vector2f(std::cos(crateroffsetangle)*crateroffset, std::sin(crateroffsetangle)*crateroffset));
                        }
                    }
                }
                crater.setPosition(sf::Vector2f(std::cos(crateroffsetangle)*crateroffset, std::sin(crateroffsetangle)*crateroffset));
                crater.setFillColor(sf::Color(planet.planet.getFillColor().r * 0.7, planet.planet.getFillColor().g * 0.7, planet.planet.getFillColor().b * 0.7));
                planet.details.push_back(crater);
            }
            planetlist.push_back(planet);
        }
        std::sort(planetlist.begin(), planetlist.end(), [](const planet& a, const planet& b) { return a.planet.getRadius() < b.planet.getRadius(); });
    }
    
    void makecomet () {
        cometlist.clear();
        int comets = 3;

        for (int i = 0; i < comets; i++) {
            comet comet;
            comet.cd = 5 + std::rand() % 15;
            comet.cometinsky = false;
            comet.comet_cooldown.restart();
            cometlist.push_back(comet);
        }
    }

    void makerocks (int rocks) {
        for (int i = 0; i < rocks; i++) {
            rock rock;
            rock.rock.setPointCount(5+std::rand() % 4);
            float radius = 5 + std::rand() % 12;
            for (int i = 0; i < rock.rock.getPointCount(); i++) {
                float angle = i * 6.2831853 / rock.rock.getPointCount();
                float variance = radius * (60 + std::rand() % 80) / 100.f;
                rock.rock.setPoint(i, sf::Vector2f(std::cos(angle)*variance, std::sin(angle)*variance));
            }
            int rockshade = 30 + std::rand() % 40;
            rock.rock.setFillColor(sf::Color(rockshade+(std::rand() % 4 * 10), rockshade, rockshade));
            rock.rock.setOutlineColor(sf::Color(rockshade/2, rockshade/2, rockshade/2));
            rock.rock.setOutlineThickness(-1);
            rock.pos = sf::Vector2f(20 + std::rand() % 1240, 20 + std::rand() % 680);
            rock.rock.setPosition(rock.pos);
            rock.drift = sf::Vector2f(-30 + std::rand() % 60, -10 + std::rand() % 20);
            do { rock.rotation = -3 + std::rand() % 6; } while (rock.rotation == 0);
            rocklist.push_back(rock);
        }
    }

    void makeblackholes (int blackholes) {
        for (int i = 0; i < blackholes; i++) {
            blackhole_bkgd bh;
            bh.bh.setRadius(std::rand() % 5 + 4);
            bh.bh.setFillColor(sf::Color::Black);
            bh.bh.setOrigin(sf::Vector2f(bh.bh.getRadius(), bh.bh.getRadius()));
            bh.pos = sf::Vector2f(50 + std::rand() % 1180, 50 + std::rand() % 620);
            bh.bh.setPosition(bh.pos);

            bh.ring.setRadius(bh.bh.getRadius() * 4.4);
            bh.ring.setFillColor(sf::Color::Transparent);
            bh.ring.setOutlineColor(sf::Color(240, 170, 0));
            bh.ring.setOutlineThickness(1);
            bh.ring.setOrigin(sf::Vector2f(bh.ring.getRadius(), bh.ring.getRadius()));
            bh.ring.setPosition(bh.bh.getPosition());

            for (int i = 0; i < 80; i++) {
                bhparticle particle;
                particle.dist = bh.bh.getRadius() * (std::rand() % 200 + 100) / 100;
                particle.angle = (std::rand() % 360) * (3.14159265358979 / 180);
                particle.speed_rad = 8 + std::rand() % 15;
                particle.speed_ang = (15 + std::rand() % 20) / 10;
                
                if (std::rand() % 3 == 0) particle.color = sf::Color(255, 180, 50);
                else if (std::rand() % 3 == 1) particle.color = sf::Color(255, 80, 20);
                else particle.color = sf::Color(255, 130, 0);
                bh.particlelist.push_back(particle);
            }
            blackholelist.push_back(bh);
        }
    }

    void makemoon () {
        moon.planet.setRadius(30);
        moon.planet.setFillColor(sf::Color(200, 200, 205));
        moon.planet.setOutlineColor(sf::Color(230, 230, 235, 190));
        moon.planet.setOutlineThickness(3);
        moon.planet.setOrigin(sf::Vector2f(30, 30));
        moon.planet.setPosition(sf::Vector2f(100, 100));
        for (int i = 0; i < std::rand() % 4 + 3; i++) {
            sf::CircleShape crater;
            crater.setRadius(moon.planet.getRadius()/(std::rand() % 6 + 4));
            float crateroffset = std::rand() % static_cast<int>(moon.planet.getRadius() * 0.7);
            float craterangle = std::rand() % 360 * (3.14159265358979 / 180);
            crater.setPosition(sf::Vector2f(std::cos(craterangle)*crateroffset, std::sin(craterangle)*crateroffset));
            bool overlapping = false;
            do {
                overlapping = false;
                if (moon.details.size() > 0) {
                    for (auto& pos: moon.details) {
                        crateroffset = std::rand() % static_cast<int>(moon.planet.getRadius() * 0.7);
                        craterangle = std::rand() % 360 * (3.14159265358979 / 180);
                        crater.setPosition(sf::Vector2f(std::cos(craterangle)*crateroffset, std::sin(craterangle)*crateroffset));
                        if (pos.getGlobalBounds().findIntersection(crater.getGlobalBounds())) {
                            overlapping = true;
                            break;
                        }
                    }
                }   
            } while (overlapping);
            crater.setFillColor(sf::Color(160, 160, 170));
            moon.details.push_back(crater);
        }
    }

    void updatesky (float deltatime) {
        float time = bkgd_clock.getElapsedTime().asSeconds();
        for (auto& pos: starlist) {
            int alpha = 170 + 85 * std::sin(time * pos.speed * pos.phase);
            sf::Color color = pos.star.getFillColor();            
            if (alpha > 255) alpha = 255;
            if (alpha < 0) alpha = 0;
            color.a = alpha;
            pos.star.setFillColor(color);
        }   
        for (auto& pos: cometlist) {
            if (!pos.cometinsky) {
                if (pos.comet_cooldown.getElapsedTime().asSeconds() >= pos.cd) {
                    pos.cometinsky = true;
                    float startypos = 50 + std::rand() % 450;
                    pos.pos = sf::Vector2f(-30, startypos);
                    float speed = 320 + std::rand() % 300;
                    float angle = (-20 + std::rand() % 40) * (3.14159265358979/180);
                    pos.velocity = (sf::Vector2f(std::cos(angle)*speed, std::sin(angle)*speed));
                    if (std::rand() % 1000 >= 500) {
                        pos.pos = sf::Vector2f(width+30, startypos);
                        pos.velocity = (sf::Vector2f(-std::cos(angle)*speed, std::sin(angle)*speed));
                    }
                    pos.trail.clear();
                }
            } else {
                pos.pos += pos.velocity * deltatime;
                pos.trail.push_back(pos.pos);
                if (pos.trail.size() > 25) {
                    pos.trail.erase(pos.trail.begin());
                }
                if (pos.pos.x > width + 200 || pos.pos.x < -70 || pos.pos.y > height + 100 || pos.pos.y < -100) {
                    pos.cometinsky = false;
                    pos.comet_cooldown.restart();
                }
            }
        }
        for (auto& pos: rocklist) {
            pos.pos += pos.drift * deltatime;
            pos.rock.setPosition(pos.pos);
            pos.rock.rotate(sf::radians(pos.rotation*deltatime));
            if (pos.pos.x < -70) pos.pos.x = width + 70;
            if (pos.pos.x > width + 70) pos.pos.x = -70;
            if (pos.pos.y < -70) pos.pos.y = height + 70;
            if (pos.pos.y > height + 70) pos.pos.y = -70;
        }
        for (auto& pos: blackholelist) {
            float eventhorizon = pos.bh.getRadius();
            for (auto& rest: pos.particlelist) {
                float speedmulti = (eventhorizon * 3) / std::max(rest.dist, 1.f) * deltatime;
                rest.angle += rest.speed_ang * speedmulti;
                rest.dist -= rest.speed_rad * deltatime;

                float alpha = std::min(255.f, (rest.dist - eventhorizon) * 20.f);
                rest.color.a = std::max(0.f, alpha);

                if (rest.dist <= eventhorizon) {
                    rest.dist = eventhorizon * 4;
                    rest.angle = std::rand() % 360 * (3.14159265358979 / 180);
                    rest.color.a = 255;
                }
            }
        }
    }

    void drawsky (sf::RenderWindow& window, sf::Vector2f center) {
        window.draw(skyblock);
        sf::Transform farparallax;
        sf::Transform midparallax;
        sf::Transform nearparallax;
        farparallax.translate(sf::Vector2f(-center.x*0.06, -center.y*0.02));
        midparallax.translate(sf::Vector2f(-center.x*0.15, -center.y*0.04));
        nearparallax.translate(sf::Vector2f(-center.x*0.24, -center.y*0.06));

        for (auto& pos: blackholelist) {
            window.draw(pos.bh, farparallax);
            pos.particle_drawer.clear();
            for (auto& rest: pos.particlelist) {
                float xpos = pos.pos.x + std::cos(rest.angle) * rest.dist;
                float ypos = pos.pos.y + std::sin(rest.angle) * rest.dist;
                pos.particle_drawer.append(sf::Vertex{sf::Vector2f(xpos, ypos), rest.color});
            }
            window.draw(pos.particle_drawer, farparallax);
            window.draw(pos.ring, farparallax);
        }
        for (auto& pos: starlist) {
            window.draw(pos.star, farparallax);
        }
        for (auto& pos: planetlist) {
            sf::Transform planetparallax;
            planetparallax.translate(sf::Vector2f(-center.x*(pos.planet.getRadius()/150), -center.y*(pos.planet.getRadius()/750)));

            sf::Transform ringparallax = planetparallax;
            ringparallax.translate(pos.planet.getPosition());
            ringparallax.rotate(sf::degrees(pos.ring_angle));
            ringparallax.scale(sf::Vector2f(1, 0.3));

            sf::Transform detailparallax = planetparallax;
            detailparallax.translate(pos.planet.getPosition());

            window.draw(pos.ring_back, ringparallax);
            window.draw(pos.planet, planetparallax);
            for (int i = 0; i < pos.details.size(); i++) {
                window.draw(pos.details[i], detailparallax);
            }
            window.draw(pos.ring_front, ringparallax);
        }
        for (auto& pos: cometlist) {
            if (!pos.cometinsky || pos.trail.size() < 2) continue;

            sf::VertexArray tail(sf::PrimitiveType::LineStrip);
            for (int i = 0; i < pos.trail.size(); i++) {
                float alphalevel = static_cast<float>(i) / static_cast<float>(pos.trail.size());
                float trailalpha = alphalevel * 230;
                sf::Color trailcolor (sf::Color(180, 230, 255, trailalpha));
                tail.append(sf::Vertex{pos.trail[i], trailcolor});
            }
            window.draw(tail, midparallax);

            sf::CircleShape head;
            head.setRadius(4);
            head.setOrigin(sf::Vector2f(4, 4));
            head.setPosition(pos.pos);
            head.setFillColor(sf::Color(255, 255, 240));
            window.draw(head, midparallax);
        }
        for (auto& pos: rocklist) {
            window.draw(pos.rock, nearparallax);
        }
        window.draw(moon.planet, midparallax);
        sf::Transform craterparallax = midparallax;
        craterparallax.translate(moon.planet.getPosition());
        for (auto& pos: moon.details) {
            window.draw(pos, craterparallax);
        }
    }
};