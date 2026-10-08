"""Charts for the README from the CSV files the study writes.

    python plot_results.py
"""
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

ROOT = Path(__file__).parent
res, fig_dir = ROOT / "results", ROOT / "figures"
fig_dir.mkdir(exist_ok=True)

b = pd.read_csv(res / "barrier_monitoring.csv")
fig, ax = plt.subplots(figsize=(7, 4))
ax.errorbar(b.steps, b.discrete, yerr=2 * b.discrete_se, fmt="o-", capsize=3, label="checked on the steps only")
ax.errorbar(b.steps, b.bridge, yerr=2 * b.bridge_se, fmt="s-", capsize=3, label="with Brownian-bridge correction")
ax.axhline(b.exact.iloc[0], color="black", lw=1, ls="--", label="closed form")
ax.set_xscale("log")
ax.set_xlabel("time steps per path")
ax.set_ylabel("up-and-out call price")
ax.set_title("Continuous barrier at 130, 200,000 paths, 2 s.e. bars")
ax.legend()
ax.grid(alpha=0.3, which="both")
fig.tight_layout()
fig.savefig(fig_dir / "barrier_monitoring.png", dpi=120)

t = pd.read_csv(res / "trinomial_convergence.csv")
fig, ax = plt.subplots(figsize=(7, 4))
ax.loglog(t.steps, t.call_error.abs(), "o-", label="European call")
ax.loglog(t.steps, t.put_error.abs(), "s-", label="European put")
ax.loglog(t.steps, t.american_put_error.abs(), "^-", label="American put")
ax.loglog(t.steps, t.richardson_call_error.abs(), "o--", label="European call, Richardson")
ax.loglog(t.steps, t.richardson_american_put_error.abs(), "^--", label="American put, Richardson")
ax.loglog(t.steps, 2.0 / t.steps, "k:", label="1 / steps")
ax.set_xlabel("tree steps")
ax.set_ylabel("absolute error")
ax.legend()
ax.grid(alpha=0.3, which="both")
fig.tight_layout()
fig.savefig(fig_dir / "trinomial_convergence.png", dpi=120)
