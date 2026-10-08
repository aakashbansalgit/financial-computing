#ifndef trinomial_h
#define trinomial_h

#include <unordered_map>
#include <cmath>
#include <algorithm>
using namespace std;

// Key structure for memoization
struct Key {
    int t, j;
    bool operator==(const Key& other) const {
        return t == other.t && j == other.j;
    }
};

// Hash function for Key
struct KeyHasher {
    size_t operator()(const Key& k) const {
        return hash<int>()(k.t) ^ hash<int>()(k.j);
    }
};

// Function declarations
void latticeParams(double r, double sigma, double dt,
                   double& u, double& pu, double& pm, double& pd);
void calculateTrinomialProbabilities(double r, double sigma, double dt, double lambda);
void calculateBinomialProbabilities(double r, double sigma, double dt);
void partA();
void partB();
void partC();
double TrinomialOptionMemoized(double S0, double K, double T, double r, double sigma, int N, bool isCall);

#endif
