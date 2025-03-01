#include <RGBmatrixPanel.h>

// Most of the signal pins are configurable, but the CLK pin has some
// special constraints.  On 8-bit AVR boards it must be on PORTB...
// Pin 11 works on the Arduino Mega.  On 32-bit SAMD boards it must be
// on the same PORT as the RGB data pins (D2-D7)...
// Pin 8 works on the Adafruit Metro M0 or Arduino Zero,
// Pin A4 works on the Adafruit Metro M4 (if using the Adafruit RGB
// Matrix Shield, cut trace between CLK pads and run a wire to A4).

#define CLK 11
#define OE   9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3

const int grid_size = 32;
int posx = grid_size/2-2;
int posy = grid_size-3;
int lastposx = posx;
int lastposy = posy;

int jump_sequence = 0;

int coinGrid[grid_size][grid_size] = {0};

const int grid_1[grid_size][grid_size] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
    {1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

int num_from_pos(int& x, int& y, int& num)
{
  num = x+y*grid_size;
}

void pos_from_num(int& x, int& y, int& num)
{
  x = num%grid_size;
  y = num/grid_size;
};

void draw_matrix(const int (&grid)[grid_size][grid_size])
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  for (int x = 0; x < grid_size; ++x)
  {
    for (int y = 0; y < grid_size; ++y)
    {
      if (grid[y][x])    {matrix.drawPixel(x,y,matrix.Color333(7,7,7));}
    }
  }
}

void draw_player()
{
  matrix.fillRect(lastposx,lastposy,2,2,matrix.Color333(0,0,0));
  matrix.fillRect(posx,posy,2,2, matrix.Color333(0,7,0));
}

bool object_in_way(int x, int y, const int (&grid)[grid_size][grid_size])
{
  if (grid[y][x])     {return 1;}
  if (grid[y][x+1])   {return 1;}
  if (grid[y+1][x])   {return 1;}
  if (grid[y+1][x+1]) {return 1;}

  return 0;
}

void get_input(const int (&grid)[grid_size][grid_size])
{
  lastposx = posx;
  lastposy = posy;
  int x_val = analogRead(A4);
  int y_val = analogRead(A5);

  if (jump_sequence >= 1)
  {
    if (jump_sequence == 1 or jump_sequence == 3)
    {
      if (!object_in_way(posx,posy-1, grid))                {posy-=1;}
      else                                                  {jump_sequence = 4;}  // should go to 5, but since we increment jump sequence, we say 4
    }

    if (jump_sequence == 7 or jump_sequence == 9 or jump_sequence == 10)
    {
      if (!object_in_way(posx, posy+1, grid))   {posy+=1;}
      else                                      {jump_sequence = 10;}     // finalize jump sequence
    }

    jump_sequence+=1;
    jump_sequence%=11;

  }

  else //if not in a jump, then try to move player down, and move left/right if input for that, or check if we should initiate jump sequence

  {
    if (!object_in_way(posx, posy+1, grid))   {posy+=1;}

    else
    {
      if (y_val > 550)  // initiate jump
      {
        if (!object_in_way(posx,posy-1, grid))     {posy-=1;   jump_sequence=1;}  // initiate jump
      }
    }
  }

  if (x_val < 490)  // move left
  {
    if (!object_in_way(posx-1,posy, grid))   {posx-=1;}
  }

  if (x_val > 550)  // move right
  {
    if (!object_in_way(posx+1,posy, grid))    {posx+=1;}
  }
  delay(100);
}

void draw_border()
{
  for (int i = 0; i < grid_size; ++i)
  {
    matrix.drawPixel(i,0,matrix.Color333(7,0,0));
    matrix.drawPixel(i,grid_size-1,matrix.Color333(7,0,0));
  }

  for (int i = 0; i < grid_size-1; ++i)
  {
    matrix.drawPixel(0,i,matrix.Color333(7,0,0));
    matrix.drawPixel(grid_size-1,i,matrix.Color333(7,0,0));
  }
}

void generate_coins(const int (&grid)[grid_size][grid_size])
{
  int randomNumber;
  int x;
  int y;

  for (int i = 0; i < 20; ++i) //Satte 20 som antall coins, men dette kan jo endres
  {
    int randomNumber = random(grid_size*grid_size);
    pos_from_num(randomNumber, x, y); 

    if (grid[y][x] != 1 && coinGrid[y][x] == 0)
    { 
      matrix.drawPixel(x, y, matrix.Color333(0, 7, 7));
      coinGrid[y][x] = 1; // Markerer at en mynt er plassert her
    }
    else
    {
      --i; // Prøver igjen
    }
  }
}

void setup() {

  matrix.begin();
  matrix.setCursor(1, 1);    // start at top left, with one pixel of spacing
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  matrix.setTextColor(matrix.Color333(7,7,7));
  matrix.println("Tower");
  matrix.setCursor(1,13);
  matrix.println("Climb");
  draw_border();
  delay(5000);
  draw_matrix(grid_1);

}

void loop() {

  get_input(grid_1);            // with the condition to "open" the door to the next level we can have checks and an integer to keep track of what grid we use.
  draw_player();
}
