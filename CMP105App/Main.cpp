#include<iostream>
#include<string.h>
#include<SFML/Graphics.hpp>
#include"Framework/Input.h"
#include<SFML/Graphics/Sprite.hpp>
#include"ThreadPool.h"
#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <time.h>
#include <thread>
#include <mutex>
#include <vector>
#include<atomic>

using std::chrono::duration_cast;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::complex;
using std::cout;
using std::endl;
using std::ofstream;
using std::thread;
using std::atomic;
using std::vector;
using std::mutex;

typedef std::chrono::steady_clock the_clock;
the_clock::time_point start;
the_clock::time_point stop;

const int width = 1280;
const int height = 720;
sf::VertexArray pixels(sf::Points, width* height);

int num_threads = std::thread::hardware_concurrency();

const float segmentHeight = height / num_threads;
const float segmentWidth = 0;

float zoom = 300.0f;
int numberofIterations = 100;
int xoffset = width / 2;
int yoffset = height / 2;

bool	loopThread = true;

//used for complex numbers
struct complex_number
{
	long double real;
	long double imaginary;
};

void compute_mandelbrot(double start, double end, int xoffset, int yoffset, int numberOfIterations, float zoom)
{

	//	std::unique_lock<std::mutex> lock();
		for (int yi = start; yi < end; yi++)
		{
			for (int xi = 0; xi < width; xi++)
			{
				//scale the pixel location to the complex plane for calculations
				long double x = ((long double)xi - xoffset) / zoom;
				long double y = ((long double)yi - yoffset) / zoom;
				complex_number c;
				c.real = x;
				c.imaginary = y;
				complex_number z = c;
				int iterations = 0; //keep track of the number of iterations
				for (int k = 0; k < numberOfIterations; k++)
				{
					complex_number z2;
					z2.real = z.real * z.real - z.imaginary * z.imaginary;
					z2.imaginary = 2 * z.real * z.imaginary;
					z2.real += c.real;
					z2.imaginary += c.imaginary;
					z = z2;
					iterations++;
					if (z.real * z.real + z.imaginary * z.imaginary > 4)
						break;
				}
				//color pixel based on the number of iterations
				if (iterations < numberOfIterations / 4.0f)
				{
					pixels[yi * width + xi].position = sf::Vector2f(xi, yi);		
					sf::Color color(iterations * 255.0f / (numberOfIterations / 4.0f), 0, 0);
					pixels[yi * width + xi].color = color;
				}
				else if (iterations < numberOfIterations / 2.0f)
				{
					pixels[yi * width + xi].position = sf::Vector2f(xi, yi);
					sf::Color color(0, iterations * 255.0f / (numberOfIterations / 2.0f), 0);
					pixels[yi * width + xi].color = color;
				}
				else if (iterations < numberOfIterations)
				{
					pixels[yi * width + xi].position = sf::Vector2f(xi, yi);
					sf::Color color(0, 0, iterations * 255.0f / numberOfIterations);
					pixels[yi * width + xi].color = color;
				}
			}
		}
	

}


int main()
{

	ThreadPool pool(num_threads);

	//create Input and window 
	sf::String title_string = "Mandelbrot";
	sf::RenderWindow window(sf::VideoMode(width, height), title_string);
	//window.setFramerateLimit(30);

	//sf::VertexArray mandelbrot(sf::Points, width * height);
	sf::Font font;
	font.loadFromFile("font/arial.ttf");
	sf::Text text;

	// select the font
	text.setFont(font); // font is a sf::Font

	// set the string to display
	text.setString("Hello world");

	// set the character size
	text.setCharacterSize(24); // in pixels, not points!

	// set the color
	text.setFillColor(sf::Color::Red);

	// set the text style
	text.setStyle(sf::Text::Bold | sf::Text::Underlined);


	for (int i = 0; i < num_threads; i++)
	{
		
		compute_mandelbrot((height / num_threads) * (i), (height / num_threads) * (i + 1), xoffset, yoffset, numberofIterations, zoom);
	
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		static bool was_pressed = false;
		//zoom into area that is left clicked
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			if (!was_pressed) {
				was_pressed = true;
				sf::Vector2i position = sf::Mouse::getPosition(window);
				xoffset -= position.x - xoffset;
				yoffset -= position.y - yoffset;
				zoom *= 2;
				numberofIterations += 200;
				for (int i = 0; i < width * height; i++)
				{
					pixels[i].color = sf::Color::Black;
				}

				for (int i = 0; i < num_threads; i++)
				{
					pool.enqueue(compute_mandelbrot, (height / num_threads) * (i), (height / num_threads) * (i + 1), xoffset, yoffset, numberofIterations, zoom);
				}
				while (!pool.isEmpty());
			}
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			if (!was_pressed) {
				was_pressed = true;
				sf::Vector2i position = sf::Mouse::getPosition(window);
				xoffset -= position.x - xoffset;
				yoffset += position.y - yoffset;
				zoom /= 2;
				numberofIterations -= 50;
				for (int i = 0; i < width * height; i++)
				{
					pixels[i].color = sf::Color::Black;
				}

				for (int i = 0; i < num_threads; i++)
				{
					pool.enqueue(compute_mandelbrot, (height / num_threads) * (i), (height / num_threads) * (i + 1), xoffset, yoffset, numberofIterations, zoom);
				}
				while (!pool.isEmpty());
			}
		}
		else
			was_pressed = false;
		//compute_mandelbrot(xoffset, yoffset, numberofIterations, zoom);
		window.clear();
		window.draw(pixels);
		window.draw(text);
		window.display();
	}
	
	return 0;
}

