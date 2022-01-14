#include "CartesianWithRotary.h"
#include "../Machine/MachineConfig.h"
#include <cmath>

namespace Kinematics {
    void CartesianWithRotary::group(Configuration::HandlerBase& handler) {}

    void CartesianWithRotary::init() { log_info("Kinematic system: " << name()); }

    /*
      cartesian_to_motors() converts from cartesian coordinates to motor space.

      All linear motions pass through cartesian_to_motors() to be planned as mc_move_motors operations.

      Parameters:
        target = an MAX_N_AXIS array of target positions (where the move is supposed to go)
        pl_data = planner data (see the definition of this type to see what it is)
        position = an MAX_N_AXIS array of where the machine is starting from for this move
    */
    static float normlize_rotary_pos(const float& pos) {
        if (pos > 360 || pos < -360) {
            return pos - floor(pos / 360) * 360;
        }
        return pos;
    }

    bool CartesianWithRotary::cartesian_to_motors(float* target, plan_line_data_t* pl_data, float* position) {
        auto n_axis = config->_axes->_numberAxis;
        if (n_axis <= 3)
            return mc_move_motors(target, pl_data);
        float normolized_pos[MAX_N_AXIS];
        float new_pos, prev_pos, delta;
        memcpy(normolized_pos, target, sizeof(float) * n_axis);
        for (int i = 3; i < n_axis; i++) {
            new_pos  = normlize_rotary_pos(target[i]);
            prev_pos = normlize_rotary_pos(position[i]);
            delta    = normlize_rotary_pos(new_pos - prev_pos);
            if (delta > 180)
                delta = delta - 360;
            else if (delta < -180)
                delta = 360 + delta;
            normolized_pos[i] = prev_pos + delta;
        }
        return mc_move_motors(normolized_pos, pl_data);
    }

    // Configuration registration
    namespace {
        KinematicsFactory::InstanceBuilder<CartesianWithRotary> registration("CartesianWithRotary");
    }
}
