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
 * @file aries_a0_reg_defines.h
 * @brief Definition of register offsets used in Aries A0
 */

#ifndef ASTERA_ARIES_A0_REG_DEFINES_H
#define ASTERA_ARIES_A0_REG_DEFINES_H

///////////////////////////////////////////////////////////////
///////////////////////// Global Regs /////////////////////////
///////////////////////////////////////////////////////////////

/** Main Micro reg offset for EEPROM assist (data)*/
#define ARIES_MM_EEPROM_ASSIST_DATA_ADDR 0x410
/** Main Micro reg offset for EEPROM assist (cmd)*/
#define ARIES_MM_EEPROM_ASSIST_CMD_ADDR  0x920

/** EEPROM ic cmd reg offset*/
#define ARIES_I2C_MST_IC_CMD_ADDR 0xd04
/** EEPROM data0 reg offset*/
#define ARIES_I2C_MST_DATA0_ADDR  0xd05
/** EEPROM data1 reg offset*/
#define ARIES_I2C_MST_DATA1_ADDR  0xd06
/** EEPROM data2 reg offset*/
#define ARIES_I2C_MST_DATA2_ADDR  0xd07
/** EEPROM data3 reg offset*/
#define ARIES_I2C_MST_DATA3_ADDR  0xd08
/** EEPROM cmd reg offset*/
#define ARIES_I2C_MST_CMD_ADDR    0xd09

/** HW reset reg */
#define ARIES_HW_RST_ADDR  0x600

/** Main Micro Heartbeat reg */
#define ARIES_MM_HEARTBEAT_ADDR  0x923

/** Enable/Disable thermal shutdown */
#define ARIES_EN_THERMAL_SHUTDOWN 0x420

/** Start MM SRAM status reg */
#define ARIES_MM_SRAM_STATUS 0xcba

/** I2C_MST_BB_OUTPUT reg address */
#define ARIES_I2C_MST_BB_OUTPUT_ADDRESS 0xd0b

/** I2C_MST_INIT_CTRL reg address */
#define ARIES_I2C_MST_INIT_CTRL_ADDRESS 0xd0a

///////////////////////////////////////////////////////////////
////////////////////////// Main SRAM //////////////////////////
///////////////////////////////////////////////////////////////

/** AL Main SRAM DMEM offset (A0) */
#define AL_MAIN_SRAM_DMEM_OFFSET (64*1024)

/** AL Path SRAM DMEM offset (A0) */
#define AL_PATH_SRAM_DMEM_OFFSET (45*1024)

/** SRAM read command */
#define AL_TG_RD_LOC_IND_SRAM 0x16

/** SRAM write command */
#define AL_TG_WR_LOC_IND_SRAM 0x17


///////////////////////////////////////////////////////////
////////////////////////// Micros /////////////////////////
///////////////////////////////////////////////////////////

/** Offset for main micro FW info */
#define ARIES_MAIN_MICRO_FW_INFO (96*1024 - 128)

/** Offset for path micro FW info */
#define ARIES_PATH_MICRO_FW_INFO_ADDRESS (48*1024 - 256)

/** Link struct offset in Main Micro */
#define ARIES_LINK_0_MM_BASE_ADDR 0x10669

/** Link path struct size address*/
#define ARIES_LINK_PATH_STRUCT_SIZE_ADDR 0x17faa

/** Size of each link element in aries_link_path_struct */
#define ARIES_LINK_ADDR_EL_SIZE 2

/** Path micro link struct size */
#define ARIES_LINK_PATH_STRUCT_SIZE 38

///////////////////////////////////////////////////////////
/////////////////// Path Micro Members ////////////////////
///////////////////////////////////////////////////////////

/** FW Info (Major) offset location in struct */
#define ARIES_MM_FW_VERSION_MAJOR 0

/** FW Info (Minor) offset location in struct */
#define ARIES_MM_FW_VERSION_MINOR 1

/** FW Info (Build no.) offset location in struct */
#define ARIES_MM_FW_VERSION_BUILD 2

/** AL print info struct address (Main Micro) */
#define ARIES_MM_AL_PRINT_INFO_STRUCT_ADDR 4

/** Aries Link Struct Addr offset */
#define ARIES_MM_LINK_STRUCT_ADDR_OFFSET 10

/**AL print info struct address (Path Micro) */
#define ARIES_PM_AL_PRINT_INFO_STRUCT_ADDR 4

/** GP Ctrl status struct address (Main Micro) */
#define ARIES_MM_GP_CTRL_STS_STRUCT_ADDR 6

/** GP Ctrl status struct address (Path Micro) */
#define ARIES_PM_GP_CTRL_STS_STRUCT_ADDR 6

/** Pma temp code offset location in struct */
#define ARIES_MM_PMA_TJ_ADC_CODE_OFFSET 26

/** Offset to enable LTSSM logger */
#define ARIES_PRINT_INFO_STRUCT_PRINT_EN_OFFSET 0

/** Offset to enable one batch mode inside LTSSM logger */
#define ARIES_PRINT_INFO_STRUCT_ONE_BATCH_MODE_EN_OFFSET 2

/** Offset to enable one batch write inside LTSSM logger */
#define ARIES_PRINT_INFO_STRUCT_ONE_BATCH_WR_EN_OFFSET 3

/** Offset to get write_offset inside LTSSM logger */
#define ARIES_PRINT_INFO_STRUCT_WR_PTR_OFFSET 5

/** Offset to get write_offset inside LTSSM logger */
#define ARIES_PRINT_INFO_STRUCT_LNK_RECOV_ENTRIES_PTR_OFFSET 7

/** Offset to get print_class_en inside LTSSM logger for Path Micro */
#define ARIES_PM_PRINT_INFO_STRUCT_PRINT_CLASS_EN_OFFSET 7

/** Offset to get print_class_en inside LTSSM logger for Main Micro */
#define ARIES_MM_PRINT_INFO_STRUCT_PRINT_CLASS_EN_OFFSET 15

/** Offset to get print buffer inside LTSSM logger */
#define ARIES_PRINT_INFO_STRUCT_PRINT_BUFFER_OFFSET 23

/** Offset to get FW state inside GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_FW_STATE 0

/** Offset to get last speed inside GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_PCIE_GEN 2

/** Offset to get last preset reqs for lane 0 inside GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_PRESET_REQS_LN0 5

/** Offset to get last preset reqs for lane 1 inside GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_PRESET_REQS_LN1 15

/** Offset to get last FOMs for lane 0 inside GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FOMS_LN0 25

/** Offset to get last FOMs for lane 1 inside GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FOMS_LN1 35

/** Offset to get preset val for lane 0 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRESET_LN0 45

/** Offset to get preset val for lane 1 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRESET_LN1 46

/** Offset to get pre cursor val for lane 0 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRE_LN0 47

/** Offset to get pre cursor val for lane 1 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PRE_LN1 48

/** Offset to get cursor val for lane 0 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_CUR_LN0 49

/** Offset to get cursor val for lane 1 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_CUR_LN1 50

/** Offset to get post cursor val for lane 0 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PST_LN0 51

/** Offset to get post cursor val for lane 1 in GP_CTRL_STS_STRUCT */
#define ARIES_CTRL_STS_STRUCT_LAST_EQ_FINAL_REQ_PST_LN1 52

/** Offset to link width member in link struct*/
#define ARIES_LINK_STRUCT_WIDTH_OFFSET 1

/** Offset to detected link width member in link struct*/
#define ARIES_LINK_STRUCT_DETECTED_WIDTH_OFFSET 46

/** Offset to link rate member in link struct*/
#define ARIES_LINK_STRUCT_RATE_OFFSET 6

/** Offset to link state member in link struct*/
#define ARIES_LINK_STRUCT_STATE_OFFSET 10

/** Offset seperating 2 links in Aries Link struct */
#define ARIES_LINK_STRUCT_LINK_SEP_OFFSET 126


///////////////////////////////////////////////////////////
////////////////////// PMA registers //////////////////////
///////////////////////////////////////////////////////////

/** PMA Slice 0 Cmd register address*/
#define ARIES_PMA_QS0_CMD_ADDRESS     0x4400

/** PMA Slice 0 Address_1 register address*/
#define ARIES_PMA_QS0_ADDR_1_ADDRESS  0x4401

/** PMA Slice 0 Address_0 register address*/
#define ARIES_PMA_QS0_ADDR_0_ADDRESS  0x4402

/** PMA Slice 0 Data_0 register address*/
#define ARIES_PMA_QS0_DATA_0_ADDRESS  0x4403

/** PMA Slice 0 Data_1 register address*/
#define ARIES_PMA_QS0_DATA_1_ADDRESS  0x4404

/** PMA offset for SUP_DIG_MPLLB_OVRD_IN_0*/
#define ARIES_PMA_SUP_DIG_MPLLB_OVRD_IN_0  0x13

/** Reg offset for PMA reg LANE_DIG_ASIC_RX_OVRD_IN_0 */
#define ARIES_PMA_LANE_DIG_ASIC_TX_OVRD_IN_0 0x1001

/** Reg offset for PMA reg LANE_DIG_ASIC_RX_OVRD_IN_0 */
#define ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_0 0x1017

/** Reg offset for PMA reg LANE_DIG_ASIC_RX_OVRD_IN_3 */
#define ARIES_PMA_LANE_DIG_ASIC_RX_OVRD_IN_3 0x101a

/** Reg offset for PMA reg LANE_DIG_ASIC_RX_ASIC_IN_1 */
#define ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_IN_1 0x1029

/** Reg offset for PMA reg LANE_DIG_TX_LBERT_CTL */
#define ARIES_PMA_LANE_DIG_TX_LBERT_CTL 0x1072

/** Reg offset for PMA reg LANE_DIG_RX_LBERT_CTL */
#define ARIES_PMA_LANE_DIG_RX_LBERT_CTL 0x108c

/** Reg offset for PMA reg LANE_DIG_RX_DPLL_FREQ */
#define ARIES_PMA_LANE_DIG_RX_DPLL_FREQ_ADDRESS 0x1094

/** Reg offset for PMA register LANE_DIG_RX_ADPTCTL_ATT_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_ATT_STATUS 0x10ab

/** Reg offset for PMA register LANE_DIG_RX_ADPTCTL_VGA_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_VGA_STATUS 0x10ac

/** Reg offset for PMA register LANE_DIG_RX_ADPTCTL_CTLE_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_CTLE_STATUS 0x10ad

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP1_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP1_STATUS 0x10ae

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP2_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP2_STATUS 0x10af

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP3_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP3_STATUS 0x10b0

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP4_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP4_STATUS 0x10b1

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP5_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP5_STATUS 0x10b2

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP6_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP6_STATUS 0x10ce

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP7_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP7_STATUS 0x10cf

/** Reg offset for PMA register DIG_RX_ADPTCTL_DFE_TAP8_STATUS */
#define ARIES_PMA_LANE_DIG_RX_ADPTCTL_DFE_TAP8_STATUS 0x10d0

/** Reg offset for PMA register DIG_PCS_XF_RX_OVRD_IN_1 at Lane 0*/
#define ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_1 0x2006

/** Reg offset for PMA register DIG_PCS_XF_RX_OVRD_IN_7 at Lane 0*/
#define ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_7 0x200c

/** Reg offset for PMA register DIG_PCS_XF_RX_ADAPT_FOM at Lane 0 */
#define ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_ADAPT_FOM_ADDRESS 0x201b

/** Reg offset for PMA register RAWLANEN_DIG_RX_CTL_RX_ADAPT_MM_FOM at Lane 0 */
#define ARIES_PMA_RAWLANE_DIG_RX_CTL_RX_ADAPT_MM_FOM_ADDRESS 0x2109

/** Reg offset for PMA register DIG_PCS_XF_RX_OVRD_IN_9 at Lane 0*/
#define ARIES_PMA_RAWLANE_DIG_PCS_XF_RX_OVRD_IN_9 0x203a

/** Reg offset for PMA register DIG_RX_CTL_RX_ADAPT_MM_FOM at Lane 0 */
#define ARIES_RAWLANE_DIG_RX_CTL_RX_ADAPT_MM_FOM_ADDRESS 0x6109

/** Reg offset for PMA register DIG_RX_CTL_RX_ADAPT_MM_FOM at Lane 0 */
#define ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_OUT_0 0x1033

/** Reg offset for PMA register DIG_ASIC_TX_ASIC_IN_0 at Lane 0 */
#define ARIES_PMA_LANE_DIG_ASIC_TX_ASIC_IN_0 0x100d

/** Reg offset for PMA register DIG_ASIC_RX_ASIC_IN_0 at Lane 0 */
#define ARIES_PMA_LANE_DIG_ASIC_RX_ASIC_IN_0 0x1028

///////////////////////////////////////////////////////////
////////// Offsets for MM assisted PMA accesses ///////////
///////////////////////////////////////////////////////////

/** Reg offset to specify Address for MM assisted PMA Read */
#define ARIES_PMA_MM_ASSIST_REG_ADDR_OFFSET 0xd99

/** Reg offset to specify Path ID for MM assisted PMA Read */
#define ARIES_PMA_MM_ASSIST_PATH_ID_OFFSET 0xd9b

/** Reg offset to specify Command for MM assisted PMA Read */
#define ARIES_PMA_MM_ASSIST_DATA0_OFFSET 0xd9c

/** Reg offset to specify Command for MM assisted PMA Read */
#define ARIES_PMA_MM_ASSIST_CMD_OFFSET 0xd9d

/** Reg offset to specify Command for MM assisted PMA Read */
#define ARIES_PMA_MM_ASSIST_DATA1_OFFSET 0xd9e

/** Command to signify PMA read for side 0 */
#define ARIES_PMA_MM_ASSIST_SIDE0_RD 0x6

/** Command to signify PMA write for side 0 */
#define ARIES_PMA_MM_ASSIST_SIDE0_WR 0xf

///////////////////////////////////////////////////////////
////////////////////// Common offsets /////////////////////
///////////////////////////////////////////////////////////

/** Address offset between between lanes */
#define ARIES_PMA_LANE_STRIDE 0x200

/** Address offset between quad slices */
#define ARIES_QS_STRIDE 0x4000

/** Address offset between between path wrappers */
#define ARIES_PATH_WRP_STRIDE 0x1000

/** Address offset between between path lanes */
#define ARIES_PATH_LANE_STRIDE 0x400

///////////////////////////////////////////////////////////
//////////////////// Misc block offsets ///////////////////
///////////////////////////////////////////////////////////

/** Device Load check register */
#define ARIES_CODE_LOAD_REG 0x605

///////////////////////////////////////////////////////////
/////////////////// Hierarchical offsets //////////////////
///////////////////////////////////////////////////////////

////////////////////// AL Top CSRs ///////////////////////

/** AL Misc CSR Offset */
#define ARIES_MISC_CSR_OFFSET 0x0

/** AL Main Micro CSR Offset */
#define ARIES_MAIN_MICRO_CSR_OFFSET 0xc00

/** AL Quad Slice 0 Offset */
#define ARIES_QS_0_CSR_OFFSET 0x4000

/** AL Quad Slice 1 Offset */
#define ARIES_QS_1_CSR_OFFSET 0x8000

/** AL Quad Slice 2 Offset */
#define ARIES_QS_2_CSR_OFFSET 0xc000

/** AL Quad Slice 3 Offset */
#define ARIES_QS_3_CSR_OFFSET 0x10000

/////////////////// AL Quad Slice CSRs  ////////////////////

/** AL Path Wrapper 0 CSR Offset */
#define ARIES_PATH_WRAPPER_0_CSR_OFFSET 0x0

/** AL Path Wrapper 1 CSR Offset */
#define ARIES_PATH_WRAPPER_1_CSR_OFFSET 0x1000

/** AL Path Wrapper 2 CSR Offset */
#define ARIES_PATH_WRAPPER_2_CSR_OFFSET 0x2000

/** AL Path Wrapper 3 CSR Offset */
#define ARIES_PATH_WRAPPER_3_CSR_OFFSET 0x3000

/////////////////// AL Path Wrapper CSRs  ////////////////////

/** Path Micro CSR Offset */
#define ARIES_PATH_MICRO_CSR_OFFSET 0x0

/** Pac CSR Offset */
#define ARIES_PAC_CSR_OFFSET 0x400

/** Path Global CSR Offset */
#define ARIES_PATH_GLOBAL_CSR_OFFSET 0x600

/** AL Path Lane 0 CSR offset */
#define ARIES_PATH_LANE_0_CSR_OFFSET 0x800

/** AL Path Lane 1 CSR offset */
#define ARIES_PATH_LANE_1_CSR_OFFSET 0xc00

/////////////////// AL Path Global CSRs  ////////////////////

/** Reg offset for MAC-to-PHY rate and pclk_rate overrides */
#define ARIES_GBL_CSR_MAC_PHY_RATE_AND_PCLK_RATE 0xe

/////////////////// AL Path Lane X CSRs  ////////////////////

/** Captured Lane number reg offset */
#define ARIES_LN_CAPT_NUM 0x6

/** MAC-to-PHY Tx equalization setting override */
#define ARIES_MAC_PHY_TXDEEMPH 0x11c

/** MAC-to-PHY Rx polarity override */
#define ARIES_MAC_RX_POLARITY 0x121

/** Deskew delta in pclk cycles */
#define ARIES_DSK_CC_DELTA 0x160

/** Deskew status */
#define ARIES_DESKEW_STATUS 0x161

/** MAC-to-PHY Tx deemph idle observe */
#define ARIES_MAC_PHY_TXDEEMPH_OB 0x17c

/** RET_PTH Next State reg offset */
#define ARIES_RET_PTH_NEXT_STATE_OFFSET 0x61d

/** Reg offset for PIPE RxStandby */
#define ARIES_RET_PTH_LN_MAC_PHY_RXSTANDBY_ADDR 0x4922

/** Reg offset for PIPE Powerdown */
#define ARIES_RET_PTH_GBL_MAC_PHY_POWERDOWN_ADDR 0x4609

/** Reg offset for PIPE Powerdown */
#define ARIES_RET_PTH_GBL_MAC_PHY_RATE_AND_PCLK_RATE_ADDR 0x460e

/** Reg offset for PIPE Tx deemphasis */
#define ARIES_RET_PTH_LN_MAC_PHY_TXDEEMPH_ADDR 0x491c

/** Reg offset for PIPE Tx deemphasis observe */
#define ARIES_RET_PTH_LN_MAC_PHY_TXDEEMPH_OB_ADDR 0x497c

/** Reg offset for PIPE Rx polarity */
#define ARIES_RET_PTH_LN_MAC_PHY_RXPOLARITY_ADDR 0x4921

/** Reg offset for PIPE Rx termination */
#define ARIES_RET_PTH_LN_PCS_RX_TERMINATION_ADDR 0x48d8

/** Reg offset for PIPE BlockAlignControl */
#define ARIES_RET_PTH_GBL_MAC_PHY_BLOCKALIGNCONTROL_ADDR 0x460c

/** Reg offset for PIPE FomFeedback */
#define ARIES_RET_PTH_LN_PHY_MAC_FOMFEEDBACK_ADDR 0x498c

/** Reg offset for PIPE RxEqEval */
#define ARIES_RET_PTH_LN_MAC_PHY_RXEQEVAL_ADDR 0x4927

#endif
