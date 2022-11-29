#include "zthompson_binaryutils.hpp"

/*
* Author: Zach Thompson
* Date: 10/19/21
* Description: main class for MiniProject0. Executes commands setbit(s), clearbit(s), and display_binary
*/

void main() {
	uint32_t solo = 0;				// initialize starting variable
	uint32_t* solo_p = &solo;		// create pointer to variable address
	
	setbit(solo_p, 24);				// sets the 24th bit
	setbit(solo_p, 16);				// sets the 16th bit
	setbit(solo_p, 17);				// sets the 17th bit
	setbits(solo_p, 4095);			// sets the 0th-11th bits
	clearbit(solo_p, 11);			// clears the 11th bit
	clearbits(solo_p, 240);			// clears the 4th-7th bits

	printf("Binary Solo:\n\r");
	display_binary(solo);			// displays the binary conversion
	
	return;
}