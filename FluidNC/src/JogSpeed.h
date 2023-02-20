// Copyright (c) 2016 Sungeun K. Jeon for Gnea Research LLC
// Copyright (c) 2018 -	Bart Dring
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#pragma once

#include "Error.h"
#include "Planner.h"
#include "GCode.h"

static const int   MAX_BLOCK_BUSY = 3;
static const float time_for_line  = 0.20f;  // in sec

// Sets up valid jog motion received from g-code parser, checks for soft-limits, and executes the jog.
// cancelledInflight will be set to true if was not added to parser due to a cancelJog.
Error jog_speed_execute(parser_block_t* gc_block );
Error jog_speed_execute(float left_speed, float right_speed);
Error jog_speed_next();
Error jog_speed_stop();
