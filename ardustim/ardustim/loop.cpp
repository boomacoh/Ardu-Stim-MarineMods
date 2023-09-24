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

#include <SerialUI.h>
#include "defines.h"
#include "loop.h"
#include "sweep.h"

extern SUI::SerialUI mySUI;

void loop() {
  //uint16_t tmp_rpm = 0;
  //extern volatile bool adc0_read_complete;
  //extern volatile uint16_t adc0;
  /* Just handle the Serial UI, everything else is in 
   * interrupt handlers or callbacks from SerialUI.
   */

  if (mySUI.checkForUserOnce())
  {
    // Someone connected!
    mySUI.enter();
    while (mySUI.userPresent()) 
    {
      mySUI.handleRequests();
    }
  }
/*  if (adc0_read_complete == true)
  {
    adc0_read_complete = false;
    tmp_rpm = adc0 << TMP_RPM_SHIFT;
    if (tmp_rpm > TMP_RPM_CAP)
      tmp_rpm = TMP_RPM_CAP;
    reset_new_OCR1A(tmp_rpm);
  }
  */
}

