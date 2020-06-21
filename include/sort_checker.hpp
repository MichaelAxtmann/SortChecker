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

#include "hash.hpp"

#if defined(__GNUC__) || defined(__clang__)
#define CHECKER_ATTRIBUTE_ALWAYS_INLINE __attribute__ ((always_inline)) inline
#else
#define CHECKER_ATTRIBUTE_ALWAYS_INLINE inline
#endif

namespace checker {

/*!
 * Probabilistic checker for permutation algorithms
 *
 * \tparam T Type of the elements being permuted
 */
template <typename T, typename Hash = common::hash_tabulated<T>>
class SortChecker
{
public:

  /*!
   * Construct a checker
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
    post_left = T{};
    post_right = T{};
    sorted_locally = true;
  }

  /*!
   * Process an element (before sorting)
   *
   * \param v Element to process
   */
  CHECKER_ATTRIBUTE_ALWAYS_INLINE
  void add_pre(const T& v) {
    sum_pre += hash(v);
    ++count_pre;
  }

  /*!
   * Process an element (after sorting)
   *
   * \param v Element to process
   * \param comp Comparator
   */
  template<typename Comp>
  CHECKER_ATTRIBUTE_ALWAYS_INLINE
  void add_post(const T& v, Comp&& comp) {
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

  /*!
   * Verify probabilistically whether the elements before sorting are
   * a permutation of the elements after sorting. The success
   * probability depends on the hash function used.
   *
   * This function has one-sided error -- it may wrongly accept an incorrect
   * output, but will never cry wolf on a correct one.
   */
  bool is_likely_permuted() const {
    return (count_pre == count_post) && (sum_pre == sum_post);
  }

  /*!
   * Verify probabilistically whether the elements after sorting are
   * actually the sorted output of the elements before sorting. The
   * success probability depends on the hash function used.
   *
   * This function has one-sided error -- it may wrongly accept an incorrect
   * output, but will never cry wolf on a correct one.
   */
  bool is_likely_sorted() const {
    return is_likely_permuted() && sorted_locally;
  }


  /*!
   * Verify probabilistically whether the elements before sorting are
   * a permutation of the elements after sorting. The success
   * probability depends on the hash function used.
   * 
   * The function accepts a sequence of 'SortChecker' objects to which
   * the elements were distributed to.
   *
   * This function has one-sided error -- it may wrongly accept an incorrect
   * output, but will never cry wolf on a correct one.
   *
   * \param begin Iterator of the front of the checker sequence.
   * \param end Iterator of the end of the checker sequence.
   */
  template<typename Iterator>
  static bool is_likely_permuted(Iterator begin, Iterator end) {

    uint64_t cpre = 0, cpost = 0, spre = 0, spost = 0;

    // Aggregate values.
    for (; begin != end; ++begin) {
      cpre  += begin->count_pre;
      spre  += begin->sum_pre;
      cpost += begin->count_post;
      spost += begin->sum_post;
    }
    
    return (cpre == cpost) && (spost == spre);
  }


  /*!
   * Verify probabilistically whether the elements after sorting are
   * actually the sorted output of the elements before sorting. The
   * success probability depends on the hash function used.
   * 
   * The function accepts a sequence of 'SortChecker' objects to which
   * the elements were distributed to. The sorted elements have to be
   * assigned to the checker objects in sorted order -- meaning that
   * checker i only gets elements smaller or equal to the elements of
   * checker i+1.
   *
   * This function has one-sided error -- it may wrongly accept an incorrect
   * output, but will never cry wolf on a correct one.
   *
   * \param begin Iterator of the front of the checker sequence.
   * \param end Iterator of the end of the checker sequence.
   * \param comp Comparator to check that the elements are sorted among the checkers.
   */
  template<typename Comp, typename Iterator>
  static bool is_likely_sorted(Iterator begin, Iterator end, Comp comp) {
    using Checker = typename std::iterator_traits<Iterator>::value_type;

    // Elements are probably a permutation.
    bool succ = Checker::is_likely_permuted(begin, end);

    // Elements are locally sorted.
    for (auto it = begin; it != end; ++it) {
      succ &= it->sorted_locally;
    }

    // Elements are sorted among the checkers.
    
    // Find first checker which has sorted elements.
    Iterator curr = std::find_if(begin, end, [](const auto& c) {return c.post_added;});
    if (curr != end) {
      // Find next checker which has sorted elements.
      Iterator next = std::find_if(curr + 1, end, [](const auto& c) {return c.post_added;});

      while (next != end) {
	succ &= !comp(next->post_left, curr->post_right);
	
	curr = next;

	// Find next checker which has sorted elements.
	next = std::find_if(next + 1, end, [](const auto& c) {return c.post_added;});
      }
    }
    
    return succ;
  }

protected:
  //! Number of items seen in input and output
  uint64_t count_pre, count_post;
  //! Sum of hash values in input and output
  uint64_t sum_pre, sum_post;
  //! Pre and post values have been added
  bool post_added;
  //! First and last pre and post values
  T post_left, post_right;
  //! Local elements are sorted
  bool sorted_locally;
  //! Hash function
  Hash hash;
};

} // namespace checker

/******************************************************************************/
