/*******************************************************************************
 * PermutationChecker/include/checker.hpp
 *
 * Probabilistic permutation checker
 *
 * Copyright (C) 2016 Lorenz Hübschle-Schneider <lorenz@4z2.de>
 * Copyright (C) 2020 Michael Axtmann <michael.axtmann@gmail.com>
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

// Implementation adapted and simplified from the Project Thrill - http://project-thrill.org
// thrill/common/hash.hpp
// Copyright (C) 2016 Lorenz Hübschle-Schneider <lorenz@4z2.de>
// All rights reserved. Published under the BSD-2 license in the LICENSE file.

#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <string>
#include <type_traits>

namespace checker {
namespace common {

/*!
 * Tabulation Hashing, see https://en.wikipedia.org/wiki/Tabulation_hashing
 *
 * Keeps a table with size * 256 entries of type hash_t, filled with random
 * values.  Elements are hashed by treating them as a vector of 'size' bytes,
 * and XOR'ing the values in the data[i]-th position of the i-th table, with i
 * ranging from 0 to size - 1.
 */
namespace _detail {
template <size_t size, typename hash_t = uint32_t,
          typename prng_t = std::mt19937>
class tabulation_hashing
{
public:
    using hash_type = hash_t;  // make public
    using prng_type = prng_t;
    using Subtable = std::array<hash_type, 256>;
    using Table = std::array<Subtable, size>;

    tabulation_hashing(size_t seed = 0) { init(seed); }

    //! (re-)initialize the table by filling it with random values
    void init(const size_t seed) {
        prng_t rng { seed };
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < 256; ++j) {
                table[i][j] = rng();
            }
        }
    }

    //! Hash an element
    template <typename T>
    hash_type operator () (const T& x) const {
        static_assert(sizeof(T) == size, "Size mismatch with operand type");

        hash_t hash = 0;
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&x);
        for (size_t i = 0; i < size; ++i) {
            hash ^= table[i][*(ptr + i)];
        }
        return hash;
    }

protected:
    Table table;
};

} // namespace _detail

//! Tabulation hashing
template <typename T>
using hash_tabulated = _detail::tabulation_hashing<sizeof(T)>;

} // namespace common
} // namespace checker

