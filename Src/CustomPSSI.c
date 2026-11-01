/*
 * CustomPSSI.c
 *
 *  Created on: Oct 11, 2026
 *      Author: alexp
 */

#include "Custom_Drivers/CustomPSSI.h"

// Function to initialize the PSSI functionality for receiving in parallel.
// Sets up DMA, GPIO, and PSSI configurations
// Set for 16-bit half-words, 10 MHz ADC clock, rising-edge receiver
void PSSI_GPDMA1_INIT(uint16_t * sram_buffer){
	// Initialize clocks for GPIO, DMA, and PSSI
	RCC->AHB2ENR1 |= (1U << 3U) | (1U << 2U) | (1U << 4U) | (1U << 12U); // GPIOC:D:E, PSSIclock enable
	RCC->AHB1ENR |= (1U << 0U); // GPDMA1 clock enable

	/* Initialize GPIO pins */
	GPIOD->MODER &= 0x00000000; GPIOD->AFR[0] &= 0x00000000; GPIOD->AFR[1] &= 0x00000000; // Clear initial register

	// D[0:11] pins as alternate function mode for PSSI_D,
	GPIOC->MODER |= (2U << 2U*6U); // PC6 D0
	GPIOC->MODER |= (2U << 2U*7U); // PC7 D1
	GPIOC->MODER |= (2U << 2U*8U); // PC8 D2
	GPIOC->MODER |= (2U << 2U*9U); // PC9 D3
	GPIOC->MODER |= (2U << 2U*11U); // PC11 D4
	GPIOD->MODER |= (2U << 2U*3U); // PD3 D5
	GPIOE->MODER |= (2U << 2U*5U); // PE5 D6
	GPIOE->MODER |= (2U << 2U*6U); // PE6 D7
	GPIOC->MODER |= (2U << 2U*10U); // PC10 D8
	GPIOC->MODER |= (2U << 2U*12U); // PC12 D9
	//GPIOD->MODER |= (2U << 2U*10U); // PD6 D10
	//GPIOD->MODER |= (2U << 2U*11U); // PD2 D11

	// set alternate function mode to PSSI logic
	GPIOC->AFR[0] |= (0xAU << 4U*6U); // AF10 PC6 D0
	GPIOC->AFR[0] |= (0xAU << 4U*7U); // AF10 PC7 D1
	GPIOC->AFR[1] |= (0xAU << 4U*0U); // AF10 PC8 D2
	GPIOC->AFR[1] |= (0x4U << 4U*1U); // AF4 PC9 D3
	GPIOC->AFR[1] |= (0xAU << 4U*3U); // AF10 PC11 D4
	GPIOD->AFR[0] |= (0x4U << 4U*3U); // AF4 PD3 D5
	GPIOE->AFR[0] |= (0xAU << 4U*5U); // AF10 PE5 D6
	GPIOE->AFR[0] |= (0xAU << 4U*6U); // AF10 PE6 D7
	GPIOC->AFR[1] |= (0xAU << 4U*2U); // AF10 PC10 D8
	GPIOC->AFR[1] |= (0xAU << 4U*4U); // AF10 PC12 D9
	//GPIOD->AFR[0] |= (0x4U << 4U*6U); // AF4 PD6 D10
	//GPIOD->AFR[0] |= (0xAU << 4U*2U); // AF10 PD2 D11
	GPIOD->AFR[0] |= 0x01;

	// PDCK GPIO PD9
	GPIOD->MODER &= ~(3U << 2U*9U); // Set to input mode

	// OE GPIO PD1
	GPIOD->MODER |= (1U << 2U*1U); // Set to output mode
	GPIOD->OTYPER &= ~(1U << 1U); // Set to push-pull because unidirectional

	DMA_INIT(sram_buffer); // Config DMA
	PSSI_INIT(); // Config PSSI

}

// Enable respective DMA registers
void DMA_INIT(uint16_t * sram_buffer){
	// DMA READS FROM PSSIR_DR contains 4 bytes // all must include byte 0
	/* A DMA channel in the general-purpose
	DMA controller must be configured to perform transfers from/to PSSI_DR. */
	GPDMA1_Channel0->CCR &= ~(1U << 0U); // Disable DMA for register updating
	GPDMA1_Channel0->CCR |= (3U << 22U) | (1U << 8U) | (1U << 10U); // High-Priority channel // Enable TCIE, DTEIE
	GPDMA1_Channel0->CTR1 = (1U << 19U) | (2U << 16U) | (0U << 3U) | (2U << 0U) |
			(1U << 30U) | (0U << 14U); // Destination Increments, 32-bit dest data width, source fixed, 32-bit source data width, Dest Port 1, Source Port 0
	GPDMA1_Channel0->CTR2 = (0U << 31U) | (1U << 11U) | (1U << 9U) | (0x56U); // TCEM = 0, BREQ = 1, peripheral block level; SWREQ = 0, REQSEL used, REQSEL = 86 for PSSI
	GPDMA1_Channel0->CBR1 = (0x02U); //16-bit transfers or two bytes
	GPDMA1_Channel0->CSAR = (uint32_t) &PSSI->DR; // Source Address (PSSI data register)
	GPDMA1_Channel0->CDAR = (uint32_t) sram_buffer; // address of start SRAM block

	NVIC_EnableIRQ(GPDMA1_Channel0_IRQn); // Enable GPDMA interrupt vector

	GPDMA1_Channel0->CCR |= (1U << 0U); // enable DMA
}

// Enable respective PSSI registers
void PSSI_INIT(){
	/* Initialize PSSI Pins */
	PSSI->CR &= 0x00000000U; // Clear CR
	PSSI->CR &= ~(1U << 14U); // disable PSSI

	PSSI->CR |= (0U << 31U) | (1U << 30U) | (3U << 10U) |
			(1U << 5U); // PSSI receive mode, enable DMA, 16-bit mode, rising-edge input capture
	//PSSI->IER |= (1U << 1U); // enable overrun interrupt //:: MIGHT NOT NEED
	PSSI->CR |= (1U << 14U); // enable PSSI
}

// if GPDMA interrupts needed
void GPDMA1_Channel0_IRQHandler(void){
	// Complete transfer flag check

	// Transfer complete flag check
}
