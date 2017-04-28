#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
Adafruit_SSD1306 display = Adafruit_SSD1306();

// Microseconds between each analogRead of the col/row pins.
const int READ_DELAY = 1;

// Microseconds to wait after a button press.
const int PRESS_DELAY = 150;

// Just a blank character.
const char BAD_KEY = ' ';

// Display debugging info in the Serial Monitor
const bool DEBUG = false;

// Setup the keypad matrix.
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

int tones[ROWS][COLS]= {
  {3000, 3200, 3400},
  {3600, 3800, 4000},
  {4200, 4400, 4600},
  {4800, 5000, 5200}
};
int tone_duration = 120;

// Input pins used the microcontroller.
byte row_pins[ROWS] = {13, 12, 11, 10}; // Right top, left top, right bottom, left bottom
byte col_pins[COLS] = {6, 5, 1}; // Top left, top right, bottom
byte backlight_pin = 0;
byte piezo_pin = 9;

// Where read values will be stored.
bool row_values[ROWS] = {};
bool col_values[COLS] = {};

// Keeps track of which key was pressed.
char key;
int key_row;
int key_col;
bool released;

void setup() {
  if ( DEBUG ) {
    Serial.begin( 9600 );
  }

  pinMode( backlight_pin, OUTPUT );
  digitalWrite( backlight_pin, HIGH );

  for ( int x = 0; x < ROWS; x++ ) {
    pinMode( row_pins[x], INPUT );
  }

  for ( int x = 0; x < COLS; x++ ) {
    pinMode( col_pins[x], INPUT );
  }

  clear_rows_cols();

  key = BAD_KEY;

  released = true;

  display.begin( SSD1306_SWITCHCAPVCC, 0x3C );

  display.display();
  delay( 1000 );
  display.clearDisplay();
  display.display();
  display.setTextSize( 1 );
  display.setTextColor( WHITE );
  display.setCursor( 0, 0 );
}

void loop() {
  key = get_key_press();
  if ( key != BAD_KEY ) {
    tone( piezo_pin, tones[key_row][key_col], tone_duration );

    display.print( key );
    display.display();

    // Wait so keys don't repeot too often.
    delay( PRESS_DELAY );
  }
}

// Empty the row and col values.
void clear_rows_cols() {
  for ( int x = 0; x < ROWS; x++ ) {
    row_values[x] = LOW;
  }

  for ( int x = 0; x < COLS; x++ ) {
    col_values[x] = LOW;
  }
}

// Read a new sample for each row/col.
void read_values() {
  int val;

  clear_rows_cols();

  // Read one sample.
  for ( int x = 0; x < ROWS; x++ ) {
    val = digitalRead( row_pins[x] );
    row_values[x] = val;
    delay( READ_DELAY );
  }

  for ( int x = 0; x < COLS; x++ ) {
    val = digitalRead( col_pins[x] );
    col_values[x] = val;
    delay( READ_DELAY );
  }
}

int row_pressed() {
  int row = -1;
  bool multiple = false;

  for ( int x = 0; x < ROWS; x++ ) {
    if ( row_values[x] == HIGH ) {
      if ( row > -1 ) {
        multiple = true;
      }

      row = x;
    }
  }

  if ( multiple ) {
    row = -1;
  }

  return row;
}

int col_pressed() {
  int col = -1;
  bool multiple = false;

  for ( int x = 0; x < COLS; x++ ) {
    if ( col_values[x] == HIGH ) {
      if ( col > -1 ) {
        multiple = true;
      }

      col = x;
    }
  }

  if ( multiple ) {
    col = -1;
  }

  return col;
}

char get_key_press() {
  read_values();

  key_row = row_pressed();
  key_col = col_pressed();

  // Make sure a row and col is detected for a valid key press.
  if ( key_row > -1 and key_col > -1 ) {
    print_values();

    if ( released ) {
      released = false;

      return keys[key_row][key_col];
    } else {
      return BAD_KEY;
    }
  } else {
    released = true;
    return BAD_KEY;
  }
}

void print_values() {
  if ( ! DEBUG ) {
    return;
  }

  Serial.print( "ROWS: " );
  for ( int x = 0; x < ROWS; x++ ) {
    Serial.print( row_values[x] );
    Serial.print( " " );
  }

  Serial.print( " COLS: " );
  for ( int x = 0; x < COLS; x++ ) {
    Serial.print( col_values[x] );
    Serial.print( " " );
  }
  Serial.println();
}

