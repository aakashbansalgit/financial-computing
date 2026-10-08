#include "trinomial.h"
#include <vector>
#include <iostream>

// Up factor and branch probabilities for the pricing lattice.
void latticeParams(double r, double sigma, double dt,
                   double& u, double& pu, double& pm, double& pd) {
    double nu = r - 0.5 * sigma * sigma;
    double dx = sigma * sqrt(3.0 * dt);
    double a = (sigma * sigma * dt + nu * nu * dt * dt) / (dx * dx);
    u  = exp(dx);
    pu = 0.5 * (a + nu * dt / dx);
    pd = 0.5 * (a - nu * dt / dx);
    pm = 1.0 - a;
}

// Function to calculate trinomial probabilities
void calculateTrinomialProbabilities(double r, double sigma, double dt, double lambda) {
    // Trinomial probabilities
    double pu = 1 / (2 * lambda * lambda) + (r / (2 * sigma)) * sqrt(dt / lambda);
    double pd = 1 / (2 * lambda * lambda) - (r / (2 * sigma)) * sqrt(dt / lambda);
    double pm = 1.0 - pu - pd; // Probability of staying the same

    cout << "Trinomial Model Probabilities (lambda = " << lambda << "):" << endl;
    cout << "p_u (up-tick)   = " << pu << endl;
    cout << "p_d (down-tick) = " << pd << endl;
    cout << "p_m (stay)      = " << pm << endl;
}

// Function to calculate binomial probabilities
void calculateBinomialProbabilities(double r, double sigma, double dt) {
    // Binomial probabilities
    double p_u = 0.5 * (1 + (r / sigma) * sqrt(dt));
    double p_d = 1 - p_u; // Down-tick probability

    cout << "Binomial Model Probabilities:" << endl;
    cout << "p_u (up-tick)   = " << p_u << endl;
    cout << "p_d (down-tick) = " << p_d << endl;
    cout << "Note: p_m (stay) does not exist in the binomial model.\n" << endl;
}

// Function for part (a): Verifying trinomial vs binomial model when lambda = 1
void partA() {
    cout << "=== Part (a): Verifying when lambda = 1 ===" << endl;

    // Model parameters
    double r = 0.05;       // Risk-free rate
    double sigma = 0.2;    // Volatility
    double T = 1.0;        // Time to maturity
    int N = 10;             // Number of time steps
    double dt = T / N;     // Time step size
    double lambda = 1.0;   // Lambda value for trinomial model

    // Calculate and display trinomial probabilities
    calculateTrinomialProbabilities(r, sigma, dt, lambda);

    // Calculate and display binomial probabilities
    calculateBinomialProbabilities(r, sigma, dt);

    cout << "When lambda = 1, the trinomial model probabilities for up-tick and down-tick "
        << "should match those of the binomial model, and the stay probability (p_m) should be 0.\n" << endl;
}


// Function for part (b): Trinomial Option Pricing (Iterative version)
void partB() {
    cout << "=== Part (b): Trinomial Option Pricing ===" << endl;

    // Model parameters
    double S0 = 100.0;  // Initial stock price
    double K = 100.0;   // Strike price
    double T = 1.0;     // Time to maturity (in years)
    double r = 0.05;    // Risk-free rate
    double sigma = 0.2; // Volatility
    int N = 100;        // Number of steps
    double dt = T / N;  // Time step size

    // Trinomial lattice parameters (log-space, dx = sigma * sqrt(3 dt)).
    // The probabilities match the first two moments of ln(S) over one step.
    // An earlier version used the binomial probabilities here, which set pm
    // to zero and doubled the variance per step.
    double u, pu, pm, pd;
    latticeParams(r, sigma, dt, u, pu, pm, pd);
    double discount = exp(-r * dt);

    // Lattice for call and put prices
    vector<vector<double>> callPrice(N + 1, vector<double>(2 * N + 1, 0.0));
    vector<vector<double>> putPrice(N + 1, vector<double>(2 * N + 1, 0.0));

    // Terminal condition for call and put options
    for (int j = -N; j <= N; j++) {
        double ST = S0 * pow(u, j); // Stock price at terminal node
        callPrice[N][j + N] = max(0.0, ST - K); // Call option payoff
        putPrice[N][j + N] = max(0.0, K - ST);  // Put option payoff
    }

    // Backward induction to calculate option prices
    for (int t = N - 1; t >= 0; t--) {
        for (int j = -t; j <= t; j++) {
            callPrice[t][j + N] = discount * (pu * callPrice[t + 1][j + 1 + N] +
                pm * callPrice[t + 1][j + N] +
                pd * callPrice[t + 1][j - 1 + N]);

            putPrice[t][j + N] = discount * (pu * putPrice[t + 1][j + 1 + N] +
                pm * putPrice[t + 1][j + N] +
                pd * putPrice[t + 1][j - 1 + N]);
        }
    }

    // Output option prices at the root node
    cout << "European Call Option Price (Trinomial Lattice): " << callPrice[0][N] << endl;
    cout << "European Put Option Price (Trinomial Lattice): " << putPrice[0][N] << "\n" << endl;
}


// Function for part (c): Trinomial Option Pricing (Memoized version)
void partC() {
    cout << "=== Part (c): Memoized Trinomial Option Pricing ===" << endl;

    // Model parameters
    double S0 = 100.0;  // Initial stock price
    double K = 100.0;   // Strike price
    double T = 1.0;     // Time to maturity (in years)
    double r = 0.05;    // Risk-free rate
    double sigma = 0.2; // Volatility
    int N = 100;        // Number of steps
    bool isCall = true; // Option type: true for call, false for put

    // Call the trinomial option pricing function (memoized version)
    double callOptionPrice = TrinomialOptionMemoized(S0, K, T, r, sigma, N, isCall);
    double putOptionPrice = TrinomialOptionMemoized(S0, K, T, r, sigma, N, !isCall);

    cout << "European Call Option Price (Memoized Trinomial Lattice): " << callOptionPrice << endl;
    cout << "European Put Option Price (Memoized Trinomial Lattice): " << putOptionPrice << "\n" << endl;
}

// Recursive function to compute option value with memoization
double OptionValue(int t, int j, int N, double S0, double K, double T, double r, double sigma, double u, double d, double pu, double pd, double pm, bool isCall, unordered_map<Key, double, KeyHasher>& memo) {
    if (t == N) {
        double ST = S0 * pow(u, j);
        return isCall ? max(0.0, ST - K) : max(0.0, K - ST);
    }

    Key key = { t, j };
    if (memo.find(key) != memo.end()) {
        return memo[key];
    }

    double dt = T / N;
    double discount = exp(-r * dt);

    double value = discount * (pu * OptionValue(t + 1, j + 1, N, S0, K, T, r, sigma, u, d, pu, pd, pm, isCall, memo) +
        pm * OptionValue(t + 1, j, N, S0, K, T, r, sigma, u, d, pu, pd, pm, isCall, memo) +
        pd * OptionValue(t + 1, j - 1, N, S0, K, T, r, sigma, u, d, pu, pd, pm, isCall, memo));

    memo[key] = value;
    return value;
}

// Main function for trinomial option pricing using memoization
double TrinomialOptionMemoized(double S0, double K, double T, double r, double sigma, int N, bool isCall) {
    double dt = T / N;
    double u, pu, pm, pd;
    latticeParams(r, sigma, dt, u, pu, pm, pd);
    double d = 1 / u;

    unordered_map<Key, double, KeyHasher> memo;

    return OptionValue(0, 0, N, S0, K, T, r, sigma, u, d, pu, pd, pm, isCall, memo);
}