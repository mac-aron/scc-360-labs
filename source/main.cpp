// Library imports for debugging only
#include "MicroBit.h"
NRF52Pin usbTx(ID_PIN_USBTX, MICROBIT_PIN_UART_TX, PIN_CAPABILITY_DIGITAL);
NRF52Pin usbRx(ID_PIN_USBRX, MICROBIT_PIN_UART_RX, PIN_CAPABILITY_DIGITAL);
NRF52Serial serial(usbTx, usbRx, NRF_UARTE0);


// The Lab 2 execrises no longer requrie to define my own addresses and bits
// Values from the nrf52 SDK will be used from now on







int main() {
    while(true);
}