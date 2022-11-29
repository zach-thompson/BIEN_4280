#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*
* Author: Zach Thompson
* Date: 10/19/21
* Description: Establishes the functions setbit, clearbit, setbits, clearbits, and display_binary
*/

#ifndef GUARD
#define GUARD

void setbit(uint32_t* addr, uint8_t whichbit);

void clearbit(uint32_t* addr, uint8_t whichbit);

void setbits(uint32_t* addr, uint32_t bitmask);

void clearbits(uint32_t* addr, uint32_t bitmask);

void display_binary(uint32_t num);

#endif