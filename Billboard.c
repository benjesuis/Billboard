/*
 * Billboard.c
 *
 *  Created on: Jan 5, 2018
 *  Author: GlennXavier
 */

#include "Billboard.h"
#include <avr/io.h>
#include <avr/delay.h>

// MCU Clock Speed
#define F_CPU 12000000L

double pastAltitudes[40];
double altitude;
// TODO Remove global variables

void setup() {
	// Set Pin Modes
	DDRB = 0x11111111;
	DDRC = 0x11111111;
	DDRD = 0x11111111;
	// TODO Set correct pins
}

int main (void) {
	setup();

	// Flight Status Variables
	int launch;
	int apogee;
	int land;

	while(1) {

	}
	return (0);
}

/**
 * Checks if the rocket has launched
 */
int checkLaunch() {
	int flag = 0;
	for (int i = 0; i < 40; i++) {
		if ((altitude - pastAltitudes[i]) > 100) {
			flag = 1;
		}
	}
	cleanPastAltitudes();
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
		}
	}
	cleanPastAltitudes();
	return flag;
}

/**
 * Checks if the rocket has landed
 */
int checkLanded() {
	int flag = 0;
	// Rocket has landed
	if ((abs(pastAltitudes[0] - altitude)) < 5) {
		flag = 1; // No need to call cleanPastAltitudes since it is no longer needed
	}
	// Rocket has not landed
	cleanPastAltitudes();
	return flag;
}

/*
 * Retains only 40 past altitudes
 */
void cleanPastAltitudes() {
	int difference = sizeof(pastAltitudes) / sizeof(int) - 39;
	for (int i = 0; i < 40; i++) {
		pastAltitudes[i] = pastAltitudes[i + difference];
	}
}

/**
 * Generates String to write to SD card
 */
char* poll() {



}

/**
 * Returns the acceleration for 4 consecutive altitudes
 */
float getAcceleration(float altitudes[]) {
	float speeds[4];
	//50ms per poll
	speeds[0] = (altitudes[1] - altitudes[0])/0.05;
	speeds[1] = (altitudes[3] - altitudes[2])/0.05;
	return (speeds[1] - speeds[0])/0.05;
}

/**
 * Returns g-Forces for a given acceleration
 */
float getgForce(float acceleration) {
	return acceleration/9.80665;
}


/**
 * Writes string to SD card
 */
void write(char* string) {

}
