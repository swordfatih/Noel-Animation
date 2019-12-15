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
#include <chrono>

/////////////////////////////////////////////////
// Helpers
/////////////////////////////////////////////////
namespace helper
{
/////////////////////////////////////////////////
static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());

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
            m_characters.clear();
        }
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
    void            update(float delta_time)
    {
        m_delta = std::move(delta_time);

        if(m_update)
        {
            stylize();
            m_update = false;
        }

        manage();
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
    void            manage()
    {
        if(m_clock.getElapsedTime() > sf::milliseconds(m_delay))
        {
            if(m_characters.size() < m_character_count)
            {
                std::uniform_int_distribution<std::mt19937::result_type> distribution_char(97, 122);
                m_characters.emplace_back();
                m_characters.back().setString(distribution_char(rng));
                m_characters.back().setFont(m_font);
                m_characters.back().setFillColor(m_body_color);
                m_characters.back().setCharacterSize(m_character_size);
                m_characters.back().setStyle(m_style);
                m_characters.back().setPosition(-m_characters.back().getLocalBounds().left, m_characters.size() * m_character_bounds.y - m_characters.back().getLocalBounds().top);
            }

            for(auto& character: m_characters)
            {
                std::uniform_int_distribution<std::mt19937::result_type> distribution_char(97, 122);
                character.setString(distribution_char(rng));
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
    float                   m_delay = 75;
    bool                    m_update = true;
    float                   m_delta = 0;
};

/////////////////////////////////////////////////
void    load_rains(std::vector<Rain>& rains, const sf::Vector2f& rain_grid, const sf::Vector2u& window_size)
{
    for(size_t position = 0; position < rain_grid.x * 1.5; ++position)
    {
        auto& rain = rains.emplace_back();

        std::uniform_int_distribution<std::mt19937::result_type> distribution_count(2, 30);
        rain.setCount(distribution_count(rng));

        std::uniform_int_distribution<std::mt19937::result_type> distribution_horizontal_axis(0, rain_grid.x);
        std::uniform_int_distribution<std::mt19937::result_type> distribution_vertical_axis(0, rain_grid.y);

        float vertical_axis_position = 0;

        if(position % 2 == 0)
        {
            vertical_axis_position = (distribution_vertical_axis(rng) - rain_grid.y / 2) * (window_size.y / rain_grid.y);
        }
        else
        {
            vertical_axis_position = distribution_vertical_axis(rng) * (window_size.y / rain_grid.y);
        }

        rain.setPosition(distribution_horizontal_axis(rng) * (window_size.x / rain_grid.x), vertical_axis_position);
    }
}

/////////////////////////////////////////////////
void    update_rain(std::vector<Rain>& rains, const sf::Vector2f& rain_grid, const sf::Vector2u& window_size, float delta_time)
{
    for(size_t position = 0; position < rains.size(); ++position)
    {
        rains[position].move(0, 3);
        rains[position].update(delta_time);

        if(rains[position].getPosition().y > window_size.y)
        {
            std::uniform_int_distribution<std::mt19937::result_type> distribution_count(2, 30);
            rains[position].setCount(distribution_count(rng));

            std::uniform_int_distribution<std::mt19937::result_type> distribution_horizontal_axis(0, rain_grid.x);
            std::uniform_int_distribution<std::mt19937::result_type> distribution_vertical_axis(0, rain_grid.y / 2);

            rains[position].setPosition(distribution_horizontal_axis(rng) * (window_size.x / rain_grid.x), (distribution_vertical_axis(rng) - rain_grid.y / 1.5) * (window_size.y / rain_grid.y));
        }
    }
}

} // namespace helper

/////////////////////////////////////////////////
// Main stream
/////////////////////////////////////////////////
int main()
{
    sf::RenderWindow window(sf::VideoMode(940, 680), "Brute Force weNorth.24.12");
    window.setFramerateLimit(60);

    sf::Shader shader;
    shader.loadFromFile("shaders/outline.frag", sf::Shader::Fragment);

    helper::Rain reference_rain;
    reference_rain.update(0);
    sf::Vector2f rain_grid = {window.getSize().x / reference_rain.getCharacterBounds().x, window.getSize().y / reference_rain.getCharacterBounds().y};

    std::vector<helper::Rain> rains;
    helper::load_rains(rains, rain_grid, window.getSize());

    sf::Font font;
    font.loadFromFile("fonts/matrix-code.ttf");

    sf::Text text_title("WELCOME", font, 64);
    text_title.setFillColor({255, 255, 255, 150});
    text_title.setPosition(window.getSize().x / 2 - text_title.getGlobalBounds().width / 2 - text_title.getLocalBounds().left, window.getSize().y / 2 - text_title.getGlobalBounds().height / 2 - text_title.getLocalBounds().top);

    sf::Text text_login("L", font, 32);
    text_login.setFillColor({200, 200, 200, 255});
    text_login.setPosition(window.getSize().x / 2 - text_login.getGlobalBounds().width / 2 - text_login.getLocalBounds().left, text_title.getPosition().y + text_title.getGlobalBounds().height * 2 - text_login.getLocalBounds().top);

    sf::Text text_password("", font, 32);
    text_password.setFillColor({200, 200, 200, 255});
    text_password.setPosition(window.getSize().x / 2 - text_password.getGlobalBounds().width / 2 - text_password.getLocalBounds().left, text_login.getPosition().y + text_login.getGlobalBounds().height * 2 - text_password.getLocalBounds().top);

    sf::RectangleShape shape_cursor({2, text_login.getGlobalBounds().height});
    text_login.setString("");

    sf::Clock clock_cursor_blink;
    sf::Clock clock_text_write;
    sf::Clock clock_delta;

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

        update_rain(rains, rain_grid, window.getSize(), clock_delta.restart().asSeconds());

        std::string login_to_write = "LOGIN SANTA";
        if(clock_text_write.getElapsedTime() > sf::milliseconds(300) && text_login.getString().getSize() < login_to_write.size())
        {
            text_login.setString(text_login.getString() + login_to_write[text_login.getString().getSize()]);
            text_login.setPosition(window.getSize().x / 2 - text_login.getGlobalBounds().width / 2 - text_login.getLocalBounds().left, text_title.getPosition().y + text_title.getGlobalBounds().height * 2 - text_login.getLocalBounds().top);
            shape_cursor.setPosition(text_login.getPosition().x + text_login.getGlobalBounds().width + shape_cursor.getSize().x, text_login.getPosition().y + text_login.getLocalBounds().top);

            clock_text_write.restart();
        }

        std::string password_to_write = "PASSWORD HACK";
        if(clock_text_write.getElapsedTime() > sf::milliseconds(300) && text_password.getString().getSize() < password_to_write.size() && text_login.getString().getSize() >= login_to_write.size())
        {
            text_password.setString(text_password.getString() + password_to_write[text_password.getString().getSize()]);
            text_password.setPosition(window.getSize().x / 2 - text_password.getGlobalBounds().width / 2 - text_password.getLocalBounds().left, text_login.getPosition().y + text_login.getGlobalBounds().height * 2 - text_password.getLocalBounds().top);
            shape_cursor.setPosition(text_password.getPosition().x + text_password.getGlobalBounds().width + shape_cursor.getSize().x, text_password.getPosition().y + text_login.getLocalBounds().top);

            clock_text_write.restart();
        }

        if(clock_cursor_blink.getElapsedTime() > sf::milliseconds(200))
        {
            if(shape_cursor.getFillColor() == sf::Color::Transparent)
            {
                shape_cursor.setFillColor(sf::Color::White);
            }
            else
            {
                shape_cursor.setFillColor(sf::Color::Transparent);
            }

            clock_cursor_blink.restart();
        }

        std::cout << helper::rng() << std::endl;

        window.clear();
        window.draw(text_title);

        for(auto& rain: rains)
        {
            window.draw(rain, &shader);
        }

        window.draw(text_login);
        window.draw(text_password);
        window.draw(shape_cursor);

        window.display();
    }

    return 0;
}
