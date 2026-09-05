#include "SFML/Graphics.hpp"
#include <algorithm>
#include "vars.h"
#include "scenes.h"
#include "players.h"
#include "tilesettings.h"

void entity::rotateobject(sf::Vector2f& edge, tilemap& map, sf::Shape& shape, float deltatime, float movespeed, bool swimming, bool zerogactive, bool grounded, int nearestedge) {
    auto vertices = getvertices(shape);
    std::sort(vertices.begin(), vertices.end(), [](sf::Vector2f pt1, sf::Vector2f pt2) {return pt1.y > pt2.y; });
    sf::Vector2f btm1 = vertices[0];
    sf::Vector2f btm2 = vertices[1];
    sf::Vector2f center = sf::Vector2f(shape.getPosition().x, shape.getPosition().y + playerdim / 2);
    if (btm1.x > btm2.x) std::swap(btm1, btm2);
    bool grounded_left = map.cliffCheck(btm1);
    bool grounded_right = map.cliffCheck(btm2);
    bool grounded_center = map.cliffCheck(center);
    bool cantipright = grounded && std::abs(btm1.y - btm2.y) < 0.5 && grounded_left && !grounded_right && !grounded_center && !zerogactive;
    bool cantipleft = grounded && std::abs(btm1.y - btm2.y) < 0.5 && grounded_right && !grounded_left && !grounded_center && !zerogactive;	

    if (cantipright && !tipping_right && !tipping_left) {
        tipping_right = true;
        initialtip = shape.getRotation().asDegrees();
        edge = sf::Vector2f(std::floor(center.x / playerdim) * playerdim, shape.getPosition().y + playerdim / 2);
    } else if (cantipleft && !tipping_right && !tipping_left) {
        tipping_left = true;
        initialtip = shape.getRotation().asDegrees();
        edge = sf::Vector2f(std::ceil(center.x / playerdim) * playerdim, shape.getPosition().y + playerdim / 2);
    }

    float tilt = shape.getRotation().asDegrees() - initialtip;
    if (tilt > 180) tilt -= 360;
    if (tilt < -180) tilt += 360;
    if (!freefallingtip && (tipping_right || tipping_left) && (tilt > 45 || tilt < -45)) {
        freefallingtip = true;
    }

    if (freefallingtip) {
        float direction = tipping_right ? 1 : -1;
        shape.rotate(sf::degrees(direction * deltatime * 100));
        if ((grounded) || zerogactive) {
            freefallingtip = false;
            tipping_right = false;
            tipping_left = false;
            settlepoint = -1;
        }
    }
    else if (tipping_right) {
        tipShape(edge, shape, deltatime, 1);
    } else if (tipping_left) {
        tipShape(edge, shape, deltatime, -1);
    }
    else if ((swimming || zerogactive) && shape.getRotation().asDegrees() != 0) {
        rotating = false;
        rotation = 0;
        shape.getRotation().asDegrees() > 180 ? shape.rotate(sf::degrees(1)) : shape.rotate(sf::degrees(-1));
        if (std::abs(shape.getRotation().asDegrees()) < 5) shape.setRotation(sf::degrees(0));
    } 
    else if (grounded && std::abs(velocity.x) <= 225.f) {
        rotation = 0.f;
        float nearestangle = 360.f;
        float currentangle = shape.getRotation().asDegrees();
        if (settlepoint < 0) {
            for (int i = 0; i <= (360 / nearestedge); i++) {
                if (std::abs(std::fmod(currentangle - i * nearestedge + 540.f, 360.f) - 180.f) < std::abs(std::fmod(currentangle - nearestangle + 540.f, 360.f) - 180.f)) {
                    nearestangle = i * nearestedge;
                }
            }
            settlepoint = nearestangle;
        }
        float dist = std::fmod(currentangle - settlepoint + 540.f, 360.f) - 180.f;
        if (dist > 180 * deltatime) shape.rotate(sf::degrees(-180 * deltatime)); 
        else if (dist < -180 * deltatime) shape.rotate(sf::degrees(180 * deltatime));
        else { 
            shape.setRotation(sf::degrees(settlepoint));
            settlepoint = -1; 
        }
    }
    else if (rotating) {
        if (std::abs(rotation) <= std::abs(velocity.x / (playerdim))) rotation += 1.15 * velocity.x / movespeed; 
        shape.rotate(sf::radians(rotation * deltatime));
        rotation *= 0.90;
    }
}



square::square() {
    playershape = sf::RectangleShape(sf::Vector2f(playerdim, playerdim));
    playershape.setFillColor(sf::Color(255, 30, 0));
    playershape.setOrigin(sf::Vector2f(playershape.getLocalBounds().size.x/2, playershape.getLocalBounds().size.y/2));

    velocity = (sf::Vector2f(0.f, gravity));
    grounded = false;
}

sf::Shape& square::shape() {
    return playershape;
}

void square::jump (float deltatime) {
    if (sf::Keyboard::isKeyPressed ( sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
        if (grounded && !swimming && !zerogactive) {
            velocity.y = blockonhead ? 0 : -jumpforce;
            grounded = false;
            jumpcount -= 1;
        }
    }
    if (swimming) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            if (velocity.y <= 350.f) velocity.y += 30.f;;
            grounded = false;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
            grounded = false;
        }
    } 
}

void square::updatepos (float deltatime, tilemap& map) {
    if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
        if (velocity.x > movespeed && grounded) velocity.x -= circleaccel*0.7;
        else if (velocity.x > movespeed && !grounded) velocity.x -= 0;
        else velocity.x = movespeed;
        rotating = true;
    } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
        if (velocity.x < -movespeed && grounded) velocity.x += circleaccel*0.7;
        else if (velocity.x < -movespeed && !grounded) velocity.x -= 0;
        else velocity.x = -movespeed;
        !(zerogactive || swimming) ? rotating = true : rotating = false;
    } else {
        zerogactive || swimming || inblackhole ? velocity.x *= 0.71 : velocity.x *= 0.f; 
    }
    if (zerogactive) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            velocity.y = 115.f;
        }
        else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -115.f;
        }
        else {
            velocity.y *= 0.71;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
            velocity.x = 115.f;
        } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
            velocity.x = -115.f;
        }
    } 

    if (swimming) {
        gravity = 250.f; 
    } else if (zerogactive) {
        gravity = 0.f;
    } else if (!swimming && !zerogactive) { 
        gravity = 1800.f;
    }

    
    if (!grounded && velocity.y <= 1050) velocity.y += gravity * deltatime;
    shape().move(velocity * deltatime);

    if (playershape.getPosition().y > 780) restart = true;

    sf::Vector2f bound = playershape.getPosition();
    bound.x = std::clamp(bound.x, 10.f, (float)width-playerdim/2);
    playershape.setPosition(bound);
    swimming = false;
    zerogactive = false;

    if (tp_timer.getElapsedTime().asSeconds() >= 3.f && triangleshade < 255) {
        triangleshade += 2;
    }
    if ((tp_timer.getElapsedTime().asSeconds() < 3.f && tped) && triangleshade > 135) {
        triangleshade -= 2;
    }
}

void square::drawscreen (sf::RenderWindow& window)  {
    window.draw(playershape);
}



circle::circle() {
    playershape = sf::CircleShape(playerdim/2);
    playershape.setFillColor(sf::Color(0, 145, 255));
    playershape.setOrigin(sf::Vector2f(playershape.getLocalBounds().size.x/2, playershape.getLocalBounds().size.y/2));

    velocity = (sf::Vector2f(0.f, gravity));
    grounded = false;
}

sf::Shape& circle::shape()  {
    return playershape;
}

void circle::jump (float deltatime)  {
    if (sf::Keyboard::isKeyPressed ( sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
        if (grounded && !swimming && !zerogactive) {
            velocity.y -= blockonhead ? 0 : jumpforce;
            grounded = false;
            jumpcount -= 1;
        }
    }
    if (swimming) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            if (velocity.y <= 350.f) velocity.y += 30.f;;
            grounded = false;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
            grounded = false;
        }
    } 
}

void circle::updatepos (float deltatime, tilemap& map)  {
    if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
        if (velocity.x <= circlemax && !zerogactive) velocity.x += circleaccel;
        else if (zerogactive) velocity.x = movespeed;
        rotating = true;
    } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
        if (velocity.x >= -circlemax && !zerogactive) velocity.x += -circleaccel;
        else if (zerogactive) velocity.x = -movespeed;
        rotating = true;
    } else if (swimming || zerogactive) {
        velocity.x *= 0.7; 
    } else {
        if (velocity.x > 0) velocity.x -= circleaccel/2;
        else if (velocity.x < 0) velocity.x += circleaccel/2;
        else velocity.x = 0;
    }
    if (zerogactive) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            velocity.y = 125.f;
        }
        else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
        }
        else {
            velocity.y *= 0.71;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
            velocity.x = 115.f;
        } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
            velocity.x = -115.f;
        }
    } 

    if (swimming) {
        gravity = 250.f; 
    } else if (zerogactive) {
        gravity = 0.f;
    } else if (!swimming && !zerogactive) { 
        gravity = 1800.f;
    }

    
    if (!grounded && velocity.y <= 1050) velocity.y += gravity * deltatime;
    shape().move(velocity * deltatime);

    if (playershape.getPosition().y > 780) restart = true;

    sf::Vector2f bound = playershape.getPosition();
    bound.x = std::clamp(bound.x, 10.f, (float)width-playerdim/2);
    playershape.setPosition(bound);
    swimming = false;
    zerogactive = false;

    if (tp_timer.getElapsedTime().asSeconds() >= 3.f && triangleshade < 255) {
        triangleshade += 2;
    }
    if ((tp_timer.getElapsedTime().asSeconds() < 3.f && tped) && triangleshade > 135) {
        triangleshade -= 2;
    }
}

void circle::drawscreen (sf::RenderWindow& window)  {
    window.draw(playershape);
}



triangle::triangle() {
    playershape.setPointCount(3);
    playershape.setPoint(0, sf::Vector2f(0, 17.3));
    playershape.setPoint(1, sf::Vector2f(10, 0));
    playershape.setPoint(2, sf::Vector2f(20, 17.3));
    playershape.setFillColor(sf::Color(0, 255, 0));

    playershape.setOrigin(sf::Vector2f(10, 11.526));
    velocity = (sf::Vector2f(0.f, gravity));
    grounded = false;
}

sf::Shape& triangle::shape()  {
    return playershape;
}

void triangle::jump (float deltatime)  {
    if (sf::Keyboard::isKeyPressed ( sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
        if (grounded && !swimming && !zerogactive) {
            velocity.y -= blockonhead ? 0 : jumpforce;
            grounded = false;
            jumpcount -= 1;
        }
    }
    if (swimming) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            if (velocity.y <= 350.f) velocity.y += 30.f;;
            grounded = false;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
            grounded = false;
        }
    } 
}

void triangle::updatepos (float deltatime, tilemap& map)  {
    if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
        if (velocity.x > movespeed && grounded) velocity.x -= circleaccel*0.7;
        else if (velocity.x > movespeed && !grounded) velocity.x -= 0;
        else velocity.x = movespeed;
        rotating = true;
    } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
        if (velocity.x < -movespeed && grounded) velocity.x += circleaccel*0.7;
        else if (velocity.x < -movespeed && !grounded) velocity.x -= 0;
        else velocity.x = -movespeed;
        rotating = true;
    } else {
        zerogactive || swimming ? velocity.x *= 0.71 : velocity.x *= 0.f; 
    }

    if (tp_timer.getElapsedTime().asSeconds() >= 3.f && triangleshade < 255) {
        triangleshade += 2;
    }
    if ((tp_timer.getElapsedTime().asSeconds() >= 3.f || !tped) && triangleshade == 255) {
        if ((sf::Keyboard::isKeyPressed (sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::RShift))) {
            if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
                if (!map.predictCollision(shape(), sf::Vector2f(50, 0))) {
                    playershape.setPosition(sf::Vector2f(playershape.getPosition().x + 50, playershape.getPosition().y));
                    velocity = sf::Vector2f(0, 0);
                    tp_timer.restart();
                    tped = true;
                }
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
                if (!map.predictCollision(shape(), sf::Vector2f(-50, 0))) {
                    playershape.setPosition(sf::Vector2f(playershape.getPosition().x - 50, playershape.getPosition().y));
                    velocity = sf::Vector2f(0, 0);
                    tp_timer.restart();
                    tped = true;
                }
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
                if (!map.predictCollision(shape(), sf::Vector2f(0, -50))) {                        
                    playershape.setPosition(sf::Vector2f(playershape.getPosition().x, playershape.getPosition().y - 50));
                    velocity = sf::Vector2f(0, 0);
                    tp_timer.restart();
                    tped = true;
                }
            } else if (((sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S))) && shape().getPosition().y < (height-40)) {
                if (!map.predictCollision(shape(), sf::Vector2f(0, 50))) {
                    playershape.setPosition(sf::Vector2f(playershape.getPosition().x, playershape.getPosition().y + 50));
                    velocity = sf::Vector2f(0, 0);
                    tp_timer.restart();
                    tped = true;
                }
            }
        }
    }
    if ((tp_timer.getElapsedTime().asSeconds() < 3.f && tped) && triangleshade > 135) {
        triangleshade -= 2;
    }

    if (zerogactive) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            velocity.y = 125.f;
        }
        else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
        }
        else {
            velocity.y *= 0.71;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
            velocity.x = 115.f;
        } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
            velocity.x = -115.f;
        }
    } 

    if (swimming) {
        gravity = 250.f; 
    } else if (zerogactive) {
        gravity = 0.f;
    } else if (!swimming && !zerogactive) { 
        gravity = 1800.f;
    }

    playershape.setFillColor(sf::Color(0, triangleshade, 0));
    if (!grounded && velocity.y <= 1050) velocity.y += gravity * deltatime;
    shape().move(velocity * deltatime);

    if (playershape.getPosition().y > 780) restart = true;

    sf::Vector2f bound = playershape.getPosition();
    bound.x = std::clamp(bound.x, 10.f, (float)width-playerdim/2);
    playershape.setPosition(bound);
    swimming = false;
    zerogactive = false;
}

void triangle::drawscreen (sf::RenderWindow& window)  {
    window.draw(playershape);
}



hexagon::hexagon() {
    playershape.setPointCount(6);
    playershape.setPoint(0, sf::Vector2f(5.f, 0.f));
    playershape.setPoint(1, sf::Vector2f(15.f, 0.f));
    playershape.setPoint(2, sf::Vector2f(20.f, 9.f));
    playershape.setPoint(3, sf::Vector2f(15.f, 18.f));
    playershape.setPoint(4, sf::Vector2f(5.f, 18.f));
    playershape.setPoint(5, sf::Vector2f(0.f, 9.f));
    playershape.setFillColor(sf::Color(255, 210, 0));
    playershape.setOrigin(sf::Vector2f(playershape.getLocalBounds().size.x/2, playershape.getLocalBounds().size.y/2));

    velocity = sf::Vector2f(0.f, gravity);
    grounded = false;
}

sf::Shape& hexagon::shape() {
    return playershape;
}

void hexagon::jump (float deltatime) {
    if (sf::Keyboard::isKeyPressed ( sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
        if (jumpcount > 0 && !swimming && !zerogactive && jump_timer.getElapsedTime().asMilliseconds() > 200) {
            if (!blockonhead) velocity.y = -jumpforce;
            jumpcount -= 1;
            jump_timer.restart();
        }
    }
    if (swimming) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            if (velocity.y <= 350.f) velocity.y += 30.f;;
            grounded = false;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
            grounded = false;
        }
    }
}

void hexagon::updatepos (float deltatime, tilemap& map) {
    if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
        if (velocity.x > movespeed && grounded) velocity.x -= circleaccel*0.7;
        else if (velocity.x > movespeed && !grounded) velocity.x -= 0;
        else velocity.x = movespeed;
        rotating = true;
    } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
        if (velocity.x < -movespeed && grounded) velocity.x += circleaccel*0.7;
        else if (velocity.x < -movespeed && !grounded) velocity.x -= 0;
        else velocity.x = -movespeed;
        rotating = true;
    } else {
        zerogactive || swimming ? velocity.x *= 0.71 : velocity.x *= 0.f; 
    }

    if (zerogactive) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            velocity.y = 125.f;
        }
        else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
        }
        else {
            velocity.y *= 0.71;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
            velocity.x = 115.f;
        } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
            velocity.x = -115.f;
        }
    } 

    if (swimming) {
        gravity = 250.f; 
    } else if (zerogactive) {
        gravity = 0.f;
    } else if (!swimming && !zerogactive) { 
        gravity = 1800.f;
    }

    if (!grounded && velocity.y <= 1050) velocity.y += gravity * deltatime;
    shape().move(velocity * deltatime);

    if (playershape.getPosition().y > 780) restart = true;

    sf::Vector2f bound = playershape.getPosition();
    bound.x = std::clamp(bound.x, 10.f, (float)width-playerdim/2);
    playershape.setPosition(bound);
    swimming = false;
    zerogactive = false;

    if (tp_timer.getElapsedTime().asSeconds() >= 3.f && triangleshade < 255) {
        triangleshade += 2;
    }
    if ((tp_timer.getElapsedTime().asSeconds() < 3.f && tped) && triangleshade > 135) {
        triangleshade -= 2;
    }
}

void hexagon::drawscreen (sf::RenderWindow& window) {
    window.draw(playershape);
}



octagon::octagon() {
    playershape.setPointCount(8);
    playershape.setPoint(0, sf::Vector2f(6, 0));
    playershape.setPoint(1, sf::Vector2f(14, 0));
    playershape.setPoint(2, sf::Vector2f(20, 6));
    playershape.setPoint(3, sf::Vector2f(20, 14));
    playershape.setPoint(4, sf::Vector2f(14, 20));
    playershape.setPoint(5, sf::Vector2f(6, 20));
    playershape.setPoint(6, sf::Vector2f(0, 14));
    playershape.setPoint(7, sf::Vector2f(0, 6));
    playershape.setFillColor(sf::Color(160, 100, 200));
    playershape.setOrigin(sf::Vector2f(playershape.getLocalBounds().size.x/2, playershape.getLocalBounds().size.y/2));

    velocity = sf::Vector2f(0.f, gravity);
    grounded = false;
}

sf::Shape& octagon::shape() {
    return playershape;
}

void octagon::jump (float deltatime) {
    if (sf::Keyboard::isKeyPressed ( sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
        if (grounded && !swimming && !zerogactive) {
            if (!blockonhead) velocity.y = -jumpforce;
            grounded = false;
            walljumped = false;
            jumpcount -= 1;
        }
        else if (!grounded && !swimming && !zerogactive && wallhuggingright && !jumpkeyheld) {
            walljumped = false;
            if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::LShift)) {
                velocity.y = -0.92 * jumpforce;
                velocity.x = -1.1 * jumpforce;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::RShift)) {
                velocity.y = -1.28* jumpforce;
                velocity.x = -0.36 * jumpforce;
            } else {
                velocity.y = -jumpforce;
                velocity.x = -movespeed;
            }
            walljumped = true;
        } else if (!grounded && !swimming && !zerogactive && wallhuggingleft && !jumpkeyheld) {
            walljumped = false;
            if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::LShift)) {
                velocity.y = -0.92 * jumpforce;
                velocity.x = 1.1 * jumpforce;
            } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::RShift)) {
                velocity.y = -1.28 * jumpforce;
                velocity.x = 0.36 * jumpforce;
            } else {
                velocity.y = -jumpforce;
                velocity.x = movespeed;
            }
            walljumped = true;
        }
    } else {
        if (!grounded && !swimming && !zerogactive && wallhuggingright) {
            velocity.y = 0;
        } else if (!grounded && !swimming && !zerogactive && wallhuggingleft) {
            velocity.y = 0;
        }
    }
    if (swimming) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            if (velocity.y <= 350.f) velocity.y += 30.f;;
            grounded = false;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
            grounded = false;
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) jumpkeyheld = true;
    else jumpkeyheld = false;
}

void octagon::updatepos (float deltatime, tilemap& map) {
    if (walljumpcancel) walljumped = false;
    if (!walljumped) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
            if (velocity.x > movespeed && grounded) velocity.x -= circleaccel*0.7;
            else if (velocity.x > movespeed && !grounded) velocity.x -= 0;
            else velocity.x = movespeed;
            rotating = true;
        } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
            if (velocity.x < -movespeed && grounded) velocity.x += circleaccel*0.7;
            else if (velocity.x < -movespeed && !grounded) velocity.x -= 0;
            else velocity.x = -movespeed;
            rotating = true;
        } else {
            zerogactive || swimming ? velocity.x *= 0.71 : velocity.x *= 0.f; 
        }
    }

    if ((sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) && !grounded) {
        if (walljumped) walljumped = false;
        else velocity.x = 0;
    }

    if (zerogactive) {
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::S)) {
            velocity.y = 125.f;
        }
        else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::W)) {
            velocity.y = -125.f;
        }
        else {
            velocity.y *= 0.71;
        }
        if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::D)) {
            velocity.x = 115.f;
        } else if (sf::Keyboard::isKeyPressed (sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed (sf::Keyboard::Key::A)) {
            velocity.x = -115.f;
        }
    } 

    if (swimming) {
        gravity = 250.f; 
    } else if (zerogactive) {
        gravity = 0.f;
    } else if (!swimming && !zerogactive) { 
        gravity = 1800.f;
    }

    if (landed) {
        walljumped = false;
    }
    walljumpcancel = false;

    if (!grounded && velocity.y <= 1050) velocity.y += gravity * deltatime;
    shape().move(velocity * deltatime);

    if (playershape.getPosition().y > 780) restart = true;

    sf::Vector2f bound = playershape.getPosition();
    bound.x = std::clamp(bound.x, 10.f, (float)width-playerdim/2);
    playershape.setPosition(bound);
    swimming = false;
    zerogactive = false;

    if (tp_timer.getElapsedTime().asSeconds() >= 3.f && triangleshade < 255) {
        triangleshade += 2;
    }
    if ((tp_timer.getElapsedTime().asSeconds() < 3.f && tped) && triangleshade > 135) {
        triangleshade -= 2;
    }
}

void octagon::drawscreen (sf::RenderWindow& window) {
    window.draw(playershape);
}