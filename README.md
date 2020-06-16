# SortChecker

```
sort_checker::Checker<T, std::less<>> checker;
std::vector<int> v;
// Fill v
checker.add_pre(v.data(), v.data() + size);
// Permute v
checker.add_post(v.data(), v.data() + size);
checker.is_likely_permutation()
```
