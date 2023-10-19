#include <SFML/Graphics.hpp>
#include <math.h>
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DegreeRadian 0.0174533 // one degree in radians

void displayPlayer(sf::RenderWindow*);
void checkKeyboardInput();
void drawMap2D(sf::RenderWindow*);
void drawRays2D(sf::RenderWindow*);
float distance(float ax, float ay, float bx, float by, float angle);

float player_x, player_y, player_deltax, player_deltay, player_angle;
float player_size = 8.0f;
float speed = 0.01f;

int map_x = 8, map_y = 8, map_size = 64;
int map[] =
{
	1,1,1,1,1,1,1,1,
	1,0,0,0,0,0,0,1,
	1,0,1,0,0,1,0,1,
	1,0,0,0,0,0,0,1,
	1,0,1,0,0,1,0,1,
	1,0,0,1,1,0,0,1,
	1,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1
};

int main() {
	float windowHeight = 1024.0f;
	float windowWidth = 512.0f;

	sf::RenderWindow window(sf::VideoMode(windowHeight, windowWidth), "Raycaster");

	player_size = 8;
	player_x = player_y = 300;
	player_deltax = cos(player_angle) * 5;
	player_deltay = sin(player_angle) * 5;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}
		
		checkKeyboardInput();

		window.clear(sf::Color::Color(80, 80, 80, 255));
		drawMap2D(&window);
		displayPlayer(&window);
		drawRays2D(&window);
		window.display();
	}
	
	return 0;
}

void displayPlayer(sf::RenderWindow *win) {
	sf::RectangleShape player(sf::Vector2f(player_size, player_size));
	player.setFillColor(sf::Color::Yellow);
	player.setPosition(sf::Vector2f(player_x, player_y));
	win->draw(player);

	sf::RectangleShape viewPoint(sf::Vector2f(2, 2));
	viewPoint.setFillColor(sf::Color::Yellow);
	viewPoint.setPosition(sf::Vector2f(player_x + player_deltax * 5, player_y + player_deltay * 5));
	win->draw(viewPoint);
}

void checkKeyboardInput() {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { 
		player_angle -= 0.1f*speed; 
		if (player_angle < 0) { 
			player_angle += 2 * PI; 
		} 
		player_deltax = cos(player_angle) * 5;
		player_deltay = sin(player_angle) * 5;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { 
		player_angle += 0.1f*speed;
		if (player_angle > 2*PI) {
			player_angle -= 2*PI;
		}
		player_deltax = cos(player_angle) * 5;
		player_deltay = sin(player_angle) * 5;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { 
		player_x += player_deltax*speed;
		player_y += player_deltay*speed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { 
		player_x -= player_deltax*speed;
		player_y -= player_deltay*speed;
	}
}

void drawMap2D(sf::RenderWindow *win){
	int x, y, x_offset, y_offset;
	sf::Color color;
	for (y = 0; y < map_y; y++) {
		for (x = 0; x < map_x; x++) {
			if (map[y * map_x + x] == 1) {
				color = sf::Color::White;
			}
			else {
				color = sf::Color::Black;
			}
			x_offset = x * map_size;
			y_offset = y * map_size;
			sf::RectangleShape cell(sf::Vector2f(map_size- 1, map_size - 1));
			cell.setPosition(x_offset + 1, y_offset + 1);
			cell.setFillColor(color);
			win->draw(cell);
		}
	}
}

void drawRays2D(sf::RenderWindow* win) {
	int ray, mx, my, mp, depth_of_field;
	float ray_x, ray_y, ray_angle, x_offset, y_offset, distance_final;
	sf::Color wallColor;
	ray_angle = player_angle-DegreeRadian*30;
	if (ray_angle < 0) {
		ray_angle += 2 * PI;
	}
	if (ray_angle > 2 * PI) {
		ray_angle -= 2 * PI;
	}
	for (ray = 0; ray < 60; ray++) {
		//check horizontal lines
		depth_of_field = 0;
		float disH = 999999999; 
		float hx = player_x, hy = player_y;
		float aTan = -1 / tan(ray_angle);
		if (ray_angle > PI) {
			ray_y = (((int)player_y >> 6) << 6) - 0.0001; //round to nearest 64th with bit magic
			ray_x = (player_y - ray_y) * aTan + player_x;
			y_offset = -64;
			x_offset = -y_offset * aTan;
		}
		if (ray_angle < PI) {
			ray_y = (((int)player_y >> 6) << 6) + 64; //round to nearest 64th with bit magic
			ray_x = (player_y - ray_y) * aTan + player_x;
			y_offset = 64;
			x_offset = -y_offset * aTan;
		}
		if (ray_angle == 0 || ray_angle == PI) {
			ray_x = player_x;
			ray_y = player_y;
			depth_of_field = 8;
		}
		while (depth_of_field < 8) {
			mx = (int)(ray_x) >> 6;
			my = (int)(ray_y) >> 6;
			mp = my * map_x + mx;
			if (mp < map_x * map_y && mp > 0 &&map[mp] == 1 ) {
				hx = ray_x;
				hy = ray_y;
				disH = distance(player_x, player_y, hx, hy, ray_angle);
				depth_of_field = 8;
			}
			else {
				ray_x += x_offset;
				ray_y += y_offset;
				depth_of_field += 1;
			}

		}


		//check vertical lines
		depth_of_field = 0;
		float disV = 999999999;
		float vx = player_x, vy = player_y;
		float nTan = -tan(ray_angle);
		if (ray_angle > P2 && ray_angle < P3) {
			ray_x = (((int)player_x >> 6) << 6) - 0.0001; //round to nearest 64th with bit magic
			ray_y = (player_x - ray_x) * nTan + player_y;
			x_offset = -64;
			y_offset = -x_offset * nTan;
		}
		if (ray_angle < P2 || ray_angle > P3) {
			ray_x = (((int)player_x >> 6) << 6) + 64; //round to nearest 64th with bit magic
			ray_y = (player_x - ray_x) * nTan + player_y;
			x_offset = 64;
			y_offset = -x_offset * nTan;
		}
		if (ray_angle == 0 || ray_angle == PI) {
			ray_x = player_x;
			ray_y = player_y;
			depth_of_field = 8;
		}
		while (depth_of_field < 8) {
			mx = (int)(ray_x) >> 6;
			my = (int)(ray_y) >> 6;
			mp = my * map_x + mx;
			if (mp < map_x * map_y && mp > 0 && map[mp] == 1) {
				vx = ray_x;
				vy = ray_y;
				disV = distance(player_x, player_y, vx, vy, ray_angle);
				depth_of_field = 8;
			}
			else {
				ray_x += x_offset;
				ray_y += y_offset;
				depth_of_field += 1;
			}

		}
		if (disV < disH) {
			ray_x = vx;
			ray_y = vy;
			distance_final = disV;
			wallColor = sf::Color::Color(0, 200, 150);
		}
		if (disH < disV) {
			ray_x = hx;
			ray_y = hy;
			distance_final = disH;
			wallColor = sf::Color::Color(0, 180, 120);
		}
		sf::CircleShape rp(5.0f);
		rp.setFillColor(sf::Color::Red);
		rp.setPosition(ray_x, ray_y);
		win->draw(rp);

		//Draw 3D Walls

		float ca = player_angle - ray_angle;
		if (ca < 0) {
			ca += 2 * PI;
		}
		if (ca > 2 * PI) {
			ca -= 2 * PI;
		}
		distance_final *= cos(ca);

		float lineH = (map_size * 320) / distance_final;
		if (lineH > 320) {
			lineH = 320;
		}

		float lineOffset = 160 - lineH / 2;
		sf::RectangleShape line3d(sf::Vector2f(8, lineH));
		line3d.setPosition(sf::Vector2f(ray * 8 + 530, lineOffset));
		line3d.setFillColor(wallColor);
		win->draw(line3d);

		ray_angle += DegreeRadian;
		if (ray_angle < 0) {
			ray_angle += 2 * PI;
		}
		if (ray_angle > 2 * PI) {
			ray_angle -= 2 * PI;
		}
	}
}

float distance(float ax, float ay, float bx, float by, float angle) {
	return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}