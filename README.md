# CPath

> Made By Braedon Wooding

> Fast (see benchmarks at the bottom), efficient (optimised towards reducing dynamic allocations), and simple

A simple and nice looking cross platform C FileSystem.  I'll put up some documentation and tests up soon.

It worked out really nicely from my basic idea in my head.  Check out the `example.c` file if you want some basic idea of how it works.

## Why

Because I wanted a filesystem for my IMGUI Widget library but I didn't want to only support C++17 so I needed a preferably early standardised C++ (or C) library.  While there existed plenty of variants their APIs weren't particularly nice or they didn't try to reduce includes or various other issues.  The key thing for me was wanting to just have a single while loop function to both check if there is a next one and then assign the next one rather than having to split it into potentially 3! Which some had (i.e. a hasNext, a getFile, and a moveNext).

I also felt that the code was quite obfuscated for some and it felt like they overused the #ifdefs to try to get as much similar as possible rather than trying to just make it more readable at the cost of a small amount of local duplication.

I chose to make this very young i.e. C89/C99 ish since I was already going quite far with making it work with all versions of C++.

## Credits

I would like to put a thanks out to a lot of other similar libraries for giving me some source to compare against to make sure I was using the APIs correctly and some ideas for how to structure my API.

## Benchmarks

> Model Name: MacBook Air

> Model Identifier: MacBookAir7,2

> Processor Name: Intel Core i5

> Processor Speed: 1.6 GHz

> Number of Processors: 1

> Total Number of Cores: 2

> L2 Cache (per Core): 256 KB

> L3 Cache: 3 MB

> Memory: 8 GB

Resultant stats running the given shell file:

| Test        | User Time | System Time  | Total |
| ------------- |-------------| -----|-------|
| Python (os.walk) | 0.121s | 0.063s | 0.184s |
| CPath (Recursion) | 0.012s | 0.031s | 0.043s |
| CPath (Stack) | 0.013s | 0.035s | 0.048s |
| `tree` | 0.183s | 0.139s | 0.322s |
| `find` | 0.013s | 0.058s | 0.071s |

Observations

- This is done on `-O3` BUT has no difference (from what I can see) for `-O1` and `-O2` the only difference is `-O0` which is consistently around 5% slower.
- Yes this does show that CPath is faster than `find` which I'm quite proud of
  - The user time is the same which is not that surprising since they both are written in C
  - The system time is different indicating I do less syscalls
