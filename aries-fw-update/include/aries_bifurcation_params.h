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
 * @file aries_bifurcation_params.h
 * @brief Definition of structs used to define bifurcation settings in Aries
 */

#ifndef ASTERA_ARIES_BIFURCATION_PARAMS_H_
#define ASTERA_ARIES_BIFURCATION_PARAMS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Struct defining paramaters for a given link inside link set
*/
typedef struct AriesBifurcationLinkParams
{
    int startLane;  /**< Start lane for a link */
    int linkWidth;  /**< Width of a this link */
    int linkId;     /**< Link num inside link set */
} AriesBifurcationLinkParamsType;


/**
 * @brief Struct defining paramaters for the entire link
*/
typedef struct AriesBifurcationParams
{
    int numLinks; /**< Num links part of this link set */
    // Max links inside a link set is 8
    AriesBifurcationLinkParamsType links[8]; /**< Link properties in link set */
} AriesBifurcationParamsType;

#endif // ASTERA_ARIES_BIFURCATION_PARAMS_H_

#ifdef __cplusplus
}
#endif
