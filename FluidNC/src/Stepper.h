// Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
// Copyright (c) 2009-2011 Simen Svale Skogsrud
// Copyright (c) 2018 -	Bart Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

/*
  Stepper.h - stepper motor driver: executes motion plans of planner.c using the stepper motors
*/

#include "EnumItem.h"

#include <cstdint>

namespace Stepper {
    void pulse_func();

    // Enable steppers, but cycle does not start unless called by motion control or realtime command.
    void wake_up();

    // Stops stepping
    void go_idle();

    // Reset the stepper subsystem variables
    void reset();

    // Changes the run state of the step segment buffer to execute the special parking motion.
    void parking_setup_buffer();

    // Restores the step segment buffer to the normal run state after a parking motion.
    void parking_restore_buffer();

    // Reloads step segment buffer. Called continuously by realtime execution system.
    void prep_buffer();

    // Called by planner_recalculate() when the executing block is updated by the new plan.
    void update_plan_block_parameters();

    // Called by realtime status reporting if realtime rate reporting is enabled in config.h.
    float get_realtime_rate();

    extern uint32_t isr_count;  // for debugging only
}
#ifdef DEBUG_STEPPING
extern uint32_t st_seq;
extern uint32_t st_seq0;
extern uint32_t pl_seq0;
extern uint32_t seg_seq0;
extern uint32_t seg_seq1;
extern uint32_t seg_seq_act;
extern uint32_t seg_seq_exp;

#endif
