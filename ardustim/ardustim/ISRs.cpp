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
#include "structures.h"
#include "sweep.h"
#include "user_defaults.h"
#include <inttypes.h>
#include <Arduino.h>
#include <SerialUI.h>

/* Sensistive stuff used in ISR's */
extern volatile uint8_t fraction;
extern volatile uint8_t selected_wheel;
extern volatile uint16_t adc0; /* POT RPM */
extern volatile uint16_t adc1; /* Pot Wheel select */
extern volatile uint32_t oc_remainder;
/* Setting rpm to any value over 0 will enabled sweeping by default */
/* Stuff for handling prescaler changes (small tooth wheels are low RPM) */
extern volatile uint8_t analog_port;
extern volatile bool adc0_read_complete;
extern volatile bool adc1_read_complete;
extern volatile bool reset_prescaler;
extern volatile bool normal;
extern volatile bool sweep_reset_prescaler; /* Force sweep to reset prescaler value */
extern volatile bool sweep_lock;
extern volatile uint8_t output_invert_mask; /* Don't invert anything */
extern volatile uint8_t sweep_direction;
extern volatile byte total_sweep_stages;
extern volatile int8_t sweep_stage;
extern volatile uint8_t prescaler_bits;
extern volatile uint8_t last_prescaler_bits;
extern volatile uint8_t mode;
extern volatile uint16_t new_OCR1A; /* sane default */
extern volatile uint16_t edge_counter;

/* Less sensitive globals */
extern uint8_t bitshift;
extern uint16_t sweep_low_rpm;
extern uint16_t sweep_high_rpm;
extern uint16_t sweep_rate;

extern sweep_step *SweepSteps; /* Global pointer for the sweep steps */

wheels Wheels[MAX_WHEELS] = {
  /* Pointer to friendly name string, pointer to edge array, RPM Scaler, Number of edges in the array */
  { dizzy_four_cylinder_friendly_name, dizzy_four_cylinder, 0.03333, 4 },
  { dizzy_six_cylinder_friendly_name, dizzy_six_cylinder, 0.05, 6 },
  { dizzy_eight_cylinder_friendly_name, dizzy_eight_cylinder, 0.06667, 8 },
  { sixty_minus_two_friendly_name, sixty_minus_two, 1.0, 120 },
  { sixty_minus_two_with_cam_friendly_name, sixty_minus_two_with_cam, 1.0, 240 },
  { thirty_six_minus_one_friendly_name, thirty_six_minus_one, 0.6, 72 },
  { four_minus_one_with_cam_friendly_name, four_minus_one_with_cam, 0.06667, 16 },
  { eight_minus_one_friendly_name, eight_minus_one, 0.13333, 16 },
  { six_minus_one_with_cam_friendly_name, six_minus_one_with_cam, 0.15, 36 },
  { twelve_minus_one_with_cam_friendly_name, twelve_minus_one_with_cam, 0.6, 144 },
  { fourty_minus_one_friendly_name, fourty_minus_one, 0.66667, 80 },
  { dizzy_four_trigger_return_friendly_name, dizzy_four_trigger_return, 0.075, 9 },
  { oddfire_vr_friendly_name, oddfire_vr, 0.2, 24 },
  { optispark_lt1_friendly_name, optispark_lt1, 3.0, 720 },
  { twelve_minus_three_friendly_name, twelve_minus_three, 0.4, 48 },
  { thirty_six_minus_two_two_two_friendly_name, thirty_six_minus_two_two_two, 0.6, 72 },
  { thirty_six_minus_two_two_two_with_cam_friendly_name, thirty_six_minus_two_two_two_with_cam, 0.6, 144 },
  { fourty_two_hundred_wheel_friendly_name, fourty_two_hundred_wheel, 0.6, 72 },
  { thirty_six_minus_one_with_cam_fe3_friendly_name, thirty_six_minus_one_with_cam_fe3, 0.6, 144 },
  { six_g_seventy_two_with_cam_friendly_name, six_g_seventy_two_with_cam, 0.6, 144 },
  { buell_oddfire_cam_friendly_name, buell_oddfire_cam, 0.33333, 80 },
  { gm_ls1_crank_and_cam_friendly_name, gm_ls1_crank_and_cam, 3.0, 720 },
  { lotus_thirty_six_minus_one_one_one_one_friendly_name, lotus_thirty_six_minus_one_one_one_one, 0.3, 72 },
  { honda_rc51_with_cam_friendly_name, honda_rc51_with_cam, 0.2, 48 },
  { thirty_six_minus_one_with_second_trigger_friendly_name, thirty_six_minus_one_with_second_trigger, 0.6, 144 },
  { thirty_six_minus_one_plus_one_with_cam_ngc4_friendly_name, thirty_six_minus_one_plus_one_with_cam_ngc4, 3.0, 720 },
  { weber_iaw_with_cam_friendly_name, weber_iaw_with_cam, 0.6, 144 },
  { fiat_one_point_eight_sixteen_valve_with_cam_friendly_name, fiat_one_point_eight_sixteen_valve_with_cam, 3.0, 720 },
  { three_sixty_nissan_cas_friendly_name, three_sixty_nissan_cas, 3.0, 720 },
  { twenty_four_minus_two_with_second_trigger_friendly_name, twenty_four_minus_two_with_second_trigger, 0.3, 72 },
  { yamaha_eight_tooth_with_cam_friendly_name, yamaha_eight_tooth_with_cam, 0.26667, 64 },
  { gm_four_tooth_with_cam_friendly_name, gm_four_tooth_with_cam, 0.03333, 8 },
  { gm_six_tooth_with_cam_friendly_name, gm_six_tooth_with_cam, 0.05, 12 },
  { gm_eight_tooth_with_cam_friendly_name, gm_eight_tooth_with_cam, 0.06667, 16 },
  { volvo_d12acd_with_cam_friendly_name, volvo_d12acd_with_cam, 2.0, 480 },
  { mazda_thirty_six_minus_two_two_two_with_six_tooth_cam_friendly_name, mazda_thirty_six_minus_two_two_two_with_six_tooth_cam, 1.5, 360 },
  { sixty_minus_two_with_4X_cam_friendly_name, sixty_minus_two_with_4X_cam, 1.0, 240 },
  { gen4_dodge_srt_v10_sixty_minus_two_with_cam_friendly_name, gen4_dodge_srt_v10_sixty_minus_two_with_cam, 1.0, 240 },
  { twenty_four_minus_one_with_cam_friendly_name, twenty_four_minus_one_with_cam, 0.4, 96 },
  { four_g_sixty_three_with_cam_friendly_name, four_g_sixty_three_with_cam, 3, 720 },
  { seadoo_thirty_six_minus_two_friendly_name, seadoo_thirty_six_minus_two, 0.6, 72 },
  { buick_gm_18x_3x_friendly_name, buick_gm_18x_3x, 0.6, 72 },

};


//! ADC ISR for alternating between ADC pins 0 and 1
/*!
 * Reads ADC ports 0 and 1 alternately. Port 0 is RPM, Port 1 is for
 * future fun (possible crank/cam advance (VVT))
 */
ISR(ADC_vect){
  if (analog_port == 0)
  {
    adc0 = ADCL | (ADCH << 8);
    adc0_read_complete = true;
    /* Flip to channel 1 */
    //ADMUX = B01000000 | 1 ;
    //analog_port = 1;
    return;
  } 
//  if (analog_port == 1)
//  {
//    adc1 = ADCL | (ADCH << 8);
//    adc1_read_complete = true;
//    /* Flip to channel 0 */
//    /* Tell it to read ADC0, clear MUX0..3 */
//    ADMUX = B01000000 | 0 ;
//    analog_port = 0;
//    /* Trigger another conversion */
//    return;
//  }
}


/* This is the "low speed" 1000x/second sweeper interrupt routine
 * who's sole purpose in life is to reset the output compare value
 * for timer zero to change the output RPM.  In cases where the RPM
 * change per ISR is LESS than one LSB of the counter a set of modulus
 * variables are used to handle fractional values.
 */
ISR(TIMER2_COMPA_vect) {
//  PORTD = (1 << 7);
  if ( mode != LINEAR_SWEPT_RPM)
  {
//    PORTD = (0 << 7);
    return;
  }
  /* IF the sweep parameters are being changed, abort the ISR so we
   * don't use half-set values and get things really screwed up
   */
  if (sweep_lock)
  {  
 //   PORTD = (0 << 7);
    return;
  }
  sweep_lock = true; /* Set semaphore */
  /* Check flag to see if we need to reset the prescaler for the timer.
   * if so, clear that flag, set another for the high speed ISR to check for
   * and reprogram the timer when it next runs. Store the last prescaler bits
   * for comparison against during sweep stage changes
   */
  if (sweep_reset_prescaler)
  {
    sweep_reset_prescaler = false;
    reset_prescaler = true;
    prescaler_bits = SweepSteps[sweep_stage].prescaler_bits;
    last_prescaler_bits = prescaler_bits;
  }
  /* Sweep code */
  if (sweep_direction == ASCENDING)
  {
    /* So we don't have to work in floating point (super expensive and slow)
     * we work in a larger scale and keep the remainder per ISR around as 
     * an integer, when that overcomes the threshold we increment the 
     * fractional component and decrement the remainder by that same threshold
     */
    oc_remainder += SweepSteps[sweep_stage].remainder_per_isr;
    while (oc_remainder > FACTOR_THRESHOLD)
    {
      fraction++;
      oc_remainder -= FACTOR_THRESHOLD;
    }
    /* new_OCR1A is the new Output Compare Register (1a) value, it
     * determines how long it is between each tooth interrupt. The longer
     * it is the LOWER the RPM of hte signal will be.  NOTE: this value
     * goes hand in hand with the prescaler, as that determines the divisor
     * for the clock, so you need to take both of them into account
     * tcnt_per_isr is the tooth count CHANGE per ISR (accelerating or 
     * decelerating depending on sweep direction). Since we're in the 
     * ascending side (RPM going up, OCR1A going down) we will be reducing
     * new_OCR1A by tcnt_per_isr + whatever fractional amount until it's 
     * below the ending_ocr value, at that point this stage is completed
     * and we increment the stage.
     */
    if (new_OCR1A > SweepSteps[sweep_stage].ending_ocr)
    {
      new_OCR1A -= (SweepSteps[sweep_stage].tcnt_per_isr + fraction);
      fraction = 0;
    }

    /* Stage endd, increament stage counter, reset remainder to 0 */
    else /* END of the stage, find out where we are */
    {
      sweep_stage++;
      oc_remainder = 0;
    /* Check if there's a next stage by making sure we're not over the end,
     * if so, then reset new_OCR1A to the beginning value from the 
     * structure, check if hte prescaler bits need to change, if they do 
     * we set a flag to do so on the next ISR iteration (1ms later)
     */
      if (sweep_stage < total_sweep_stages)
      {
        /* Toggle  when changing stages */
        //PORTD &= ~(1<<7); /* turn DBG pin off */
        //PORTD |= (1<<7);  /* Turn DBG pin on */
        new_OCR1A = SweepSteps[sweep_stage].beginning_ocr;
        if (SweepSteps[sweep_stage].prescaler_bits != last_prescaler_bits)
          sweep_reset_prescaler = true;
      }
      /* End of sweep stages, reverse direction, decrement sweep_stage by one
       * Set the direction flag to descending, Reset new_OCR1A to the end
       * value (remember opposite direction!), Check prescaler bits and 
       * set flag to reset if necessary
       */
      else /* END of line, time to reverse direction */
      {
        sweep_stage--; /*Bring back within limits */
        sweep_direction = DESCENDING;
        new_OCR1A = SweepSteps[sweep_stage].ending_ocr;
        if (SweepSteps[sweep_stage].prescaler_bits != last_prescaler_bits)
          sweep_reset_prescaler = true;
        //PORTD |= 1 << 7;  /* Debugging, ascending */
      }
      /* Reset fractionals or next round */
    }
  }
  /* Descending RPM, which means new_OCR1A should be CLIMBING,
   * increment the oc_remainder, check if it's over the threshold, if 
   * so increment the fraction and decrement the remainder by the fraction.
   */
  else /* Descending */
  {
    oc_remainder += SweepSteps[sweep_stage].remainder_per_isr;
    while (oc_remainder > FACTOR_THRESHOLD)
    {
      fraction++;
      oc_remainder -= FACTOR_THRESHOLD;
    }
    /* Check if new_OCR1A is less than the sweep stage threshold, if it
     * still is, increase new_OCR1A by the tooth count change per ISR and the
     * fractional component
     */
    if (new_OCR1A < SweepSteps[sweep_stage].beginning_ocr)
    {
      new_OCR1A += (SweepSteps[sweep_stage].tcnt_per_isr + fraction);
      fraction = 0;
    }
    /* new_OCR1A has exceeded the OCR threshold, decrement the sweep stage,
     * reset the remainder to 0 an check to make sure sweep_stage hasn't gone
     * below zero
     */
    else /* End of stage */
    {
      sweep_stage--;
      oc_remainder = 0;
      /* Check that sweep_stage hasn't gone negative, if not, reset 
       * new_OCR1a to the starting value for this stage, check prescaler
       * bits against last ones and set flag if needed
       */
      if (sweep_stage >= 0)
      {
        new_OCR1A = SweepSteps[sweep_stage].ending_ocr;
        if (SweepSteps[sweep_stage].prescaler_bits != last_prescaler_bits)
          sweep_reset_prescaler = true;
      }
      /* Sweep stage went negative, bring it back to zero, flip the direction
       * back to ASCENDING, reset new_OCR1A to starting value for this stage
       * and check prescaler bits and set flag to update if needed
       */
      else /*End of the line */
      {
        sweep_stage++; /*Bring back within limits */
        sweep_direction = ASCENDING;
        new_OCR1A = SweepSteps[sweep_stage].beginning_ocr;
        if (SweepSteps[sweep_stage].prescaler_bits != last_prescaler_bits)
          sweep_reset_prescaler = true;
        PORTD &= ~(1<<7);  /*Descending  turn pin off */
      }
    }
  }
  sweep_lock = false;
//  PORTD = (0 << 7);
}


/* Pumps the pattern out of flash to the port 
 * The rate at which this runs is dependent on what OCR1A is set to
 * the sweeper in timer2 alters this on the fly to alow changing of RPM
 * in a very nice way
 */
ISR(TIMER1_COMPA_vect) {
  /* The tables are in flash so we need pgm_read_byte() */
  /* This is VERY simple, just walk the array and wrap when we hit the limit */
  PORTB = output_invert_mask ^ pgm_read_byte(&Wheels[selected_wheel].edge_states_ptr[edge_counter]);   /* Write it to the port */
  /* Normal direction: overflow handling */
  if (normal)
  {
    edge_counter++;
    if (edge_counter == Wheels[selected_wheel].wheel_max_edges) {
      edge_counter = 0;
    }
  }
  else /* Reverse Rotation: overflow handling */
  {
    if (edge_counter == 0)
      edge_counter = Wheels[selected_wheel].wheel_max_edges;
    edge_counter--;
  }

  /* Reset Prescaler only if flag is set */
  if (reset_prescaler)
  {
    TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12)); /* Clear CS10, CS11 and CS12 */
    TCCR1B |= prescaler_bits;                                                                                               
    reset_prescaler = false;
  }
  /* Reset next compare value for RPM changes */
  OCR1A = new_OCR1A;  /* Apply new "RPM" from Timer2 ISR, i.e. speed up/down the virtual "wheel" */
}
