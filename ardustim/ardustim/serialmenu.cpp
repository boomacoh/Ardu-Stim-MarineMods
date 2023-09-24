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

#include "defines.h"
#include "enums.h"
#include "wheel_defs.h"
#include <avr/pgmspace.h>
#include <math.h>
#include <util/delay.h>
#include <SerialUI.h>
#include "serialmenu.h"
#include "structures.h"
#include "sweep.h"
#include "user_defaults.h"

/* External Global Variables */
unsigned long wanted_rpm = DEFAULT_RPM;
extern SUI::SerialUI mySUI;
extern sweep_step *SweepSteps; /* Global pointer for the sweep steps */
extern wheels Wheels[];        /* Array of wheel structures */
extern uint8_t mode;           /* Sweep or fixed */
extern uint8_t total_sweep_stages;
extern uint16_t sweep_low_rpm;
extern uint16_t sweep_high_rpm;
extern uint16_t sweep_rate;

/* Volatile variables (USED in ISR's) */
extern volatile uint8_t selected_wheel;
extern volatile uint8_t camSignalBitShift;
extern volatile uint8_t sweep_direction;
extern volatile int8_t sweep_stage;
extern volatile bool normal;
extern volatile bool sweep_lock;
extern volatile bool sweep_reset_prescaler;
extern volatile uint16_t edge_counter;
extern volatile uint16_t new_OCR1A;
extern volatile uint32_t oc_remainder;

/* Local globals for serialUI state tracking */
bool fixed = true;
bool swept = false;
//! Initializes the serial port and sets up the Menu
/*!
 * Sets up the serial port and menu for the serial user interface
 * Sets user input timeout to 20 seconds and overall interactivity timeout at 30
 * at which point it'll disconnect the user
 */
void serial_setup() {
  mySUI.begin(9600);
  mySUI.setTimeout(20000);   /* Tiem to wait for input from druid4arduino */
  mySUI.setMaxIdleMs(30000); /* disconnect if no response from host in 30 sec */
  SUI::Menu *mainMenu = mySUI.topLevelMenu();
  SUI::Menu *wheelMenu;
  SUI::Menu *shiftCAMenu;
  SUI::Menu *advMenu;
  /* Simple all on one menu... */
  /* Menu strungs are in the header file */
  mainMenu->setName(F("ArduStim Main Menu"));
  mainMenu->addCommand(F("Information"), show_info_cb, F("Retrieve data and current settings"));
  mainMenu->addCommand(F("Set Fixed RPM"), set_rpm_cb, F("Set Fixed RPM"));
  mainMenu->addCommand(F("Set Swept RPM"), sweep_rpm_cb, F("Sweep the RPM (min,max,rate(rpm/sec))"));
  // mainMenu->addCommand(F(""), shift_cam, F("Shift CAM Bit Signals"));
  shiftCAMenu = mainMenu->subMenu(F("Shift CAM Bit"), F("Shift CAM Bit Signals (L,R)"));
  shiftCAMenu->addCommand(F("Left"), shift_cam_left, F("Shift CAM Bits to the Left"));
  shiftCAMenu->addCommand(F("Right"), shift_cam_right, F("Shift CAM Bits to the Left"));
  wheelMenu = mainMenu->subMenu(F("Wheel Options"), F("Wheel Options, (list,choose,select)"));
  wheelMenu->addCommand(F("Next wheel"), select_next_wheel_cb, F("Pick the next wheel pattern"));
  wheelMenu->addCommand(F("Previous wheel"), select_previous_wheel_cb, F("Pick the previous wheel pattern"));
  wheelMenu->addCommand(F("List wheels"), list_wheels_cb, F("List all wheel patterns"));
  wheelMenu->addCommand(F("Choose wheel"), select_wheel_cb, F("Choose a specific wheel pattern by number"));
  advMenu = mainMenu->subMenu(F("Advanced Options"), F("Advanced Options (polarity,glitch)"));
  advMenu->addCommand(F("Reverse Wheel Dir"), reverse_wheel_direction_cb, F("Reverse the wheel's direction of rotation"));
  advMenu->addCommand(F("Invert Primary"), toggle_invert_primary_cb, F("Invert Primary (crank) signal polarity"));
  advMenu->addCommand(F("Invert Secondary"), toggle_invert_secondary_cb, F("Invert Secondary (cam) signal polarity"));
  mainMenu->addCommand(F("Exit"), do_exit, F("Exit (and terminate Druid)"));
  /* Not implemented yet */
  //advMenu->addCommand(pri_glitch_key,primary_glitch_cb,pri_glitch_help);
  //advMenu->addCommand(sec_glitch_key,secondary_glitch_cb,sec_glitch_help);
  //These use way more memory than I would have hoped for... :(
  //mySUI.trackState(F("RPM"), &wanted_rpm);
  //mySUI.trackState(F("Fixed RPM"), &fixed);
  //mySUI.trackState(F("Swept RPM"), &swept);
}

/* Helper function to spit out amount of ram remainig */
//! Returns the amount of freeRAM
/*!
 * Figures out the amount of free RAM remaining nad returns it to the caller
 * \return amount of free memory
 */
uint16_t freeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

/* SerialUI Callbacks */
//! Inverts the polarity of the primary output signal
void toggle_invert_primary_cb() {
  extern uint8_t output_invert_mask;
  output_invert_mask ^= 0x01; /* Flip crank invert mask bit */
  mySUI.print(F("Primary Signal: "));
  if (output_invert_mask & 0x01) {
    print_inverted();
  } else {
    print_normal();
  }
}

void print_normal() {
  mySUI.println(F("Normal"));
}
void print_inverted() {
  mySUI.println(F("Inverted"));
}

//! Inverts the polarity of the secondary output signal
void toggle_invert_secondary_cb() {
  extern uint8_t output_invert_mask;
  output_invert_mask ^= 0x02; /* Flip cam invert mask bit */
  mySUI.print(F("Secondary Signal: "));
  if (output_invert_mask & 0x02)
    print_inverted();
  else
    print_normal();
}


//! Returns info about status, mode and free RAM
void show_info_cb() {
  mySUI.println(F("Welcome to ArduStim, written by David J. Andruczyk"));
  mySUI.print(F("Free RAM: "));
  mySUI.print(freeRam());
  mySUI.println(F("bytes."));
  mySUI.println(F("Currently selected Wheel pattern: "));
  mySUI.print(selected_wheel + 1);
  mySUI.print(F(":"));
  mySUI.println((const __FlashStringHelper *)Wheels[selected_wheel].decoder_name);
  display_rpm_info();
}


//! Displays RPM output depending on mode
void display_rpm_info() {
  if (mode == FIXED_RPM) {
    mySUI.print(F("Fixed RPM mode, Currently: "));
    mySUI.print(wanted_rpm);
    mySUI.println(F(" RPM"));
  }
  if (mode == LINEAR_SWEPT_RPM) {
    mySUI.print(F("Swept RPM mode From: "));
    mySUI.print(sweep_low_rpm);
    mySUI.print(F("<->"));
    mySUI.print(sweep_high_rpm);
    mySUI.print(F(" at: "));
    mySUI.print(sweep_rate);
    mySUI.println(F(" RPM/sec"));
  }
}
//! Display newly selected wheel information
/*!
 * Resets the output compare register for the newly changed wheel, then
 * resets edge_counter (wheel array index) to 0 and displays the new
 * wheel information to the end user
 */
void display_new_wheel() {
  if (mode != LINEAR_SWEPT_RPM)
    reset_new_OCR1A(wanted_rpm);
  else
    compute_sweep_stages(&sweep_low_rpm, &sweep_high_rpm);
  edge_counter = 0;  // Reset to beginning of the wheel pattern */
  mySUI.println(F("New Wheel chosen: "));
  mySUI.print(selected_wheel + 1);
  mySUI.print(F(": "));
  mySUI.println((const __FlashStringHelper *)Wheels[selected_wheel].decoder_name);
  display_rpm_info();
}


//! Prompts user for new wheel ID
/*!
 * Presents user with a numeric data prompt, waits for input, then verifies
 * they inputted a valid choice, then changes the running wheel pattern to the
 * user selected one and reruns the RPM calc (As oit's pattern specific) and then
 * resets the wheel position to 0 to avoid starting in a position off the end of 
 * the wheel pattern array
 */
void select_wheel_cb() {
  mySUI.showEnterNumericDataPrompt();
  byte newWheel = mySUI.parseInt();
  if ((newWheel < 1) || (newWheel > (MAX_WHEELS + 1))) {
    mySUI.returnError("Wheel ID out of range");
    return;
  }
  selected_wheel = newWheel - 1; /* use 1-MAX_WHEELS range */
  display_new_wheel();
}


//! Selects the next wheel in the list
/*!
 * Selects the next wheel, if at the end, wrap to the beginning of the list,
 * re-calculate the OCR1A value (RPM) and reset, return user information on the
 * selected wheel and current RPM
 */
void select_next_wheel_cb() {
  if (selected_wheel == (MAX_WHEELS - 1))
    selected_wheel = 0;
  else
    selected_wheel++;

  display_new_wheel();
}

//
//! Selects the previous wheel in the list
/*!
 * Selects the nex, if at the beginning, wrap to the end of the list,
 * re-calculate the OCR1A value (RPM) and reset, return user information on the
 * selected wheel and current RPM
 */
void select_previous_wheel_cb() {
  if (selected_wheel == 0)
    selected_wheel = MAX_WHEELS - 1;
  else
    selected_wheel--;

  display_new_wheel();
}


//! Changes the RPM based on user input
/*!
 * Prompts user for new RPM, reads it, validates it's within range, sets lock to
 * prevent a race condition with the sweeper, free's memory of SweepSteps 
 * structure IF allocated, sets the mode to fixed RPM, recalculates the new OCR1A 
 * value based on the user specificaed RPM and sets it and then removes the lock
 */
void set_rpm_cb() {
  mySUI.showEnterNumericDataPrompt();
  uint32_t newRPM = mySUI.parseULong();
  if (newRPM < 10) {
    mySUI.returnError("Invalid RPM, RPM too low");
    return;
  }
  /* Spinlock */
  while (sweep_lock)
    _delay_us(1);
  sweep_lock = true;
  if (SweepSteps)
    free(SweepSteps);
  mode = FIXED_RPM;
  fixed = true;
  swept = false;
  reset_new_OCR1A(newRPM);
  wanted_rpm = newRPM;

  mySUI.print(F("New RPM chosen: "));
  mySUI.println(wanted_rpm);
  sweep_lock = false;
}


//! Returns a list of user selectable wheel patterns
/*!
 * Iterates through the list of wheel patterns and prints them back to the user
 */
void list_wheels_cb() {
  byte i = 0;
  for (i = 0; i < MAX_WHEELS; i++) {
    mySUI.print(i + 1);
    mySUI.print(F(": "));
    mySUI.println((const __FlashStringHelper *)Wheels[i].decoder_name);
  }
}


//! Toggle the wheel direction, useful for debugging
/*!
 * Reverses the emitting wheel pattern direction.  Used mainly as a debugging aid
 * in case the wheel pattern was coded incorrectly in reverse.
 */
void reverse_wheel_direction_cb() {
  mySUI.print(F("Wheel Direction: "));
  if (normal) {
    normal = false;
    mySUI.println(F("Reversed"));
  } else {
    normal = true;
    print_normal();
  }
}


//! Parses input from user and setups up RPM sweep
/*!
 * Provides the user with a prompt and request input then parses a 3 param 
 * comma separate list from the user, validates the input
 * and determins the appropriate Ouput Compare threshold values and
 * prescaler settings as well as the amount to increment with each sweep
 * ISR iteration.  It breaks up the sweep range into octaves and linearily
 * changes OC threshold between those points, mainly due to the fact that the 
 * relationship between RPM and output compare register is an inverse
 * relationship, NOT a linear one, by working in octaves, we end up with
 * a smoother sweep rate, that doesn't accelerate as it approaches the higher
 * RPM threshold.   Since the arduino canot do floating point FAST in an ISR
 * we use this to keep things as quick as possible. This function takes
 * no parameters (it cannot due to SerialUI) and returns void
 */
void sweep_rpm_cb() {
  uint16_t tmp_low_rpm;
  uint16_t tmp_high_rpm;
  uint8_t j;
  //uint8_t y;
  char sweep_buffer[20] = { 0 };

  mySUI.showEnterDataPrompt();
  //y = mySUI.readBytesToEOL(sweep_buffer,20);
  mySUI.readBytesToEOL(sweep_buffer, 20);
  /* Debugging 
  mySUI.print(F("Read: "));
  mySUI.print(y);
  mySUI.println(F(" charactors from the user")); 
  mySUI.print(F("Fed: "));
  mySUI.println(sweep_buffer);
  */
  j = sscanf(sweep_buffer, "%i,%i,%i", &tmp_low_rpm, &tmp_high_rpm, &sweep_rate);
  /* Debugging
  mySUI.print(F("Fields: "));
  mySUI.println(j);
  mySUI.print(F("Low: "));
  mySUI.println(tmp_low_rpm);
  mySUI.print(F("High: "));
  mySUI.println(tmp_high_rpm);
  mySUI.print(F("Sweep Rate: "));
  mySUI.println(sweep_rate);
  */
  // Validate input ranges
  if ((j == 3) && (tmp_low_rpm >= 10) && (tmp_low_rpm < 51200) && (tmp_high_rpm >= 10) && (tmp_high_rpm < 51200) && (sweep_rate >= 1) && (sweep_rate < 51200) && (tmp_low_rpm < tmp_high_rpm)) {
    mySUI.print(F("Sweeping from: "));
    mySUI.print(tmp_low_rpm);
    mySUI.print(F("<->"));
    mySUI.print(tmp_high_rpm);
    mySUI.print(F(" at: "));
    mySUI.print(sweep_rate);
    mySUI.println(F(" RPM/sec"));

    compute_sweep_stages(&tmp_low_rpm, &tmp_high_rpm);
  } else {
    mySUI.returnError(F("Range error !(10-50000,10-50000,1-50000)!"));
  }
}


void compute_sweep_stages(uint16_t *tmp_low_rpm, uint16_t *tmp_high_rpm) {
  uint8_t total_stages;
  uint32_t low_rpm_tcnt;
  uint32_t high_rpm_tcnt;

  /* Spin until unlocked, then lock */
  while (sweep_lock)
    _delay_us(1);
  sweep_lock = true;

  // Get OC Register values for begin/end points
  low_rpm_tcnt = (uint32_t)(8000000.0 / (((float)(*tmp_low_rpm)) * Wheels[selected_wheel].rpm_scaler));
  high_rpm_tcnt = (uint32_t)(8000000.0 / (((float)(*tmp_high_rpm)) * Wheels[selected_wheel].rpm_scaler));

  // Get number of frequency doublings, rounding
  total_stages = (uint8_t)ceil(log((float)(*tmp_high_rpm) / (float)(*tmp_low_rpm)) / (2 * LOG_2));
  if (SweepSteps)
    free(SweepSteps);
  /* Debugging 
  mySUI.print(F("low TCNT: "));
  mySUI.println(low_rpm_tcnt);
  mySUI.print(F("high rpm raw TCNT: "));
  mySUI.println(high_rpm_tcnt);
  */
  SweepSteps = build_sweep_steps(&low_rpm_tcnt, &high_rpm_tcnt, &total_stages);

  /* VERY BROKEN CODE 
    SweepSteps[i+1].prescaler_bits = SweepSteps[i].prescaler_bits;
    SweepSteps[i+1].ending_ocr = SweepSteps[i].ending_ocr;
    SweepSteps[i].ending_ocr =  (0.38 * (float)(SweepSteps[i].beginning_ocr - SweepSteps[i].ending_ocr)) + SweepSteps[i].ending_ocr;
    SweepSteps[i+1].beginning_ocr = SweepSteps[i].ending_ocr;
  }
  */

  for (uint8_t i = 0; i < total_stages; i++) {
    uint16_t this_step_low_rpm = get_rpm_from_tcnt(&SweepSteps[i].beginning_ocr, &SweepSteps[i].prescaler_bits);
    uint16_t this_step_high_rpm = get_rpm_from_tcnt(&SweepSteps[i].ending_ocr, &SweepSteps[i].prescaler_bits);
    /* How much RPM changes this stage */
    uint16_t rpm_span_this_stage = this_step_high_rpm - this_step_low_rpm;
    /* How much TCNT changes this stage */
    uint16_t steps = (uint16_t)(1000 * (float)rpm_span_this_stage / (float)sweep_rate);
    float per_isr_tcnt_change = (float)(SweepSteps[i].beginning_ocr - SweepSteps[i].ending_ocr) / steps;
    uint32_t scaled_remainder = (uint32_t)(FACTOR_THRESHOLD * (per_isr_tcnt_change - (uint16_t)per_isr_tcnt_change));
    SweepSteps[i].tcnt_per_isr = (uint16_t)per_isr_tcnt_change;
    SweepSteps[i].remainder_per_isr = scaled_remainder;

    /* Debugging
    mySUI.print(F("sweep step: "));
    mySUI.println(i);
    mySUI.print(F("steps: "));
    mySUI.println(steps);
    mySUI.print(F("Beginning tcnt: "));
    mySUI.print(SweepSteps[i].beginning_ocr);
    mySUI.print(F(" for RPM: "));
    mySUI.println(this_step_low_rpm);
    mySUI.print(F("ending tcnt: "));
    mySUI.print(SweepSteps[i].ending_ocr);
    mySUI.print(F(" for RPM: "));
    mySUI.println(this_step_high_rpm);
    mySUI.print(F("prescaler bits: "));
    mySUI.println(SweepSteps[i].prescaler_bits);
    mySUI.print(F("tcnt_per_isr: "));
    mySUI.println(SweepSteps[i].tcnt_per_isr);
    mySUI.print(F("scaled remainder_per_isr: "));
    mySUI.println(SweepSteps[i].remainder_per_isr);
    mySUI.print(F("FP TCNT per ISR: "));
    mySUI.println(per_isr_tcnt_change,6);
    mySUI.print(F("End of step: "));
    mySUI.println(i);
    */
  }
  total_sweep_stages = total_stages;
  /*
  mySUI.print(F("Total sweep stages: "));
  mySUI.println(total_sweep_stages);
  */
  /* Reset params for Timer2 ISR */
  sweep_stage = 0;
  sweep_direction = ASCENDING;
  sweep_reset_prescaler = true;
  new_OCR1A = SweepSteps[sweep_stage].beginning_ocr;
  oc_remainder = 0;
  mode = LINEAR_SWEPT_RPM;
  fixed = false;
  swept = true;
  sweep_high_rpm = *tmp_high_rpm;
  sweep_low_rpm = *tmp_low_rpm;
  sweep_lock = false;
}


//! Gets RPM from the TCNT value
/*!
 * Gets the RPM value based on the passed TCNT and prescaler
 * \param tcnt pointer to Output Compare register value
 * \param prescaler_bits point to prescaler bits enum
 */
uint16_t get_rpm_from_tcnt(uint16_t *tcnt, uint8_t *prescaler_bits) {
  //extern wheels Wheels[];
  uint8_t bitshift;
  bitshift = get_bitshift_from_prescaler(prescaler_bits);
  return (uint16_t)((float)(8000000 >> bitshift) / (Wheels[selected_wheel].rpm_scaler * (*tcnt)));
}


//! Gets bitshift value from prescaler enumeration
/*!
 * Gets the bit shift value based on the prescaler enumeration passed
 * \param prescaler_bits the enumeration to analyze
 * \returns the necessary bitshift associated with the prescale value
 */
uint8_t get_bitshift_from_prescaler(uint8_t *prescaler_bits) {
  switch (*prescaler_bits) {
    case PRESCALE_1024:
      return 10;
    case PRESCALE_256:
      return 8;
    case PRESCALE_64:
      return 6;
    case PRESCALE_8:
      return 3;
    case PRESCALE_1:
      return 0;
  }
  return 0;
}

//! Shift Signal on the CAM Signal (8 Bits)
/*!
 * Prompts user for direction to shift bits on the CAM Signal Bits
 */
void shift_cam_left() {
  mySUI.showEnterNumericDataPrompt();
  uint32_t newBitShift = mySUI.parseULong();
  camSignalBitShift = newBitShift;
}
void shift_cam_right() {
  mySUI.showEnterNumericDataPrompt();
  uint32_t newBitShift = mySUI.parseULong();
  camSignalBitShift = -newBitShift;
}


void do_exit() {
  // though you can always just use the "quit" command from
  // the top level menu, this demonstrates using exit(), which
  // will automatically close the Druid4Arduino GUI, if
  // being used.
  mySUI.print(F("Exit requested, terminating GUI if present"));
  mySUI.exit();
}
