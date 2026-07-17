#pragma once

#include <vector>
#include <functional>

namespace SZM {
namespace Math {

/**
 * @brief Open-source inspired Numerical Solvers for Ordinary Differential Equations (ODEs)
 * These solvers are foundational for real physics (mechanics, particle systems, orbital dynamics).
 */
class ODESolver {
public:
    /**
     * @brief 4th-Order Runge-Kutta (RK4) integration method.
     * Widely used in open-source physics simulations for its balance of stability and accuracy.
     * 
     * @tparam StateType The type of the state (e.g., double, std::vector<double>, Eigen::VectorXd).
     *                   Must support +, *, and scalar multiplication.
     * @param currentState The current state variables (e.g., position, velocity).
     * @param t The current time.
     * @param dt The time step size.
     * @param derivativeFunc A lambda or std::function returning the derivative (rate of change) 
     *                       at a given state and time: f(state, time).
     * @return StateType The new state after time step dt.
     */
    template <typename StateType>
    static StateType RK4(const StateType& currentState, 
                         double t, 
                         double dt, 
                         const std::function<StateType(const StateType&, double)>& derivativeFunc) 
    {
        StateType k1 = derivativeFunc(currentState, t);
        StateType k2 = derivativeFunc(currentState + k1 * (0.5 * dt), t + 0.5 * dt);
        StateType k3 = derivativeFunc(currentState + k2 * (0.5 * dt), t + 0.5 * dt);
        StateType k4 = derivativeFunc(currentState + k3 * dt, t + dt);

        return currentState + (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (dt / 6.0);
    }

    /**
     * @brief Forward Euler integration method.
     * Faster but less stable than RK4. Useful for soft-body physics or systems with 
     * very small time steps where performance is critical.
     * 
     * @tparam StateType The type of the state.
     * @param currentState The current state variables.
     * @param t The current time.
     * @param dt The time step size.
     * @param derivativeFunc The derivative function f(state, time).
     * @return StateType The new state.
     */
    template <typename StateType>
    static StateType Euler(const StateType& currentState, 
                           double t, 
                           double dt, 
                           const std::function<StateType(const StateType&, double)>& derivativeFunc) 
    {
        return currentState + derivativeFunc(currentState, t) * dt;
    }
};

} // namespace Math
} // namespace SZM
