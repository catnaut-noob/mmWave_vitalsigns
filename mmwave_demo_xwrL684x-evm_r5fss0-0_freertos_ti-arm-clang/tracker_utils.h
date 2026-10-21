/*
 * Copyright (C) 2024-25 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * tracker_utils.h
 *
 * Group tracker (gtrack 3D) integration helpers for the iwrl6844 mmwave_demo.
 * Provides the gtrack memory-allocation hooks and the CLI command handlers
 * that populate gMmwMssMCB.trackerCfg.
 */

#ifndef TRACKER_UTILS_H
#define TRACKER_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Size of the local RAM region backing the gtrack heap (bytes). */
#define MMWDEMO_TRACKER_HEAP_MEM_SIZE   (32U * 1024U)

/*! @brief TLV types streamed to the host for the tracker output.
 *  Values match the visualizer / SDK 3D people tracking convention. */
#define MMWDEMO_OUTPUT_MSG_TRACKERPROC_3D_TARGET_LIST    (1010U)
#define MMWDEMO_OUTPUT_MSG_TRACKERPROC_TARGET_INDEX      (1011U)

/* Note: the gtrack memory/logging hooks (gtrack_alloc/gtrack_free/gtrack_log)
 * are declared by <alg/gtrack/gtrack.h> and defined in tracker_utils.c. */

/* Copy sensorPosition/orientation into the tracker scenery config and
 * derive the demo tilt/height parameters. Call before tracker config. */
void  MmwDemo_FillTrackerSensorPositionCfg(void);

/* Tracker CLI command handlers */
int32_t MmwDemo_CLITrackingCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLIStaticBoundaryBoxCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLIBoundaryBoxCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLIGatingParamCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLIStateParamCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLIAllocationParamCfg(int32_t argc, char *argv[]);
int32_t MmwDemoCLIMaxAccelerationParamCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLIPresenceParamCfg(int32_t argc, char *argv[]);
int32_t MmwDemo_CLISensorPositionCfg(int32_t argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* TRACKER_UTILS_H */
