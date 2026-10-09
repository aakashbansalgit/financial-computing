#include "barrier_option.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>

using namespace std;

// Function to simulate a single asset path
vector<double> simulateAssetPath(double S0, double r, double sigma, double T, int nSteps, mt19937& rng) {
    vector<double> path(nSteps + 1);
    path[0] = S0; // Initial price

    double dt = T / nSteps;
    normal_distribution<double> dist(0.0, 1.0); // Standard normal distribution

    for (int i = 1; i <= nSteps; ++i) {
        double Z = dist(rng); // Random draw
        path[i] = path[i - 1] * exp((r - 0.5 * sigma * sigma) * dt + sigma * sqrt(dt) * Z);
    }

    return path;
}

// Function to check if a path violates the barrier condition
bool checkBarrier(const vector<double>& path, double B, bool isUpAndOut) {
    bool barrierBreached = false;

    for (double price : path) {
        if (price >= B) {
            barrierBreached = true;
            break;
        }
    }

    if (isUpAndOut) {
        return !barrierBreached; // Up-and-Out: Option is invalid if the barrier is breached
    }
    else {
        return barrierBreached; // Up-and-In: Option is valid if the barrier is breached
    }
}

// Function to check if a path hits the barrier from below
bool hitsBarrierFromBelow(const vector<double>& path, double B) {
    for (double price : path) {
        if (price >= B) {
            return true; // Barrier is hit
        }
    }
    return false;
}

// Function for Part A: Simulate and calculate valid paths for Up-and-Out
void partA(double S0, double r, double sigma, double T, double B, int nSteps, int nSimulations) {
    cout << "=== Part (a): Up-and-Out Barrier Option Simulation ===" << endl;

    // Random number generator
    random_device rd;
    mt19937 rng(rd());

    // Simulate paths and count valid ones
    int validPaths = 0;
    for (int i = 0; i < nSimulations; ++i) {
        vector<double> path = simulateAssetPath(S0, r, sigma, T, nSteps, rng);
        if (checkBarrier(path, B, true)) {
            ++validPaths;
        }
    }

    // Calculate proportion of valid paths
    double proportionValid = static_cast<double>(validPaths) / nSimulations;
    cout << "Barrier Level: " << B << endl;
    cout << "Proportion of valid paths (Up-and-Out): " << proportionValid << "\n" << endl;
}

// Function for Part B: Simulate and calculate probabilities for hitting the barrier
void partB(double S0, double r, double sigma, double T, int nSteps, int nSimulations) {
    cout << "=== Part (b): Probability of Hitting Barrier from Below ===" << endl;

    // Random number generator
    random_device rd;
    mt19937 rng(rd());

    // Barrier levels: From 1.5 * S0 to 6.0 * S0
    vector<double> barriers;
    for (double factor = 1.5; factor <= 6.0; factor += 0.5) {
        barriers.push_back(factor * S0);
    }

    // Simulate paths and estimate probabilities for each barrier level
    cout << "Barrier Level\tProbability of Hitting Barrier" << endl;
    for (double B : barriers) {
        int hitCount = 0;
        for (int i = 0; i < nSimulations; ++i) {
            vector<double> path = simulateAssetPath(S0, r, sigma, T, nSteps, rng);
            if (hitsBarrierFromBelow(path, B)) {
                ++hitCount;
            }
        }

        // Calculate probability
        double probability = static_cast<double>(hitCount) / nSimulations;
        cout << B << "\t\t" << probability << endl;
    }
}


// Function for Part C: Price barrier call options and write results to files
void partC(double S0, double r, double sigma, double T, double K, double B, int nSteps) {
    cout << "=== Part (c): Continuous Barrier Call Option Pricing ===" << endl;

    // Random number generator
    random_device rd;
    mt19937 rng(rd());

    // File streams for output
    ofstream upAndOutFile("up_and_out.txt");
    ofstream upAndInFile("up_and_in.txt");

    // Simulate for different numbers of simulations
    for (int nSimulations = 10; nSimulations <= 1000; nSimulations += 10) {
        double upAndOutPayoff = 0.0;
        double upAndInPayoff = 0.0;

        // Simulate paths
        for (int i = 0; i < nSimulations; ++i) {
            vector<double> path = simulateAssetPath(S0, r, sigma, T, nSteps, rng);
            double ST = path.back(); // Terminal price

            if (checkBarrier(path, B, true)) { // Up-and-Out
                upAndOutPayoff += max(ST - K, 0.0);
            }
            if (checkBarrier(path, B, false)) { // Up-and-In
                upAndInPayoff += max(ST - K, 0.0);
            }
        }

        // Calculate average payoffs
        upAndOutPayoff /= nSimulations;
        upAndInPayoff /= nSimulations;

        // Discount back to present value
        double discountFactor = exp(-r * T);
        double upAndOutPrice = upAndOutPayoff * discountFactor;
        double upAndInPrice = upAndInPayoff * discountFactor;

        // Write results to files
        upAndOutFile << nSimulations << " " << upAndOutPrice << endl;
        upAndInFile << nSimulations << " " << upAndInPrice << endl;

        // Print results for verification
        cout << "Simulations: " << nSimulations
            << " | Up-and-Out Price: " << upAndOutPrice
            << " | Up-and-In Price: " << upAndInPrice << endl;
    }

    // Close files
    upAndOutFile.close();
    upAndInFile.close();

    cout << "Results written to 'up_and_out.txt' and 'up_and_in.txt'." << endl;
}
