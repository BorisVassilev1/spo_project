#pragma once
#include <chrono>
#include <yoghurtgl.h>

#define BENCH(x, n, f)                                         \
	{                                                          \
		auto t1 = std::chrono::high_resolution_clock::now();   \
		for (int i = 0; i < n; ++i) {                          \
			x;                                                 \
		}                                                      \
		auto t2	  = std::chrono::high_resolution_clock::now(); \
		auto diff = t2 - t1;                                   \
		f(diff / n);                                           \
	}
