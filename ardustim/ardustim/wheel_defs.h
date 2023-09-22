/* vim: set syntax=c expandtab sw=2 softtabstop=2 autoindent smartindent smarttab : */
/*
 * Arbritrary wheel pattern generator wheel definitions
 *
 * copyright 2014 David J. Andruczyk
 * 
 * Ardu-Stim software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ArduStim software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with any ArduStim software.  If not, see http://www.gnu.org/licenses/
 *
 */
#ifndef __WHEEL_DEFS_H__
#define __WHEEL_DEFS_H__

#include <avr/pgmspace.h>

/* Wheel patterns! 
  *
  * Wheel patterns define the pin states and specific times. The ISR runs 
  * at a constant speed related to the requested RPM. The request RPM is 
  * scaled based on the LENGTH of each wheel's array.  The reference is 
  * the 60-2 which was the first decoder designed which has 120 "edges" 
  * (transitions" for each revolution of the wheel. Any other wheel that 
  * also has 120 edges has and RPM scaling factor of 1.0. IF a wheel has 
  * less edges needed to "describe" it, it's number of edges are divided by 120 to
  * get the scaling factor which is applied to the RPM calculation.
  * There is an enumeration (below) that lists the defined wheel types, 
  * as well as an array listing the rpm_scaling factors with regards to 
  * each pattern.
  * 
  * NOTE: There is MORE THAN ONE WAY to define a wheel pattern.  You can 
  * use more edges to get to 1 deg accuracy but the side effect is that 
  * your maximum RPM is capped because of that. Currently 60-2 can run 
  * up to about 60,000 RPM, 360and8 can only do about 10,000 RPM becasue 
  * it has 6x the number of edges...  The less edges, the faster it can go... :)
  * 
  * Using more edges allows you to do things like vary the dutycycle,  
  * i.e. a simple non-missing tooth 50% duty cycle wheel can be defined 
  * with only 2 entries if you really want, but I didn't do it that way 
  * for some of the simple ones as it made it seem somewhat confusing
  * to look at as it required you to keep the rpm_scaler factor in mind.  
  * Most/all patterns show the pulses you're receive for one revolution
  * of a REAL wheel on a real engine.
  */

/* Wheel types we know about...
   * This enumerations is the INDEX into the Wheels[] array of structures
   * defined in main file. That struct contains pointers to the following:
   * wheel name in a user friendly string
   * pointer to the wheel edge array used by the ISR
   * RPM scaling factor (num_edges/120 for crank wheels)
   * Number of edges in the edge array above, needed by the ISR 
   */
typedef enum {


  EIGHT_CAM_ONE_CRANK,
  INVERTED_EIGHT_CAM_ONE_CRANK,
  SIXTY_MINUS_TWO_WITH_4X_CRANK,
  SIXTY_MINUS_THREE_WITH_4X_CRANK,

  MAX_WHEELS,
} WheelType;
const char eight_cam_one_crank_friendly_name[] PROGMEM = "8Cam with 1 Crank";
const char inverted_eight_cam_one_crank_friendly_name[] PROGMEM = "Inverted 8Cam with 1 Crank";
const char sixty_minus_two_with_4X_cam_friendly_name[] PROGMEM = "GM 60-2 with 4X cam";
const char sixty_minus_three_with_4X_cam_friendly_name[] PROGMEM = "GM 60-3 with 4X cam";

/* Very simple 50% duty cycle */

const unsigned char eight_cam_one_crank_array[] PROGMEM = {
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0
};

/* Bosch 60-3 pattern with 2nd trigger on rotation 2,
  * GM 4X CAM */

const unsigned char eight_cam_one_crank[] PROGMEM = { /* 60-2 */
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 251, 251, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 247, 247, 255, 255, 255, 255, 255, 255, 255, 255,
                                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

const unsigned char inverted_eight_cam_one_crank_array[] PROGMEM = {
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0
};

/* Bosch 60-3 pattern with 2nd trigger on rotation 2,
  * GM 4X CAM */

const unsigned char inverted_eight_cam_one_crank[] PROGMEM = { /* 60-2 */
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0,
                                                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const unsigned char sixty_minus_two_with_4X_cam[] PROGMEM = {
  /* 60-2 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 1-5 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 6-10 */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 11-15 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 16-20 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 21-25 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 26-30 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 31-35 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 36-40 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 41-45 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 46-50 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 51-55 */
  1, 0, 1, 0, 1, 0, 0, 0, 0, 0, /* teeth 56-58 and 59-60 MISSING */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 1-5 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 6-10 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 11-15 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 16-20 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 21-25 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 26-30 */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 31-35 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 36-40 */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 41-45 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 46-50 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 51-55 */
  3, 2, 3, 2, 3, 2, 2, 2, 2, 2  /* teeth 56-58 and 59-60 MISSING */
};

const unsigned char sixty_minus_three_with_4X_cam[] PROGMEM = {
  /* 60-2 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 1-5 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 6-10 */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 11-15 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 16-20 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 21-25 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 26-30 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 31-35 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 36-40 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 41-45 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 46-50 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 51-55 */
  1, 0, 1, 0, 0, 0, 0, 0, 0, 0, /* teeth 56-58 and 58-60 MISSING */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 1-5 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 6-10 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 11-15 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 16-20 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 21-25 */
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0, /* teeth 26-30 */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 31-35 */
  3, 2, 3, 2, 3, 0, 1, 0, 1, 0, /* teeth 36-40 */
  1, 0, 1, 0, 3, 2, 3, 2, 3, 2, /* teeth 41-45 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 46-50 */
  3, 2, 3, 2, 3, 2, 3, 2, 3, 2, /* teeth 51-55 */
  3, 2, 3, 2, 2, 2, 2, 2, 2, 2  /* teeth 56-58 and 58-60 MISSING */
};

#endif
