# SortChecker

A simple but fast algorithm that checks for sorted/permuted output. The hash function (CRC32) used in this algorithm may not neet your theoretical requirements. If you need an high quality checker, we advice to use a better hash function and/or apply the checker multiple times with different hash functions. For a detailed discussion, we refer to the article [Communication Efficient Checking of Big Data Operations](https://ieeexplore.ieee.org/abstract/document/8425218).

## Example

The checker can be used sequentially:
```
#include <vector>
#include <iostream>
#include <checker.hpp>

checker::SortChecker<int> checker;
std::vector<int> v;
std::less comp;

// Fill v

for (const auto e : v) {
	checker.add_pre(e);
}

// Permute v

for (const auto e : v) {
	checker.add_post(e, comp);
}

std::cout << "Permutation of input: " << checker.is_likely_permutation() << std::endl;
std::cout << "Sorted output: " << checker.is_likely_sorted() << std::endl;
```

The checker can also be used by multiple threads:
```
#include <vector>
#include <iostream>
#include <checker.hpp>

using Checker = checker::SortChecker<int>;

int num_threads = 4;
std::vector<Checker> checker(num_threads);
std::vector<std::vector<int>> v(num_threads);
std::less comp;

// Fill v[i]

// Executed by thread i
for (const auto e : v[i]) {
	checker[i].add_pre(e);
}

// Permute v[i]

// Executed by thread i
for (const auto e : v[i]) {
	checker[i].add_post(e, comp);
}

std::cout << "Permutation of input: " << Checker::is_likely_permutation(checker.begin(), checker.end()) << std::endl;
std::cout << "Sorted output: " << Checker::is_likely_sorted(checker.begin(), checker.end(), comp) << std::endl;
```
