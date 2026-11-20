#include <SFML/Graphics.hpp>
#include <iostream>
#include "SandboxScreen.h"

//this is huzaifa's branch

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "Vault Heist: The Living Vault");
    window.setFramerateLimit(60);

    // Put any .ttf at <project folder>/assets/font.ttf (the Windows Arial fallback below is for convenience).
    sf::Font font;
    if (!font.openFromFile("assets/font.ttf") && !font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        std::cout << "Could not load a font. Put a .ttf at assets/font.ttf\n";
        return 1;
    }

    SandboxScreen sandbox(font);
    Screen* active = &sandbox;      // later: a ScreenManager that switches menu / Act 1..4 / sandbox

    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else
                active->handleEvent(*event);
        }

        float dt = clock.restart().asSeconds();
        active->update(dt);
        active->draw(window);
        window.display();
    }
}