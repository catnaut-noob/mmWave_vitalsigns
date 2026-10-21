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

#include <stdint.h>
#include <string.h>
#include <math.h>

#include <drivers/hw_include/csl_complex_math_types.h>

#include <source/mmwave_demo.h>
#include <source/mmw_cli.h>
#include "vital_signs_proc.h"
#include "vitalsign.h"

extern MmwDemo_MSS_MCB gMmwMssMCB;

/* Persistent state owned by vitalsign.c */
extern vsFeature         vitalSignsOutput;
extern vsAntennaGeometry vitalSignsAntenna;
extern float             vsDataAngleFftOutBufLogAbsSum[VS_NUM_ANGLE_FFT * VS_NUM_ANGLE_FFT];
extern uint16_t          vsLastFramePeakIdxI;
extern uint16_t          vsLastFramePeakIdxJ;
extern uint16_t          heartHistIndex;
extern uint16_t          breathHistIndex;
extern uint16_t          vsMeanCntOffset0;
extern uint16_t          vsMeanCntOffset1;
extern cplxf_t           vsDataMeanBuf[80];
extern cplxf_t           vsDataPerFrame[VS_NUM_RANGE_SEL_BIN * VS_NUM_VIRTUAL_CHANNEL];

/* Counters consumed by MmwDemo_runVitalSigns (defined here, used by vitalsign.c) */
uint32_t vsLoop      = 0;
uint32_t vsDataCount = 0;
uint16_t vsRangeBin  = 0;
uint16_t indicateNoTarget = 1;

void VitalSigns_init(void)
{
    vsLoop           = 0;
    vsDataCount      = 0;
    vsRangeBin       = 0;
    indicateNoTarget = 1;

    vsLastFramePeakIdxI = 0;
    vsLastFramePeakIdxJ = 0;
    heartHistIndex      = 0;
    breathHistIndex     = 0;
    vsMeanCntOffset0    = 0;
    vsMeanCntOffset1    = 30;

    memset((void *)&vitalSignsOutput, 0, sizeof(vitalSignsOutput));
    memset((void *)&vsDataMeanBuf[0], 0, sizeof(vsDataMeanBuf));
    memset((void *)&vsDataAngleFftOutBufLogAbsSum[0], 0,
           sizeof(vsDataAngleFftOutBufLogAbsSum));
}

vsFeature *VitalSigns_getOutput(void)
{
    return &vitalSignsOutput;
}

/*
 * Populate vitalSignsAntenna from the active antenna geometry that the
 * point cloud chain already computes (MmwDemo_calcActiveAntennaGeometry).
 */
static void VitalSigns_fillAntennaGeometry(void)
{
    uint32_t i;

    for (i = 0; i < VS_NUM_VIRTUAL_CHANNEL; i++)
    {
        vitalSignsAntenna.vsActiveAntennaGeometryCfg[i].row =
            gMmwMssMCB.activeAntennaGeometryCfg.ant[i].row;
        vitalSignsAntenna.vsActiveAntennaGeometryCfg[i].col =
            gMmwMssMCB.activeAntennaGeometryCfg.ant[i].col;
    }

    vitalSignsAntenna.numAntCol     = gMmwMssMCB.numAntCol;
    vitalSignsAntenna.numAntRow     = gMmwMssMCB.numAntRow;
    vitalSignsAntenna.numRxAntennas = gMmwMssMCB.numRxAntennas;
    vitalSignsAntenna.numTxAntennas = gMmwMssMCB.numTxAntennas;
    vitalSignsAntenna.numRangeBins  = gMmwMssMCB.numRangeBins;
    /* FORMAT_2 stride: number of chirps per frame = chirpsInBurst * burstsInFrame */
    vitalSignsAntenna.numChirpsPerFrame =
        (uint32_t)gMmwMssMCB.mmWaveCfg.frameCfg.numOfChirpsInBurst *
        (uint32_t)gMmwMssMCB.mmWaveCfg.frameCfg.numOfBurstsInFrame;
}

void VitalSigns_process(uintptr_t radarCubeBaseAddr,
                        uint16_t  targetRangeBin,
                        uint16_t  noTarget)
{
    indicateNoTarget = noTarget;
    if (noTarget)
    {
        vsLoop = 0;
    }
    else
    {
        vsRangeBin = targetRangeBin;
    }

    VitalSigns_fillAntennaGeometry();

    /* The radar cube base address is passed straight through; the sample
     * extraction (which is FORMAT_2 specific) happens inside
     * MmwDemo_runVitalSigns(). */
    vsLoop = MmwDemo_runVitalSigns((uint32_t)radarCubeBaseAddr,
                                   indicateNoTarget, vsLoop, vitalSignsAntenna);
}
