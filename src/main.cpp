#include "utils/mouse.hpp"
#include "utils/parameters.hpp"
#include "geometry/camera3d.hpp"
#include "geometry/solid3d.hpp"
#include "geometry/geometry.hpp"

#define NODEMO

#ifdef NODEMO
Solid3d getNextShape(const Solid3d& shape) {
  std::map<Vector3d, std::vector<Vector3d>> kMap;
  for (const Segment3d& edge : shape.edges) {
    Vector3d midpoint = (edge.a + edge.b) * 0.5;
    midpoint.set_color(sf::Color::White);
    kMap[edge.a].push_back(midpoint);
    kMap[edge.b].push_back(midpoint);
  }

  Solid3d nextShape;
  for (const auto& vertex : kMap) {
    std::vector<Vector3d> midpoints = vertex.second;

    for (size_t i = 0; i < midpoints.size() - 1; i++) {
      size_t nextVertex = i + 1;
      double length = (midpoints[i] - midpoints[nextVertex]).norm();

      for (size_t j = i + 2; j < midpoints.size(); j++) {
        double currentLength = (midpoints[i] - midpoints[j]).norm();
        if (currentLength < length) {
          nextVertex = j;
          length = currentLength;
        }
      }

      if (nextVertex != i + 1) {
        std::swap(midpoints[i + 1], midpoints[nextVertex]);
      }
    }

    for (size_t i = 0; i < midpoints.size(); i++) {
      Segment3d edge = Segment3d(midpoints[i], midpoints[(i + 1) % midpoints.size()]);

      if (std::find(nextShape.edges.begin(), nextShape.edges.end(), edge) == nextShape.edges.end()) {
        nextShape.add_segment(Segment3d(midpoints[i], midpoints[(i + 1) % midpoints.size()]));
      }
    }
  }

  return nextShape;
}

int main() {
  // setup window
  sf::ContextSettings window_settings;
  window_settings.antialiasingLevel = 8;
  sf::RenderWindow window(sf::VideoMode(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT), "3D-engine", sf::Style::Close | sf::Style::Resize, window_settings);
  window.setVerticalSyncEnabled(true);
  window.setKeyRepeatEnabled(false);
  Mouse::setPosition(sf::Vector2i(INITIAL_WINDOW_WIDTH / 2, INITIAL_WINDOW_HEIGHT / 2), window);
  window.setMouseCursorVisible(false);

  // create camera
  Camera3d camera(Vector3d(0, -120, -230), -30, 0, 0, Parameters::window_width, Parameters::window_height);

  sf::Clock loop_timer;
  sf::Clock current_time;
  srand(time(NULL));
  loop_timer.restart();
  current_time.restart();

  Solid3d k;
  k.add_segment(Segment3d(Vector3d(-100, 0, 0, sf::Color::White), Vector3d(100, 0, 0, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(100, 0, 0, sf::Color::White), Vector3d(0, 0, 100 * sqrt(3), sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(0, 0, 100 * sqrt(3), sf::Color::White), Vector3d(-100, 0, 0, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(-100, 0, 0, sf::Color::White), Vector3d(0, -sqrt(square(200) - square(100) - square(100 * sqrt(3) / 3)), 100 * sqrt(3) / 3, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(100, 0, 0, sf::Color::White), Vector3d(0, -sqrt(square(200) - square(100) - square(100 * sqrt(3) / 3)), 100 * sqrt(3) / 3, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(0, 0, 100 * sqrt(3), sf::Color::White), Vector3d(0, -sqrt(square(200) - square(100) - square(100 * sqrt(3) / 3)), 100 * sqrt(3) / 3, sf::Color::White)));

  while (window.isOpen())
  {
    sf::Event event;

    // handle events
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        window.close();
      if (event.type == sf::Event::Resized) {
        window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
        Parameters::update_window_size(event.size.width, event.size.height);
        std::cout << "Window resized: "
          << event.size.width
          << " x "
          << event.size.height << std::endl;
        camera.reload_frustrum(Parameters::window_width, Parameters::window_height);
      }

      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
        k = getNextShape(k);
      }
    }

    // rotate camera
    camera.rotate(Mouse::get_move_x(window), Mouse::get_move_y(window));
    Mouse::setPosition(sf::Vector2i(INITIAL_WINDOW_WIDTH / 2, INITIAL_WINDOW_HEIGHT / 2), window);

    // move camera
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
      camera.move(Camera3d::DIRECTION::FRONT);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
      camera.move(Camera3d::DIRECTION::BACK);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
      camera.move(Camera3d::DIRECTION::RIGHT);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
      camera.move(Camera3d::DIRECTION::LEFT);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
      camera.move(Camera3d::DIRECTION::UP);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
      camera.move(Camera3d::DIRECTION::DOWN);

    // rendering
    window.clear();

    k.render_solid(window, Parameters::window_width, Parameters::window_height, camera);

    window.display();

    // other
    Parameters::print_mean_CPU_usage(std::cout, loop_timer.getElapsedTime().asMilliseconds());

    sf::sleep(sf::milliseconds(MAX_MAIN_LOOP_DURATION - loop_timer.getElapsedTime().asMilliseconds()));
    loop_timer.restart();
  }

  return EXIT_SUCCESS;
}
#else
using std::cout;
using std::endl;


int main()
{
    sf::Clock loop_timer;
    sf::Clock current_time;

	srand(time(NULL));


    // setup window
    sf::ContextSettings window_settings;
    window_settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT), "3D-engine", sf::Style::Close | sf::Style::Resize, window_settings);
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);
    Mouse::setPosition(sf::Vector2i(INITIAL_WINDOW_WIDTH / 2, INITIAL_WINDOW_HEIGHT / 2), window);


    // create solid: colorful plane
    Solid3d colorful_plane;
    colorful_plane.add_segment(Segment3d(Vector3d(-150, 0, -150, sf::Color::Cyan ),
                                         Vector3d(150 , 0, -150, sf::Color::Blue )));
    colorful_plane.add_segment(Segment3d(Vector3d(150 , 0, -150, sf::Color::Blue ),
                                         Vector3d(150 , 0, 150 , sf::Color::Red  )));
    colorful_plane.add_segment(Segment3d(Vector3d(150 , 0, 150 , sf::Color::Red  ),
                                         Vector3d(-150, 0, 150 , sf::Color::Green)));
    colorful_plane.add_segment(Segment3d(Vector3d(-150, 0, 150 , sf::Color::Green),
                                         Vector3d(-150, 0, -150, sf::Color::Cyan )));

    // create solid: cube inside cube
    Cube3d big_cube(Vector3d(100, 0, 0), 50);
    Cube3d small_cube(Vector3d(100, 0, 0), 25);

    // create solid: rotating sphere
    Sphere3d sphere(Vector3d(-90, 0, 0), 40, 30, 50);

    // create camera
    Camera3d camera(Vector3d(0, -120, -230), -30, 0, 0, Parameters::window_width, Parameters::window_height);


    loop_timer.restart();
    current_time.restart();

    while (window.isOpen())
    {
        sf::Event event;

        // handle events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                Parameters::update_window_size(event.size.width, event.size.height);
                cout << "Window resized: "
                     << event.size.width
                     << " x "
                     << event.size.height << endl;
                camera.reload_frustrum(Parameters::window_width, Parameters::window_height);
            }
        }

        // rotate camera
        camera.rotate(Mouse::get_move_x(window), Mouse::get_move_y(window));

        // move camera
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            camera.move(Camera3d::DIRECTION::FRONT);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            camera.move(Camera3d::DIRECTION::BACK);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            camera.move(Camera3d::DIRECTION::RIGHT);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            camera.move(Camera3d::DIRECTION::LEFT);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            camera.move(Camera3d::DIRECTION::UP);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
            camera.move(Camera3d::DIRECTION::DOWN);

        // rendering
        window.clear();

        colorful_plane.render_solid(window, Parameters::window_width, Parameters::window_height, camera);

        small_cube.rotate(Vector3d(50, 0, 0), Vector3d(0, 1, 0), 1, false);
        big_cube.rotate(Vector3d(50, 0, 0), Vector3d(0, 1, 0), 1, false);

        small_cube.rotate(Vector3d(), Vector3d(1, 1, 0), 2, true);
        big_cube.render_solid(window, Parameters::window_width, Parameters::window_height, camera);
        small_cube.render_solid(window, Parameters::window_width, Parameters::window_height, camera);

        sphere.rotate(Vector3d(), Vector3d(0, 1, 1), 3, true);
        sphere.render_solid(window, Parameters::window_width, Parameters::window_height, camera);

        window.display();

        // other
        Parameters::print_mean_CPU_usage(cout, loop_timer.getElapsedTime().asMilliseconds());

        sf::sleep(sf::milliseconds(MAX_MAIN_LOOP_DURATION - loop_timer.getElapsedTime().asMilliseconds()));
        loop_timer.restart();
    }

    cout << endl;
    
	return 0;
}
#endif