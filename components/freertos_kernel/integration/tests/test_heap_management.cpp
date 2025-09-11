/* Copyright 2025 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: MIT
 */

#include "gtest/gtest.h"
#include <stdint.h>

extern "C" {
#include "heap_management.h"
#include "alloc_fakes.h"
}

class TestHeapManagement : public ::testing::Test {
public:
    TestHeapManagement()
    {
        RESET_FAKE( test_malloc );
        RESET_FAKE( test_calloc );
        RESET_FAKE( test_free );
    }
};

TEST_F( TestHeapManagement, calls_malloc_and_returns_same_pointer )
{
    static uint8_t dummy[ 32 ];

    test_malloc_fake.return_val = dummy;
    void * allocated_ptr = pvPortMalloc( 32 );
    ASSERT_EQ( allocated_ptr, static_cast<void *>( dummy ) );

    EXPECT_EQ( test_malloc_fake.call_count, 1 );
    EXPECT_EQ( test_malloc_fake.arg0_val, static_cast<size_t>( 32 ) );
}

TEST_F( TestHeapManagement, calling_malloc_with_size_zero_may_return_null )
{
    test_malloc_fake.return_val = nullptr;
    void * allocated_ptr = pvPortMalloc( 0 );
    ASSERT_EQ( allocated_ptr, nullptr );

    EXPECT_EQ( test_malloc_fake.call_count, 1 );
    EXPECT_EQ( test_malloc_fake.arg0_val, static_cast<size_t>( 0 ) );
}

TEST_F( TestHeapManagement, calling_malloc_with_size_zero_may_return_pointer )
{
    static uint8_t dummy[ 1 ];

    test_malloc_fake.return_val = dummy;
    void * allocated_ptr = pvPortMalloc( 0 );
    ASSERT_EQ( allocated_ptr, static_cast<void *>( dummy ) );

    EXPECT_EQ( test_malloc_fake.call_count, 1 );
    EXPECT_EQ( test_malloc_fake.arg0_val, static_cast<size_t>( 0 ) );
}

TEST_F( TestHeapManagement, returns_null_when_malloc_fails )
{
    test_malloc_fake.return_val = nullptr;
    void * allocated_ptr = pvPortMalloc( 64 );
    ASSERT_EQ( allocated_ptr, nullptr );

    EXPECT_EQ( test_malloc_fake.call_count, 1 );
    EXPECT_EQ( test_malloc_fake.arg0_val, static_cast<size_t>( 64 ) );
}

TEST_F( TestHeapManagement, calls_free_with_same_pointer )
{
    static uint8_t dummy[ 16 ];
    void * allocated_ptr = static_cast<void *>( dummy );

    vPortFree( allocated_ptr );

    EXPECT_EQ( test_free_fake.call_count, 1 );
    EXPECT_EQ( test_free_fake.arg0_val, allocated_ptr );
}

TEST_F( TestHeapManagement, calls_free_with_null )
{
    vPortFree( nullptr );
    EXPECT_EQ( test_free_fake.call_count, 1 );
    EXPECT_EQ( test_free_fake.arg0_val, nullptr );
}

TEST_F( TestHeapManagement, calls_calloc_and_returns_same_pointer )
{
    static uint64_t dummy[ 4 ];

    test_calloc_fake.return_val = dummy;
    void * allocated_ptr = pvPortCalloc( 4, 8 );
    ASSERT_EQ( allocated_ptr, static_cast<void *>( dummy ) );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 4 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 8 );
}

TEST_F( TestHeapManagement, calling_calloc_with_num_zero_may_return_null )
{
    test_calloc_fake.return_val = nullptr;
    void * allocated_ptr = pvPortCalloc( 0, 8 );
    ASSERT_EQ( allocated_ptr, nullptr );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 0 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 8 );
}

TEST_F( TestHeapManagement, calling_calloc_with_num_zero_may_return_pointer )
{
    static uint8_t dummy[ 1 ];

    test_calloc_fake.return_val = dummy;
    void * allocated_ptr = pvPortCalloc( 0, 8 );
    ASSERT_EQ( allocated_ptr, static_cast<void *>( dummy ) );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 0 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 8 );
}

TEST_F( TestHeapManagement, calling_calloc_with_size_zero_may_return_null )
{
    test_calloc_fake.return_val = nullptr;
    void * allocated_ptr = pvPortCalloc( 8, 0 );
    ASSERT_EQ( allocated_ptr, nullptr );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 8 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 0 );
}

TEST_F( TestHeapManagement, calling_calloc_with_size_zero_may_return_pointer )
{
    static uint8_t dummy[ 1 ];

    test_calloc_fake.return_val = dummy;
    void * allocated_ptr = pvPortCalloc( 8, 0 );
    ASSERT_EQ( allocated_ptr, static_cast<void *>( dummy ) );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 8 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 0 );
}

TEST_F( TestHeapManagement, calling_calloc_zero_zero_may_return_null )
{
    test_calloc_fake.return_val = nullptr;
    void * allocated_ptr = pvPortCalloc( 0, 0 );
    ASSERT_EQ( allocated_ptr, nullptr );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 0 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 0 );
}

TEST_F( TestHeapManagement, calling_calloc_zero_zero_may_return_pointer )
{
    static uint8_t dummy[ 1 ];

    test_calloc_fake.return_val = dummy;
    void * allocated_ptr = pvPortCalloc( 0, 0 );
    ASSERT_EQ( allocated_ptr, static_cast<void *>( dummy ) );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 0 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 0 );
}

TEST_F( TestHeapManagement, returns_null_when_calloc_fails )
{
    test_calloc_fake.return_val = nullptr;
    void * allocated_ptr = pvPortCalloc( 16, 8 );
    ASSERT_EQ( allocated_ptr, nullptr );

    EXPECT_EQ( test_calloc_fake.call_count, 1 );
    EXPECT_EQ( test_calloc_fake.arg0_val, 16 );
    EXPECT_EQ( test_calloc_fake.arg1_val, 8 );
}

TEST_F( TestHeapManagement, xPortGetFreeHeapSize_returns_zero )
{
    ASSERT_EQ( xPortGetFreeHeapSize(), 0 );
}

TEST_F( TestHeapManagement, xPortGetMinimumEverFreeHeapSize_returns_zero )
{
    ASSERT_EQ( xPortGetMinimumEverFreeHeapSize(), 0 );
}
