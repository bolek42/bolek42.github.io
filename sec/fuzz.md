# Intro (In Dev)
There are tons of fuzzers out there and there are also very good ones such as AFL, so why did i write my own one?
First of all I learned a lot, while doing so and second if everyone uses the same fuzzer, everyone will find the same bugs.
Using a fuzzer that has a different approach (but might not be as efficient as other ones) will maybe result in new Bugs.

There are different approaches, to build a fuzzer.
This one starts with a valid file and tries to increase the code coverage by mutating the input file.

# Coverage Guided Fuzzing
By just mutating the file blindly, without any feedback, is called 'dumb fuzzing'.
It is fairly easy to implement but is also limited in the results.
For open source software it is often possible to get some sort of feedback or metric, if a mutation makes 'sense' to the application.
One approach is to use codecoverage.
A large bitmap is generated, wheras each bit is associated with one basic block in the code.
If the basic block is hit, the corresponding bit is set to 1 and 0 otherwise.
Such a bitmap can greatly improove the performance of a fuzzer (e.g. AFL uses the same approach)

# Seed Files
My first target for fuzzing was libav and other video encoders, therefore I needed a huge set of files for a initial coverage.
I downloaded all files from samples.mplayerhq.hu and started to compute the code coverage for each individual file.
As I started fuzzing them I realized, that most of them are way too big to be a good fuzzing seed.
So for a preprocess step, I cut every file down to 1s using the same encoding.

# Mutations
Mutations are used to change the input to the program.
These can be for example bitflips, arithmetic operations, inserting or duplicating known data.

Instead of directly mutating the binary data, each mutation is descibed in a more abstract form like type and offset of the mutation.
Each testcase, that is derived from the same seed file is described as a sorted list of mutations.
To generate a new testcase, new mutations are simply added to this list.
This has the advantage, that combining mutations of different test cases is easy as combining two lists.

# Results
I mostly focused on libav fuzzing, as this yielded the most bugs.
Here are the results:

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

