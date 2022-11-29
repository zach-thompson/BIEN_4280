#include "zthompson_binaryutils.hpp"
/*
* This contains the following function definitions:
* setbit
* clearbit
* setbits
* clearbits
* display_binary
*/

/*
* This function sets only the bit at the position given by the whichbit argument of the provided 32-bit address
*/
void setbit(uint32_t* addr, uint8_t whichbit) {
	uint32_t mask = 1;							// mask = 0000 0000 0000 0000 0000 0000 0000 0001
	*addr = *addr | (mask << whichbit);			// Logical OR of addr value and the mask left-shifted by 'whichbit' bits
	return;
}

/*
* This function clears only the bit at the position given by the whichbit argument of the provided 32-bit address
*/
void clearbit(uint32_t* addr, uint8_t whichbit) {
	uint32_t mask = 1;							// mask = 0000 0000 0000 0000 0000 0000 0000 0001
	*addr = *addr &~ (mask << whichbit);			// logical AND NOT of addr value and the mask left-shifted by 'whichbit' bits
	return;
}

/*
* This function sets only the bits defined in the bitmask argument
*/
void setbits(uint32_t* addr, uint32_t bitmask) {
	*addr = *addr | bitmask;						// logical OR of addr value and the bitmask
	return;
}

/*
* This function clears only the bits defined in the bitmask argument
*/
void clearbits(uint32_t* addr, uint32_t bitmask) {
	*addr = *addr & ~bitmask;					// logical AND of addr value and the inverse of the bitmask
	return;
}

/*
* This function displays a binary representation of the num argument to the console
*/
void display_binary(uint32_t num) {
	for (int i = 31; i >= 0; i--) {				// runs a FOR loop for 31 > i >= 0
		if (num >= pow(2.0,i)) {				// checks if num is greater than or equal to 2^i
			printf("1");						// prints 1 for bit(i) if greater
			num = num - pow(2.0,i);				// subtracts 2^i from num - effectively removes bit(i) from num
		}
		else printf("0");						// prints 0 for bit(i) if num is less than 2^i
	}
	return;
}