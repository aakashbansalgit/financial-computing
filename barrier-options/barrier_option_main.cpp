#include "barrier_option.h"
#include <iostream>

int main() {
    // Shared model parameters
    double S0 = 1.0;       // Initial price (given in question)
    double r = 0.08;       // Expected return (risk-free rate, 8%)
    double sigma = 0.3;    // Volatility (30%)
    double T = 5.0;        // Time to maturity (5 years)
    int nSteps = 30;       // Number of time steps (given as an example)
    double K = 1.0;        // Strike price (assume same as initial price for simplicity)
    double B = 1.2;        // Barrier level (assume 20% above S0 as a reasonable choice)
    int nSimulations = 100000; // Number of simulations for Part A and Part B


    // Run Part A
    partA(S0, r, sigma, T, B, nSteps, nSimulations);

    // Run Part B
    partB(S0, r, sigma, T, nSteps, nSimulations);

    // Run Part C
    partC(S0, r, sigma, T, K, B, nSteps);

    return 0;
}
