/*
 * Copyright 2020 Astera Labs, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not
 * use this file except in compliance with the License. You may obtain a copy
 * of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/*
 * @file aries_bifurcation_params.c
 * @brief Implementation of the bifurcation seetings lookup table.
 */

#include "include/aries_bifurcation_params.h"

#ifdef __cplusplus
extern "C"
{
#endif

AriesBifurcationParamsType bifurcationModes[36] = {
    /**< Bifurcation properties for x16 */
    {1, {{0, 16, 0}}},
    /**< Bifurcation properties for x8 */
    {1, {{0, 8, 0}}},
    /**< Bifurcation properties for x4 */
    {1, {{0, 4, 0}}},
    /**< Bifurcation properties for x8x8 */
    {2, {{0, 8, 0}, {8, 8, 1}}},
    /**< Bifurcation properties for x4x4x8 */
    {3, {{0, 4, 0}, {4, 4, 1}, {8, 8, 2}}},
    /**< Bifurcation properties for x8x4x4 */
    {3, {{0, 8, 0}, {8, 4, 1}, {12, 4, 2}}},
    /**< Bifurcation properties for x4x4x4x4 */
    {4, {{0, 4, 0}, {4, 4, 1}, {8, 4, 2}, {12, 4, 3}}},
    /**< Bifurcation properties for x2x2x2x2x2x2x2x2 */
    {8,
     {{0, 2, 0},
      {2, 2, 1},
      {4, 2, 2},
      {6, 2, 3},
      {8, 2, 4},
      {10, 2, 5},
      {12, 2, 6},
      {14, 2, 7}}},
    /**< Bifurcation properties for x2x2x4x8 */
    {4, {{0, 2, 0}, {2, 2, 1}, {4, 4, 2}, {8, 8, 3}}},
    /**< Bifurcation properties for x4x2x2x8 */
    {4, {{0, 4, 0}, {4, 2, 1}, {6, 2, 2}, {8, 8, 3}}},
    /**< Bifurcation properties for x8x4x2x2 */
    {4, {{0, 8, 0}, {8, 4, 1}, {12, 2, 2}, {14, 2, 3}}},
    /**< Bifurcation properties for x8x2x2x4 */
    {4, {{0, 8, 0}, {8, 2, 1}, {10, 2, 2}, {12, 4, 3}}},
    /**< Bifurcation properties for x8x2x2x2x2 */
    {5, {{0, 8, 0}, {8, 2, 1}, {10, 2, 2}, {12, 2, 3}, {14, 2, 4}}},
    /**< Bifurcation properties for x2x2x2x2x8 */
    {5, {{0, 2, 0}, {2, 2, 1}, {4, 2, 2}, {6, 2, 3}, {8, 8, 4}}},
    /**< Bifurcation properties for x4x4x4x2x2 */
    {5, {{0, 4, 0}, {4, 4, 1}, {8, 4, 2}, {12, 2, 3}, {14, 2, 4}}},
    /**< Bifurcation properties for x4x4x2x2x4 */
    {5, {{0, 4, 0}, {4, 4, 1}, {8, 2, 2}, {10, 2, 3}, {12, 4, 4}}},
    /**< Bifurcation properties for x4x2x2x4x4 */
    {5, {{0, 4, 0}, {4, 2, 1}, {6, 2, 2}, {8, 4, 3}, {12, 4, 4}}},
    /**< Bifurcation properties for x2x2x4x4x4 */
    {5, {{0, 2, 0}, {2, 2, 1}, {4, 4, 2}, {8, 4, 3}, {12, 4, 4}}},
    /**< Bifurcation properties for x4x4x2x2x2x2 */
    {6, {{0, 4, 0}, {4, 4, 1}, {8, 2, 2}, {10, 2, 3}, {12, 2, 4}, {14, 2, 5}}},
    /**< Bifurcation properties for x4x2x2x4x2x2 */
    {6, {{0, 4, 0}, {4, 2, 1}, {6, 2, 2}, {8, 4, 3}, {12, 2, 4}, {14, 2, 5}}},
    /**< Bifurcation properties for x4x2x2x2x2x4 */
    {6, {{0, 4, 0}, {4, 2, 1}, {6, 2, 2}, {8, 2, 3}, {10, 2, 4}, {12, 4, 5}}},
    /**< Bifurcation properties for x2x2x4x4x2x2 */
    {6, {{0, 2, 0}, {2, 2, 1}, {4, 4, 2}, {8, 4, 3}, {12, 2, 4}, {14, 2, 5}}},
    /**< Bifurcation properties for x2x2x4x2x2x4 */
    {6, {{0, 2, 0}, {2, 2, 1}, {4, 4, 2}, {8, 2, 3}, {10, 2, 4}, {12, 4, 5}}},
    /**< Bifurcation properties for x4x4x4x4x2x2 */
    {6, {{0, 4, 0}, {4, 4, 1}, {8, 4, 2}, {12, 4, 3}, {16, 2, 4}, {18, 2, 5}}},
    /**< Bifurcation properties for x4x2x2x2x2x2x2 */
    {7,
     {{0, 4, 0},
      {4, 2, 1},
      {6, 2, 2},
      {8, 2, 3},
      {10, 2, 4},
      {12, 2, 5},
      {14, 2, 6}}},
    /**< Bifurcation properties for x2x2x4x2x2x2x2 */
    {7,
     {{0, 2, 0},
      {2, 2, 1},
      {4, 4, 2},
      {8, 2, 3},
      {10, 2, 4},
      {12, 2, 5},
      {14, 2, 6}}},
    /**< Bifurcation properties for x2x2x2x2x4x2x2 */
    {7,
     {{0, 2, 0},
      {2, 2, 1},
      {4, 2, 2},
      {6, 2, 3},
      {8, 4, 4},
      {12, 2, 5},
      {14, 2, 6}}},
    /**< Bifurcation properties for x2x2x2x2x2x2x4 */
    {7,
     {{0, 2, 0},
      {2, 2, 1},
      {4, 2, 2},
      {6, 2, 3},
      {8, 2, 4},
      {10, 2, 5},
      {12, 4, 6}}},
    /**< Bifurcation properties for x4x4 */
    {2, {{0, 4, 0}, {4, 4, 1}}},
    /**< Bifurcation properties for x4x2x2 */
    {3, {{0, 4, 0}, {4, 2, 1}, {6, 2, 2}}},
    /**< Bifurcation properties for x2x2x4 */
    {3, {{0, 2, 0}, {2, 2, 1}, {4, 4, 2}}},
    /**< Bifurcation properties for x2x2x2x2 */
    {4, {{0, 2, 0}, {2, 2, 1}, {4, 2, 2}, {6, 2, 3}}},
    /**< Bifurcation properties for x2x2 */
    {2, {{0, 2, 0}, {2, 2, 1}}},
    /**< Bifurcation properties for x4x8x4 */
    {3, {{0, 4, 0}, {4, 8, 1}, {12, 4, 2}}},
    /**< Bifurcation properties for x2 */
    {1, {{0, 2, 0}}},
    /**< Bifurcation properties for x1 */
    {1, {{0, 1, 0}}}};

#ifdef __cplusplus
}
#endif
