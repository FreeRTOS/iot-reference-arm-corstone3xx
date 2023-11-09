/* Copyright 2022-2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ETHOS_U55_H_
#define ETHOS_U55_H_

#define EthosU_IRQn               ( 16 )         /* Ethos-Uxx Interrupt */
#define SEC_ETHOS_U55_BASE        ( 0x40004000 ) /* Ethos-U55 base address*/
#define SEC_ETHOS_U55_TA0_BASE    ( 0x48103000 ) /* Ethos-U55's timing adapter 0 base address */
#define SEC_ETHOS_U55_TA1_BASE    ( 0x48103200 ) /* Ethos-U55's timing adapter 1 base address */

/* TA0 configuration */
/* Disabled for CS310 as the TA is not available */
/* #define TA0_BASE      SEC_ETHOS_U55_TA0_BASE */
#define TA0_MAXR         0
#define TA0_MAXW         0
#define TA0_MAXRW        0
#define TA0_RLATENCY     0
#define TA0_WLATENCY     0
#define TA0_PULSE_ON     0
#define TA0_PULSE_OFF    0
#define TA0_BWCAP        0
#define TA0_PERFCTRL     0
#define TA0_PERFCNT      0
#define TA0_MODE         1
#define TA0_HISTBIN      0
#define TA0_HISTCNT      0

/* TA1 configuration */
/* Disabled for CS310 as the TA is not available */
/* #define TA1_BASE      SEC_ETHOS_U55_TA1_BASE */
#define TA1_MAXR         0
#define TA1_MAXW         0
#define TA1_MAXRW        0
#define TA1_RLATENCY     0
#define TA1_WLATENCY     0
#define TA1_PULSE_ON     0
#define TA1_PULSE_OFF    0
#define TA1_BWCAP        0
#define TA1_PERFCTRL     0
#define TA1_PERFCNT      0
#define TA1_MODE         1
#define TA1_HISTBIN      0
#define TA1_HISTCNT      0

#endif /* ETHOS_U55_H_ */
