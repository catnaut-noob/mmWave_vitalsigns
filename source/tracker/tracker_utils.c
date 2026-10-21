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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <kernel/dpl/HeapP.h>
#include <alg/gtrack/gtrack.h>

#include <source/mmwave_demo.h>
#include <source/mmw_cli.h>
#include <source/tracker/tracker_utils.h>

extern MmwDemo_MSS_MCB gMmwMssMCB;

unsigned int gGtrackMemoryUsed = 0;

/* ---- gtrack library hooks (called from inside libgtrack) ---- */
void *gtrack_alloc(unsigned int numElements, unsigned int sizeInBytes)
{
    gGtrackMemoryUsed += numElements * sizeInBytes;
    return HeapP_alloc(&gMmwMssMCB.CoreLocalTrackerHeapObj, numElements * sizeInBytes);
}

void gtrack_free(void *pFree, unsigned int sizeInBytes)
{
    gGtrackMemoryUsed -= sizeInBytes;
    HeapP_free(&gMmwMssMCB.CoreLocalTrackerHeapObj, pFree);
}

void gtrack_log(GTRACK_VERBOSE_TYPE level, const char *format, ...)
{
    (void)level;
    (void)format;
}

/* ---- sensor position -> tracker scenery config ---- */
void MmwDemo_FillTrackerSensorPositionCfg(void)
{
    memcpy(&gMmwMssMCB.trackerCfg.staticCfg.sceneryParams.sensorPosition,
           &gMmwMssMCB.sceneryParams.sensorPosition, sizeof(GTRACK_sensorPosition));
    memcpy(&gMmwMssMCB.trackerCfg.staticCfg.sceneryParams.sensorOrientation,
           &gMmwMssMCB.sceneryParams.sensorOrientation, sizeof(GTRACK_sensorOrientation));

    gMmwMssMCB.trackerCfg.staticCfg.sensorAzimuthTilt =
        gMmwMssMCB.trackerCfg.staticCfg.sceneryParams.sensorOrientation.azimTilt * 3.1415926f / 180.f;
    gMmwMssMCB.trackerCfg.staticCfg.sensorElevationTilt =
        gMmwMssMCB.trackerCfg.staticCfg.sceneryParams.sensorOrientation.elevTilt * 3.1415926f / 180.f;
    gMmwMssMCB.trackerCfg.staticCfg.sensorHeight =
        gMmwMssMCB.trackerCfg.staticCfg.sceneryParams.sensorPosition.z;
}

/* ---- sensorPosition CLI ---- */
int32_t MmwDemo_CLISensorPositionCfg(int32_t argc, char *argv[])
{
    if (argc != 6)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    gMmwMssMCB.sceneryParams.sensorPosition.x = (float)atof(argv[1]);
    gMmwMssMCB.sceneryParams.sensorPosition.y = (float)atof(argv[2]);
    gMmwMssMCB.sceneryParams.sensorPosition.z = (float)atof(argv[3]);
    gMmwMssMCB.sceneryParams.sensorOrientation.azimTilt = (float)atof(argv[4]);
    gMmwMssMCB.sceneryParams.sensorOrientation.elevTilt = (float)atof(argv[5]);
    return 0;
}

/* ---- trackingCfg ---- */
int32_t MmwDemo_CLITrackingCfg(int32_t argc, char *argv[])
{
    GTRACK_moduleConfig config;

    if (argc >= 2)
    {
        gMmwMssMCB.trackerCfg.staticCfg.trackerEnabled = (uint8_t)atoi(argv[1]);
    }
    if (gMmwMssMCB.trackerCfg.staticCfg.trackerEnabled != 1)
    {
        return 0;
    }
    /* Require at least: trackingCfg <enable> <paramSet> <numPoints> <numTracks> */
    if (argc < 5)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }

    memset((void *)&config, 0, sizeof(GTRACK_moduleConfig));
    gMmwMssMCB.trackerCfg.staticCfg.trackingParamSet = (uint32_t)atoi(argv[2]);

    config.stateVectorType          = GTRACK_STATE_VECTORS_3DA;
    config.verbose                  = GTRACK_VERBOSE_NONE;
    config.maxNumPoints             = (uint16_t)atoi(argv[3]);
    config.maxNumTracks             = (uint16_t)atoi(argv[4]);
    /* Optional args: <maxDoppler(0.1 m/s)> <framePeriod(ms)>; use defaults if absent */
    config.maxRadialVelocity        = (argc >= 6) ? ((float)atoi(argv[5]) * 0.1f) : 5.0f;
    config.deltaT                   = (argc >= 7) ? ((float)atoi(argv[6]) * 0.001f) : 0.1f;
    config.boresightFilteringEnable = 0;

    memcpy((void *)&gMmwMssMCB.trackerCfg.staticCfg.gtrackModuleConfig,
           (void *)&config, sizeof(GTRACK_moduleConfig));
    return 0;
}

/* ---- staticBoundaryBox ---- */
int32_t MmwDemo_CLIStaticBoundaryBoxCfg(int32_t argc, char *argv[])
{
    GTRACK_sceneryParams *sp = &gMmwMssMCB.trackerCfg.staticCfg.sceneryParams;
    if (argc != 7)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    sp->numStaticBoxes = 1;
    sp->staticBox[0].x1 = (float)atof(argv[1]);
    sp->staticBox[0].x2 = (float)atof(argv[2]);
    sp->staticBox[0].y1 = (float)atof(argv[3]);
    sp->staticBox[0].y2 = (float)atof(argv[4]);
    sp->staticBox[0].z1 = (float)atof(argv[5]);
    sp->staticBox[0].z2 = (float)atof(argv[6]);
    return 0;
}

/* ---- boundaryBox (1 or 2 boxes) ---- */
int32_t MmwDemo_CLIBoundaryBoxCfg(int32_t argc, char *argv[])
{
    GTRACK_sceneryParams *sp = &gMmwMssMCB.trackerCfg.staticCfg.sceneryParams;
    if (argc != 7 && argc != 13)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    sp->numBoundaryBoxes = 1;
    sp->boundaryBox[0].x1 = (float)atof(argv[1]);
    sp->boundaryBox[0].x2 = (float)atof(argv[2]);
    sp->boundaryBox[0].y1 = (float)atof(argv[3]);
    sp->boundaryBox[0].y2 = (float)atof(argv[4]);
    sp->boundaryBox[0].z1 = (float)atof(argv[5]);
    sp->boundaryBox[0].z2 = (float)atof(argv[6]);
    if (argc == 13)
    {
        sp->numBoundaryBoxes = 2;
        sp->boundaryBox[1].x1 = (float)atof(argv[7]);
        sp->boundaryBox[1].x2 = (float)atof(argv[8]);
        sp->boundaryBox[1].y1 = (float)atof(argv[9]);
        sp->boundaryBox[1].y2 = (float)atof(argv[10]);
        sp->boundaryBox[1].z1 = (float)atof(argv[11]);
        sp->boundaryBox[1].z2 = (float)atof(argv[12]);
    }
    return 0;
}

/* ---- gatingParam ---- */
int32_t MmwDemo_CLIGatingParamCfg(int32_t argc, char *argv[])
{
    GTRACK_gatingParams *gp = &gMmwMssMCB.trackerCfg.staticCfg.gatingParams;
    if (argc != 6)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    gp->gain          = (float)atof(argv[1]);
    gp->limits.width  = (float)atof(argv[2]);
    gp->limits.depth  = (float)atof(argv[3]);
    gp->limits.height = (float)atof(argv[4]);
    gp->limits.vel    = (float)atof(argv[5]);
    return 0;
}

/* ---- stateParam ---- */
int32_t MmwDemo_CLIStateParamCfg(int32_t argc, char *argv[])
{
    GTRACK_stateParams *st = &gMmwMssMCB.trackerCfg.staticCfg.stateParams;
    if (argc != 7)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    st->det2actThre     = (uint16_t)atoi(argv[1]);
    st->det2freeThre    = (uint16_t)atoi(argv[2]);
    st->active2freeThre = (uint16_t)atoi(argv[3]);
    st->static2freeThre = (uint16_t)atoi(argv[4]);
    st->exit2freeThre   = (uint16_t)atoi(argv[5]);
    st->sleep2freeThre  = (uint16_t)atoi(argv[6]);
    return 0;
}

/* ---- allocationParam ---- */
int32_t MmwDemo_CLIAllocationParamCfg(int32_t argc, char *argv[])
{
    GTRACK_allocationParams *ap = &gMmwMssMCB.trackerCfg.staticCfg.allocationParams;
    if (argc != 7)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    ap->snrThre         = (float)atof(argv[1]);
    ap->snrThreObscured = (float)atof(argv[2]);
    ap->velocityThre    = (float)atof(argv[3]);
    ap->pointsThre      = (uint16_t)atoi(argv[4]);
    ap->maxDistanceThre = (float)atof(argv[5]);
    ap->maxVelThre      = (float)atof(argv[6]);
    return 0;
}

/* ---- maxAcceleration ---- */
int32_t MmwDemoCLIMaxAccelerationParamCfg(int32_t argc, char *argv[])
{
    if (argc != 4)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    gMmwMssMCB.trackerCfg.staticCfg.accelerationParams[0] = (float)atof(argv[1]);
    gMmwMssMCB.trackerCfg.staticCfg.accelerationParams[1] = (float)atof(argv[2]);
    gMmwMssMCB.trackerCfg.staticCfg.accelerationParams[2] = (float)atof(argv[3]);
    gMmwMssMCB.trackerCfg.staticCfg.isCliCmdMaxAccelParamReceived = 1;
    return 0;
}

/* ---- presenceBoundaryBox ---- */
int32_t MmwDemo_CLIPresenceParamCfg(int32_t argc, char *argv[])
{
    GTRACK_presenceParams *pp = &gMmwMssMCB.trackerCfg.staticCfg.presenceParams;
    if (argc != 7)
    {
        CLI_write("Error: Invalid usage of the CLI command\n");
        return -1;
    }
    /* Presence thresholds hardcoded (matches reference behaviour) */
    pp->pointsThre        = 3;
    pp->velocityThre      = 0.5f;
    pp->on2offThre        = 10;
    pp->numOccupancyBoxes = 1;
    pp->occupancyBox[0].x1 = (float)atof(argv[1]);
    pp->occupancyBox[0].x2 = (float)atof(argv[2]);
    pp->occupancyBox[0].y1 = (float)atof(argv[3]);
    pp->occupancyBox[0].y2 = (float)atof(argv[4]);
    pp->occupancyBox[0].z1 = (float)atof(argv[5]);
    pp->occupancyBox[0].z2 = (float)atof(argv[6]);
    return 0;
}


