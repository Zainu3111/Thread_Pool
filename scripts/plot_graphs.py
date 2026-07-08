import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("../tests/benchmark_results.csv")
df.columns = df.columns.str.strip()
#print(df.columns)
fib = df[df["Benchmark"].str.contains("Merge-Sort")]

for benchmark in fib["Benchmark"].unique():
    subset = fib[fib["Benchmark"] == benchmark]
    plt.plot(
        subset["InputSize"],
        subset["Time(ms)"],
        marker="o",
        label=benchmark
    )

plt.title("Merge-Sort Benchmark")
plt.xlabel("Input Size")
plt.ylabel("Time (ms)")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("fib.png")
plt.show()
