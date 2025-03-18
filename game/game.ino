#include <RGBmatrixPanel.h>
#include <EEPROM.h>
#include <TimeLib.h>

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
#define audioPin 52
#define pressPin 40

int start_t = now();

const int grid_size = 32;
int posx = grid_size/2-2;
int posy = grid_size-3;
int lastposx = posx;
int lastposy = posy;
int stage = 1;

int total_coins = 0;  // only for first level
int collected_coins = 0;
bool new_stage = false;

int jump_sequence = 0;

unsigned long lastDropTime = 0;
const int dropInterval = 7000;  
const int lavaTapX = 11;         // X-position of the lava "tap"
const int lavaTapY = 16;         // Y-position of the lava source
const int lavaPoolY = grid_size - 4; // Y-position of the lava pool

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
uint32_t lavaGrid[grid_size] =
{
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000000000000000000000000000000,
0b00000011100000000000000000000000,
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
0b00000000001111100000000000000000,
0b00000000000100000000000000000000,
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
0b00000111111100000000000000000000,
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
0b10011111111110000000000000000111,
0b10000000000000000111111111111111,
0b10000000000000011100000000000111,
0b10000000000001110000000000000011,
0b10000000000111000000000000000001,
0b10000000011100000000000000000001,
0b10000001110000000000000000000001,
0b11111000000000000000000000000001,
0b10000000000000000000000000000001,
0b10000000000000000000110000000001,
0b10011111110000000111110000000111,
0b10001111100000000000000000000111,
0b10000111000000000000000000011111,
0b10000010000000000000001110011111,
0b10000000000000000000001100001111,
0b10000000000000000000001100000111,
0b10000000000000000001111000000011,
0b10000000000000000001110000000001,
0b10000000000000000001100000000001,
0b10000000000000000111000000000001,
0b10000000000000000110000000000001,
0b10000000000000011100000000000001,
0b10000000000000011000000000000001,
0b10011000000001110000000000000001,
0b10011100000000000000000000000001,
0b10011110000000000000000000000001,  
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b11111111111111111111111111111111};

const uint32_t grid_2[grid_size] =
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

const uint32_t grid_3[grid_size] =
{
0b11111111111111111111111111111111,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b10000000000010111000011100000111,
0b10000100010000001110000000000011,
0b11000011100000000000000000000001,
0b10000000000000000000000001100001,
0b10000000000000000000000000000001,
0b10000011111000001111000000000001,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b10000000000011000000000000000001,
0b11110101010000000000011111000001,
0b10000000000000000000000000000011,
0b10000000000000000000000000000111,
0b10000000010000010000011100000001,
0b10000000111011100000000000000001,
0b10000000000000000000000000000001,
0b11000000000000000000000000000011,
0b11110010101010101000001110000001,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b10000000000000000000000000111111,
0b10000010000010000000000000000001,
0b11000000000000000011100000000001,
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b10001000000010000000000001000001,
0b10000111111100000001111110000001,  
0b10000000000000000000000000000001,
0b10000000000000000000000000000001,
0b11111111111111111111111111111111};


void simple_melody()
{
  tone(audioPin, 523, 150); // C5
  delay(200);
  tone(audioPin, 659, 150); // E5
  delay(200);
  tone(audioPin, 784, 200); // G5
  delay(250);
  tone(audioPin, 880, 100); // A5
  delay(150);
  tone(audioPin, 784, 150); // G5
  delay(200);
  tone(audioPin, 659, 150); // E5
  delay(200);
  tone(audioPin, 523, 300); // C5
  delay(350);
  tone(audioPin, 392, 200); // G4
  delay(250);
  tone(audioPin, 440, 150); // A4
  delay(200);
  tone(audioPin, 494, 150); // B4
  delay(200);
  tone(audioPin, 523, 300); // C5
  delay(350);

  noTone(audioPin);
}

void jump_sound() {
  tone(audioPin, 400, 30);  // Start low
  delay(30);
  tone(audioPin, 500, 30);
  delay(30);
  tone(audioPin, 600, 40);  // Quickly rise in pitch
  delay(40);
  tone(audioPin, 700, 50);
  delay(50);
  noTone(audioPin);
}

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
        if (!object_in_way(posx,posy-1, grid))     {posy-=1;   jump_sequence=1; jump_sound();}  // initiate jump
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

void draw_lava(const uint32_t (&grid)[grid_size])
{
  for (int x = 0; x < grid_size; ++x)
  {
    for (int y = 0; y < grid_size; ++y)
    {
      if (getMatrixValue(x,y,grid))    {matrix.drawPixel(x,y,matrix.Color333(6,2,0));}
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

void check_lava()
{
  if (getMatrixValue(posx,posy,lavaGrid))      {reset_stage();}
  if (getMatrixValue(posx+1,posy,lavaGrid))    {reset_stage();}
  if (getMatrixValue(posx,posy+1,lavaGrid))    {reset_stage();}
  if (getMatrixValue(posx+1,posy+1,lavaGrid))  {reset_stage();}
  if (getMatrixValue(posx,posy-1,lavaGrid))    {reset_stage();}
  if (getMatrixValue(posx+1,posy-1,lavaGrid))  {reset_stage();}
}

void drop_lava() {
    static int lavaY = lavaTapY;  
    static bool dropping = false; 

  
      delay(200);
      unsigned long currentTime = now();

      if (!dropping && (currentTime - lastDropTime >= dropInterval)) {
          lavaY = lavaTapY;  // Reset the drop position after 7 seconds
          dropping = true;
          lastDropTime = currentTime;
      }

      if (dropping) {
          // Clear previous lava drop position
          setMatrixValue(lavaTapX, lavaY, 0, lavaGrid);

          if (lavaY < lavaPoolY) {
              lavaY++;  // Move the drop down
              setMatrixValue(lavaTapX, lavaY, 1, lavaGrid);
          } else {
              dropping = false; // Stop when reaching the lava pool
          }
      }
}


void show_stage(int stage)
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setCursor(10, 8);    // start at top left, with one pixel of spacing
  matrix.setTextSize(2);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  matrix.setTextColor(matrix.Color333(7,7,7));
  matrix.println(stage);
  simple_melody();
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
  if (stage!=4)
  {
    show_stage(stage);
  }
  else
  {
    draw_border();
    matrix.setCursor(1, 1);    // start at top left, with one pixel of spacing
    matrix.setTextSize(1);     // size 1 == 8 pixels high
    matrix.setTextWrap(false);
    matrix.setTextColor(matrix.Color333(7,7,7));
    matrix.print("Game");
  }
}

void intro_animation()
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

  if (stage == 2) {draw_matrix(grid_2); draw_border(); generate_coins(grid_2); intro_animation();}
  if (stage == 3) {draw_matrix(grid_3); draw_border(); intro_animation(); draw_lava(lavaGrid); total_coins = 0;}

}

void clear_EEPROM()   // only use this when absolutely needed
{
  for (int i = 0; i < EEPROM.length(); ++i)
  {
    EEPROM.write(i,0);
  }
}

// the setup of eeprom memory, 
// Use 2 bytes to hold time, so maximum time is 2^(16) units of time, probably use seconds, that seems like a reasonable "frequency", 2^8 seconds (256) is probably a little slow, at least in the beginning
// We can also do up to 256 minutes (byte 0) and up to 60 seconds(byte 1)
// Since 1 byte is 256 possibilities, it's easy to implement that 1 byte can hold 9 letters, since we need 26 letters for english alphabet
// Or we can just have one byte represent 1 letter, and allocate say 20 letters to each score, so that 1 score takes up 22 bytes
// Then top 10 highscore only takes then 220 bytes, which is a lot less memory than the eeprom memory we have, so probably the best option
// a time of 0 means that there is no-one occupying that place

void ascii_name(char (&letters)[3], int (&numbers)[3])
{
  for (int i = 0; i < 3; ++i)
  {
    if (numbers[i]==0)  {letters[i] = ' ';}
    else                {letters[i] = char(numbers[i]);}
  }
}

void print_letters(char (&letters)[3])
{
  for (int i = 0; i < 3; ++i)
  {
    matrix.setCursor(1+(i)*10, 11);
    matrix.print(letters[i]);
  }
}

void get_name(int (&name)[3])
{
  int current_letter = 65;

  for (int i = 0; i < 3; ++i)
  {
    bool found_letter = false;
    char letters[3] = {0};

    while (!found_letter)
    {
      matrix.fillScreen(matrix.Color333(0,0,0));
      matrix.setCursor(1, 1);    // start at top left, with one pixel of spacing
      matrix.setTextSize(1);     // size 1 == 8 pixels high
      matrix.setTextWrap(false);
      matrix.setTextColor(matrix.Color333(7,7,7));
      ascii_name(letters, name);
      matrix.println("Name");
      print_letters(letters);
      matrix.setCursor(7, 21);
      if (current_letter != 91 and current_letter != 92)
      {
        matrix.print(char(current_letter));
      }
      else {
        if (current_letter == 91)
        {
          matrix.print("Done");
        }
        else
        {
          matrix.print("Back");
        }
      }

      delay(200);


      if (digitalRead(pressPin) == LOW)
      {

        if (current_letter == 91) {name[i] = 0;}
        else
        {
          if (current_letter == 92) {name[i] = 0; if (i > 0) {name[i-1] = 0; i-=2;}}
          else                      {name[i] = current_letter;}
        }
        found_letter = true;
      }
      else
      {
        int y_val = analogRead(A5);
        if (y_val < 400)  {current_letter+=1;}
        if (y_val > 600)  {current_letter-=1;}

        if (current_letter < 65)  {current_letter = 92;}
        if (current_letter > 92)  {current_letter = 65;}
      }
    }

    if (current_letter == 91) {break;}

  }
}

void high_score_names()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  for (int i = 0; i < 4; ++i)
  {
    matrix.setCursor(1, i*8);    // start at top left, with one pixel of spacing
    matrix.setTextSize(1);     // size 1 == 8 pixels high
    matrix.setTextWrap(false);
    matrix.setTextColor(matrix.Color333(7,7,7));
    matrix.print(i+1);
    matrix.setCursor(12, i*8);
    char name[3] = {};
    for (int j = 0; j < 3; ++j)
    {
      if (EEPROM.read(5*i+2+j) == 0)  {name[j] = ' ';}
      else                            {name[j] = char(EEPROM.read(5*i+2+j));}
      matrix.setCursor(12+j*6, i*8);
      matrix.print(name[j]);
    }
  }
}

void high_score_numbers()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  for (int i = 0; i < 4; ++i)
  {
    matrix.setCursor(1, i*8);    // start at top left, with one pixel of spacing
    matrix.setTextSize(1);     // size 1 == 8 pixels high
    matrix.setTextWrap(false);
    matrix.setTextColor(matrix.Color333(7,7,7));
    matrix.print(i+1);
    matrix.setCursor(12, i*8);
    matrix.print(EEPROM.read(5*i));
    matrix.fillRect(18,(i+1)*8-1,1,1,matrix.Color333(7,7,7));
    matrix.setCursor(20,i*8);
    matrix.print(EEPROM.read(5*i+1));
  }
}

void view_high_score()
{
  int i = 0;
  high_score_names();
  delay(1000);

  while(true)
  {
    if (digitalRead(pressPin) == LOW)
    {
      i+=1; if (i%2) {high_score_numbers();} else {high_score_names();}
      delay(20);
      int num = 0;
      for (int j = 0; j < 50; ++j)
      {
        if (digitalRead(pressPin) == LOW) {++num;}
        delay(20);
      }
      if (num == 50)  {break;}
    }
  }
}

void losers_message()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setCursor(1, 3);
  matrix.setTextSize(1);
  matrix.print("Not");
  matrix.setCursor(1, 12);
  matrix.print("Good");
  matrix.setCursor(1, 21);
  matrix.print("Yet");
  delay(1000);
  while(true)
  {
    if (digitalRead(pressPin) == LOW)
    {
      break;
    }
  }
}

void winnners_message()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setCursor(1,3);
  matrix.setTextSize(1);
  matrix.print("You");
  matrix.setCursor(1, 12);
  matrix.print("Are");
  matrix.setCursor(1, 21);
  matrix.print("Good");
  delay(1000);
  while(true)
  {
    if (digitalRead(pressPin) == LOW)
    {
      break;
    }
  }
}

void end_screen()
{
  matrix.fillScreen(matrix.Color333(0,0,0));
  matrix.setCursor(1,3);
  matrix.setTextSize(1);
  matrix.print("The");
  matrix.setCursor(1, 12);
  matrix.print("End");
}

void new_score(int total_seconds)
{
  int minutes = total_seconds / 60;
  int seconds = total_seconds % 60;
  int push = 4;

  bool empty_slot = false;

  for (int i = 0; i < 4; ++i)
  {
    if (EEPROM.read(5*i) == 0 and EEPROM.read(5*i+1) == 0)
    {
      push = i;
      empty_slot = true;
      break;
    }
  }

  if (!empty_slot)
  {
    for (int i = 3; i >= 0; --i)
    {
      if (EEPROM.read(5*i) > minutes or (EEPROM.read(5*i) == minutes and EEPROM.read(5*i+1) > seconds))
      {
        push = i;
      }

      else
      {
        break;
      }
    }
  }

  if (push<4)
  {

    winnners_message();

    int name[3] = {};
    get_name(name);   // get name from players input

    for (int i = 3; i > push; --i)
    {
      EEPROM.write(5*i,EEPROM.read(5*(i-1)));
      EEPROM.write(5*i+1,EEPROM.read(5*(i-1)+1));
      for (int j = 0; j < 3; ++j)
      {
        EEPROM.write(5*i+2+j,EEPROM.read(5*(i-1)+2+j));
      }
    }

    EEPROM.write(5*push,minutes);
    EEPROM.write(5*push+1,seconds);

    for (int j = 0; j < 3; ++j)
    {
      EEPROM.write(5*push+2+j,name[j]);
    }

  }

  else
  {
    losers_message();
  }

  view_high_score();


}


void setup() {

  pinMode(pressPin, INPUT_PULLUP);
  matrix.begin();

  matrix.fillScreen(matrix.Color333(0,0,0));

  matrix.setCursor(1, 1);    // start at top left, with one pixel of spacing
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  matrix.setTextColor(matrix.Color333(7,7,7));
  matrix.println("Tower");
  matrix.setCursor(1,13);
  matrix.println("Climb");
  draw_border();
  simple_melody();
  
  show_stage(stage);
  draw_matrix(grid_1);
  draw_border();
  // generate_coins(grid_1);

}

void loop() {

  if (stage == 1) {get_input(grid_1);}
  if (stage == 2) {get_input(grid_2);}
  if (stage == 3) {get_input(grid_3); drop_lava();}
  if (stage == 4) {new_score(now()-start_t); end_screen(); while(true){;}}
  check_coin();
  check_lava();
  draw_player();

  if (collected_coins == total_coins)
  {

    matrix.drawPixel(grid_size-1,1,matrix.Color333(0,0,0));
    matrix.drawPixel(grid_size-1,2,matrix.Color333(0,0,0));
    collected_coins = -1;   // means that you can move to the next stage
  }
  if (new_stage)  {reset_stage();}
  delay(100);
  // noTone(audioPin);

}
