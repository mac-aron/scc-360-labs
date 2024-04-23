#include "MicroBit.h"
#include "nrf_timer.h"
// #include <nrf52.h> // problematic!
#include <nrf52_bitfields.h>
// #include <core_cm4.h>

// Library imports for debugging only
NRF52Pin usbTx(ID_PIN_USBTX, MICROBIT_PIN_UART_TX, PIN_CAPABILITY_DIGITAL);
NRF52Pin usbRx(ID_PIN_USBRX, MICROBIT_PIN_UART_RX, PIN_CAPABILITY_DIGITAL);
NRF52Serial serial(usbTx, usbRx, NRF_UARTE0);

/**
 * All of the following information is taken directly from the nRF52833 Objective Product Specification v0.7
 * 
 * Columns and rows for the LED matrix are mapped to the following GPIO pins:
 *  COL1 = P0.28 [B11]
 *  COL2 = P0.11 [T2]
 *  COL3 = P0.31 [A8]
 *  COL4 = P1.05 [A16]
 *  COL5 = P0.30 [B9]
 *  ROW1 = P0.21 [AC17]
 *  ROW2 = P0.22 [AD18]
 *  ROW3 = P0.15 [AD10]
 *  ROW4 = P0.24 [AC20]
 *  ROW5 = P0.19 [A14]
*/

// Arrays to map rows and columns to their respective GPIO pins 
const int COL_PINS[] = {28, 11, 31, 5, 30};   // P0.28, P0.11, P0.31, P1.05, P0.30
const int ROW_PINS[] = {21, 22, 15, 24, 19};  // P0.21, P0.22, P0.15, P0.24, P0.19

// GPIO base addresses
#define GPIO0_BASE 0x50000000   // P0.00 - P0.31
#define GPIO1_BASE 0x50000300   // P1.00 - P1.09

// GPIO port 1 registers
#define GPIO0_OUT (*(volatile unsigned int *)(GPIO0_BASE + 0x504))
#define GPIO0_OUTSET (*(volatile unsigned int *)(GPIO0_BASE + 0x508))
#define GPIO0_OUTCLR (*(volatile unsigned int *)(GPIO0_BASE + 0x50C))
#define GPIO0_DIRSET (*(volatile unsigned int *)(GPIO0_BASE + 0x518))
#define GPIO0_DIRCLR (*(volatile unsigned int *)(GPIO0_BASE + 0x51C))

// GPIO port 2 registers
#define GPIO1_OUT (*(volatile unsigned int *)(GPIO1_BASE + 0x504))
#define GPIO1_OUTSET (*(volatile unsigned int *)(GPIO1_BASE + 0x508))
#define GPIO1_OUTCLR (*(volatile unsigned int *)(GPIO1_BASE + 0x50C))
#define GPIO1_DIRSET (*(volatile unsigned int *)(GPIO1_BASE + 0x518))
#define GPIO1_DIRCLR (*(volatile unsigned int *)(GPIO1_BASE + 0x51C))

// Busy-wait delay function
#define ITER_BUSY_WAIT 5500000 // 5.5 million iterations for about a 1 seconds delay

// Smiley face pattern for testing
int SMILEY_PATTERN[25] = {
    0, 0, 0, 0, 0,
    0, 1, 0, 1, 0,
    0, 0, 0, 0, 0,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0
};

int ZERO[25] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0
};

int ONE[25] = {
    0, 1, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0
};

int TWO[25] = {
    0, 1, 1, 0, 0,
    1, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    1, 1, 1, 1, 0
};

int THREE[25] = {
    1, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    1, 1, 1, 1, 0
};

int FOUR[25] = {
    1, 0, 0, 1, 0,
    1, 0, 0, 1, 0,
    1, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 0, 0, 1, 0
};

int FIVE[25] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    1, 1, 1, 1, 0
};

int SIX[25] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    1, 0, 0, 1, 0,
    1, 1, 1, 1, 0
};

int SEVEN[25] = {
    1, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    1, 0, 0, 0, 0
};

int EIGHT[25] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 1, 0,
    0, 1, 1, 0, 0,
    1, 0, 0, 1, 0,
    1, 1, 1, 1, 0
};

int NINE[25] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 1, 0,
    1, 1, 1, 1, 0,
    0, 0, 0, 1, 0,
    1, 1, 1, 1, 0
};

// Array of number patterns
int* numberPatterns[10] = {
    ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE
};

/**
 * @brief Simple busy-wait delay function NOT using a hardware timer.
 * 
 * @param duration Duration of the delay in milliseconds. 
 * @return None
 */
void busyWaitTimer(int duration) {
    duration *= (ITER_BUSY_WAIT / 1000); // Divide by 1000 to convert to seconds

    for (volatile int i = 0; i < duration; i++) {
        // Do nothing, just delay
    }
}

/**
 * @brief Configures and starts a hardware timer with a specified duration.
 * 
 * @param duration The duration of the timer in milliseconds.
 * @return None
 */
void hardwareTimer(int duration) {
    // Ensure the timer is stopped and reset
    NRF_TIMER0->TASKS_STOP = 1;  // Stop the timer
    NRF_TIMER0->TASKS_CLEAR = 1; // Clear the timer

    // Configure the timer
    NRF_TIMER0->MODE = NRF_TIMER_MODE_TIMER;      // Set the timer to Timer Mode
    NRF_TIMER0->BITMODE = NRF_TIMER_BIT_WIDTH_32; // Set the timer to 32-bit mode
    NRF_TIMER0->PRESCALER = 4;                    // Set prescaler to 4 for 1MHz (1 tick = 1 microsecond)

    // Prepare compare event
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;            // Clear compare event register
    NRF_TIMER0->CC[0] = duration * 1000;          // Set up compare register for duration in microseconds

    // Start timer
    NRF_TIMER0->TASKS_START = 1;

    // Wait for compare event
    while (NRF_TIMER0->EVENTS_COMPARE[0] == 0) {
        // Wait here until the compare event fires
    }

    // Cleanup
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;            // Clear the event
    NRF_TIMER0->TASKS_STOP = 1;                   // Stop the timer
}


/**
 * @brief Configures and starts a hardware timer with interrupt.
 * 
 * @param duration The duration of the timer in milliseconds.
 * @return None
 */
void hardwareTimerWithInterrupt(int duration){
    // Ensure the timer is stopped and reset
    NRF_TIMER1->TASKS_STOP = 1;  // Stop the timer
    NRF_TIMER1->TASKS_CLEAR = 1; // Clear the timer

    // Configure the timer
    NRF_TIMER1->MODE = NRF_TIMER_MODE_TIMER;      // Set the timer to Timer Mode
    NRF_TIMER1->BITMODE = NRF_TIMER_BIT_WIDTH_32; // Set the timer to 32-bit mode
    NRF_TIMER1->PRESCALER = 4;                    // Set prescaler to 4 for 1MHz (1 tick = 1 microsecond)

    // Enable interrupt
    NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk; // Enable interrupt for compare event
    NVIC_EnableIRQ(TIMER1_IRQn);                        // Enable the interrupt in the NVIC

    // Start the timer
    NRF_TIMER1->TASKS_START = 1;                
}


/**
 * @brief Initialises the micro:bit LED matrix.
 * 
 * This function sets the direction of specific GPIO pins to output mode.
 * It configures the GPIO0 pins for columns 1, 2, 3, and 5, GPIO1 pin for 
 * column 4, and GPIO0 pins for rows 1 to 5 all as outputs.
 * 
 * @param None
 * @return None
 */
void initialiseMatrixGPIO() {
    int i;

    // Set direction of GPIO0 pins to output for rows
    for (i = 0; i < 5; i++) {
        GPIO0_DIRSET = (1 << ROW_PINS[i]);
    }

    // Set direction of GPIO0 pins to output for columns
    for (i = 0; i < 5; i++) {
        if (i == 3) { // Column 4 is on GPIO1
            GPIO1_DIRSET = (1 << COL_PINS[i]);
        } else {
            GPIO0_DIRSET = (1 << COL_PINS[i]);
        }
    }
}

/**
 * @brief Clears the LED matrix by setting all rows to low and all columns to high.
 * 
 * This function ensures that all rows of the LED matrix are set to low, preventing other LEDs from lighting up.
 * 
 * @param None
 * @return None
 */
void clearMatrix() {
    int i;

    // First, ensure all rows are low to prevent other LEDs from lighting up
    for (i = 0; i < 5; i++) {
        GPIO0_OUTCLR = (1 << ROW_PINS[i]);
    }

    // Then, ensure all columns are high (because common cathode - low activates the LED)
    for (i = 0; i < 5; i++) {
        if (i == 3) { // Column 4 is on GPIO1
            GPIO1_OUTSET = (1 << COL_PINS[i]);
        } else {
            GPIO0_OUTSET = (1 << COL_PINS[i]);
        }
    }
}

/**
 * @brief Clear the LED matrix row by only setting the row to low, and all collumns high. 
 * 
 * This is a more optimised variant of the clearMatrix function, as it only sets one row 
 * to low, instead of iterating through all rows in the LED matrix.
 * 
 * Even better optimisation can be achieved if only the active columns are set back to high, 
 * instead of all columns. However, the actual actual lookup would use more clock cycles than
 * just setting all columns to low. 
 * 
 * 
 * @param row The row number to clear (1-5).
 * @return None
 */
void clearRow(int row){

    // First, ensure the row is low to prevent other LEDs from lighting up
    GPIO0_OUTCLR = (1 << ROW_PINS[row-1]);

    // Then, ensure all columns are high (because common cathode - low activates the LED)
    for (int i = 0; i < 5; i++) {
        if (i == 3) { // Column 4 is on GPIO1
            GPIO1_OUTSET = (1 << COL_PINS[i]);
        } else {
            GPIO0_OUTSET = (1 << COL_PINS[i]);
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
    GPIO0_OUTSET = (1 << ROW_PINS[row - 1]);

    // And activate the selected column by setting it low
    if (column == 4) { // Column 4 is on GPIO1
        GPIO1_OUTCLR = (1 << COL_PINS[column - 1]);
    } else {
        GPIO0_OUTCLR = (1 << COL_PINS[column - 1]);
    }
}

/**
 * @brief Turns off a specific LED on a micro:bit LED matrix.
 *
 * @param row The row number of the LED (1-5).
 * @param column The column number of the LED (1-5).
 * @return None
 */
void turnOffLED(int row, int column){
    // Deactivate the selected row (set low)
    GPIO0_OUTCLR = (1 << ROW_PINS[row - 1]);

    // And deactivate the selected column by setting it high
    if (column == 4) { // Column 4 is on GPIO1
        GPIO1_OUTSET = (1 << COL_PINS[column - 1]);
    } else {
        GPIO0_OUTSET = (1 << COL_PINS[column - 1]);
    }
}

/**
 * @brief Display an image on the LED matrix based on a 25-element array.
 *
 * This function display an image on the micro:bit display by iterating over each 
 * row, and setting the correct column LEDs on each row. By clearing each row  
 * and display the next one with (very little delay) we can achieve the illusion
 * of displaying an image on the LED matrix. 
 * 
 * This can be optimised with 
 * (1) a more efficient row refreshing function,
 * (2) by analysing the image, and grouping pixels together to reduce the number of 
 *  register writes.
 * 
 * @param image A 25-element array with 0s and 1s representing the LED states.
 * @param timerFunction The timer function to use for the delays.
 * @return None
 */
void displayImage(int image[25], void (*timerFunction)(int delay)) {
    clearMatrix();  // Clear the matrix before displaying the image
    while(true){
        for (int row = 1; row <= 5; row++) {  // Loop over each row (1-based index)
            clearRow(row);  // Clear the previous state of this row

            for (int col = 1; col <= 5; col++) {  // Loop over each column in this row (1-based index)
                int index = (row - 1) * 5 + (col - 1);  // Calculate the index in the image array
                if (image[index] == 1) {
                    turnOnLED(row, col);  // Turn on the LED if the image array at index is 1
                }
            }
            timerFunction(4);  // Wait for 4 millisecond (50Hz refresh rate)
            clearRow(row);  // Clear the row after displaying it
        }
    }
}

/**
 * @brief Task 1 on lab 2. Display a smiley face on the LED matrix, and 
 * use a busy wait timer.
 * 
 * @param None
 * @return None
 */
void beHappy(){
    initialiseMatrixGPIO(); // Initialise the GPIO pins for the LED matrix
    displayImage(SMILEY_PATTERN, busyWaitTimer);
}

/**
 * @brief Task 2 on lab 2. Display a smiley face on the LED matrix, and 
 * use a hardware timer.
 * 
 * @param None
 * @return None
 */
void beVeryHappy(){
    initialiseMatrixGPIO(); // Initialise the GPIO pins for the LED matrix
    displayImage(SMILEY_PATTERN, hardwareTimer);
}

/**
 * @brief Task 3 on lab 2. Display a smiley face on the LED matrix, and use 
 * a hardware timer with an interrupt.
 * 
 * @param None
 * @return None
 */
void beHappyAndFree(){
    initialiseMatrixGPIO(); // Initialise the GPIO pins for the LED matrix
    displayImage(SMILEY_PATTERN, hardwareTimerWithInterrupt);
}

/**
 * @brief Task 4 on lab 2. Display a smiley face on the LED matrix, and 
 * use a hardware timer with an interrupt.
 * 
 * @param None
 * @return None
 */
void showNumber(int number){
    initialiseMatrixGPIO(); // Initialise the GPIO pins for the LED matrix
    displayImage(numberPatterns[number], hardwareTimerWithInterrupt);
}

int main() {
    beHappy();        // Task 1
    // beVeryHappy();    // Task 2
    // beHappyAndFree(); // Task 3
    // showNumber(6);    // Task 4
    while(true);
}