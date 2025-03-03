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
int stage = 1;

int total_coins = 20;
int collected_coins = 0;
bool new_stage = false;

int jump_sequence = 0;

uint32_t coinGrid[grid_size] =
{
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000
};

const uint32_t grid_1[grid_size] =
{
0b11111111111111111111111111111111,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b10011100111010111000011100000111,
0b10000100100000001110000000000011,
0b10000111100000000000000000000001,
0b10000000000000000000000011100001,
0b10000000000000000000000000000001,
0b10000000011000001111000000000001,
0b10000001110000000000000000000001,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b11110000000000000000011111000001,
0b10000000000000000000000000000011,
0b10000000111110000000000000000001,
0b10000000100000000000000000000001,
0b10000000100000000000000000110001,
0b11111000100000000000000000000001,
0b10000000000000000000000000000011,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b11111111111100000001111111111111,
0b11111111000000000000011111111111,
0b10000000000000000000000000000001,
0b10000000000000111000000000000001,
0b10000000000000001110000000000001,
0b10000000000000000000000000000001,
0b11111110000000000000000011111111,
0b11111111100000000001111111111111,  
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b11111111111111111111111111111111};

int getMatrixValue(int x, int y, const uint32_t (&grid)[grid_size]) {
    // Use bitwise operations to check if the xth bit in the yth row is 1
    return (grid[y] >> (31-x)) & 1;
}

void setMatrixValue(int x, int y, int value, uint32_t (&grid)[grid_size]) {
    if (value == 1) {
        // Set the xth bit of the yth row to 1
        grid[y] |= (1UL << (31 - x));
    } else if (value == 0) {
        // Set the xth bit of the yth row to 0
        grid[y] &= ~(1UL << (31 - x));
    }
}

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

int num_from_pos(int& x, int& y, int& num)
{
  num = x+y*grid_size;
}

void pos_from_num(int& x, int& y, int& num)
{
  x = num%grid_size;
  y = num/grid_size;
}

void draw_matrix(const uint32_t (&grid)[grid_size])
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  for (int x = 0; x < grid_size; ++x)
  {
    for (int y = 0; y < grid_size; ++y)
    {
      if (getMatrixValue(x,y,grid))    {matrix.drawPixel(x,y,matrix.Color333(7,7,7));}
    }
  }
}

void draw_player()
{
  matrix.fillRect(lastposx,lastposy,2,2,matrix.Color333(0,0,0));
  matrix.fillRect(posx,posy,2,2, matrix.Color333(0,7,0));
}

bool object_in_way(int x, int y, const uint32_t (&grid)[grid_size])
{
  if (getMatrixValue(x,y,grid))     {return 1;}
  if (getMatrixValue(x+1,y,grid))   {return 1;}
  if (getMatrixValue(x,y+1,grid))   {return 1;}
  if (getMatrixValue(x+1,y+1,grid)) {return 1;}

  return 0;
}

bool check_if_done(int x, int y)
{
  if (collected_coins == -1 && x == grid_size-2 && y == 1)
  {
    new_stage = true;
    return true;
  }
  else
  {
    return false;
  }
}

void get_input(const uint32_t (&grid)[grid_size])
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

  if (x_val < 450)  // move left
  {
    if (!object_in_way(posx-1,posy, grid))   {posx-=1;}
  }

  if (x_val > 550)  // move right
  {
    if (!object_in_way(posx+1,posy, grid) or check_if_done(posx+1,posy))    {posx+=1;}
  }
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

void generate_coins(const uint32_t (&grid)[grid_size])
{
  long randomNumber;
  int x;
  int y;

  for (int i = 0; i < total_coins; ++i) //Satte 20 som antall coins, men dette kan jo endres
  {
    randomNumber = random(long(grid_size*grid_size));
    int r = int(randomNumber);
    pos_from_num(x, y, r);

    if (getMatrixValue(x,y,grid) != 1 && getMatrixValue(x,y,grid) == 0)
    { 
      matrix.drawPixel(x, y, matrix.Color333(7, 7, 0));
      setMatrixValue(x,y,1,coinGrid);
    }
    else
    {
      --i; // Prøver igjen
    }
  }
}
void check_coin()
{
  if (getMatrixValue(posx,posy,coinGrid))      {++collected_coins; setMatrixValue(posx,posy,0,coinGrid);}
  if (getMatrixValue(posx+1,posy,coinGrid))    {++collected_coins; setMatrixValue(posx+1,posy,0,coinGrid);}
  if (getMatrixValue(posx,posy+1,coinGrid))    {++collected_coins; setMatrixValue(posx,posy+1,0,coinGrid);}
  if (getMatrixValue(posx+1,posy+1,coinGrid))  {++collected_coins; setMatrixValue(posx+1,posy+1,0,coinGrid);}
}

void start_animation(int stage)
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setCursor(10, 8);    // start at top left, with one pixel of spacing
  matrix.setTextSize(2);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  matrix.setTextColor(matrix.Color333(7,7,7));
  matrix.println(stage);
  delay(1000);
}

void reset_animation()
{
  delay(500);
  posx+=1;
  draw_player();
  delay(500);
  matrix.drawPixel(grid_size-1,1,matrix.Color333(0,0,0));
  matrix.drawPixel(grid_size-1,2,matrix.Color333(0,0,0));
  delay(500);
  matrix.fillScreen(matrix.Color333(0,0,0));
  start_animation(stage);

}

void start_cut_scene()
{
  delay(500);
  matrix.drawPixel(13, 31, matrix.Color333(0,0,0));
  matrix.drawPixel(14, 31, matrix.Color333(0,0,0));
  matrix.drawPixel(15, 31, matrix.Color333(0,0,0));
  matrix.drawPixel(16, 31, matrix.Color333(0,0,0));
  delay(500);
  matrix.drawPixel(14, 31, matrix.Color333(0,7,0));
  matrix.drawPixel(15, 31, matrix.Color333(0,7,0));
  delay(100);
  matrix.drawPixel(14, 30, matrix.Color333(0,7,0));
  matrix.drawPixel(15, 30, matrix.Color333(0,7,0));
  delay(100);
  draw_player();
  matrix.drawPixel(14, 31, matrix.Color333(0,0,0));
  matrix.drawPixel(15, 31, matrix.Color333(0,0,0));
  delay(200);
  posy-=1;
  draw_player();
  delay(100);
  matrix.drawPixel(13, 31, matrix.Color333(7,0,0));
  matrix.drawPixel(14, 31, matrix.Color333(7,0,0));
  matrix.drawPixel(15, 31, matrix.Color333(7,0,0));
  matrix.drawPixel(16, 31, matrix.Color333(7,0,0));
  delay(100);
  lastposy = posy;
  posy += 1;
  draw_player();
  lastposy = posy;
}  

void reset_stage()
{
  stage += 1;

  reset_animation();

  posx = grid_size/2-2;
  posy = grid_size-3;
  lastposx = posx;
  lastposy = posy;

  total_coins = 20;
  collected_coins = 0;
  new_stage = false;

  jump_sequence = 0;

  if (stage == 2) {draw_matrix(grid_1); draw_border(); generate_coins(grid_1);}

  start_cut_scene();

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
  delay(4000);
  start_animation(stage);
  draw_matrix(grid_1);
  draw_border();
  generate_coins(grid_1);
}

void loop() {

  if (stage == 1) {get_input(grid_1);}
  if (stage == 2) {get_input(grid_1);}
  check_coin();
  draw_player();

  if (collected_coins == total_coins)
  {
    matrix.drawPixel(grid_size-1,1,matrix.Color333(0,0,0));
    matrix.drawPixel(grid_size-1,2,matrix.Color333(0,0,0));
    collected_coins = -1;   // means that you can move to the next stage
  }
  if (new_stage)  {reset_stage();}
  delay(100);

}
