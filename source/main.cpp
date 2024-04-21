// The following imports are for debugging only
#include "MicroBit.h"
NRF52Pin usbTx(ID_PIN_USBTX, MICROBIT_PIN_UART_TX, PIN_CAPABILITY_DIGITAL);
NRF52Pin usbRx(ID_PIN_USBRX, MICROBIT_PIN_UART_RX, PIN_CAPABILITY_DIGITAL);
NRF52Serial serial(usbTx, usbRx, NRF_UARTE0);

// For this task, I did not bring standalone LEDs with me, so I will use the onboard 5x5 LED matrix to demonstrate the code
// Using the onboard 5x5 LED matrix means I need to use different GPIO pins than the ones specified in the task description.
// All of the following information is taken directly from the nRF52833 Objective Product Specification v0.7

// COL1 = P0.28 [B11]
// COL2 = P0.11 [T2]
// COL3 = P0.31 [A8]
// COL4 = P1.05 [A16]
// COL5 = P0.30 [B9]

// ROW1 = P0.21 [AC17]
// ROW2 = P0.22 [AD18]
// ROW3 = P0.15 [AD10]
// ROW4 = P0.24 [AC20]
// ROW5 = P0.19 [A14]

// GPIO base addresses
#define GPIO0_BASE   0x50000000   // P0.00 - P0.31
#define GPIO1_BASE   0x50000300   // P1.00 - P1.09

#define GPIO0_OUT (*(volatile unsigned int *)(GPIO0_BASE + 0x504))
#define GPIO0_OUTSET (*(volatile unsigned int *)(GPIO0_BASE + 0x508))
#define GPIO0_OUTCLR (*(volatile unsigned int *)(GPIO0_BASE + 0x50C))
#define GPIO0_DIRSET (*(volatile unsigned int *)(GPIO0_BASE + 0x518))

#define GPIO1_OUT (*(volatile unsigned int *)(GPIO1_BASE + 0x504))
#define GPIO1_OUTSET (*(volatile unsigned int *)(GPIO1_BASE + 0x508))
#define GPIO1_OUTCLR (*(volatile unsigned int *)(GPIO1_BASE + 0x50C))
#define GPIO1_DIRSET (*(volatile unsigned int *)(GPIO1_BASE + 0x518))

// Arrays to map rows and columns to their respective GPIO pins
int columnPins[] = {28, 11, 31, 5, 30};  // Pins for columns 1-5
int rowPins[] = {21, 22, 15, 24, 19};    // Pins for rows 1-5

/**
 * @brief Simple busy-wait delay function.
 * 
 * The micro:bit v2 has a clock speed of 64 MHz. I found 5.5M itterations to work well for a 1 second delay.
 *
 * @param duration Duration of the delay in seconds. 
 * @return None
 */
void delay(float duration) {

    float iterations = 5500000;  // 64 MHz
    duration *= iterations;      // Convert seconds to total loop iterations

    for (volatile int i = 0; i < duration; i++) {
        // Do nothing, just delay
    }
}

/**
 * @brief Sets the direction of GPIO pins to output.
 * 
 * This function sets the direction of specific GPIO pins to output mode.
 * It configures the GPIO0 pins for columns 1, 2, 3, and 5 as outputs,
 * and GPIO0 pins for rows 1 to 5 as outputs. It also configures GPIO1 pin
 * for column 4 as an output.
 * 
 * @param None
 * @return None
 */
void setDirectionToOutput() {
    // Set direction of GPIO0 pins to output
    GPIO0_DIRSET = (1 << 28) | (1 << 11) | (1 << 31) | (1 << 30);  // Columns 1, 2, 3, and 5 as outputs
    GPIO0_DIRSET = (1 << 21) | (1 << 22) | (1 << 15) | (1 << 24) | (1 << 19); // Rows 1 to 5 as outputs

    // Set direction of GPIO1 pin to output (Column 4)
    GPIO1_DIRSET = (1 << 5);  // Column 4 as output
}
    
/**
 * @brief Turns on all LEDs on the microbit.
 * 
 * This function sets the necessary GPIO pins to turn on the LEDs. 
 * By setting OUTCLR to 1 is sets the pins to low.
 * By setting OUTSET to 1 is sets the pins to high.
 * 
 * This is representative of Task 1 on Excercise 1
 * 
 * @param None
 * @return None
 */
void turnOnAllLEDs() {
    // Turn on LEDs by setting GPIO0 columns low and rows high
    GPIO0_OUTCLR = (1 << 28) | (1 << 11) | (1 << 31) | (1 << 30);  // Set columns 1, 2, 3, and 5 low
    GPIO0_OUTSET = (1 << 21) | (1 << 22) | (1 << 15) | (1 << 24) | (1 << 19); // Set rows 1 to 5 high

    // Turn on LEDs by setting GPIO1 column low
    GPIO1_OUTCLR = (1 << 5);  // Set column 4 low
}

/**
 * @brief Resets the LED matrix by setting all rows to low.
 * 
 * This function ensures that all rows of the LED matrix are set to low, preventing other LEDs from lighting up.
 * 
 * @param None
 * @return None
 */
void resetMatrix() {
    // First, ensure all rows are low to prevent other LEDs from lighting up
    int i;
    for (i = 0; i < 5; i++) {
        GPIO0_OUTCLR = (1 << rowPins[i]);
    }

    // Then, ensure all columns are high (because common cathode - low activates the LED)
    for (i = 0; i < 5; i++) {
        if (i == 3) { // Column 4 is on GPIO1
            GPIO1_OUTSET = (1 << columnPins[i]);
        } else {
            GPIO0_OUTSET = (1 << columnPins[i]);
        }
    }
}

/**
 * @brief Turns on a specific LED on a micro:bit LED matrix.
 *
 * @param row The row number of the LED (1-5).
 * @param column The column number of the LED (1-5).
 * @return None
 */
void turnOnLED(int row, int column) {

    // Activate the selected row (set high)
    GPIO0_OUTSET = (1 << rowPins[row - 1]);

    // And activate the selected column by setting it low
    if (column == 4) { // Column 4 is on GPIO1
        GPIO1_OUTCLR = (1 << columnPins[column - 1]);
    } else {
        GPIO0_OUTCLR = (1 << columnPins[column - 1]);
    }
}

/**
 * @brief Displays a pattern on the top row of the micro:bit LED matrix based on a 1D array.
 *
 * This is representative of Task 2 on Excercise 1
 * 
 * @param pattern A 1D array representing the state (on/off) of each LED in the top row.
 * @return None
 */
void displayTopRow(const int pattern[5]) {

    int column;
    for (column = 0; column < 5; column++) {
        if (pattern[column] == 1) {
            turnOnLED(1, column + 1);  // +1 because the function expects 1-based index
        }
    }
}

/**
 * @brief Flashes the first LED in the matrix.
 *
 * This function repeatedly turns the first LED on and off, pausing between states
 * using a busy-wait delay.
 *
 * @param flashes Number of times to flash the LED.
 * @param waitTime Duration of the delay in busy-wait iterations in seconds.
 * @return None
 */
void flashLED(int flashes, float waitTime) {
    int row = 1, column = 1;  // Coordinates for the first LED

    for (int i = 0; i < flashes; i++) {
        turnOnLED(row, column);  // Turn on the first LED
        delay(waitTime);         // Delay
        resetMatrix();           // Turn off the first LED
        delay(waitTime);         // Delay
    }
}

/**
 * @brief Displays a rolling counter from 0 to 255 on the LED matrix.
 *
 * Displays each number as an 8-bit binary value, wrapping from 255 back to 0.
 * The cycle should take approximately 30 seconds to complete, with each
 * count displayed for about 117 milliseconds.
 * 
 * Since I don't have LEDs, I will use the first 4 LEDs in the top 2 rows on the 
 * matrix to display the binary representation of the number.
 * 
 * This is representative of Task 3 on Excercise 1
 * 
 * 
 * @param None
 * @return None
 */
void rollingCounter() {
    int displayPins[16] = {
        // Assuming use of the first two rows (1-based index for row and column)
        rowPins[0], columnPins[0], // Bit 0
        rowPins[0], columnPins[1], // Bit 1
        rowPins[0], columnPins[2], // Bit 2
        rowPins[0], columnPins[3], // Bit 3
        rowPins[1], columnPins[0], // Bit 4
        rowPins[1], columnPins[1], // Bit 5
        rowPins[1], columnPins[2], // Bit 6
        rowPins[1], columnPins[3]  // Bit 7
    };

    while (true) {
        for (int count = 0; count < 256; count++) {
            resetMatrix(); // Clear the matrix at the start of each count
            for (int bit = 0; bit < 8; bit++) {
                if (count & (1 << bit)) {
                    int row = bit < 4 ? 1 : 2; // First four bits in row 1, next four in row 2
                    int column = bit % 4 + 1;  // Column calculation
                    turnOnLED(row, column);
                }
            }
            serial.printf("Count: %d\n", count);
            delay(0.117); // Approximately 117ms per number to complete 30 seconds for the full range 0-255
        }
    }
}

int main() {
    setDirectionToOutput();
    resetMatrix();  // Ensure all LEDs are initially off
    
    // All user code
    rollingCounter();

    while(true);
}