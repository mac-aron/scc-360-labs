// GPIO base addresses, port 0 and port 1
// P0.00 - P0.31 = 0x50000000
// P1.00 - P1.09 = 0x50000300

// Using the onboard 5x5 LED matrix
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

#define GPIO0_BASE   0x50000000
#define GPIO1_BASE   0x50000300

#define GPIO0_OUTSET (*(volatile unsigned int *)(GPIO0_BASE + 0x508))
#define GPIO0_OUTCLR (*(volatile unsigned int *)(GPIO0_BASE + 0x50C))
#define GPIO0_DIRSET (*(volatile unsigned int *)(GPIO0_BASE + 0x518))

#define GPIO1_OUTSET (*(volatile unsigned int *)(GPIO1_BASE + 0x508))
#define GPIO1_OUTCLR (*(volatile unsigned int *)(GPIO1_BASE + 0x50C))
#define GPIO1_DIRSET (*(volatile unsigned int *)(GPIO1_BASE + 0x518))

void turnOn() {
    // Set direction of GPIO0 pins to output
    GPIO0_DIRSET = (1 << 28) | (1 << 11) | (1 << 31) | (1 << 30);  // Columns 1, 2, 3, and 5 as outputs
    GPIO0_DIRSET = (1 << 21) | (1 << 22) | (1 << 15) | (1 << 24) | (1 << 19); // Rows 1 to 5 as outputs

    // Set direction of GPIO1 pin to output (Column 4)
    GPIO1_DIRSET = (1 << 5);  // Column 4 as output

    // Turn on LEDs by setting GPIO0 columns low and rows high
    GPIO0_OUTCLR = (1 << 28) | (1 << 11) | (1 << 31) | (1 << 30);  // Set columns 1, 2, 3, and 5 low
    GPIO0_OUTSET = (1 << 21) | (1 << 22) | (1 << 15) | (1 << 24) | (1 << 19); // Set rows 1 to 5 high

    // Turn on LEDs by setting GPIO1 column low
    GPIO1_OUTCLR = (1 << 5);  // Set column 4 low
}

int main() {
    turnOn();
    while (true); // Keep the microcontroller running
}
