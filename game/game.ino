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

int posx = 14;
int posy = 29;
int lastposx = 14;
int lastposy = 29;

int jump_sequence = 0;

int grid[32*32] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
    1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,
    1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

int num_from_pos(int& x, int& y, int& num)
{
  num = x+y*32;
}

void pos_from_num(int& x, int& y, int& num)
{
  x = num%32;
  y = num/32;
};

void draw_matrix()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  int x = 0;
  int y = 0;
  for (int i = 0; i < 32*32; ++i)
  {
    int current = grid[i];

    if (current==1)
      {
        pos_from_num(x,y,i);
        matrix.drawPixel(x,y,matrix.Color333(7,7,7));
      }
  }
}

void draw_player()
{
  matrix.fillRect(lastposx,lastposy,2,2,matrix.Color333(0,0,0));
  matrix.fillRect(posx,posy,2,2, matrix.Color333(0,7,0));
}

bool object_in_way(int x,int y)
{
  int num = 0;

  num_from_pos(x,y,num);
  if (grid[num])  {return 1;}

  int new_x = x+1;
  num_from_pos(new_x,y,num);
  if (grid[num])  {return 1;}

  int new_y = y+1;
  num_from_pos(x,new_y,num);
  if (grid[num])  {return 1;}

  num_from_pos(new_x,new_y,num);
  if (grid[num])  {return 1;}

  return 0;

}

void get_input()
{
  lastposx = posx;
  lastposy = posy;
  int x_val = analogRead(A4);
  int y_val = analogRead(A5);

  if (jump_sequence >= 1)
  {
    if (jump_sequence == 1 or jump_sequence == 3)
    {
      if (posy>1 and !object_in_way(posx,posy-1))     {posy-=1;}
      else                                            {jump_sequence = 4;}  // should go to 5, but since we increment jump sequence, we say 4
    }

    // if (jump_sequence == 2 or jump_sequence == 4 or jump_sequence == 5 or jump_sequence == 6 or jump_sequence == 8)
    // {
    //   // do nothing, so can probably remove this  
    // }

    if (jump_sequence == 7 or jump_sequence == 9 or jump_sequence == 10)
    {
      if (!object_in_way(posx, posy+1))   {posy+=1;}
      else                                {jump_sequence = 10;}
    }

    jump_sequence+=1;
    jump_sequence%=11;

  }

  else //if not in a jump, then try to move player down, and move left/right if input for that, or check if we should initiate jump sequence

  {
    if (!object_in_way(posx, posy+1))   {posy+=1;}

    else
    {
      if (y_val > 550)  // initiate jump
      {
        if (posy > 1 and !object_in_way(posx,posy-1))     {posy-=1;   jump_sequence=1;}
      }
    }
  }

  if (x_val < 490)  // move left
  {
    if (posx > 1 and !object_in_way(posx-1,posy))   {posx-=1;}
  }

  if (x_val > 550)  // move right
  {
    if (posx < 29 and !object_in_way(posx+1,posy))    {posx+=1;}
  }
  delay(100);
}

void draw_border()
{
  for (int i = 0; i < 32; ++i)
  {
    matrix.drawPixel(i,0,matrix.Color333(7,0,0));
    matrix.drawPixel(i,31,matrix.Color333(7,0,0));
  }

  for (int i = 0; i < 31; ++i)
  {
    matrix.drawPixel(0,i,matrix.Color333(7,0,0));
    matrix.drawPixel(31,i,matrix.Color333(7,0,0));
  }
}


void setup() {
  // put your setup code here, to run once:

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
  draw_matrix();

}

void loop() {
  // put your main code here, to run repeatedly:
  get_input();
  // draw_matrix();
  draw_player();
}
