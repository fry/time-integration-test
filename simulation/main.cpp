#include <iostream>
#include <vector>

#include <x3d/Vector3.hpp>
#include <SFML/Graphics.hpp>
#include "Camera.hpp"
#include <sstream>

template<class T, class U>
std::vector<T> integrate(U& data, T start_val, double start_time, double end_time, double step_size, T (*calc)(U& data, const T& y, double t)) {
	int n = std::ceil((end_time - start_time) / step_size);
	std::vector<T> result_data;
	result_data.reserve(n + 1);
	result_data.push_back(start_val);

	T current_val = start_val;
	double current_time = start_time;
	for (int i = 0; i < n; i++) {
		current_val += step_size * calc(data, current_val, current_time);
		result_data.push_back(current_val);
		current_time += step_size;
	}

	return result_data;
}

struct lorenz_data {
	double sigma, r, b;
};

x3d::Vector3d lorenz_step(lorenz_data& data, const x3d::Vector3d& y, double t) {
	x3d::Vector3d result;
	result.x = data.sigma * (y.y - y.x);
	result.y = (data.r - y.z) * y.x - y.y;
	result.z = y.x * y.y - data.b * y.z;
	return result;
}

void draw_line_set(std::vector<x3d::Vector3d>& points) {
	if (points.empty())
		return;

	auto end = points.end();
	auto iter = points.begin();

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, &points.front());
	glDrawArrays(GL_LINE_STRIP, 0, points.size());
	glDisableClientState(GL_VERTEX_ARRAY);
}

int main(int argc, char** argv) {
	lorenz_data data;
	data.sigma = 10;
	data.r = 28;
	data.b = 8.0 / 3.0;

	x3d::Vector3d start;
	start.x = 0;
	start.y = 1;
	start.z = 0;

	double time_end = 50;
	double time_step = 0.0001;
	auto result = integrate(data, start, 0, time_end, time_step, lorenz_step);

	const int screen_width = 800;
	const int screen_height = 600;
  sf::RenderWindow app(sf::VideoMode(screen_width, screen_height), "wnd");
	const int center_x = screen_width / 2;
  const int center_y = screen_height / 2;
	const sf::Input& input = app.GetInput();

  app.PreserveOpenGLStates(true);

  // Enable Z-buffer read and write
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glClearDepth(1.f);

  // Setup a perspective projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.f, (float) screen_width / screen_height, 0.1f, 500.f);

	glClearColor(0, 0, 0, 1);

	Camera cam;
	cam.move_global(x3d::Vector3f(0, 0, 10));
	float move_amount = 0.015;
	bool is_mouse_looking = false;

  while (app.IsOpened()) {
    sf::Event event;
    while (app.GetEvent(event)) {
      // Close window : exit
      if (event.Type == sf::Event::Closed)
          app.Close();

      // Escape key : exit
      if (event.Type == sf::Event::KeyPressed) {
				if (event.Key.Code == sf::Key::Escape) {
          app.Close();
				} else if (event.Key.Code == sf::Key::Num2) { // Increase speed
          move_amount *= 2;
        } else if (event.Key.Code == sf::Key::Num1) { // Decrease speed
          move_amount /= 2;
        } else if (event.Key.Code == sf::Key::Q) {
					data.r --;
					result = integrate(data, start, 0, time_end, time_step, lorenz_step);
				} else if (event.Key.Code == sf::Key::E) {
					data.r ++;
					result = integrate(data, start, 0, time_end, time_step, lorenz_step);
				}
      } else if (event.Type == sf::Event::MouseButtonPressed) {
        // Hide and center mouse when pressing right mouse button
        if (event.MouseButton.Button == sf::Mouse::Right) {
          app.SetCursorPosition(center_x, center_y);
          app.ShowMouseCursor(false);
          is_mouse_looking = true;
        }
      } else if (event.Type == sf::Event::MouseButtonReleased) {
        // Show mouse again when releasing
        if (event.MouseButton.Button == sf::Mouse::Right) {
          app.ShowMouseCursor(true);
          is_mouse_looking = false;
        }
      } else if (event.Type == sf::Event::MouseMoved) {
        // Adjust camera when pressing right mouse button
        if (is_mouse_looking) {
          const int rel_x = event.MouseMove.X - center_x;
          const int rel_y = event.MouseMove.Y - center_y;

          const float factor = 1 / 60.0;
          cam.rotate_global(rel_x * factor, x3d::Vector3f::UnitY);
          cam.rotate_local(rel_y * factor, x3d::Vector3f::UnitX);

          app.SetCursorPosition(center_x, center_y);
        }
			}

      // Adjust the viewport when the window is resized
      if (event.Type == sf::Event::Resized)
          glViewport(0, 0, event.Size.Width, event.Size.Height);
    }

    // Update camera movement
    
    if (input.IsKeyDown(sf::Key::W)) {
      cam.move_local(x3d::Vector3f::UnitZ * move_amount);
    } 
    if (input.IsKeyDown(sf::Key::S)) {
      cam.move_local(-x3d::Vector3f::UnitZ * move_amount);
    }
    if (input.IsKeyDown(sf::Key::A)) {
      cam.move_local(-x3d::Vector3f::UnitX * move_amount);
    }
    if (input.IsKeyDown(sf::Key::D)) {
      cam.move_local(x3d::Vector3f::UnitX * move_amount);
    }

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glLoadMatrixf(cam.apply());
		draw_line_set(result);

		std::ostringstream stream;
		stream << "Sigma: " << data.sigma << std::endl
					 << "R: " << data.r << std::endl
					 << "B: " << data.b << std::endl;

		sf::String text(stream.str());
		text.SetPosition(10, 10);
		text.SetColor(sf::Color::White);
		text.SetSize(14);
		app.Draw(text);

		app.Display();
	}				
}
