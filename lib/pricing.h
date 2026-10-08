// Option pricing library: closed forms, a Monte Carlo barrier pricer with
// variance reduction and a Brownian-bridge correction, and a trinomial tree
// with American exercise.
#pragma once

#include <cstdint>

namespace pricing {

struct Market {
    double spot, rate, vol, div = 0.0;
};

// ---- closed forms ---------------------------------------------------------

double norm_cdf(double x);
double bs_call(const Market& m, double strike, double T);
double bs_put(const Market& m, double strike, double T);

// Up-and-out call with a continuously monitored barrier above the strike
// (Reiner and Rubinstein, 1991).
double up_out_call(const Market& m, double strike, double barrier, double T);

// ---- Monte Carlo ----------------------------------------------------------

enum class Monitoring {
    Discrete,        // barrier checked only on the simulation dates
    BrownianBridge,  // also accounts for crossings between dates
};

struct McOptions {
    std::int64_t paths = 100000;
    int steps = 50;
    Monitoring monitoring = Monitoring::Discrete;
    bool antithetic = false;
    bool control_variate = false;  // the vanilla call, whose price is known
    std::uint64_t seed = 42;
};

struct McResult {
    double price, std_error;
};

McResult mc_up_out_call(const Market& m, double strike, double barrier, double T,
                        const McOptions& opt);

// ---- trinomial tree -------------------------------------------------------

enum class Exercise { European, American };

double trinomial(const Market& m, double strike, double T, int steps, bool call,
                 Exercise ex = Exercise::European);

}  // namespace pricing
