#include <iostream>
#include <math.h>
#include <SFML/Graphics.hpp>

const int WIDTH  = 1000;
const int HEIGHT = 800;
const int SCL = 50;
const int COLS = WIDTH / SCL; const int ROWS = HEIGHT / SCL;
const int POPULATION_SIZE = 188;
const int GRAPH_SIZE = 188;
const float RADIUS = 7;
const float PI = 3.14159265;
const float SPEED = 1;

sf::Uint8 *pixels = new sf::Uint8[WIDTH * GRAPH_SIZE * 4];
int nProgression = 0;
int nOffset = 1;

class Person;
std::vector<Person*> cells[COLS * ROWS];

float dist(sf::Vector2f v1, sf::Vector2f v2) { return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y)); }
int modulo(int a,int N) { return (a % N + N) %N; }
sf::Vector2f vectorFromAngle(float angle, float mag) { return sf::Vector2f(cos(angle) * mag, sin(angle) * mag); }

class Person
{
private:
	sf::CircleShape circle;

public:
	sf::Vector2f vPos;
	sf::Vector2f vVel;
	sf::Vector2f vCurVel;
	int cellX, cellY;
	bool bInfected, bHealed;
	int nTimer;
	bool bMove;	

	Person()
	{
		vPos.x = RADIUS + rand() % (int)(WIDTH - 2 * RADIUS);  vPos.y = RADIUS + rand() % (int)(HEIGHT - 2 * RADIUS - GRAPH_SIZE);
		vVel = vectorFromAngle((float)rand() / (float)RAND_MAX * 2*PI, SPEED);
		vCurVel = vVel;
		circle = sf::CircleShape(RADIUS, 32);
		bInfected = false; bHealed = false;
		nTimer = 0;
		bMove = (rand() < RAND_MAX * 0.5);
	}

	~Person()
	{
	}
	
	bool collision()
	{
		sf::Vector2f vVelCopy;
		int x, y;
		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
			{
				x = modulo(cellX + i, COLS);
				y = modulo(cellY + j, ROWS);
				for (Person* p : cells[x + y * COLS])
				{
					if (p != this)
					{
						if (dist(vPos, p->vPos) < 2 * RADIUS && p->bInfected && !bHealed)
							bInfected = true;
					}
				}
			}
		
		return false;
	}

	void move()
	{
		if (bMove)
			vPos += vVel;
		
		if (vPos.x < RADIUS || vPos.x > WIDTH - RADIUS)
			vVel.x *= -1;
		if (vPos.y < RADIUS || vPos.y > HEIGHT - RADIUS - GRAPH_SIZE)
			vVel.y *= -1;	

		if (bInfected)
			nTimer++;

		if (nTimer > 1200) // 20 sec
		{
			bInfected = false;
			bHealed = true;
		}	

		circle.setPosition(vPos.x - RADIUS, vPos.y - RADIUS);
	}

	void display(sf::RenderWindow &_window) 
	{
		if (bInfected)
			circle.setFillColor(sf::Color(240, 100, 150));
		else if (bHealed)
			circle.setFillColor(sf::Color(150, 210, 190));
		else
			circle.setFillColor(sf::Color(200, 10, 220)); 
		_window.draw(circle); 
	}
};

void updateGraph(int nInfected, int nHealthy, int nHealed)
{
	int nStartX = nOffset * nProgression;
	
	if (nStartX + nOffset > WIDTH)
		return;
	
	for (int x = nStartX; x < nStartX + nOffset; x++)
	{
		for (int y = 0; y < nHealthy * GRAPH_SIZE / POPULATION_SIZE; y++)
		{
			pixels[(x + WIDTH*y)*4 + 0] = 200;	//r
			pixels[(x + WIDTH*y)*4 + 1] = 10;	//g	
			pixels[(x + WIDTH*y)*4 + 2] = 220;	//b	
		}
		
		for (int y = nHealthy * GRAPH_SIZE / POPULATION_SIZE; y < (nHealthy + nHealed) * GRAPH_SIZE / POPULATION_SIZE; y++)
		{
			pixels[(x + WIDTH*y)*4 + 0] = 150;	//r
			pixels[(x + WIDTH*y)*4 + 1] = 210;	//g	
			pixels[(x + WIDTH*y)*4 + 2] = 190;	//b	
		}
	
		for (int y = (nHealthy + nHealed) * GRAPH_SIZE / POPULATION_SIZE; y < GRAPH_SIZE; y++)
		{
			pixels[(x + WIDTH*y)*4 + 0] = 240;	//r
			pixels[(x + WIDTH*y)*4 + 1] = 100;	//g	
			pixels[(x + WIDTH*y)*4 + 2] = 150;	//b	
		}
	}
}

int main()
{
	srand(std::time(0));
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Simulitis - A Simple Pandemic Simulation");
	window.setFramerateLimit(60);
	int nFrameCount = 0;

	for (int i = 0; i < WIDTH * GRAPH_SIZE * 4; i++)
		pixels[i] = 255;
	
	sf::Texture texture;
	texture.create(WIDTH, GRAPH_SIZE);
	sf::Sprite sprite(texture);
	sprite.setPosition(0, HEIGHT - GRAPH_SIZE);

	Person people[POPULATION_SIZE];
	people[0].bInfected = true;
	
	int nHealthy, nInfected, nHealed;	
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		
		window.clear(sf::Color(208, 217, 237));
		
		int x, y;
		for (Person &p : people)
		{
			// Define cell
			x = (int)p.vPos.x / SCL; y = (int)p.vPos.y / SCL;
			cells[x + y * COLS].push_back(&p);
			
			p.cellX = x; p.cellY = y;
		}
	
		nInfected = 0; nHealthy = 0; nHealed = 0;	
		for (Person &p : people)
		{
			p.collision();
			p.move();
			p.display(window);
		
			nInfected += (p.bInfected == true);
			nHealthy += (p.bInfected == false && p.bHealed == false);
			nHealed += (p.bHealed == true);
		}

		for (std::vector<Person*> &cell : cells)
			cell.clear();
		
		if (nFrameCount % 5 == 0)		
		{
			updateGraph(nInfected, nHealthy, nHealed);
			nProgression++;
		}
		nFrameCount++;

		texture.update(pixels);
		window.draw(sprite);
	
		window.display();
	
	}	
}
