#include "pricing.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace pricing {

double norm_cdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }

double bs_call(const Market& m, double K, double T) {
    double s = m.vol * std::sqrt(T);
    double d1 = (std::log(m.spot / K) + (m.rate - m.div + 0.5 * m.vol * m.vol) * T) / s;
    return m.spot * std::exp(-m.div * T) * norm_cdf(d1) - K * std::exp(-m.rate * T) * norm_cdf(d1 - s);
}

double bs_put(const Market& m, double K, double T) {
    // put-call parity
    return bs_call(m, K, T) - m.spot * std::exp(-m.div * T) + K * std::exp(-m.rate * T);
}

double up_out_call(const Market& m, double K, double H, double T) {
    if (m.spot >= H) return 0.0;
    if (H <= K) return 0.0;
    double st = m.vol * std::sqrt(T);
    double lam = (m.rate - m.div + 0.5 * m.vol * m.vol) / (m.vol * m.vol);
    double x1 = std::log(m.spot / H) / st + lam * st;
    double y = std::log(H * H / (m.spot * K)) / st + lam * st;
    double y1 = std::log(H / m.spot) / st + lam * st;
    double a = m.spot * std::exp(-m.div * T), b = K * std::exp(-m.rate * T);
    double p2l = std::pow(H / m.spot, 2 * lam), p2l2 = std::pow(H / m.spot, 2 * lam - 2);
    double up_in = a * norm_cdf(x1) - b * norm_cdf(x1 - st)
                   - a * p2l * (norm_cdf(-y) - norm_cdf(-y1))
                   + b * p2l2 * (norm_cdf(-y + st) - norm_cdf(-y1 + st));
    return bs_call(m, K, T) - up_in;
}

// One simulated path, driven by the given normals. Returns the discounted
// barrier payoff and the discounted vanilla payoff (the control).
namespace {
struct PathOut {
    double barrier, vanilla;
};

PathOut run_path(const Market& m, double K, double H, double T, int steps,
                 Monitoring mon, const std::vector<double>& z, double sign) {
    const double dt = T / steps;
    const double drift = (m.rate - m.div - 0.5 * m.vol * m.vol) * dt;
    const double diff = m.vol * std::sqrt(dt);
    const double logH = std::log(H);
    double x = std::log(m.spot);
    bool alive = true;
    double survive = 1.0;  // probability of no crossing between dates
    for (int i = 0; i < steps; ++i) {
        double next = x + drift + diff * sign * z[i];
        if (alive) {
            if (next >= logH) {
                alive = false;
            } else if (mon == Monitoring::BrownianBridge) {
                // Given both ends below H, the chance the path touched H in
                // between is exp(-2 (logH - x)(logH - next) / (sigma^2 dt)).
                survive *= 1.0 - std::exp(-2.0 * (logH - x) * (logH - next) / (m.vol * m.vol * dt));
            }
        }
        x = next;   // keep going: the vanilla control needs the terminal value
    }
    const double disc = std::exp(-m.rate * T);
    const double vanilla = disc * std::max(std::exp(x) - K, 0.0);
    return {alive ? vanilla * survive : 0.0, vanilla};
}
}  // namespace

McResult mc_up_out_call(const Market& m, double K, double H, double T, const McOptions& opt) {
    std::mt19937_64 rng(opt.seed);
    std::normal_distribution<double> n01(0.0, 1.0);
    std::vector<double> z(opt.steps);

    // Each sample is one path, or the average of a path and its mirror image.
    std::vector<double> y, c;
    y.reserve(opt.paths);
    c.reserve(opt.paths);
    for (std::int64_t p = 0; p < opt.paths; ++p) {
        for (int i = 0; i < opt.steps; ++i) z[i] = n01(rng);
        PathOut a = run_path(m, K, H, T, opt.steps, opt.monitoring, z, 1.0);
        if (opt.antithetic) {
            PathOut b = run_path(m, K, H, T, opt.steps, opt.monitoring, z, -1.0);
            a.barrier = 0.5 * (a.barrier + b.barrier);
            a.vanilla = 0.5 * (a.vanilla + b.vanilla);
        }
        y.push_back(a.barrier);
        c.push_back(a.vanilla);
    }

    const double n = static_cast<double>(y.size());
    double my = 0, mc = 0;
    for (std::size_t i = 0; i < y.size(); ++i) { my += y[i]; mc += c[i]; }
    my /= n;
    mc /= n;

    double beta = 0.0;
    if (opt.control_variate) {
        // beta = Cov(Y, C) / Var(C), estimated from the same sample
        double cov = 0, var = 0;
        for (std::size_t i = 0; i < y.size(); ++i) {
            cov += (y[i] - my) * (c[i] - mc);
            var += (c[i] - mc) * (c[i] - mc);
        }
        beta = cov / var;
    }
    double exact_c = bs_call(m, K, T);
    double sum = 0, sum2 = 0;
    for (std::size_t i = 0; i < y.size(); ++i) {
        double v = y[i] - beta * (c[i] - exact_c);
        sum += v;
        sum2 += v * v;
    }
    double mean = sum / n;
    double var = (sum2 / n - mean * mean) * n / (n - 1);
    return {mean, std::sqrt(var / n)};
}

double trinomial(const Market& m, double K, double T, int N, bool call, Exercise ex) {
    // Log-space tree with dx = sigma sqrt(3 dt); probabilities match the mean
    // and variance of ln(S) over a step.
    double dt = T / N;
    double nu = m.rate - m.div - 0.5 * m.vol * m.vol;
    double dx = m.vol * std::sqrt(3.0 * dt);
    double a = (m.vol * m.vol * dt + nu * nu * dt * dt) / (dx * dx);
    double pu = 0.5 * (a + nu * dt / dx), pd = 0.5 * (a - nu * dt / dx), pm = 1.0 - a;
    double disc = std::exp(-m.rate * dt);

    auto payoff = [&](double S) { return call ? std::max(S - K, 0.0) : std::max(K - S, 0.0); };
    // Two buffers: each time level is computed from the one after it, so the
    // update must not read values it has already overwritten.
    std::vector<double> V(2 * N + 1), W(2 * N + 1);
    for (int j = -N; j <= N; ++j) V[j + N] = payoff(m.spot * std::exp(j * dx));
    for (int t = N - 1; t >= 0; --t) {
        for (int j = -t; j <= t; ++j) {
            double cont = disc * (pu * V[j + 1 + N] + pm * V[j + N] + pd * V[j - 1 + N]);
            W[j + N] = ex == Exercise::American ? std::max(cont, payoff(m.spot * std::exp(j * dx))) : cont;
        }
        std::swap(V, W);
    }
    return V[N];
}

}  // namespace pricing
