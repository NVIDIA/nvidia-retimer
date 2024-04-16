/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
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

extern "C" {
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

class TestFwupdate : public testing::Test {
    public:
	TestFwupdate()
	{
	}

	~TestFwupdate()
	{
	}
};

TEST_F(TestFwupdate, crc32)
{
	unsigned char *str = NULL;
	EXPECT_EQ(0, crc32(str, 0));

	// single_char
	unsigned char str1[] = "a";

	EXPECT_EQ(3904355907, crc32(str1, 1));

	// random_file
	unsigned char str2[] = "dsafgkhdfhskgsdf";

	EXPECT_EQ(3709972603, crc32(str2, strlen(((const char *)str))));
}

TEST_F(TestFwupdate, checkDigit_i2c)
{
	// empty_file
	char *ss = NULL;

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

TEST_F(TestFwupdate, checkfpgaready)
{
}

TEST_F(TestFwupdate, read_fwVersion)
{
}

TEST_F(TestFwupdate, copy_image_to_fpga)
{
}

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

TEST_F(TestFwupdate, readFwVerionOverSMBPBI)
{
}

TEST_F(TestFwupdate, startRetimerFwUpdate)
{
}

TEST_F(TestFwupdate, readRetimerfw)
{
}

} // namespace phosphor
