// Experiments for the README. Writes CSV files to the directory given as the
// first argument (default: results).
//
//   build/study results

#include <chrono>
#include <cmath>
#include <cstdio>
#include <initializer_list>
#include <string>

#include "../lib/pricing.h"

using namespace pricing;

int main(int argc, char** argv) {
    std::string out = argc > 1 ? argv[1] : "results";
    const Market m{100.0, 0.05, 0.20};
    const double K = 100.0, H = 130.0, T = 1.0;
    const double exact = up_out_call(m, K, H, T);

    // 1. Discrete monitoring bias, and the Brownian bridge that removes it.
    {
        FILE* f = std::fopen((out + "/barrier_monitoring.csv").c_str(), "w");
        std::fprintf(f, "steps,exact,discrete,discrete_se,bridge,bridge_se\n");
        for (int steps : {12, 50, 250, 1000}) {
            McOptions o;
            o.paths = 200000;
            o.steps = steps;
            o.control_variate = true;
            McResult d = mc_up_out_call(m, K, H, T, o);
            o.monitoring = Monitoring::BrownianBridge;
            McResult b = mc_up_out_call(m, K, H, T, o);
            std::fprintf(f, "%d,%.5f,%.5f,%.5f,%.5f,%.5f\n", steps, exact, d.price, d.std_error, b.price, b.std_error);
            std::printf("steps %4d  exact %.4f  discrete %.4f (%.4f)  bridge %.4f (%.4f)\n",
                        steps, exact, d.price, d.std_error, b.price, b.std_error);
        }
        std::fclose(f);
    }

    // 2. Variance reduction at a fixed budget of paths.
    {
        FILE* f = std::fopen((out + "/variance_reduction.csv").c_str(), "w");
        std::fprintf(f, "method,price,std_error,variance_ratio,seconds\n");
        double base_var = 0.0;
        struct Case { const char* name; bool anti, cv; };
        for (Case c : {Case{"plain", false, false}, Case{"antithetic", true, false},
                       Case{"control variate", false, true}, Case{"both", true, true}}) {
            McOptions o;
            o.paths = 100000;
            o.steps = 50;
            o.monitoring = Monitoring::BrownianBridge;
            o.antithetic = c.anti;
            o.control_variate = c.cv;
            auto t0 = std::chrono::steady_clock::now();
            McResult r = mc_up_out_call(m, K, H, T, o);
            double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
            double var = r.std_error * r.std_error;
            if (base_var == 0.0) base_var = var;
            std::fprintf(f, "%s,%.5f,%.6f,%.2f,%.3f\n", c.name, r.price, r.std_error, base_var / var, secs);
            std::printf("%-16s %.4f  se %.5f  variance cut %.1fx  %.2fs\n", c.name, r.price, r.std_error, base_var / var, secs);
        }
        std::fclose(f);
    }

    // 3. Trinomial tree convergence, European and American.
    {
        FILE* f = std::fopen((out + "/trinomial_convergence.csv").c_str(), "w");
        std::fprintf(f, "steps,call_error,put_error,american_put,american_put_error,"
                        "richardson_call_error,richardson_american_put_error\n");
        const double c = bs_call(m, K, T), p = bs_put(m, K, T);
        const double am_ref = trinomial(m, K, T, 20000, false, Exercise::American);
        for (int n : {25, 50, 100, 200, 400, 800, 1600}) {
            double call_n = trinomial(m, K, T, n, true);
            double ce = call_n - c;
            double pe = trinomial(m, K, T, n, false) - p;
            double am = trinomial(m, K, T, n, false, Exercise::American);
            // The error is close to a constant times 1/n, so 2 V(2n) - V(n)
            // cancels the leading term (Richardson extrapolation).
            double rc = 2 * trinomial(m, K, T, 2 * n, true) - call_n - c;
            double ra = 2 * trinomial(m, K, T, 2 * n, false, Exercise::American) - am - am_ref;
            std::fprintf(f, "%d,%.6e,%.6e,%.6f,%.6e,%.6e,%.6e\n", n, ce, pe, am, am - am_ref, rc, ra);
        }
        std::printf("american put reference (20,000 steps) %.6f, european put %.6f\n", am_ref, p);
        std::fclose(f);
    }
    return 0;
}
