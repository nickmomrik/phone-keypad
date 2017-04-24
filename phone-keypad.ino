// Make sure to install QuickStats
// http://playground.arduino.cc/Main/QuickStats
#include <QuickStats.h>

// Microseconds between each analogRead of the col/row pins.
const int READ_DELAY = 1;

// Microseconds to wait after a button press.
const int PRESS_DELAY = 200;

// How many read samples to average.
// TODO: Change the logic to do rolling sampling instead of reading them all at once.
const int SAMPLES = 30;

// Deviation of the row/col values must be at least this much to trigger a button press.
const float MIN_DEVIATION = 2;

// Just a blank character.
const char BAD_KEY = ' ';

// Display debugging info in the Serial Monitor
const bool DEBUG = true;

// Setup the keypad matrix.
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Analog pins used the microcontroller.
byte col_pins[COLS] = {A0, A1, A2};
byte row_pins[ROWS] = {A3, A4, A5, A7};

// Where read values will be stored.
float col_values[COLS] = {};
float row_values[ROWS] = {};

// Rolling samples for averaging.
float col_samples[COLS][SAMPLES];
float row_samples[ROWS][SAMPLES];

// Which sample gets updated next.
int next_sample;
bool samples_full;

// Keeps track of which key was pressed.
int key;

QuickStats stats;

void setup() {
  Serial.begin( 9600 );

  clear_rows_cols();

  key = BAD_KEY;
}

void loop() {
  key = get_key_press();
  if ( key != BAD_KEY ) {
    Serial.print( "KEY: " );
    Serial.write( key );
    Serial.println( "" );

    // Wait so keys don't repeot too often.
    delay( PRESS_DELAY );
  }
}

// Empty the row and col values.
void clear_rows_cols() {
  for ( int x = 0; x < ROWS; x++ ) {
    row_values[x] = 0;

    for ( int y = 0; y < SAMPLES; y++ ) {
      row_samples[x][y] = 0;
    }
  }

  for ( int x = 0; x < COLS; x++ ) {
    col_values[x] = 0;

    for ( int y = 0; y < SAMPLES; y++ ) {
      col_samples[x][y] = 0;
    }
  }

  next_sample = 0;
  samples_full = false;
}

// Read a new sample for each row/col.
void read_samples() {
  int val;

  // Read one sample.
  for ( int x = 0; x < ROWS; x++ ) {
    val = analogRead( row_pins[x] );
    row_samples[x][next_sample] = val;
    delay( READ_DELAY );
  }
  for ( int x = 0; x < COLS; x++ ) {
    val = analogRead( col_pins[x] );
    col_samples[x][next_sample] = val;
    delay( READ_DELAY );
  }

  next_sample++;

  // All samples have been collected
  if ( next_sample >= SAMPLES ) {
    samples_full = true;

    // Create an average from the samples and update the values.
    for ( int x = 0; x < ROWS; x++ ) {
      row_values[x] = stats.average( row_samples[x], SAMPLES );
    }

    for ( int x = 0; x < COLS; x++ ) {
      col_values[x] = stats.average( col_samples[x], SAMPLES );
    }

    // Rolling samples, so start updating from the beginning of the arrays.
    next_sample = 0;
  }
}

int row_pressed() {
  // The first row always stays higher, even when another row is being triggered.
  // So the logic is more complex than detecting the pressed column.
  
  int max_value = 0;
  int row_with_max = -1;
  int sub_size = ROWS - 1;
  float sub_rows[sub_size] = {};
  
  // Setup an array of values without the first row.
  for ( int i = 0; i < sub_size; i++ ) {
    sub_rows[i] = row_values[i + 1];
  }
  float deviation = stats.stdev( sub_rows, sub_size );
  float threshold = stats.average( sub_rows, sub_size ) + deviation;

  for ( int x = 0; x < ROWS; x++ ) {
    // The OR checks to see if rows after the first are triggering high.
    if ( row_values[x] > max_value or
      ( 0 == row_with_max and deviation > MIN_DEVIATION and row_values[x] > threshold and abs( max_value - row_values[x] ) < deviation ) ) {
      max_value = row_values[x];
      row_with_max = x;
    }
  }

  // When the first row is still the highest, use appropriate statistics.
  if ( row_with_max == 0 ) {
    deviation = stats.stdev( row_values, ROWS );
    threshold = stats.average( row_values, ROWS );
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

  // Find the col with the highest value.
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
  read_samples();

  // Don't check for key presses until there are enough samples.
  if ( ! samples_full ) {
    return BAD_KEY;
  }

  int key_row = row_pressed();
  int key_col = col_pressed();

  // Make sure a row and col is detected for a valid key press.
  if ( key_row > -1 and key_col > -1 ) {
    print_rows();
    print_cols();

    // Reset the values and samples.
    clear_rows_cols();

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

