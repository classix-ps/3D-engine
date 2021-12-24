#include "utils/mouse.hpp"
#include "utils/parameters.hpp"
#include "geometry/camera3d.hpp"
#include "geometry/solid3d.hpp"
#include "geometry/geometry.hpp"

#include <future>

#define USAGE

std::atomic_bool shapeReady = false;
std::atomic_bool quit = false;

std::string getStats(const Solid3d& shape) {
  std::string stats;

  size_t faces, edges, vertices;
  std::map<Vector3d, size_t> edgesPerVertex;
  
  edges = shape.edges.size();
  for (const Segment3d& edge : shape.edges) {
    edgesPerVertex[edge.a]++;
    edgesPerVertex[edge.b]++;
  }
  vertices = edgesPerVertex.size();
  faces = edges - vertices + 2;

  std::map<size_t, size_t> edgesPerVertexOccurences;
  for (const auto& vertex : edgesPerVertex) {
    edgesPerVertexOccurences[vertex.second]++;
  }

  stats += "# of faces: " + std::to_string(faces) + "\n";
  stats += "# of edges: " + std::to_string(edges) + "\n";
  stats += "# of vertices: " + std::to_string(vertices) + "\n";
  stats += "Edges per vertex:\n";
  for (const auto& edgesCount : edgesPerVertexOccurences) {
    stats += "\t" + std::to_string(edgesCount.first) + " edges: " + std::to_string(edgesCount.second) + " occurences\n";
  }

  return stats;
}

Solid3d getNextShape(const Solid3d& shape) {
  std::map<Vector3d, std::vector<Vector3d>> kMap;
  for (const Segment3d& edge : shape.edges) {
    if (quit) {
      return shape;
    }

    Vector3d midpoint = (edge.a + edge.b) * 0.5;
    midpoint.set_color(sf::Color::White);
    kMap[edge.a].push_back(midpoint);
    kMap[edge.b].push_back(midpoint);
  }

  Solid3d nextShape;
  for (const auto& vertex : kMap) {
    if (quit) {
      return shape;
    }

    std::vector<Vector3d> midpoints = vertex.second;

    // order points to be connected in correct order to create polygon
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

    // connect midpoints of vertex
    for (size_t i = 0; i < midpoints.size(); i++) {
      Segment3d edge = Segment3d(midpoints[i], midpoints[(i + 1) % midpoints.size()]);

      if (std::find(nextShape.edges.begin(), nextShape.edges.end(), edge) == nextShape.edges.end()) {
        nextShape.add_segment(Segment3d(midpoints[i], midpoints[(i + 1) % midpoints.size()]));
      }
    }
  }

  shapeReady = true;
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
  Camera3d camera(Vector3d(0, -120, -230), -10, 0, 0, Parameters::window_width, Parameters::window_height);

  sf::Clock loop_timer;
  sf::Clock load_timer;
  srand(time(NULL));
  loop_timer.restart();
  load_timer.restart();

  Solid3d k;
  k.add_segment(Segment3d(Vector3d(-100, 0, 0, sf::Color::White), Vector3d(100, 0, 0, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(100, 0, 0, sf::Color::White), Vector3d(0, 0, 100 * sqrt(3), sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(0, 0, 100 * sqrt(3), sf::Color::White), Vector3d(-100, 0, 0, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(-100, 0, 0, sf::Color::White), Vector3d(0, -sqrt(square(200) - square(100) - square(100 * sqrt(3) / 3)), 100 * sqrt(3) / 3, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(100, 0, 0, sf::Color::White), Vector3d(0, -sqrt(square(200) - square(100) - square(100 * sqrt(3) / 3)), 100 * sqrt(3) / 3, sf::Color::White)));
  k.add_segment(Segment3d(Vector3d(0, 0, 100 * sqrt(3), sf::Color::White), Vector3d(0, -sqrt(square(200) - square(100) - square(100 * sqrt(3) / 3)), 100 * sqrt(3) / 3, sf::Color::White)));

  sf::Font font;
  font.loadFromFile("../Resources/arial.ttf");

  sf::Text iterText("0", font, 32);
  iterText.setPosition(5.f, 0.f);

  sf::Text statHeader("Shape statistics", font, 32);
  statHeader.setStyle(sf::Text::Underlined);
  statHeader.setPosition(5.f, 70.f);
  sf::Text statText(getStats(k), font, 32);
  statText.setPosition(5.f, 105.f);

  sf::Text loadingText("Loading next shape...", font, 32);
  loadingText.setFillColor(sf::Color(80, 80, 80));
  loadingText.setPosition(800.f, 1120.f);

  std::future<Solid3d> newK;

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

      if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && !shapeReady) {
        newK = std::async(std::launch::async, getNextShape, k);
        load_timer.restart();
      }
    }

    // update shape (if needed)
    if (shapeReady) {
      k = newK.get();
      iterText.setString(std::to_string(std::stoi(iterText.getString().toAnsiString()) + 1));
      statText.setString(getStats(k));
      shapeReady = false;
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
    window.draw(iterText);
    window.draw(statHeader);
    window.draw(statText);
    if (newK.valid()) {
      if (load_timer.getElapsedTime().asMilliseconds() > 500) {
        std::string loading = loadingText.getString().toAnsiString();
        size_t dots = loading.size() - loading.find('.');
        if (dots == 3) {
          loadingText.setString(loading.substr(0, loading.size() - 3));
        }
        else {
          loadingText.setString(loading + '.');
        }
        load_timer.restart();
      }
      window.draw(loadingText);
    }

    window.display();

    // other
#ifdef USAGE
    Parameters::print_mean_CPU_usage(std::cout, loop_timer.getElapsedTime().asMilliseconds());
#endif

    sf::sleep(sf::milliseconds(MAX_MAIN_LOOP_DURATION - loop_timer.getElapsedTime().asMilliseconds()));
    loop_timer.restart();
  }

  quit = true;
  if (newK.valid()) {
    newK.wait();
  }
  return EXIT_SUCCESS;
}

/*
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
*/