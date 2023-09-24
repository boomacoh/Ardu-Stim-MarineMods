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

#ifndef __SERIAL_MENU_H__
#define __SERIAL_MENU_H__
 
#include <SerialUI.h>
//#include "structures.h"

/* Structures */

/* Prototypes */
/* Callbacks */
void show_info_cb(void);
void select_next_wheel_cb(void);
void select_previous_wheel_cb(void);
void toggle_invert_primary_cb(void);
void toggle_invert_secondary_cb(void);
void list_wheels_cb(void);
void select_wheel_cb(void);
void set_rpm_cb(void);
void sweep_rpm_cb(void);
void reverse_wheel_direction_cb(void);
void do_exit(void);
/* Callbacks */

/* General functions */
void display_rpm_info(void);
void serial_setup(void);
void display_new_wheel(void);
void print_normal(void);
void print_inverted(void);
void compute_sweep_stages(uint16_t *, uint16_t *);
uint16_t get_rpm_from_tcnt(uint16_t *, uint8_t *);
uint8_t get_bitshift_from_prescaler(uint8_t *);

#endif
