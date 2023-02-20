// Copyright (c) 2016 Sungeun K. Jeon for Gnea Research LLC
// Copyright (c) 2018 -	Bart Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "JogSpeed.h"

#include "Machine/MachineConfig.h"
#include "MotionControl.h"  // mc_linear
#include "Stepper.h"        // st_prep_buffer, st_wake_up
#include "Limits.h"         // constrainToSoftLimits()
#include "Protocol.h"
#include "Planner.h"

static bool jog_speed_do_move(float left_speed, float right_speed);
static float current_left_speed  = 0.0f;
static float current_right_speed = 0.0f;

    // Sets up valid jog motion received from g-code parser, checks for soft-limits, and executes the jog.
// cancelledInflight will be set to true if was not added to parser due to a cancelJog.
Error jog_speed_execute( parser_block_t* gc_block ) {
    float left_speed = gc_block->values.xyz[X_AXIS];
    float right_speed = gc_block->values.xyz[Y_AXIS];
    return jog_speed_execute( left_speed, right_speed);
}

Error jog_speed_execute(float left_speed, float right_speed) {
    log_info(" jog_speed_execute ");

    if (rtMotionCancel)
        return Error::JogCancelled;

    if (sys.state == State::Idle) {
        log_info(" START MOVE ");
        if (!jog_speed_do_move(left_speed, right_speed))
            return Error::JogCancelled;
        if (plan_get_current_block() != NULL) {  // Check if there is a block to execute.
            sys.state = State::JogSpeed;
            Stepper::prep_buffer();
            Stepper::wake_up();  // NOTE: Manual start. No state machine required.
        }
    } else if (sys.state == State::JogSpeed ) {
        //report_realtime_status(Uart0);
        log_info(" UPDATE SPEED Blocks" << plan_get_block_buffer_available());
        if (plan_get_block_buffer_available() < (BLOCK_BUFFER_SIZE - 2))//       MAX_BLOCK_BUSY
            return Error::Ok;
        log_info(" UPDATE SPEED YES ");
        if (!jog_speed_do_move(left_speed, right_speed))
            return Error::JogCancelled;
    } else {
        const State state = sys.state;
        auto        it    = StateName.find(state);
        const char* name  = it == StateName.end() ? "<invalid>" : it->second;
        log_info(" UPDATE SPEED NO " << name);
    }
    return Error::Ok;
}
Error jog_speed_next() {
    if (sys.state == State::JogSpeed) {
        log_info(" AUTO UPDATE SPEED Blocks" << plan_get_block_buffer_available() );
        if (plan_get_block_buffer_available() < (BLOCK_BUFFER_SIZE - (MAX_BLOCK_BUSY-1)))
            return Error::Ok;
        log_info(" AUTO UPDATE SPEED YES  ");
        if (!jog_speed_do_move(current_left_speed, current_right_speed))
            return Error::JogCancelled;
    }
    return Error::Ok;
}
 
Error jog_speed_stop() {
    if (sys.state != State::JogSpeed) {
        log_info("JOG SPEED STOP NO ");
        return Error::Ok;
    }
    if (sys.suspend.bit.jogCancel)
        return Error::Ok;

    rtMotionCancel = true;
    log_info(" JOG SPEED STOP STOPPED ");
    protocol_execute_realtime();
    return Error::Ok;
}

static bool jog_speed_do_move(float left_speed, float right_speed) {
    plan_line_data_t  plan_data;
    plan_line_data_t* pl_data = &plan_data;
    memset(pl_data, 0, sizeof(plan_line_data_t));  // Zero pl_data struct

    float xyz[MAX_N_AXIS];

    //1 sec path
    memset(xyz, 0, sizeof(float) * MAX_N_AXIS);
    pl_data->feed_rate = float(std::max(fabs(left_speed), fabs(right_speed)));

    float dist_left  = left_speed * time_for_line / 60.0f;
    float dist_right = right_speed * time_for_line / 60.0f;

    gc_sync_position();

    xyz[X_AXIS] = gc_state.position[X_AXIS] + dist_left;
    xyz[Y_AXIS] = gc_state.position[Y_AXIS] + dist_right;

    pl_data->motion.noFeedOverride = 0;  // WAS 1
    pl_data->is_jog                = true;
    // Valid jog command. Plan, set state, and execute.
    if (!mc_move_motors(xyz, pl_data))
        return false;
    memcpy(gc_state.position, xyz, sizeof(xyz));
    current_left_speed  = left_speed;
    current_right_speed = right_speed;

    String str;
    str += "Speed left: ";
    str += left_speed;
    str += " Speed right: ";
    str += right_speed;
    str += " dist_left: ";
    str += dist_left;
    str += " dist_right: ";
    str += dist_right;
    str += " xPos: ";
    str += gc_state.position[X_AXIS];
    log_info(str.c_str());
    return true;
}

