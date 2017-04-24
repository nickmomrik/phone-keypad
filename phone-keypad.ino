#include <QuickStats.h>

const int READ_DELAY = 2;
const int PRESS_DELAY = 100;
const int READ_SAMPLES = 15;
const float MIN_DEVIATION = 2;
const char BAD_KEY = ' ';
const bool DEBUG = true;

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte col_pins[COLS] = {A0, A1, A2};     // blue green purple
byte row_pins[ROWS] = {A3, A4, A5, A7}; // yellow gray orange brown

float col_values[COLS] = {};
float row_values[ROWS] = {};

int key = BAD_KEY;

QuickStats stats;

void setup() {
  Serial.begin( 9600 );

  clear_rows();
  clear_cols();
}

void loop() {
  key = get_key_press();
  if ( key != BAD_KEY ) {
    Serial.print( "KEY: " );
    Serial.write( key );
    Serial.println( "" );

    delay( PRESS_DELAY );
  }
}

void clear_rows() {
  for ( int x = 0; x < ROWS; x++ ) {
    row_values[x] = 0;  
  }
}

void clear_cols() {
  for ( int x = 0; x < COLS; x++ ) {
    col_values[x] = 0;  
  }
}

void read_rows() {
  int val;

  clear_rows();

  for ( int i = 0; i < READ_SAMPLES; i++ ) {
    for ( int x = 0; x < ROWS; x++ ) {
      val = analogRead( row_pins[x] );
      row_values[x] += val;
      delay( READ_DELAY );
    }
  }

  for ( int x = 0; x < ROWS; x++ ) {
    row_values[x] = row_values[x] / READ_SAMPLES;
  }
}

void read_cols() {
  int val;

  clear_cols();

  for ( int i = 0; i < READ_SAMPLES; i++ ) {
    for ( int x = 0; x < COLS; x++ ) {
      val = analogRead( col_pins[x] );
      col_values[x] += val;
      delay( READ_DELAY );
    }
  }

  for ( int x = 0; x < COLS; x++ ) {
    col_values[x] = col_values[x] / READ_SAMPLES;
  }
}

int row_pressed() {
  // Rows seem to be special in that the first row always stays higher while other rows are pressed
  
  int max_value = 0;
  int row_with_max = -1;
  int sub_size = ROWS - 1;
  float sub_rows[sub_size] = {};
  
  // Skip first row
  for ( int i = 0; i < sub_size; i++ ) {
    sub_rows[i] = row_values[i + 1];
  }

  float deviation = stats.stdev( row_values, ROWS );
  float threshold = stats.average( row_values, ROWS );
  float sub_deviation = stats.stdev( sub_rows, sub_size );
  float sub_threshold = stats.average( sub_rows, sub_size ) + sub_deviation;

  for ( int x = 0; x < ROWS; x++ ) {
    if ( row_values[x] > max_value or
      ( 0 == row_with_max and sub_deviation > MIN_DEVIATION and row_values[x] > threshold ) ) {
      max_value = row_values[x];
      row_with_max = x;
    }
  }

  if ( row_with_max != 0 ) {
    deviation = sub_deviation;
    threshold = sub_threshold;
  }

  if ( deviation > MIN_DEVIATION and max_value > threshold ) {
    return row_with_max;
  } else {
    return -1;
  }

}

int col_pressed() {
  int max_value = 0;
  int col_with_max = -1;
  float deviation;

  for ( int x = 0; x < COLS; x++ ) {
    if ( col_values[x] > max_value ) {
      max_value = col_values[x];
      col_with_max = x;
    }
  }

  deviation = stats.stdev( col_values, COLS );

  if ( deviation > MIN_DEVIATION and max_value > ( stats.average( col_values, COLS ) + deviation ) ) {
    return col_with_max;
  } else {
    return -1;
  }
}

char get_key_press() {
  read_rows();
  read_cols();

  int key_row = row_pressed();
  int key_col = col_pressed();

  if ( key_row > -1 and key_col > -1 ) {
    print_rows();
    print_cols();

    return keys[key_row][key_col];
  } else {
    return BAD_KEY;
  }
}

void print_rows() {
  if ( ! DEBUG ) {
    return;
  }

  Serial.print( "ROWS: " );
  for ( int x = 0; x < ROWS; x++ ) {
    Serial.print( row_values[x] );
    Serial.print( " " );
  }
  Serial.println( "" );
}

void print_cols() {
  if ( ! DEBUG ) {
    return;
  }

  Serial.print( "COLS: " );
  for ( int x = 0; x < COLS; x++ ) {
    Serial.print( col_values[x] );
    Serial.print( " " );
  }
  Serial.println( "" );
}

