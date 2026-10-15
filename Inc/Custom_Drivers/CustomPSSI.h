/*
 * CustomPSSI.h
 *
 *  Created on: Oct 10, 2026
 *      Author: alexp
 */

#ifndef INC_CUSTOM_DRIVERS_CUSTOMPSSI_H_
#define INC_CUSTOM_DRIVERS_CUSTOMPSSI_H_

#include "stm32u575xx.h"
#include <stdio.h>

#define PERIPH_BASE_C (0x40000000UL) // Peripheral base address
#define AHB1PERIPH_BASE_C (PERIPH_BASE + 0x00020000UL)
#define AHB2PERIPH_BASE_C (PERIPH_BASE + 0x02020000UL)
#define AHB3PERIPH_BASE_C (PERIPH_BASE + 0x06020000UL)
#define RCC_BASE_C (AHB3PERIPH_BASE + 0x0C00UL)
#define GPIOA_BASE_C (AHB2PERIPH_BASE + 0x00000UL)
#define GPIOB_BASE_C (AHB2PERIPH_BASE + 0x00400UL)
#define PSSI_BASE_C (AHB2PERIPH_BASE + 0x0C400UL)
#define GPDMA1_BASE_C (AHB1PERIPH_BASE)

// Function to initialize the PSSI functionality for receiving in parallel.
// Sets up DMA, GPIO, and PSSI configurations
// Set for 16-bit half-words, 10 MHz ADC clock, rising-edge receiver
void PSSI_GPDMA1_INIT();
void DMA_INIT(); // Enable respective DMA registers
void PSSI_INIT(); // Enable respective PSSI registers

#endif /* INC_CUSTOM_DRIVERS_CUSTOMPSSI_H_ */
