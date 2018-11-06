from pylab import *

data = [
	loadtxt("benchmark.txt", skiprows=3),
#	loadtxt("benchmark_stars.txt", skiprows=3),
#	loadtxt("benchmark_circles.txt", skiprows=3),
]

names = [
	"PolyMath F32",
	"PolyMath F64",
	"Boost F32",
	"Boost F64",
	"Clipper",
	"Geos",
]
markers = [".-", ".--", ".-.", ".:"]

for d in data:
	d[where(d == 0)] = nan

close("all")

if True:
	figure("Benchmark 1", figsize=(8, 6))
	for i in range(len(names)):
		p = loglog(data[0][:, 1], data[0][:, 2 + i], markers[0], label=names[i])
		for j in range(1, len(data)):
			loglog(data[j][:, 1], data[j][:, 2 + i], markers[j], color=p[0].get_color())
	x = logspace(1, 8, 71)
	loglog(x, 1e-8 * x * log(x), "k-", label="O(n*log(n))")
	grid()
	legend(loc="best", fontsize="small")
	xlabel("Vertices")
	ylabel("Time (s)")
	tight_layout()
	show()

if True:
	figure("Benchmark 2", figsize=(8, 6))
	for i in range(len(names)):
		p = loglog(data[0][:, 1], data[0][:, 2 + i] / data[0][:, 1], markers[0], label=names[i])
		for j in range(1, len(data)):
			loglog(data[j][:, 1], data[j][:, 2 + i] / data[j][:, 1], markers[j], color=p[0].get_color())
	x = logspace(1, 8, 71)
	loglog(x, 1e-8 * log(x), "k-", label="O(n*log(n))")
	grid()
	legend(loc="upper left", fontsize="small")
	xlabel("Vertices")
	ylabel("Time/vertex (s)")
	tight_layout()
	show()

