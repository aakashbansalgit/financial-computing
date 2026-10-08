// Minimal checks; exits non-zero on the first failure.
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "../lib/pricing.h"

using namespace pricing;

static void check(bool ok, const char* what) {
    std::printf("%s  %s\n", ok ? "pass" : "FAIL", what);
    if (!ok) std::exit(1);
}

int main() {
    const Market m{100.0, 0.05, 0.20, 0.01};
    const double K = 100.0, T = 1.0;

    check(std::fabs(bs_call(m, K, T) - bs_put(m, K, T)
                    - (m.spot * std::exp(-m.div * T) - K * std::exp(-m.rate * T))) < 1e-12,
          "put-call parity");
    check(std::fabs(trinomial(m, K, T, 1600, true) - bs_call(m, K, T)) < 1.5e-3,
          "trinomial European call, 1,600 steps, within 0.0015 of Black-Scholes");
    check(std::fabs(trinomial(m, K, T, 1600, false) - bs_put(m, K, T)) < 1.5e-3,
          "trinomial European put, 1,600 steps, within 0.0015 of Black-Scholes");
    check(trinomial(m, K, T, 800, false, Exercise::American) > bs_put(m, K, T) + 0.05,
          "American put worth more than European");

    const Market n{100.0, 0.05, 0.20};
    const double H = 130.0, exact = up_out_call(n, K, H, T);
    check(up_out_call(n, K, 90.0, T) == 0.0, "up-and-out call with barrier below strike is worthless");
    check(up_out_call(n, K, 1e6, T) - bs_call(n, K, T) < 1e-9 &&
              bs_call(n, K, T) - up_out_call(n, K, 1e6, T) < 1e-6,
          "a barrier far away gives the vanilla price");

    McOptions o;
    o.paths = 100000;
    o.steps = 50;
    o.monitoring = Monitoring::BrownianBridge;
    o.control_variate = true;
    McResult r = mc_up_out_call(n, K, H, T, o);
    check(std::fabs(r.price - exact) < 4 * r.std_error, "bridge Monte Carlo within 4 s.e. of the closed form");

    o.monitoring = Monitoring::Discrete;
    McResult d = mc_up_out_call(n, K, H, T, o);
    check(d.price - exact > 4 * d.std_error, "discrete monitoring overprices a continuous barrier");
    return 0;
}
