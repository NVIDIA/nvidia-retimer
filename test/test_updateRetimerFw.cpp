/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION &
 * AFFILIATES. All rights reserved. SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdint>
#include <fstream>
#include <iostream>

extern "C"
{
#include "updateRetimerFwOverI2C.h"
}

#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace phosphor
{
using ::testing::_;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrEq;

class TestFwupdate : public testing::Test
{
  public:
    TestFwupdate() {}

    ~TestFwupdate() {}
};

TEST_F(TestFwupdate, crc32)
{
    unsigned char* str = NULL;
    EXPECT_EQ(0, crc32(str, 0));

    // single_char
    unsigned char str1[] = "a";

    EXPECT_EQ(3904355907, crc32(str1, 1));

    // random_file
    unsigned char str2[] = "dsafgkhdfhskgsdf";

    EXPECT_EQ(3709972603, crc32(str2, strlen(((const char*)str))));
}

TEST_F(TestFwupdate, checkDigit_i2c)
{
    // empty_file
    char* ss = NULL;

    EXPECT_EQ(1, checkDigit_i2c((ss)));

    //  lower boundary
    char ss1[] = "0";

    EXPECT_EQ(0, checkDigit_i2c(ss1));

    // upper boundary
    char ss2[] = "13";

    EXPECT_EQ(1, checkDigit_i2c(ss2));
}

TEST_F(TestFwupdate, checkDigit_retimer)
{
    // empty_file
    char ss[] = "0";

    EXPECT_EQ(0, checkDigit_retimer(ss));

    //  lower boundary
    char ss1[] = "8";

    EXPECT_EQ(0, checkDigit_retimer(ss1));

    // upper boundary
    char ss2[] = "9";

    EXPECT_EQ(1, checkDigit_retimer(ss2));
}

TEST_F(TestFwupdate, checkfpgaready) {}

TEST_F(TestFwupdate, read_fwVersion) {}

unsigned char* readfile(std::string filePath, size_t& len)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[fileSize];
    file.read(buffer, fileSize);

    len = fileSize;
    return (unsigned char*)buffer;
}

TEST_F(TestFwupdate, parsecompositeimage)
{
    unsigned char* buf = (unsigned char*)calloc(1, 2048);
    // Test 1: Test bare image
    update_operation* update_ops = NULL;
    int update_ops_count = 0;
    int ret = 0;
    ret = parseCompositeImage(buf, 2048, "pldm version string", &update_ops,
                              &update_ops_count);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(update_ops_count, 1);
    std::string uoVersionStringObj;
    if (update_ops)
    {
        EXPECT_EQ(update_ops[0].applyBitmap, 0xFF);
        EXPECT_EQ(update_ops[0].imageCrc, 0x86A2E870);
        EXPECT_EQ(update_ops[0].imageLength, 2048);
        EXPECT_EQ(update_ops[0].startOffset, 0);
        uoVersionStringObj = update_ops[0].versionString;
        EXPECT_EQ(uoVersionStringObj, "pldm version string");
        free(update_ops);
    }

    // Test 2: Test bad image with UUID but nothing else
    memcpy(buf, CompositeImageHeaderUuid, sizeof(CompositeImageHeaderUuid));
    ret = parseCompositeImage(buf, 2048, "pldm version string", &update_ops,
                              &update_ops_count);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(update_ops_count, 0);
    EXPECT_FALSE(update_ops);

    // Test 3: Test valid CompositeImageHeader but file length does not match
    const unsigned char mph[] = {
        0x8C, 0x28, 0xD7, 0x7A, 0x97, 0x07, 0x43, 0xD7, 0xBC, 0x13,
        0xC1, 0x2B, 0x3A, 0xBB, 0x4B, 0x87, 0x01, 0x00, 0x08, 0x00,
        0x28, 0x02, 0x20, 0x00, 0x1D, 0xFA, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF2, 0xCA, 0x55, 0x72};
    memcpy(buf, mph, sizeof(mph));
    ret = parseCompositeImage(buf, 2048, "pldm version string", &update_ops,
                              &update_ops_count);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(update_ops_count, 0);
    EXPECT_FALSE(update_ops);

    // Test 4: Test CompositeImageHeader incorrect CRC
    const unsigned char mph2[] = {
        0x8C, 0x28, 0xD7, 0x7A, 0x97, 0x07, 0x43, 0xD7, 0xBC, 0x13,
        0xC1, 0x2B, 0x3A, 0xBB, 0x4B, 0x87, 0x01, 0x00, 0x08, 0x00,
        0x28, 0x02, 0x20, 0x00, 0x1D, 0xFA, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00};
    memcpy(buf, mph2, sizeof(mph2));
    ret = parseCompositeImage(buf, 2048, "pldm version string", &update_ops,
                              &update_ops_count);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(update_ops_count, 0);
    EXPECT_FALSE(update_ops);

    // All tests using buf are done, free it
    free(buf);

    // Test 5: valid 8-component image
    size_t fwLen = 0;
    unsigned char* fw = nullptr;
    fw = readfile("./test-composite-8-components.bin", fwLen);
    if (fw)
    {
        ret = parseCompositeImage(fw, fwLen, "pldm version string", &update_ops,
                                  &update_ops_count);
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(update_ops_count, 8);
        if (update_ops)
        {
            for (int i = 0; i < update_ops_count; i++)
            {
                EXPECT_EQ(update_ops[i].applyBitmap, 1 << i);
                EXPECT_EQ(update_ops[i].imageCrc, 0x8E7869CC);
                EXPECT_EQ(update_ops[i].imageLength, 0x40000);
                EXPECT_EQ(update_ops[i].startOffset,
                          sizeof(CompositeImageHeader) +
                              8 * sizeof(ComponentHeader) + i * 0x40000);
                uoVersionStringObj = update_ops[i].versionString;
                EXPECT_EQ(uoVersionStringObj, "2.9.7");
            }
            free(update_ops);
        }
        free(fw);
    }

    // Test 6: invalid image, valid MPH hCRC but not others
    fw = readfile("./test-composite-invalid-ComponentHeaders.bin", fwLen);
    if (fw)
    {
        ret = parseCompositeImage(fw, fwLen, "pldm version string", &update_ops,
                                  &update_ops_count);
        EXPECT_NE(ret, 0);
        EXPECT_EQ(update_ops_count, 0);

        free(fw);
    }
}

TEST_F(TestFwupdate, copy_image_to_fpga) {}

TEST_F(TestFwupdate, check_writeNackError)
{
    // empty_file
    char ss[] = "0";

    EXPECT_EQ(0, checkDigit_retimer(ss));

    //  lower boundary
    char ss1[] = "8";

    EXPECT_EQ(0, checkDigit_retimer(ss1));

    // upper boundary
    char ss2[] = "9";

    EXPECT_EQ(1, checkDigit_retimer(ss2));
}

TEST_F(TestFwupdate, readFwVerionOverSMBPBI) {}

TEST_F(TestFwupdate, startRetimerFwUpdate) {}

TEST_F(TestFwupdate, readRetimerfw) {}

} // namespace phosphor
