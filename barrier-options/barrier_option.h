#ifndef BARRIER_OPTION_H
#define BARRIER_OPTION_H

#include <vector>
#include <random>
using namespace std;

// Function to simulate a single asset path using Geometric Brownian Motion
vector<double> simulateAssetPath(double S0, double r, double sigma, double T, int nSteps, mt19937& rng);

// Function to check if a path violates the barrier condition (Up-and-Out or Up-and-In)
bool checkBarrier(const vector<double>& path, double B, bool isUpAndOut);

// Function to check if a path hits the barrier from below
bool hitsBarrierFromBelow(const vector<double>& path, double B);

// Function for Part A: Simulate and calculate valid paths for Up-and-Out
void partA(double S0, double r, double sigma, double T, double B, int nSteps, int nSimulations);

// Function for Part B: Simulate and calculate probabilities for hitting the barrier
void partB(double S0, double r, double sigma, double T, int nSteps, int nSimulations);

// Function for Part C
void partC(double S0, double r, double sigma, double T, double K, double B, int nSteps);
#endif
