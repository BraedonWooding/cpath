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

This are the results from running the relevant shell files

I also compared against other currently available file systems using their supplied method of traversal or one that would match the closest to what the examples showed.  To try not to give them a disadvantage in speed.

> Note: If you feel that I somehow wrote these scripts inefficiently please submit a PR with fixes and I'll run the benchmarks.  Eventually we'll run these benchmarks on multiple machines (eventually) but for now it's easier to just not.

### Recursive vs Stack through a large directory system

> This test takes a while to run since creating the files can take a while and we are running a 100 tests to get a reasonable average.  Note: I sorted the list after creation.

| Test                     | User   | System | Total  |
| ------------------------ | ------ | ------ | ------ |
| CPath (Recursion in C)   | 0.020s | 0.064s | 0.084s |
| CPath (Recursive in cpp) | 0.023s | 0.064s | 0.088s |
| CPath (Emplace in C)     | 0.021s | 0.072s | 0.093s |
| CPath (Emplace in cpp)   | 0.030s | 0.071s | 0.101s |
| find                     | 0.020s | 0.117s | 0.137s |
| Python (os.walk)         | 0.158s | 0.081s | 0.240s |
| Cute Files               | 0.041s | 0.241s | 0.281s |
| TinyDir                  | 0.050s | 0.238s | 0.288s |
| tree                     | 0.351s | 0.233s | 0.584s |

Observations

- This is done on `-O3` BUT has no difference (from what I can see) for `-O1` and `-O2` the only difference is `-O0` which is consistently around 5% slower.
- Yes this does show that CPath is faster than `find` which I'm quite proud of
  - The user time is the same which is not that surprising since they both are written in C
  - The system time is different indicating I do less syscalls
- Interestingly enough python beats both of the other libraries, I guess this comes to show that writing good code is all about optimising the bottle necks!  Even though the user time is around 4-5x faster (still not amazing since mine runs about 8-10x faster) it is absolutely hammered by the system time being around 3x slower there.  And since that is a bottle neck it results in the whole system running signifincantly slower.
- Emplace in general seems slower and cpp does seem to suffer a minor performance cost but both of these are extremely minor and highly variable that I don't think I'll look into it too much.
