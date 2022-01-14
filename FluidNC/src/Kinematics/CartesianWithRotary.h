#pragma once
#include "Cartesian.h"

namespace Kinematics {
    class CartesianWithRotary : public Cartesian {
    public:
        CartesianWithRotary()                           = default;
        CartesianWithRotary(const CartesianWithRotary&) = delete;
        CartesianWithRotary(CartesianWithRotary&&)      = delete;
        ~CartesianWithRotary() {}

        CartesianWithRotary& operator=(const CartesianWithRotary&) = delete;
        CartesianWithRotary& operator=(CartesianWithRotary&&) = delete;

        // Kinematic Interface
        void init() override;
        bool cartesian_to_motors(float* target, plan_line_data_t* pl_data, float* position) override;

        // Configuration handlers:
        void group(Configuration::HandlerBase& handler) override;

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override { return "CartesianWithRotary"; }
    };
}  //  namespace Kinematics
