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

// Implementation adapted and simplified form the Project Thrill - http://project-thrill.org
// thrill/checkers/sort.hpp
// Copyright (C) 2016 Lorenz Hübschle-Schneider <lorenz@4z2.de>
// All rights reserved. Published under the BSD-2 license in the LICENSE file.

#pragma once
#include <algorithm>
#include <numeric>
#include <vector>

#include "../src/hash.hpp"

#if defined(__GNUC__) || defined(__clang__)
#define PERMUTE_CHECKER_ATTRIBUTE_ALWAYS_INLINE __attribute__ ((always_inline)) inline
#else
#define PERMUTE_CHECKER_ATTRIBUTE_ALWAYS_INLINE inline
#endif

namespace permute_checker {

/*!
 * Probabilistic checker for permutation algorithms
 *
 * \tparam ValueType Type of the elements being permuted
 */
template <typename ValueType>
class Checker
{

public:
  /*!
   * Construct a checker
   *
   * \param cmp_ Compare function to use
   */
  explicit Checker()
  { reset(); }

  //! Reset the checker's internal state
  void reset() {
    count_pre = 0;
    count_post = 0;
    sum_pre = 0;
    sum_post = 0;
  }

  /*!
   * Process an output element (before permuting)
   *
   * \param v Element to process
   */
  PERMUTE_CHECKER_ATTRIBUTE_ALWAYS_INLINE
  void add_pre(const ValueType& v) {
    sum_pre += hash(v);
    ++count_pre;
  }

  /*!
   * Process an output element (after permuting)
   *
   * \param v Element to process
   */
  PERMUTE_CHECKER_ATTRIBUTE_ALWAYS_INLINE
  void add_post(const ValueType& v) {

    sum_post += hash(v);
    ++count_post;
  }
  
  void combine_pre(Checker<ValueType>* begin, Checker<ValueType>* end) {
    std::for_each(begin, end, [this](const Checker<ValueType>& checker) {
				this->count_pre += checker.count_pre;
				this->sum_pre += checker.sum_pre;
			      });
    std::for_each(begin, end, [this](Checker<ValueType>& checker) {
				checker.count_pre = this->count_pre;
				checker.sum_pre = this->sum_pre;
			      });
  }

  void combine_post(Checker<ValueType>* begin, Checker<ValueType>* end) {
    std::for_each(begin, end, [this](const Checker<ValueType>& checker) {
				this->count_post += checker.count_post;
				this->sum_post += checker.sum_post;
			      });
    std::for_each(begin, end, [this](Checker<ValueType>& checker) {
				checker.count_post = this->count_post;
				checker.sum_post = this->sum_post;
			      });
  }

  /*!
   * Verify probabilistically whether the output elements at all workers are a
   * permutation of the input elements.  Success probability depends on the
   * hash function used.
   *
   * This function has one-sided error -- it may wrongly accept an incorrect
   * output, but will never cry wolf on a correct one.
   */
  bool is_likely_permutation() {
    return (count_pre == count_post) && (sum_pre == sum_post);
  }

protected:
  //! Number of items seen in input and output
  uint64_t count_pre, count_post;
  //! Sum of hash values in input and output
  uint64_t sum_pre, sum_post;
  //! Hash function
  common::hash_crc32<ValueType> hash;
};

} // namespace permute_checker

/******************************************************************************/
