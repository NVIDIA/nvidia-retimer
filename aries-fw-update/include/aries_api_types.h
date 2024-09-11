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
 * @file aries_api_types.h
 * @brief Definition of enums and structs used by aries_api.
 */

#ifndef ASTERA_ARIES_SDK_API_TYPES_H_
#define ASTERA_ARIES_SDK_API_TYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Aries device part number enum.
 *
 * Enumeration of values for Aries products.
 */
typedef enum AriesDevicePart
{
    ARIES_PTX16 = 0x0, /**< PTx16xxx part numbers */
    ARIES_PTX08 = 0x1  /**< PTx08xxx part numbers */
} AriesDevicePartType;

/**
 * @brief Aries Bifurcation enum.
 *
 * Enumeration of values for Link bifurcation modes.
 */
typedef enum AriesBifurcation
{
    ARIES_PTX16_X16 = 0x0,              /**< PTx16 x16 mode */
    ARIES_PTX16_X8X8 = 0x3,             /**< PTx16 x8x8 mode */
    ARIES_PTX16_X8X4X4 = 0x4,           /**< PTx16 x8x4x4 mode */
    ARIES_PTX16_X4X4X8 = 0x5,           /**< PTx16 x4x4x8 mode */
    ARIES_PTX16_X4X4X4X4 = 0x6,         /**< PTx16 x4x4x4x4 mode */
    ARIES_PTX16_X2X2X2X2X2X2X2X2 = 0x7, /**< PTx16 x2x2x2x2x2x2x2x2 mode */
    ARIES_PTX08_X8 = 0x1,               /**< PTx08 x8 mode */
    ARIES_PTX08_X4X4 = 0x1c,            /**< PTx08 x4x4 mode */
    ARIES_PTX08_X2X2X4 = 0x1d,          /**< PTx08 x2x2x4 mode */
    ARIES_PTX08_X4X2X2 = 0x1e,          /**< PTx08 x4x2x2 mode */
    ARIES_PTX08_X2X2X2X2 = 0x1f,        /**< PTx08 x2x2x2x2 mode */
} AriesBifurcationType;

/**
 * @brief Aries Link state enum.
 *
 * Enumeration of values for Link states.
 */
typedef enum AriesLinkStateEnum
{
    ARIES_STATE_RESET = 0x0,            /**< Reset state */
    ARIES_STATE_PROTOCOL_RESET_0 = 0x1, /**< Protocol Reset state */
    ARIES_STATE_PROTOCOL_RESET_1 = 0x2, /**< Protocol Reset state */
    ARIES_STATE_PROTOCOL_RESET_2 = 0x3, /**< Protocol Reset state */
    ARIES_STATE_PROTOCOL_RESET_3 = 0x4, /**< Protocol Reset state */
    ARIES_STATE_DETECT_OR_DESKEW = 0x5, /**< Receiver Detect or Deskew state */
    ARIES_STATE_FWD = 0x6,              /**< Forwarding state (i.e. L0) */
    ARIES_STATE_EQ_P2_0 = 0x7,          /**< Equalization Phase 2 state */
    ARIES_STATE_EQ_P2_1 = 0x8,          /**< Equalization Phase 2 state */
    ARIES_STATE_EQ_P3_0 = 0x9,          /**< Equalization Phase 3 state */
    ARIES_STATE_EQ_P3_1 = 0xa,          /**< Equalization Phase 3 state */
    ARIES_STATE_HOT_PLUG = 0xb,         /**< Hot Plug state */
    ARIES_STATE_PROTOCOL_RESET_4 = 0xc, /**< Protocol Reset state */
    ARIES_STATE_OTHER = 0xd             /**< Other (unexpected) state */
} AriesLinkStateEnumType;

/**
 * @brief Aries LTSSM Logger verbosity enum.
 *
 * Enumeration of values to define verbosity.
 */
typedef enum AriesLTSSMVerbosity
{
    ARIES_LTSSM_VERBOSITY_HIGH = 0x1, /**< High verbosity */
} AriesLTSSMVerbosityType;

/**
 * @brief Aries LTSSM Logger type enum.
 *
 * Enumeration of values for the Link and Path log types.
 */
typedef enum
{
    ARIES_LTSSM_LINK_LOGGER = 0xff,    /**< Link-level logger */
    ARIES_LTSSM_DS_LN_0_1_LOG = 0x0,   /**< Downstream, lanes 0 & 1 */
    ARIES_LTSSM_DS_LN_2_3_LOG = 0x2,   /**< Downstream, lanes 2 & 3 */
    ARIES_LTSSM_DS_LN_4_5_LOG = 0x4,   /**< Downstream, lanes 4 & 5 */
    ARIES_LTSSM_DS_LN_6_7_LOG = 0x6,   /**< Downstream, lanes 6 & 7 */
    ARIES_LTSSM_DS_LN_8_9_LOG = 0x8,   /**< Downstream, lanes 8 & 9 */
    ARIES_LTSSM_DS_LN_10_11_LOG = 0xa, /**< Downstream, lanes 10 & 11 */
    ARIES_LTSSM_DS_LN_12_13_LOG = 0xc, /**< Downstream, lanes 12 & 13 */
    ARIES_LTSSM_DS_LN_14_15_LOG = 0xe, /**< Downstream, lanes 14 & 15 */
    ARIES_LTSSM_US_LN_0_1_LOG = 0x1,   /**< Upstream, lanes 0 & 1 */
    ARIES_LTSSM_US_LN_2_3_LOG = 0x3,   /**< Upstream, lanes 2 & 3 */
    ARIES_LTSSM_US_LN_4_5_LOG = 0x5,   /**< Upstream, lanes 4 & 5 */
    ARIES_LTSSM_US_LN_6_7_LOG = 0x7,   /**< Upstream, lanes 6 & 7 */
    ARIES_LTSSM_US_LN_8_9_LOG = 0x9,   /**< Upstream, lanes 8 & 9 */
    ARIES_LTSSM_US_LN_10_11_LOG = 0xb, /**< Upstream, lanes 10 & 11 */
    ARIES_LTSSM_US_LN_12_13_LOG = 0xd, /**< Upstream, lanes 12 & 13 */
    ARIES_LTSSM_US_LN_14_15_LOG = 0xf, /**< Upstream, lanes 14 & 15 */
} AriesLTSSMLoggerEnumType;

/**
 * @brief Enumeration of values for the SRAM Program memory check
 */
typedef enum AriesSramMemoryCheck
{
    ARIES_SRAM_MM_CHECK_IDLE = 0,        /**< Idle state */
    ARIES_SRAM_MM_CHECK_IN_PROGRESS = 1, /**< Memory self-check in progress */
    ARIES_SRAM_MM_CHECK_PASS = 2,        /**< Memory self-check has passed */
    ARIES_SRAM_MM_CHECK_FAIL = 3,        /**< Memory self-check has failed */
} AriesSramMemoryCheckType;

typedef enum AriesMaxDataRate
{
    ARIES_GEN1 = 1,
    ARIES_GEN2 = 2,
    ARIES_GEN3 = 3,
    ARIES_GEN4 = 4,
    ARIES_GEN5 = 5
} AriesMaxDataRateType;

typedef enum AriesPRBSPattern
{
    DISABLED = 0,
    LFSR31 = 1,
    LFSR23 = 2,
    LFSR23_ALT = 3,
    LFSR16 = 4,
    LFSR15 = 5,
    LFSR11 = 6,
    LFSR9 = 7,
    LFSR7 = 8,
    FIXED_PAT0 = 9,
    FIXED_PAT0_ALT = 10,
    FIXED_PAT0_ALT_PAD = 11
} AriesPRBSPatternType;

/**
 * @brief Struct defining pesudo port physical info
 */
typedef struct AriesPseudoPortPins
{
    char rxPin[9];          /**< Rx pin location */
    char txPin[9];          /**< Tx pin location */
    int rxPackageInversion; /**< Rx package inversion flag */
    int txPackageInsersion; /**< Tx package inversion flag */
} AriesPseudoPortPinsType;

/**
 * @brief Struct defining one set of pseudo port pins for a give lane
 */
typedef struct AriesPins
{
    int lane;                        /**< Lane number */
    AriesPseudoPortPinsType pinSet1; /**< pseudo port pins set upstream */
    AriesPseudoPortPinsType pinSet2; /**< pseudo port pins set downstream */
} AriesPinsType;

/**
 * @brief Struct defining Aries retimer device
 *
 */
typedef struct AriesDevice
{
    AriesI2CDriverType* i2cDriver; /**< I2C driver to Aries */
    AriesFWVersionType fwVersion;  /**< FW version loaded on Aries */
    int revNumber;                 /**< Aries revision num */
    int deviceId;                  /**< Aries device ID */
    int vendorId;                  /**< Aries vendor ID */
    int i2cBus;                    /**< I2C Bus connection to host BMC */
    uint8_t lotNumber[6];          /**< 6-byte silicon lot number */
    uint8_t chipID[12];            /**< 12-byte unique chip ID */
    bool arpEnable;    /**< Flag indicating if ARP is enabled or not */
    bool codeLoadOkay; /**< Flag to indicate if code load reg value is expected
                        */
    bool
        mmHeartbeatOkay; /**< Flag indicating if Main Micro heartbeat is good */
    int mm_print_info_struct_addr; /**< AL print info struct offset (Main Micro)
                                    */
    int pm_print_info_struct_addr; /**< AL print info struct offset (Path Micro)
                                    */
    int mm_gp_ctrl_sts_struct_addr; /**< GP ctrl status struct offset (Main
                                       Micro) */
    int pm_gp_ctrl_sts_struct_addr; /**< GP ctrl status struct offset (Path
                                       Micro) */
    int linkPathStructSize;         /** Num bytes in link path struct */
    uint8_t tempCalCodePmaA[4];     /**< temp calibration codes for PMA A */
    uint8_t tempCalCodePmaB[4];     /**< temp calibration codes for PMA B */
    uint8_t tempCalCodeAvg;         /**< average temp calibration code */
    float tempAlertThreshC;         /**< temp alert (celsius) */
    float tempWarnThreshC;          /**< temp warning (celsius) */
    float maxTempC;          /**< Max. temp seen across all temp sensors */
    float currentTempC;      /**< Current average temp across all sensors */
    uint8_t minLinkFoMAlert; /**< Min. FoM value expected */
    bool deviceOkay;         /**< Device state */
    bool overtempAlert;      /**< Over temp alert indicated by reg */
    AriesDevicePartType partNumber; /** Retimer part number - x16 or x8 */
    AriesPinsType pins[16];         /** Device physical pin information */
    uint16_t minDPLLFreqAlert;      /** Min. DPLL frequency expected */
    uint16_t maxDPLLFreqAlert;      /** Max. DPLL frequency expected */
} AriesDeviceType;

/**
 * @brief Struct defining detailed Transmitter status, including electrical
 * parameters.
 */
typedef struct AriesTxState
{
    int logicalLaneNum;    /**< Captured logical Lane number */
    char* physicalPinName; /**< Physical pin name */
    int elecIdleStatus;    /**< Electrical idle status */
    int de;                /**< Current de-emphasis value (Gen-1/2 and above) */
    int pre;               /**< Current pre-cursor value (Gen-3 and above) */
    int cur;               /**< Current main-cursor value (Gen-3 and above) */
    float pst;             /**< Current post-cursor value (Gen-3 and above) */
    int lastEqRate;    /**< Last speed (e.g. 4 for Gen4) at which EQ was done */
    int lastPresetReq; /**< If final request was a preset, this is the value */
    int lastPreReq; /**< If final request was a coefficient, this is the pre */
    int lastCurReq; /**< If final request was a coefficient, this is the cur */
    int lastPstReq; /**< If final request was a coefficient, this is the pst */
} AriesTxStateType;

/**
 * @brief Struct defining detailed Receiver status, including electrical
 * parameters.
 */
typedef struct AriesRxState
{
    int logicalLaneNum;    /**< Captured logical Lane number */
    char* physicalPinName; /**< Physical pin name */
    int termination;       /**< Termination enable (1) or disable (0) status */
    int polarity;          /**< Polarity normal (0) or inverted (1) status */
    float attdB;           /** Rx Attenuator, in DB */
    float vgadB; /** Rx VGA, in DB */ //
    float ctleBoostdB;                /**< CTLE boost, in dB */
    int ctlePole;                     /**< CTLE pole code setting */
    float dfe1;                       /**< DFE tap 1 value, in mV */
    float dfe2;                       /**< DFE tap 2 value, in mV */
    float dfe3;                       /**< DFE tap 3 value, in mV */
    float dfe4;                       /**< DFE tap 4 value, in mV */
    float dfe5;                       /**< DFE tap 5 value, in mV */
    float dfe6;                       /**< DFE tap 6 value, in mV */
    float dfe7;                       /**< DFE tap 7 value, in mV */
    float dfe8;                       /**< DFE tap 8 value, in mV */
    int FoM;                          /**< Current FoM value */
    int lastEqRate;    /**< Last speed (e.g. 4 for Gen4) at which EQ was done */
    int lastPresetReq; /**< Final preset request */
    int lastPresetReqFom;   /**< Final preset request FoM value */
    int lastPresetReqM1;    /**< Final-1 preset request */
    int lastPresetReqFomM1; /**< Final-1 preset request FoM value */
    int lastPresetReqM2;    /**< Final-2 preset request */
    int lastPresetReqFomM2; /**< Final-2 preset request FoM value */
    int lastPresetReqM3;    /**< Final-3 preset request */
    int lastPresetReqFomM3; /**< Final-3 preset request FoM value */
    uint16_t DPLLCode;      /**< Current DPLL code */
} AriesRxStateType;

/**
 * @brief Struct defining detailed Pseudo Port status, including electrical
 * parameters.
 *
 * Note that each Aries Link has two Pseudo Ports, and Upstream Pseudo Port
 * (USPP) and a Downstream Pseudo Port (DSPP).
 */
typedef struct AriesPseudoPortState
{
    AriesTxStateType txState[16]; /**< Detailed Tx state */
    AriesRxStateType rxState[16]; /**< Detailed Tx state */
    uint16_t minDPLLCode;         /**< Min DPLL code seen */
    uint16_t maxDPLLCode;         /**< Max DPLL code seen */
} AriesPseudoPortStateType;

/**
 * @brief Struct defining detailed "Retimer core" status.
 *
 * Note that the "Retimer core" is sandwiched between the Upstream and
 * Downstream Pseudo Ports.
 */
typedef struct AriesRetimerCoreState
{
    float usppTempC[16];     /**< USPP Lane temperature (in degrees Celsius) */
    float dsppTempC[16];     /**< DSPP Lane temperature (in degrees Celsius) */
    int dsDeskewNs[16];      /**< Downstream Path deskew (in nanoseconds) */
    int usDeskewNs[16];      /**< Upstream Path deskew (in nanoseconds) */
    bool usppTempAlert[16];  /**< USPP lane temperature alert */
    bool dsppTempAlert[16];  /**< DSPP lane temperature alert */
    int usppPathHWState[16]; /** Upstream Path HW state */
    int dsppPathHWState[16]; /** Downstream Path HW state */
    int usppPathFWState[16]; /** Upstream Path FW state */
    int dsppPathFWState[16]; /** Downstream Path FW state */
} AriesRetimerCoreStateType;

/**
 * @brief Struct defining detailed Link status, including electrical
 * parameters.
 *
 * Note that each Aries Retimer can support multiple multi-Lane Links.
 */
typedef struct AriesLinkState
{
    float usppSpeed;              /**< USPP speed */
    float dsppSpeed;              /**< DSPP speed */
    int width;                    /**< Current width of the Link */
    AriesLinkStateEnumType state; /**< Current Link state */
    int rate;                     /**< Current data rate (1=Gen1, 5=Gen5) */
    bool linkOkay;                /**< Link is healthy (true) or not (false) */
    AriesPseudoPortStateType usppState;  /**< Current USPP state */
    AriesRetimerCoreStateType coreState; /**< Current RT core state */
    AriesPseudoPortStateType dsppState;  /**< Current DSPP state */
    int linkMinFoM; /**< Minimum FoM value across all Lanes on USPP and DSPP */
    char* linkMinFoMRx; /** Receiver corresponding to minimum FoM */
    int recoveryCount;  /** Count of entries to Recovery since L0 at current
                           speed */
} AriesLinkStateType;

/**
 * @brief Struct defining an Aries Retimer Link configuration.
 *
 * Note that each Aries Retimer can support multiple multi-Lane Links.
 */
typedef struct AriesLinkConfig
{
    AriesDevicePartType partNumber; /**< Aries part number */
    int maxWidth;                   /**< Maximum Link width (e.g. 16) */
    int startLane;                  /**< Starting (physical) Lane number */
    float tempAlertThreshC; /**< Temperature alert threshold, in celsius */
    int linkId;             /**< Link identifier (starts from 0) */
} AriesLinkConfigType;

/**
 * @brief Struct defining an Aries Link.
 *
 * Note that each Aries Retimer can support multiple multi-Lane Links.
 */
typedef struct AriesLink
{
    AriesDeviceType* device;    /**< Pointer to device struct */
    AriesLinkConfigType config; /**< struct with link config settings */
    AriesLinkStateType state;   /**< struct with link stats returned back */
} AriesLinkType;

/**
 * @brief Struct defining an Aries LTSSM Logger entry.
 */
typedef struct AriesLTSSMEntry
{
    uint8_t data; /**< Data entry from LTSSM module */
    int logType;  /**< Log type i.e. if Main Mirco (0xff) or Path Micro ID */
    int offset;   /**< Logger offset inside LTSSM module */
} AriesLTSSMEntryType;

/**
 * @brief Struct defining an Aries EEPROM delta value
 */
typedef struct AriesEEPROMDelta
{
    int address;  /**< Data address in image */
    uint8_t data; /**< Data value */
} AriesEEPROMDeltaType;

#ifdef __cplusplus
}
#endif

#endif /* ASTERA_ARIES_SDK_API_TYPES_H_ */
