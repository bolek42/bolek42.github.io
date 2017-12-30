# Intro (In Dev)
There are tons of fuzzers out there and also very good ones such as AFL, so why did I write my own one?
First of all I learned a lot, while doing so and second if everyone uses the same fuzzer, everyone will find the same bugs.

### libav
[951](https://bugzilla.libav.org/show_bug.cgi?id=951)
[952](https://bugzilla.libav.org/show_bug.cgi?id=952)
[1007](https://bugzilla.libav.org/show_bug.cgi?id=1007)
[1008](https://bugzilla.libav.org/show_bug.cgi?id=1008)
[1009](https://bugzilla.libav.org/show_bug.cgi?id=1009)
[1010](https://bugzilla.libav.org/show_bug.cgi?id=1010)
[1011](https://bugzilla.libav.org/show_bug.cgi?id=1011)
[1012](https://bugzilla.libav.org/show_bug.cgi?id=1012)
[1013](https://bugzilla.libav.org/show_bug.cgi?id=1013)
[1014](https://bugzilla.libav.org/show_bug.cgi?id=1014)
[1015](https://bugzilla.libav.org/show_bug.cgi?id=1015)
[1016](https://bugzilla.libav.org/show_bug.cgi?id=1016)
[1017](https://bugzilla.libav.org/show_bug.cgi?id=1017)
[1018](https://bugzilla.libav.org/show_bug.cgi?id=1018)

In fact the bug [952](https://bugzilla.libav.org/show_bug.cgi?id=952) (uninitialized pointer) also triggered in ffmpeg.
Sadly I did not checked it against any browser, so I noticed later that Chromium was also affected.
One bug hits three projects so I decided to request a CVE and here it is: CVE-2017-1000460


# Coverage Guided Fuzzing
By just mutating the file blindly, without any feedback, is called 'dumb fuzzing'.
It is fairly easy to implement but is also limited in the results.
For open source software it is often possible to get some sort of feedback or metric, if a mutation makes 'sense' to the application.
One approach is to use codecoverage.
A large bitmap is generated, wheras each bit is associated with one basic block in the code.
If the basic block is hit, the corresponding bit is set to 1 and 0 otherwise.
Such a bitmap can greatly improove the performance of a fuzzer (e.g. AFL uses the same approach).
Free software can be compiled with e.g. clang which already supports code coverage.

```
clang -fsanitize=address -fsanitize-coverage=bb
export ASAN_OPTIONS="coverage=1:coverage_bitset=1"
```

If the ASAN_OPTIONS environment variable is set correctly, a $binary.$pid.bitset-sancov is created containing the coverage bitmap as a long string with ASCII '0' and '1'.
For programs such as avconv, which is a 45MB binary, these bitmaps can get very large like 450.000 entries.
For easy processing, this bitmap is parsed as an integer that also allows further processing.
Let t be a bitmap of a specific testcase and c the total coverage:

| Expression   | | Description                   |
|--------------|-|-------------------------------|
| c &#124; t   | | The total coverage of c and t |
| (c ^ t) & ~c | | New blocks hit by t           |

In addition to code coverage, clang also suports the ASAN address sanitizer, which will stop the program, if any invalid memory access is detected.
A detailed error message is printed to stderr, that can be parsed, by the calling process, to detect an distinguish crashes.
The best way to detect a new crash is to use the rip of the crash, but this wil interfere with ASLR.
Therefore it is required to disable ASLR on the fuzzing machine, so a dedicated fuzzing machine is required.

Compiling programs with clang, code coverage and asan is sometimes a struggle.
For a lot of programs or libraries, it is sufficient to set the following environment variables:

```
export CC=clang
export CFLAGS=-fsanitize=address -fsanitize-coverage=edge
export CXX=clang++
export CXXFLAGS=-fsanitize=address -fsanitize-coverage=edge
export LDFLAGS=-Wc -fsanitize=address -fsanitize-coverage=edge
```

The basic concept of coverage guided fuzzing is based on a genetic algorithm to increase code coverage.
We start with a single seed file in the population and the total coverage of the population is hold as a global state.
A random element from the population is picked, mutated and the coverage is measured.
If a mutation hits a new basic block, the testcase is added to the population and the total coverage is increased.
This process is repeated over and over again until it is terminated by the user.

# Seed Files
A good set of seed files with a high initial coverage is needed to get good fuzzing results.
My first target for fuzzing was libav and other video encoders, therefore I needed a huge set of files for a initial coverage.
I downloaded all sample files from samples.mplayerhq.hu and started to compute the code coverage for each individual file.
As I started fuzzing them I realized, that most of them are way too big to be a good fuzzing seed.
So for a preprocess step, I cut every file down to 1s using the same encoding and remeasured the coverage.

The next task is to find a subset of files, with a maximum coverage.
We do not need an exact solution or a highly efficient algorithm as the main overhead is the coverage measurements.
The following pseudocode is used to select the count best testcases with high total coverage.

```
while len(seeds) > count:
    c = 0
    for s in seeds:
        s.new_blocks = new_blocks(c,s.coverage)
        c |= s.coverage
    seeds = sort(seeds, key=lambda x: x.new_blocks)[:-1]
```

# Mutations
Mutations are used to change the input to the program, for example bitflips, arithmetic operations, inserting or duplicating known data.
Each mutation is descibed as a json which holds type and offset of the mutation.
Instead of saving the mutated binary data for each testcase, only the list of mutations is saved.
To generate a new testcase, new reandom mutations are added to this list.

This method allows for merging mutations by selecting a random subset of two testcases.
The idea is to recombine known mutations that itself increased code coverage.
This way of merging seems to work quite well as somtime a whole bunch of new code blocks are discovered at once.

The propability of mutating a testcase $i$ is $P = n_i / N$ wherase $N$ is the total amount of discovered and $n_i$ the new blocks found by testcase $i$.
This focus the workload to testcases that really improved code coverage and are more likely to hit new blocks due to undiscovered code.
