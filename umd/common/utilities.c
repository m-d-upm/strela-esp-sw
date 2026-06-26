// Copyright 2025 CEI - UPM.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Juan Granja <juan.granja@upm.es>
// Milos Dordevic <milos.dordevic@upm.es>

#include "utilities.h"

#include <stdio.h>

//#define _POSIX_C_SOURCE 199309L
#include <time.h>

// https://man7.org/linux/man-pages/man2/clock_gettime.2.html
// https://stackoverflow.com/questions/7506952/understanding-the-different-clocks-of-clock-gettime

#define CLOCK_GETTIME_CLOCK_ID CLOCK_MONOTONIC_RAW
//#define CLOCK_GETTIME_CLOCK_ID CLOCK_PROCESS_CPUTIME_ID
//#define CLOCK_GETTIME_CLOCK_ID CLOCK_MONOTONIC

void examine_mem(void *buf_ptr, uint32_t offset, uint32_t size)
{
    uint8_t *ptr = (uint8_t *)buf_ptr;

    for (uint32_t i = 0; i < size; i += 2)
    {
        printf("\r%08x: ", offset + i);

        printf("%02x ", *(ptr + offset + i));

        if (i + 1 < size) {
            printf("%02x\n", *(ptr + offset + i + 1));
        } else {
            printf("--\n");
        }
        
        if (((i / 2) + 1) % 4 == 0) {
            printf("\r\n");
        }
    }
}

int validate_buffers(void *buf, void *gold, uint32_t size)
{
    int differences_counter = 0;

    uint8_t *buffer_1 = (uint8_t*)buf;
    uint8_t *buffer_2 = (uint8_t*)gold;

    for (int i = 0; i < size; i++)
    {
        if(buffer_1[i] != buffer_2[i])
        {
            ++differences_counter;
        }
    }

    if(differences_counter)
        printf("Buffer validation reported %d bytes differ...\r\n", differences_counter);
    else
        printf("Buffers validated successfully, no differences...\r\n");

    return differences_counter;
}


// https://stackoverflow.com/questions/5833094/get-a-timestamp-in-c-in-microseconds

/// Convert seconds to milliseconds
#define SEC_TO_MS(sec) ((sec) * 1000)
/// Convert seconds to microseconds
#define SEC_TO_US(sec) ((sec) * 1000000)
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec) * 1000000000)

/// Convert nanoseconds to seconds
#define NS_TO_SEC(ns)   ((ns) / 1000000000)
/// Convert nanoseconds to milliseconds
#define NS_TO_MS(ns)    ((ns) / 1000000)
/// Convert nanoseconds to microseconds
#define NS_TO_US(ns)    ((ns) / 1000)

/// Get a time stamp in milliseconds.
uint64_t millis()
{
    struct timespec ts;
    clock_gettime(CLOCK_GETTIME_CLOCK_ID, &ts);
    uint64_t ms = SEC_TO_MS((uint64_t)ts.tv_sec) + NS_TO_MS((uint64_t)ts.tv_nsec);
    return ms;
}

/// Get a time stamp in microseconds.
uint64_t micros()
{
    struct timespec ts;
    clock_gettime(CLOCK_GETTIME_CLOCK_ID, &ts);
    uint64_t us = SEC_TO_US((uint64_t)ts.tv_sec) + NS_TO_US((uint64_t)ts.tv_nsec);
    return us;
}

/// Get a time stamp in nanoseconds.
uint64_t nanos()
{
    struct timespec ts;
    clock_gettime(CLOCK_GETTIME_CLOCK_ID, &ts);
    uint64_t ns = SEC_TO_NS((uint64_t)ts.tv_sec) + (uint64_t)ts.tv_nsec;
    return ns;
}
