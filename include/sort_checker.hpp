/*******************************************************************************
 * PermutationChecker/include/checker.hpp
 *
 * Probabilistic permutation checker
 *
 * Copyright (C) 2016 Lorenz Hübschle-Schneider <lorenz@4z2.de>
 * Copyright (C) 2020 Michael Axtmann <michael.axtmann@kit.edu>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

// Implementation adapted and simplified from the Project Thrill - http://project-thrill.org
// thrill/checkers/sort.hpp
// Copyright (C) 2016 Lorenz Hübschle-Schneider <lorenz@4z2.de>
// All rights reserved. Published under the BSD-2 license in the LICENSE file.

#pragma once
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

#include "../src/hash.hpp"

#if defined(__GNUC__) || defined(__clang__)
#define CHECKER_ATTRIBUTE_ALWAYS_INLINE __attribute__ ((always_inline)) inline
#else
#define CHECKER_ATTRIBUTE_ALWAYS_INLINE inline
#endif

namespace checker {

/*!
 * Probabilistic checker for permutation algorithms
 *
 * \tparam ValueType Type of the elements being permuted
 */
template <typename T, typename Hash = common::hash_crc32<T>>
class SortChecker
{

  using ValueType = T;

public:
  /*!
   * Construct a checker
   *
   * \param cmp_ Compare function to use
   */
  explicit SortChecker()
  { reset(); }

  //! Reset the checker's internal state
  void reset() {
    count_pre = 0;
    count_post = 0;
    sum_pre = 0;
    sum_post = 0;
    post_added = false;
    post_left = ValueType{};
    post_right = ValueType{};
    sorted_locally = true;
  }

  /*!
   * Process an output element (before permuting)
   *
   * \param v Element to process
   */
  CHECKER_ATTRIBUTE_ALWAYS_INLINE
  void add_pre(const ValueType& v) {
    sum_pre += hash(v);
    ++count_pre;
  }

  /*!
   * Process an output element (after permuting)
   *
   * \param v Element to process
   * \param comp Comparator
   */
  template<typename Comp>
  CHECKER_ATTRIBUTE_ALWAYS_INLINE
  void add_post(const ValueType& v, Comp&& comp) {
    sum_post += hash(v);
    ++count_post;

    if (!post_added) {
      post_left = v;
      post_added = true;
    } else {
      sorted_locally &= !comp(v, post_right);
    }
    post_right = v;
  }

  template<typename Iterator>
  static bool is_likely_permuted(Iterator begin, Iterator end) {

    uint64_t cpre = 0, cpost = 0, spre = 0, spost = 0;

    for (; begin != end; ++begin) {
      cpre  += begin->count_pre;
      spre  += begin->sum_pre;
      cpost += begin->count_post;
      spost += begin->sum_post;
    }
    
    return (cpre == cpost) && (spost == spre);
  }

  /*!
   * Verify probabilistically whether the output elements at all
   * workers are the sorted input elements.  Success probability
   * depends on the hash function used.
   *
   * This function has one-sided error -- it may wrongly accept an incorrect
   * output, but will never cry wolf on a correct one.
   */
  template<typename Comp, typename Iterator>
  static bool is_likely_sorted(Iterator begin, Iterator end, Comp comp) {
    using Checker = typename std::iterator_traits<Iterator>::value_type;
    bool sorted = Checker::is_likely_permuted(begin, end);

    Iterator curr = std::find_if(begin, end, [](const auto& c) {return c.post_added;});
    if (curr != end) {
      Iterator next = std::find_if(curr + 1, end, [](const auto& c) {return c.post_added;});

      while (next != end) {
	sorted &= !comp(next->post_left, curr->post_right);
	
	curr = next;
	next = std::find_if(next + 1, end, [](const auto& c) {return c.post_added;});
      }
    }

    for (auto it = begin; it != end; ++it) {
      sorted &= it->sorted_locally;
    }
    
    return sorted;
  }

protected:
  //! Number of items seen in input and output
  uint64_t count_pre, count_post;
  //! Sum of hash values in input and output
  uint64_t sum_pre, sum_post;
  //! Pre and post values have been added
  bool post_added;
  //! First and last pre and post values
  ValueType post_left, post_right;
  //! Local elements are sorted
  bool sorted_locally;
  //! Hash function
  Hash hash;
};

} // namespace checker

/******************************************************************************/
