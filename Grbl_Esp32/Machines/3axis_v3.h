/*
    3axis_v3.h
    Part of Grbl_ESP32

    Pin assignments for the ESP32 Development Controller, v3.5.
    https://github.com/bdring/Grbl_ESP32_Development_Controller
    https://www.tindie.com/products/33366583/grbl_esp32-cnc-development-board-v35/

    2018    - Bart Dring
    2020    - Mitch Bradley

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MACHINE_NAME            "ESP32_V3.5"

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define Y_STEP_PIN              GPIO_NUM_14
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Z_STEP_PIN              GPIO_NUM_27
#define Z_DIRECTION_PIN         GPIO_NUM_33

#define LIMIT_MASK              B111
#define X_LIMIT_PIN             GPIO_NUM_2  // labeled X Limit
#define Y_LIMIT_PIN             GPIO_NUM_4  // labeled Y Limit
#define Z_LIMIT_PIN             GPIO_NUM_15 // labeled Z Limit

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_13

#define SPINDLE_TYPE            SPINDLE_TYPE_PWM
#define SPINDLE_PWM_PIN         GPIO_NUM_17  // labeled SpinPWM
#define SPINDLE_ENABLE_PIN      GPIO_NUM_22  // labeled SpinEnbl

#define COOLANT_MIST_PIN        GPIO_NUM_21  // labeled Mist
#define COOLANT_FLOOD_PIN       GPIO_NUM_16  // labeled Flood
#define PROBE_PIN               GPIO_NUM_32  // labeled Probe

#define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_35  // labeled Door,  needs external pullup
#define CONTROL_RESET_PIN       GPIO_NUM_34  // labeled Reset, needs external pullup
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_36  // labeled Hold,  needs external pullup
#define CONTROL_CYCLE_START_PIN GPIO_NUM_39  // labeled Start, needs external pullup
