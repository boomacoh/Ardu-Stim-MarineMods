/*
 * vim: filetype=c expandtab shiftwidth=2 tabstop=2 softtabstop=2:
 *
 * Arbritrary crank/cam wheel pattern generator
 *
 * copyright 2014-2017 David J. Andruczyk
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

#ifndef __ENUMS_H__
#define __ENUMS_H__
 
/* Enumerations */
enum {                                                                          
  PRESCALE_1 = 1,
  PRESCALE_8 = 2,
  PRESCALE_64 = 3,
  PRESCALE_256 = 4,
  PRESCALE_1024 = 5,
  RECALC
};

enum {
  DESCENDING,
  ASCENDING
};

enum {
  FIXED_RPM,
  LINEAR_SWEPT_RPM,
};

#endif
