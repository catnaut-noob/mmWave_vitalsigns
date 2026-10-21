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
 * vital_signs_proc.h
 *
 * Integration layer that bridges the iwrl6844 mmwave_demo data path
 * (RangeProc HWA v1, radar cube format DPIF_RADARCUBE_FORMAT_2) with the
 * platform independent vital-signs DSP algorithms in vitalsign.c.
 *
 * This module is only exercised when the demo is running in the
 * "vital signs" data path mode (see MmwDemo_MSS_MCB.dataPathMode).
 */

#ifndef VITAL_SIGNS_PROC_H
#define VITAL_SIGNS_PROC_H

#include <stdint.h>
#include "vitalsign.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Data path mode: classic point cloud detection chain */
#define MMWDEMO_DATAPATH_MODE_POINT_CLOUD   (0U)

/*! @brief Data path mode: vital signs (breathing/heart rate) chain */
#define MMWDEMO_DATAPATH_MODE_VITAL_SIGNS   (1U)

/*! @brief TLV type used to stream the vital signs output to the host. */
#define MMWDEMO_OUTPUT_MSG_VITAL_SIGNS      (0x410U)

/**
 * @brief  Reset all persistent vital-signs state.
 *
 * Must be called once whenever a new sensor configuration is applied
 * (i.e. from DPC_Config), so that frame counters, mean buffers and
 * circular history buffers start from a known state.
 */
void VitalSigns_init(void);

/**
 * @brief  Run one frame of vital-signs processing.
 *
 * Reads the per-virtual-antenna 1D-range-FFT samples for the selected
 * range bins out of the radar cube (DPIF_RADARCUBE_FORMAT_2 layout),
 * accumulates them and, every REFRESH_RATE frames, estimates the
 * breathing and heart rate. Results land in the global vitalSignsOutput.
 *
 * @param[in] radarCubeBaseAddr  Base address of the 1D range FFT radar cube.
 * @param[in] targetRangeBin     Range bin where the subject is located.
 * @param[in] noTarget           1 if no target is present this frame, else 0.
 */
void VitalSigns_process(uintptr_t radarCubeBaseAddr,
                        uint16_t  targetRangeBin,
                        uint16_t  noTarget);

/**
 * @brief  Accessor for the latest vital signs output structure.
 */
vsFeature *VitalSigns_getOutput(void);

#ifdef __cplusplus
}
#endif

#endif /* VITAL_SIGNS_PROC_H */
