/////////////////////////////////////////////////
// Headers
/////////////////////////////////////////////////
//SFML
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Glsl.hpp>

//Standard
#include <iostream>
#include <random>
#include <cstdint>

/////////////////////////////////////////////////
// Helpers
/////////////////////////////////////////////////
namespace helper
{
/////////////////////////////////////////////////
/// \brief Matrix rain effect class
///
/////////////////////////////////////////////////
class Rain : public sf::Drawable, public sf::Transformable
{
public:
    /////////////////////////////////////////////////
                    Rain()
    {

    }

    /////////////////////////////////////////////////
    void            setCount(float count)
    {
        if(count > 0)
        {
            m_character_count = std::move(count);
        }

        m_update = true;
    }

    /////////////////////////////////////////////////
    void            setFillColor(sf::Color body, sf::Color head)
    {
        m_body_color = std::move(body);
        m_head_color = std::move(head);

        m_update = true;
    }

    /////////////////////////////////////////////////
    void            setDelay(float delay)
    {
        if(delay > 0 && delay < 1)
        {
            m_delay = std::move(delay);
        }

        m_update = true;
    }

    /////////////////////////////////////////////////
    void            setCharacterSize(float size)
    {
        m_character_size = std::move(size);

        m_update = true;
    }

    /////////////////////////////////////////////////
    void            setStyle(sf::Text::Style style)
    {
        m_style = std::move(style);

        m_update = true;
    }

    /////////////////////////////////////////////////
    sf::Vector2f    getCharacterBounds()
    {
        return m_character_bounds;
    }

    /////////////////////////////////////////////////
    float           getCount() const
    {
        return m_character_count;
    }


    /////////////////////////////////////////////////
    void            update(std::random_device& device, float delta_time)
    {
        m_delta = std::move(delta_time);

        if(m_update)
        {
            stylize();
            m_update = false;
        }

        manage(device);
    }

private:
    /////////////////////////////////////////////////
    void            stylize()
    {
        if(!m_font.loadFromFile("fonts/matrix-code.ttf"))
        {
            std::cout << "Failed to load matrix-code.ttf font (main.cpp: Rain::stylize)" << std::endl;
        }

        sf::Text temporary_text;
        temporary_text.setCharacterSize(m_character_size);
        temporary_text.setStyle(m_style);
        temporary_text.setFont(m_font);
        temporary_text.setString("a");

        m_character_bounds = {temporary_text.getGlobalBounds().width, temporary_text.getGlobalBounds().height};
    }

    /////////////////////////////////////////////////
    void            manage(std::random_device& device)
    {
        if(m_clock.getElapsedTime() > sf::milliseconds(m_delay))
        {
            std::mt19937 random_generator(device());

            if(m_characters.size() < m_character_count)
            {
                std::uniform_int_distribution<std::mt19937::result_type> distribution_char(97, 122);
                m_characters.emplace_back();
                m_characters.back().setString(distribution_char(random_generator));
                m_characters.back().setFont(m_font);
                m_characters.back().setFillColor(m_body_color);
                m_characters.back().setCharacterSize(m_character_size);
                m_characters.back().setStyle(m_style);
                m_characters.back().setPosition(0, m_characters.size() * m_character_bounds.y);
            }

            for(auto& character: m_characters)
            {
                std::uniform_int_distribution<std::mt19937::result_type> distribution_char(97, 122);
                character.setString(distribution_char(random_generator));
                character.setFillColor(m_body_color);
            }

            m_characters.back().setFillColor(m_head_color);

            m_clock.restart();
        }
    }

    /////////////////////////////////////////////////
    virtual void    draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        for(auto& character: m_characters)
        {
            target.draw(character, states);
        }
    }

    /////////////////////////////////////////////////
    std::vector<sf::Text>   m_characters;
    sf::Font                m_font;
    sf::Clock               m_clock;
    sf::Text::Style         m_style = sf::Text::Bold;
    sf::Color               m_body_color = sf::Color::Green;
    sf::Color               m_head_color = sf::Color::White;
    sf::Vector2f            m_character_bounds = {0, 0};
    float                   m_character_count = 1;
    float                   m_character_size = 20;
    float                   m_delay = 50;
    bool                    m_update = true;
    float                   m_delta = 0;

};

} // namespace helper

/////////////////////////////////////////////////
// Main stream
/////////////////////////////////////////////////
int main()
{
    sf::RenderWindow window(sf::VideoMode(940, 680), "Brute Force weManhattan.06.30");
    window.setFramerateLimit(60);

    sf::Shader shader;
    if(!shader.loadFromFile("shaders/outline.frag", sf::Shader::Fragment))
    {
        std::cout << "Failed to load outline.frag shader (main.cpp: Rain::stylize)" << std::endl;
    }

    shader.setUniform("outline_thickness", 0.2f);
    shader.setUniform("outline_colour", sf::Glsl::Vec3{100, 100, 100});
    shader.setUniform("texture", sf::Shader::CurrentTexture);

    std::random_device device;
    sf::Vector2f grid;

    {
        helper::Rain temporary_rain;
        temporary_rain.update(device, 0);
        grid = {window.getSize().x / temporary_rain.getCharacterBounds().x, window.getSize().y / temporary_rain.getCharacterBounds().y};
    }

    std::vector<helper::Rain> rains;
    float chance = 0.4;

    for(size_t count = 0; count < grid.x; ++count)
    {
        std::mt19937 random_generator(device());
        std::uniform_int_distribution<std::mt19937::result_type> distribution_chance(0, 100);

        if(distribution_chance(random_generator) < 100 * chance)
        {
            rains.emplace_back();

            std::uniform_int_distribution<std::mt19937::result_type> distribution_vertical_axis(0, grid.y);
            rains.back().setPosition(count * (window.getSize().x / grid.x), distribution_vertical_axis(random_generator) * (window.getSize().x / grid.x));

            std::uniform_int_distribution<std::mt19937::result_type> distribution_count(2, 20);
            rains.back().setCount(distribution_count(random_generator));
        }
    }

    sf::Clock clock;
    float delta_time = 0;

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        delta_time = clock.restart().asSeconds();

        for(auto& rain: rains)
        {
            rain.update(device, delta_time);
        }

        window.clear();

        for(auto& rain: rains)
        {
            window.draw(rain, &shader);
        }

        window.display();
    }

    return 0;
}
