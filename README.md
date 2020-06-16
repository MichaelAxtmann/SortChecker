# SortChecker

A simple but fast permutation checker.

## Example

The checker can be used sequentially:
```
#include <vector>
#include <iostream>
#include <checker.hpp>

permute_checker::Checker<int> checker;
std::vector<int> v;

// Fill v

for (const auto e : v) {
	checker.add_pre(e);
}

// Permute v

for (const auto e : v) {
	checker.add_post(e);
}

std::cout << "Permutation of input: " << checker.is_likely_permutation() << std::endl;
```

The checker can also used by multiple threads and combined afterwards:
```
#include <vector>
#include <iostream>
#include <checker.hpp>

int num_threads = 4;
std::vector<permute_checker::Checker<int>> checker(num_threads);
std::vector<std::vector<int>> v(num_threads);

// Fill v[i]

// Executed by thread i
for (const auto e : v[i]) {
	checker[i].add_pre(e);
}

// Permute v[i]

// Executed by thread i
for (const auto e : v[i]) {
	checker[i].add_post(e);
}

permute_checker::Checker<int>::combine(checker.begin(), checker.end());

std::cout << "Permutation of input: " << checker[0].is_likely_permutation() << std::endl;
```
