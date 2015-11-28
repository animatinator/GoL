// Game of Life with an SDL GUI

# include "SDL/SDL.h"
# include "SDL/SDL_image.h"
# include "SDL/SDL_draw.h"

# include <iostream>
# include <string>
# include <vector>
# include <fstream>
using namespace std;

// Typedefs
typedef unsigned short int ushort;
typedef unsigned long int ulong;

enum STATE {DEAD = 0, ALIVE = 1};  // Used to determine whether the cell is alive or dead

struct cell
{  // A cell
	STATE state;
};

struct coords
{
	ushort x;
	ushort y;
};

// Simulation constants
const int CELL_SIZE = 4;
const int GRID_WIDTH = 200;
const int GRID_HEIGHT = 150;

// Screen constants
const int SCREEN_WIDTH = (CELL_SIZE * GRID_WIDTH);
const int SCREEN_HEIGHT = (CELL_SIZE * GRID_HEIGHT);
const int SCREEN_BPP = 32;

// The event structure
SDL_Event event;

// The surfaces
SDL_Surface* screen = NULL;


class CellGrid
{
	public:
		// Constructor and destructor
		CellGrid();
		~CellGrid();
		
		// Redraw/update
		void DrawCells(SDL_Surface* screen);
		void update();
		
		// Cell manipulation
		void ActivateCell(int col, int row);
		void KillCell(int col, int row);
		void Clear();
		
		vector<coords> GetLiveCells();
		vector<coords> GetDeadCells();
		
		// Neighbours
		ushort GetNeighbours(int col, int row);
		ushort TestNeighbour(int col, int row);
		
		// Getters and setters
		ulong GetGeneration() {return generation;}
		
		void TogglePause() {if (paused == true) paused = false; else paused = true;}
		
	private:
		ulong generation;
		bool paused;
		
		cell cells[GRID_HEIGHT][GRID_WIDTH];
};


// Function declarations
SDL_Surface * load_image(string filename);
void apply_surface(int x, int y, SDL_Surface *source, SDL_Surface *destination, SDL_Rect *clip);
void redraw(CellGrid* cells, bool showgrid);
void clean_up(CellGrid* cells);


int main(int argc, char* args[])
{
	bool quit = false;  // Has the user quit?
	bool showgrid = true;  // Should the grid be drawn?
	
	coords mousepos;
	
	// Initialise everything
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return 1;  // An error occurred
	}
	
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	
	// If there was an error in setting up the screen
	if (screen == NULL)
	{
		return 1;
	}
	
	// Set the screen caption
	SDL_WM_SetCaption("Game of Life", NULL);
	
	// Create the cell grid
	CellGrid* cells = new CellGrid();
	
	// While the user hasn't quit
	while (quit == false)
	{
		// While there's an event to handle
		while (SDL_PollEvent(&event))
		{
			// If the user has quit
			if (event.type == SDL_QUIT)
			{
				quit = true;  // This exits the loop
			}
			
			// Mouse motion
			else if (event.type == SDL_MOUSEMOTION)
			{
				mousepos.x = event.motion.x;
				mousepos.y = event.motion.y;
				
				// Mouse button held
				if (SDL_BUTTON(SDL_GetMouseState(NULL, NULL)) == SDL_BUTTON_LEFT)
				{
					int col, row;
					
					col = (mousepos.x / CELL_SIZE);
					row = (mousepos.y / CELL_SIZE);
					
					cells->ActivateCell(col, row);
				}
				
				else if (SDL_BUTTON(SDL_GetMouseState(NULL, NULL)) == SDL_BUTTON_MIDDLE)
				{
					int col, row;
					
					col = (mousepos.x / CELL_SIZE);
					row = (mousepos.y / CELL_SIZE);
					
					cells->KillCell(col, row);
				}
			}
			
			// Mouse click
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (SDL_BUTTON(SDL_GetMouseState(NULL, NULL)) == SDL_BUTTON_LEFT)
				{
					int col, row;
					
					col = (mousepos.x / CELL_SIZE);
					row = (mousepos.y / CELL_SIZE);
					
					cells->ActivateCell(col, row);
				}
				
				else if (SDL_BUTTON(SDL_GetMouseState(NULL, NULL)) == SDL_BUTTON_MIDDLE)
				{
					int col, row;
					
					col = (mousepos.x / CELL_SIZE);
					row = (mousepos.y / CELL_SIZE);
					
					cells->KillCell(col, row);
				}
			}
			
			// Keypress
			else if (event.type == SDL_KEYDOWN)
			{
				switch(event.key.keysym.sym)
				{
					case SDLK_p:
						cells->TogglePause();
						break;
						
					case SDLK_r:
						cells->Clear();
						break;
						
					case SDLK_g:
						if (showgrid) showgrid = false;
						else showgrid = true;
						break;
				}
			}
		}
		
		// Redraw
		redraw(cells, showgrid);
		
		// Update the screen
		if (SDL_Flip(screen) == -1)
		{
			return 1;  // An error occurred
		}
		
		// Update the cells
		cells->update();
		
		// Restrict the framerate (so as to limit CPU usage)
		SDL_Delay(10);
	}
	
	clean_up(cells);
	
		return 0;
}


// Function definitions
SDL_Surface* load_image(string filename)
{
	// Temporary storage for the image that's being loaded
	SDL_Surface* loadedimage = NULL;
	// The optimised image that will be used
	SDL_Surface* optimisedimage = NULL;
	
	// Load the image
	loadedimage = IMG_Load(filename.c_str());
	
	// If nothing went wrong when loading
	if (loadedimage != NULL)
	{
		// Create an optimised image
		optimisedimage = SDL_DisplayFormat(loadedimage);
		
		// Free the old image
		SDL_FreeSurface(loadedimage);
		
		return optimisedimage;
	}
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL)
{
	// A rect to hold offsets
	SDL_Rect offset;
	
	// Get offsets
	offset.x = x;
	offset.y = y;
	
	// Blit
	SDL_BlitSurface(source, clip, destination, &offset);
}

void redraw(CellGrid* cells, bool showgrid)
{
	SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xAA, 0xFF, 0xAA));
	
	cells->DrawCells(screen);
	
	if (showgrid)
	{// Draw a grid
		for(int i = 0; i < SCREEN_HEIGHT; i += CELL_SIZE)
		{
			Draw_Line(screen, 0, i, (SCREEN_WIDTH - 1), i, SDL_MapRGB(screen->format, 0xAA, 0xAA, 0xAA));
		}
		for (int i = 0; i < SCREEN_WIDTH; i += CELL_SIZE)
		{
			Draw_Line(screen, i, 0, i, (SCREEN_HEIGHT - 1), SDL_MapRGB(screen->format, 0xAA, 0xAA, 0xAA));
		}
	}
}

void clean_up(CellGrid* cells)
{
	// Delete the cell grid
	delete(cells);
	
	// Exit SDL
	SDL_Quit();
}


CellGrid::CellGrid()
{
	generation = 0;
	paused = true;
	
	Clear();
	
	// Begin R-pentomino creation
	cells[73][100].state = ALIVE;
	cells[73][101].state = ALIVE;
	cells[74][99].state = ALIVE;
	cells[74][100].state = ALIVE;
	cells[75][100].state = ALIVE;
	// End R-pentomino creation
}

CellGrid::~CellGrid()
{

}

void CellGrid::DrawCells(SDL_Surface* screen)
{
	for (int i = 0; i < GRID_HEIGHT; i++)
	{  // For every row
		for (int j = 0; j < GRID_WIDTH; j++)
		{  // For each item in the row
			STATE cellstate = cells[i][j].state;
			
			switch(cellstate)
			{
				case ALIVE:
					Draw_FillRect(screen, j*CELL_SIZE, i*CELL_SIZE, CELL_SIZE, CELL_SIZE, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
					break;
					
				case DEAD:
					Draw_FillRect(screen, j*CELL_SIZE, i*CELL_SIZE, CELL_SIZE, CELL_SIZE, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
					break;
			}
		}
	}
}

void CellGrid::update()
{
	if (paused) return;
	
	generation += 1;
	
	vector<coords> livecells = GetLiveCells();  // Get the living cells
	vector<coords> deadcells = GetDeadCells();  // Get the dead cells
	
	for (int dead = 0; dead < deadcells.size(); dead++)
	{  // Kill dead cells
	coords cellpos = deadcells.at(dead);
	cells[cellpos.y][cellpos.x].state = DEAD;
	}
	
	if (livecells.size() == 0) return;  // Return if there are no living cells to activate
	
	for (int live = 0; live < livecells.size(); live++)
	{  // Activating living cells
		coords cellpos = livecells.at(live);
		cells[cellpos.y][cellpos.x].state = ALIVE;
	}
}

void CellGrid::ActivateCell(int col, int row)
{
	cells[row][col].state = ALIVE;
}

void CellGrid::KillCell(int col, int row)
{
	cells[row][col].state = DEAD;
}

void CellGrid::Clear()
{
	generation = 0;
	
	for (int i = 0; i < GRID_HEIGHT; i++)
	{  // For every row
		for (int j = 0; j < GRID_WIDTH; j++)
		{  // For each item in the row
			cells[i][j].state = DEAD;
		}
	}
}

vector<coords> CellGrid::GetLiveCells()
{
	vector<coords> livecells;  // A vector to hold the living cells
	
	for (int i = 0; i < GRID_HEIGHT; i++)
	{  // For each row
		for (int j = 0; j < GRID_WIDTH; j++)
		{  // For each column in the row
			int neighbours = GetNeighbours(j, i);  // Get the number of neighbours
			
			if (cells[i][j].state == ALIVE && (neighbours == 2 || neighbours == 3))
			{  // Living cells survive if they have either two or three neighbours
				// Create the new Coords variable
				coords newcoords;
				newcoords.x = j;
				newcoords.y = i;
				// Append it to the vector
				livecells.push_back(newcoords);
			}
			
			else
			{
				if (neighbours == 3)
				{  // If the cell is not currently alive, it must have three neighbours to be born
					// Create the new Coords variable
					coords newcoords;
					newcoords.x = j;
					newcoords.y = i;
					// Append it to the vector
					livecells.push_back(newcoords);
				}
			}
		}
	}
	
		return livecells;
}

vector<coords> CellGrid::GetDeadCells()
{
	vector<coords> deadcells;  // A vector to hold the dead cells
	
	for (int i = 0; i < GRID_HEIGHT; i++)
	{  // For each row
		for (int j = 0; j < GRID_WIDTH; j++)
		{  // For each column in the row
			int neighbours = GetNeighbours(j, i);  // Get the number of neighbours
			
			if (neighbours < 2 || neighbours > 3)
			{  // If there are fewer than two or more than three neighbours, the cell dies from loneliness or overcrowding
				// Create the new Coords variable
				coords newcoords;
				newcoords.x = j;
				newcoords.y = i;
				// Append it to the vector
				deadcells.push_back(newcoords);
			}
		}
	}
	
		return deadcells;
}

ushort CellGrid::GetNeighbours(int col, int row)
{
	ushort neighbours = 0;  // The number of neighbours found
	
	// Test up, down, left, right
	neighbours += TestNeighbour(col-1, row);
	neighbours += TestNeighbour(col+1, row);
	neighbours += TestNeighbour(col, row-1);
	neighbours += TestNeighbour(col, row+1);
	// Test diagonals
	neighbours += TestNeighbour(col-1, row-1);
	neighbours += TestNeighbour(col-1, row+1);
	neighbours += TestNeighbour(col+1, row-1);
	neighbours += TestNeighbour(col+1, row+1);
	
		return neighbours;
}

ushort CellGrid::TestNeighbour(int col, int row)
{
	ushort IsNeighbour = 0;  // A boolean of sorts
	
	// Make sure it's within the array boundaries - return 0 (false) otherwise
	if (col < 0) return 0;
	if (col > GRID_WIDTH - 1) return 0;
	if (row < 0) return 0;
	if (row > GRID_HEIGHT) return 0;
	
	if (cells[row][col].state == ALIVE)
	{  // If the cell is alive, it's a neighbour
		IsNeighbour = 1;
	}
	
		return IsNeighbour;
}


// END OF PROGRAM
