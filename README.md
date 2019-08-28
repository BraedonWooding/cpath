# CPath

A simple and nice looking cross platform C FileSystem.  I'll put up some documentation and tests up soon.

It worked out really nicely from my basic idea in my head.  Check out the `example.c` file if you want some basic idea of how it works.

## Why?

Because I wanted a filesystem for my IMGUI Widget library but I didn't want to only support C++17 so I needed a preferably early standardised C++ (or C) library.  While there existed plenty of variants their APIs weren't particularly nice or they didn't try to reduce includes or various other issues.  The key thing for me was wanting to just have a single while loop function to both check if there is a next one and then assign the next one rather than having to split it into potentially 3! Which some had (i.e. a hasNext, a getFile, and a moveNext).

I also felt that the code was quite obfuscated for some and it felt like they overused the #ifdefs to try to get as much similar as possible rather than trying to just make it more readable at the cost of a small amount of local duplication.

I chose to make this very young i.e. C89/C99 ish since I was already going quite far with making it work with all versions of C++.

## Roadmap

- [ ] Complete the current stuff
- [ ] Automatic recursive iterator, basically will just be a wrapper around the current one
- [ ] Cleanup the code around how it has to awkwardly handle the first item
  - Basically since all the systems work on that you have to call 2 different functions one for the first one and one for all the rest (ugh) we have to handle that specially
    - I'm thinking putting that logic into move next rather than into open dir, that is when it detects that you are trying to move next but everything is NULL or whatevs itll reload the directory this would also have the benefit of automatically reloading the directory if you go through it.  I don't know if I like this, I could always avoid this by using some kind of flag instead.
- [ ] Small optimisations around calculating string lengths and preallocating files
- [ ] Support comparisons of paths easier and maybe even a path object
- [ ] Add C++ bindings (just some classes and methods)

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

And yes this shows that CPath is faster than `find` :O.  Note all code is compiled on highest optimisation (`-O3`) but not on undefined optimisation (`-Ofast`)
