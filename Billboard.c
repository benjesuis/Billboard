/*
 * Boston University Rocket Propulsion Group HPR Team 3
 * To be used for data collection on Project Platypus
 *
 * Created: Jan 5, 2018
 * Author: Glenn Xavier
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 12000000L // Set MCU clock speed

// Data variables
float pastAltitudes[40];
float altitude;
float acceleration;
float gForce;
float magnetometer;
float photoresistor1;
float photoresistor2;

// Flight status variables
int launch = 0;
int apogee = 0;
int landed = 0;

int main (void) {
	// Set input and output pins
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0xFF; // TODO Use correct pins

	// Setup timers and interrupts
	TCCR1B = (1 << WGM12); // Set CTC
	OCR1A = 586; // Set compare value. This sets each interrupt to 50ms
	TIMSK1 = (1 << OCIE1A); // Choose timer and interrupt mode
	sei();
	TCCR1B = (1 << CS12) | (1 <<CS10); // Set prescaler to 1024 and start timer
}

/**
 * Runs every timer interrupt
 * Implements main data collection algorithm
 */
void ISR(TIMER1_COMPA_vect) {
	poll();
	char* str = getString();
	write(str);
	// Check flight status
	if (!launch) {
		checkLaunch();
		return;
	}
	if (!apogee) {
		checkApogee();
		return;
	}
	if(!landed) {
		checkLanded();
		analyseData();
	}
}

/**
 * Checks if the rocket has launched
 */
int checkLaunch() {
	int flag = 0;
	for (int i = 0; i < 40; i++) {
		if ((altitude - pastAltitudes[i]) > 100) {
			flag = 1;
			write("LAUNCH");
		}
	}
	return flag;
}

/**
 * Checks if the rocket has reached apogee
 */
int checkApogee() {
	int flag = 0;
	for (int i = 0; i < 40; i++) {
		if ((altitude - pastAltitudes[i]) < -5) {
			flag = 1;
			write("APOGEE");
		}
	}
	return flag;
}

/**
 * Checks if the rocket has landed
 */
int checkLanded() {
	int flag = 0;
	// Rocket has landed
	if ((abs(pastAltitudes[0] - altitude)) < 5) {
		flag = 1;
		write("LANDING");
	}
	// Rocket has not landed
	return flag;
}

/**
 * Returns the acceleration for 4 consecutive altitudes
 */
void getAcceleration() {
	float speeds[2];
	//50ms per poll
	speeds[0] = (pastAltitudes[37] - pastAltitudes[36])/0.05;
	speeds[1] = (pastAltitudes[39] - pastAltitudes[38])/0.05;
	acceleration =  (speeds[1] - speeds[0])/0.05;
}

/**
 * Returns g-Forces for a given acceleration
 */
void getgForce() {
	gForce = acceleration/9.80665;
}

/**
 * Polls data from sensors and updates global variables
 */
void poll() {
	// Update altitude, magnetometer and photoresistors
	// TODO get sensor inputs
	// Update acceleration and g forces
	getAcceleration();
	getgForce();
	// Retains only 40 past altitudes
	for (int i = 0; i < 39; i++) {
		pastAltitudes[i] = pastAltitudes[i+1];
	}
	// Updates pastAltitudes
	pastAltitudes[39] = altitude;
}

/**
 * Generates String to write to SD card
 */
char* getString(float data[]) {
	return "test";
	// TODO implement getString
}

/**
 * Writes string to SD card
 */
void write(char* string) {
	// TODO implement SPI
}

/**
 * Analyses flight data and writes results to SD card
 */
void analyseData() {
	// TODO implement analyseData();
}
