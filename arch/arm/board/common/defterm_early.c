/**
 * Copyright (c) 2014 Anup Patel.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @file defterm_early.c
 * @author Anup Patel (anup@brainfault.org)
 * @brief arch default terminal early functions
 */

#include <vmm_error.h>
#include <vmm_types.h>
#include <vmm_compiler.h>
#include <vmm_host_io.h>
#include <arch_defterm.h>

u8 __aligned(0x1000) defterm_early_base[0x1000];
static void *early_base = &defterm_early_base;

#if defined(CONFIG_DEFTERM_EARLY_PL011)

#include <drv/serial/pl011.h>

/*
 * PL011 single character TX.
 */
void __init arch_defterm_early_putc(u8 ch)
{
	while (vmm_readl(early_base + UART_PL011_FR) & UART_PL011_FR_TXFF)
		;
	vmm_writeb(ch, early_base + UART_PL011_DR);
	while (vmm_readl(early_base + UART_PL011_FR) & UART_PL011_FR_BUSY)
		;
}

#elif defined(CONFIG_DEFTERM_EARLY_UART8250_8BIT)

#include <drv/serial/8250-uart.h>

/*
 * 8250/16550 (8-bit aligned registers) single character TX.
 */
void __init arch_defterm_early_putc(u8 ch)
{
	while (!(vmm_readb(early_base + UART_LSR_OFFSET) & UART_LSR_THRE))
		;
	vmm_writeb(ch, early_base + UART_THR_OFFSET);
}

#elif defined(CONFIG_DEFTERM_EARLY_UART8250_32BIT)

#include <drv/serial/8250-uart.h>

/*
 * 8250/16550 (32-bit aligned registers) single character TX.
 */
void __init arch_defterm_early_putc(u8 ch)
{
	while (!(vmm_readl(early_base + (UART_LSR_OFFSET << 2)) & UART_LSR_THRE))
		;
	vmm_writel(ch, early_base + (UART_THR_OFFSET << 2));
}

#elif defined(CONFIG_DEFTERM_EARLY_IMX)

#include <drv/serial/imx-uart.h>

void __init arch_defterm_early_putc(u8 ch)
{
	/* Wait until FIFO is full */
	while (vmm_readl(early_base + IMX21_UTS) & UTS_TXFULL) ;

	/* Send the character */
	vmm_writel(ch, early_base + URTX0);

	/* Wait until FIFO is empty */
	while (!(vmm_readl(early_base + IMX21_UTS) & UTS_TXEMPTY)) ;
}

#elif defined(CONFIG_DEFTERM_EARLY_SCIF)

#include <drv/serial/scif.h>

void __init arch_defterm_early_putc(u8 ch)
{
	u16 scfsr;

	/* Wait until FIFO is not empty */
	while (!(vmm_readw(early_base + SCIF_SCFSR) & SCFSR_TEND)) ;

	/* Send the character */
	vmm_writeb(ch, early_base + SCIF_SCFTDR);

	/* Clear required TX flags */
	scfsr = vmm_readw(early_base + SCIF_SCFSR);
	scfsr &= ~(SCFSR_TEND | SCFSR_TDFE);
	vmm_writew(scfsr, early_base + SCIF_SCFSR);
}

#elif defined(CONFIG_DEFTERM_EARLY_BCM283X_MU)

#include <drv/serial/bcm283x_mu.h>

void __init arch_defterm_early_putc(u8 ch)
{
	void *io = early_base + BCM283X_MU_IO;
	void *lsr = early_base + BCM283X_MU_LSR;

	/* Wait until FIFO is not empty */
	while (!(vmm_readw(lsr) & BCM283X_MU_LSR_TX_EMPTY)) ;

	/* Send the character */
	vmm_writeb(ch, io);
}

#elif defined(CONFIG_DEFTERM_EARLY_ZYNQ_UART)

#include <drv/serial/zynq-uart.h>

void __init arch_defterm_early_putc(u8 ch)
{
	struct uart_zynq *reg = (struct uart_zynq *)early_base;

	/* Wait until FIFO is not empty */
	while (!(vmm_readw(&reg->channel_sts) & ZYNQ_UART_SR_TXEMPTY)) ;

	/* Send the character */
	vmm_writeb(ch, (void *)&reg->tx_rx_fifo);
}

#else

void __init arch_defterm_early_putc(u8 ch)
{
	(void)early_base;
}

#endif
