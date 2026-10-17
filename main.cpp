#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <algorithm>
#include <optional>
#include <string>

// Структура для опису сходинки
struct Step {
    sf::FloatRect rect;
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Lab 5 - Variant 21: Ball on Steps");

    // Фізичні параметри
    const float ballRadius = 15.f;
    sf::Vector2f startPos = { 30.f, 50.f };
    sf::Vector2f position = startPos;

    float vx = 150.f;          // Початкова горизонтальна швидкість
    float vy = 0.f;            // Початкова вертикальна швидкість
    const float gravity = 800.f; // Прискорення вільного падіння
    const float restitution = 0.6f; // Коефіцієнт відновлення швидкості

    // Параметри сходинок
    float stepWidth = 100.f;
    float stepHeight = 60.f;
    int numSteps = 7;

    bool isPaused = false;

    // Створення графічного об'єкта кулі
    sf::CircleShape ball(ballRadius);
    ball.setFillColor(sf::Color::Red);
    ball.setOrigin({ ballRadius, ballRadius }); // Центр кулі як точка прив'язки
    ball.setPosition(position);

    // Вектор сходинок
    std::vector<sf::RectangleShape> stepShapes;

    auto updateSteps = [&]() {
        stepShapes.clear();
        for (int i = 0; i < numSteps; ++i) {
            sf::RectangleShape step({ stepWidth, stepHeight * (numSteps - i) });
            step.setPosition({ 100.f + i * stepWidth, 600.f - (numSteps - i) * stepHeight });
            step.setFillColor(sf::Color(120, 120, 120));
            step.setOutlineThickness(1.f);
            step.setOutlineColor(sf::Color::White);
            stepShapes.push_back(step);
        }
        };

    updateSteps();

    // Шлейф траєкторії
    std::vector<sf::Vector2f> trajectory;
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // Захист від великого стрибка dt

        // 1. Обробка подій
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Space) {
                    // Перезапуск
                    position = startPos;
                    vy = 0.f;
                    trajectory.clear();
                }
                if (key->code == sf::Keyboard::Key::P) {
                    isPaused = !isPaused; // Пауза
                }
                if (key->code == sf::Keyboard::Key::Up) {
                    vx += 20.f; // Збільшити vx
                }
                if (key->code == sf::Keyboard::Key::Down) {
                    vx = std::max(0.f, vx - 20.f); // Зменшити vx
                }
                if (key->code == sf::Keyboard::Key::Right) {
                    stepHeight += 5.f; // Збільшити висоту сходинки
                    updateSteps();
                }
                if (key->code == sf::Keyboard::Key::Left) {
                    stepHeight = std::max(10.f, stepHeight - 5.f);
                    updateSteps();
                }
            }
        }

        // 2. Фізичне оновлення стану
        if (!isPaused) {
            vy += gravity * dt;
            position.x += vx * dt;
            position.y += vy * dt;

            // Зіткнення зі сходинками
            for (const auto& stepShape : stepShapes) {
                sf::FloatRect bounds = stepShape.getGlobalBounds();

                // Проста колізія AABB для кулі
                if (position.x + ballRadius > bounds.position.x &&
                    position.x - ballRadius < bounds.position.x + bounds.size.x &&
                    position.y + ballRadius > bounds.position.y &&
                    position.y - ballRadius < bounds.position.y + bounds.size.y) {

                    // Перевірка зіткнення зверху (горизонтальна поверхня)
                    if (position.y < bounds.position.y + 15.f && vy > 0) {
                        position.y = bounds.position.y - ballRadius;
                        vy = -vy * restitution;
                    }
                    // Перевірка зіткнення збоку (вертикальна поверхня)
                    else if (position.x < bounds.position.x && vx > 0) {
                        position.x = bounds.position.x - ballRadius;
                        vx = -vx * restitution;
                    }
                }
            } // <--- ОСЬ ЦЯ ДУЖКА БУЛА ПРОПУЩЕНА (закриває цикл for по сходинках)

            // Межі вікна (підлога та стіни)
            if (position.y + ballRadius >= 600.f) {
                position.y = 600.f - ballRadius;
                vy = -vy * restitution;
            }
            if (position.x + ballRadius >= 800.f) {
                position.x = 800.f - ballRadius;
                vx = -vx * restitution;
            }

            ball.setPosition(position);

            // Оновлення траєкторії
            trajectory.push_back(position);
            if (trajectory.size() > 400) {
                trajectory.erase(trajectory.begin());
            }
        }

        // 3. Відображення сцени
        window.clear(sf::Color(30, 30, 30));

        // Малювання сходинок
        for (const auto& stepShape : stepShapes) {
            window.draw(stepShape);
        }

        // Малювання траєкторії
        if (trajectory.size() > 1) {
            sf::VertexArray line(sf::PrimitiveType::LineStrip, trajectory.size());
            for (size_t i = 0; i < trajectory.size(); ++i) {
                line[i].position = trajectory[i];
                line[i].color = sf::Color::Yellow;
            }
            window.draw(line);
        }

        // Малювання кулі
        window.draw(ball);

        window.display();
    }

    return 0;
}