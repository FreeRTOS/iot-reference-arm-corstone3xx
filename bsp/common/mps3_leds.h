/* Copyright 2023 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MPS3_LEDS_H__
    #define __MPS3_LEDS_H__

    #include <stdbool.h>

    #ifdef __cplusplus
    extern "C" {
    #endif

/* The goal of this file is to create a common LED API for different targets in order
 * create common example applications.
 */

/* Note: Currently this API doesn't handle the case when a peripheral is
 *       mapped to secure and non-secure region as well, but could be extended
 *       to select secure or non-secure one.
 */

/**
 * \brief Get the total number of LEDs available
 *        in the current platform, that can be
 *        read and written in one chunk.
 *
 * \return Available number of LEDs.
 */
    size_t mps3_leds_get_total_number( void );

/**
 * \brief Initializes the available LEDs on the target
 */
    void mps3_leds_init( void );

/**
 * \brief Set the state of the LEDs.
 *
 * \param[in] new_leds_state, State of the LEDs to be set.
 *                Every bit represents one physical LED.
 *
 * \return true if succeeded, false otherwise
 */
    bool mps3_leds_set_state( uint32_t new_leds_state );

/**
 * \brief Get the state of the LEDs.
 *
 * \return state of the LEDs
 *         Every bit represents one physical LED.
 */
    uint32_t mps3_leds_get_state( void );

/**
 * \brief Get the state of the specified LED pin.
 *
 * \param[in] led_number, number of the LED to be checked.
 * \return state of the LED pin
 *         Every bit represents one physical LED.
 */
    uint8_t mps3_leds_get_pin_state( uint8_t led_number );

/**
 * \brief Turn specified LEDs on.
 *
 * \param[in] leds, Positions of the LEDs to be turned on.
 *                Every bit represents one physical LED.
 *
 * \return true if succeeded, false otherwise
 */
    bool mps3_leds_turn_on( uint32_t leds );

/**
 * \brief Turn specified LEDs off.
 *
 * \param[in] leds, Positions of the LEDs to be turned off.
 *                Every bit represents one physical LED.
 *
 * \return true if succeeded, false otherwise
 */
    bool mps3_leds_turn_off( uint32_t leds );

/**
 * \brief Toggle specified LEDs.
 *
 * \param[in] leds, Positions of the LEDs to be toggled.
 *                Every bit represents one physical LED.
 *
 * \return true if succeeded, false otherwise
 */
    bool mps3_leds_toggle( uint32_t leds );

    #ifdef __cplusplus
    }
    #endif
#endif /* __MPS3_LEDS_H__ */
