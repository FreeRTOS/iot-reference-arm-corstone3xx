/* Copyright 2009-2025 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Note: This file is based on Device/ARM/ARMCM55/Source/GCC/gcc_arm.ld from
 * the git tag 5.8.0 of the CMSIS_5 repository.
 */

/*
 *-------- <<< Use Configuration Wizard in Context Menu >>> -------------------
 */

/*---------------------- Flash Configuration ----------------------------------
  <h> Flash Configuration
    <o0> Flash Base Address <0x0-0xFFFFFFFF:8>
    <o1> Flash Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
  -----------------------------------------------------------------------------*/
FLASH_NS_ML_MODEL_PARTITION_SIZE = 0x100000;
FLASH_NS_PARTITION_SIZE = 0x240000;
FLASH_S_PARTITION_SIZE = 0x40000;
ROM_START = 0x28000000;
BL2_HEADER_SIZE = 0x400;
BL2_TRAILER_SIZE = 0xC00;
DDR_START = 0x60000000;
DDR_SIZE = 0x10000000;

__ROM_BASE = (ROM_START + FLASH_S_PARTITION_SIZE + BL2_HEADER_SIZE);
__ROM_SIZE = (FLASH_NS_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE);

__MODEL_BASE = (DDR_START + BL2_HEADER_SIZE);
__MODEL_SIZE = (FLASH_NS_ML_MODEL_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE);

__DDR_NS_PARTITION_BASE = (DDR_START + FLASH_NS_ML_MODEL_PARTITION_SIZE);
__DDR_NS_PARTITION_SIZE = (DDR_SIZE - FLASH_NS_ML_MODEL_PARTITION_SIZE);

/*--------------------- Embedded RAM Configuration ----------------------------
  <h> RAM Configuration
    <o0> RAM Base Address    <0x0-0xFFFFFFFF:8>
    <o1> RAM Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
 -----------------------------------------------------------------------------*/
PROVISIONING_SIZE = 0x1000;
__RAM_BASE = 0x21000000;
__RAM_SIZE = 0x00200000;
__RAM_S_SIZE = 0x20000;
__RAM_NS_BASE = __RAM_BASE + __RAM_S_SIZE;
__RAM_NS_SIZE = (__RAM_SIZE - __RAM_S_SIZE - PROVISIONING_SIZE);

/*--------------------- Stack / Heap Configuration ----------------------------
  <h> Stack / Heap Configuration
    <o0> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
    <o1> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
  </h>
  -----------------------------------------------------------------------------*/
__STACK_SIZE = 0x00002000;
__HEAP_SIZE  = 0x000C0000;

/*
 *-------------------- <<< end of configuration section >>> -------------------
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = __ROM_BASE, LENGTH = __ROM_SIZE
  RAM   (rwx) : ORIGIN = __RAM_NS_BASE, LENGTH = __RAM_NS_SIZE
  DDR   (rwx) : ORIGIN = __DDR_NS_PARTITION_BASE, LENGTH = __DDR_NS_PARTITION_SIZE
  MODEL (rwx) : ORIGIN = __MODEL_BASE, LENGTH = __MODEL_SIZE
}

/* Linker script to place sections and symbol values.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
ENTRY(Reset_Handler)

SECTIONS
{
  .text :
  {
    _vectors = .;
    KEEP(*(.vectors))
    *(.text*)

    . = ALIGN(4);
    __copy_table_start__ = .;
    LONG (LOADADDR(.data))
    LONG (ADDR(.data))
    LONG (SIZEOF(.data) / 4)
    __copy_table_end__ = .;

    /* .zero.table */
    . = ALIGN(4);
    __zero_table_start__ = .;
    LONG (ADDR(.bss))
    LONG (SIZEOF(.bss) / 4)
    __zero_table_end__ = .;

    KEEP(*(.init))
    KEEP(*(.fini))

    /* .ctors */
    *crtbegin.o(.ctors)
    *crtbegin?.o(.ctors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    *(SORT(.ctors.*))
    *(.ctors)

    /* .dtors */
    *crtbegin.o(.dtors)
    *crtbegin?.o(.dtors)
    *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    *(SORT(.dtors.*))
    *(.dtors)

    *(.rodata*)

    KEEP(*(.eh_frame*))
  } > FLASH

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } > FLASH

  __exidx_start = .;
  .ARM.exidx :
  {
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
  } > FLASH
  __exidx_end = .;

  /**
   * Location counter can end up 2byte aligned with narrow Thumb code but
   * __etext is assumed by startup code to be the LMA of a section in RAM
   * which must be 4byte aligned
   */
  __etext = ALIGN (4);

  .data : AT (__etext)
  {
    __data_start__ = .;
    *(vtable)
    *(.data)
    *(.data.*)

    . = ALIGN(4);
    /* preinit data */
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP(*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(4);
    /* init data */
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(4);
    /* finit data */
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);

    KEEP(*(.jcr*))
    . = ALIGN(4);
    *(tasks_share)
    /* All data end */
    __data_end__ = .;

  } > RAM

  model.bin :
  {
    . = ALIGN (16);
    __nn_model_start__ = .;
    /* nn model's default space */
    *(nn_model)
    __nn_model_end__ = .;
  } > MODEL

  ddr.bin :
  {
    __ddr_start__ = .;
    . = ALIGN(16);
    /* nn model's baked in input matrices */
    *(ifm)
    . = ALIGN (16);
    /* labels */
    *(labels)
    . = ALIGN (16);
    *(.bss.NoInit.activation_buf_sram)
    *(activation_buf_dram)
    . = ALIGN (4);
    /* Buffer for transferring VSI audio data from S to NS */
    *(.bss.NoInit.vsi_audio_buffer)
    . = ALIGN (16);
    *(.bss.NoInit.ethos_u_cache)
    __ddr_end__ = .;
  } > DDR

  .bss :
  {
    . = ALIGN(4);
    __bss_start__ = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end__ = .;
  } > RAM AT > RAM

  .heap (COPY) :
  {
    . = ALIGN(8);
    __end__ = .;
    PROVIDE(end = .);
    . = . + __HEAP_SIZE;
    . = ALIGN(8);
    __HeapLimit = .;
  } > RAM
  PROVIDE(__RAM_segment_used_end__ = __HeapLimit);

  .stack (ORIGIN(RAM) + LENGTH(RAM) - __STACK_SIZE) (COPY) :
  {
    . = ALIGN(8);
    __StackLimit = .;
    . = . + __STACK_SIZE;
    . = ALIGN(8);
    __StackTop = .;
  } > RAM
  PROVIDE(__stack = __StackTop);

  /* Check if data + heap + stack exceeds RAM limit */
  ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
