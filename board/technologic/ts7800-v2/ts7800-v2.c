/*
 * Copyright (C) 2017 Mark Featherston <mark@embeddedarm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <watchdog.h>

#include "../drivers/ddr/mv-ddr-marvell/ddr3_init.h"
#include <../serdes/a38x/high_speed_env_spec.h>

#include "tsfpga.h"

DECLARE_GLOBAL_DATA_PTR;

#define TS7800_V2_SYSCON_BASE   (MBUS_PCI_MEM_BASE + 0x100000)
#define TS7800_V2_SYSCON_SIZE   0x48

#define ETH_PHY_CTRL_REG		0
#define ETH_PHY_CTRL_POWER_DOWN_BIT	11
#define ETH_PHY_CTRL_POWER_DOWN_MASK	(1 << ETH_PHY_CTRL_POWER_DOWN_BIT)

#define BOARD_GPP_OUT_ENA_LOW	0xffffffff
#define BOARD_GPP_OUT_ENA_MID	0xffffffff

#define BOARD_GPP_OUT_VAL_LOW	0x0
#define BOARD_GPP_OUT_VAL_MID	0x0
#define BOARD_GPP_POL_LOW	0x0
#define BOARD_GPP_POL_MID	0x0

static struct serdes_map board_serdes_map[] = {
	{PEX0, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{USB3_HOST0, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{SATA1, SERDES_SPEED_3_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{USB3_HOST1, SERDES_SPEED_5_GBPS, SERDES_DEFAULT_MODE, 0, 0},
	{PEX1, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
	{PEX2, SERDES_SPEED_5_GBPS, PEX_ROOT_COMPLEX_X1, 0, 0},
};

int hws_board_topology_load(struct serdes_map **serdes_map_array, u8 *count)
{
	*serdes_map_array = board_serdes_map;
	*count = ARRAY_SIZE(board_serdes_map);
	return 0;
}

static struct mv_ddr_topology_map ts7800_noecc_topology_map = {
	DEBUG_LEVEL_ERROR,
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    MV_DDR_DEV_WIDTH_16BIT,	/* sdram device width */
	    MV_DDR_DIE_CAP_4GBIT,	/* die capacity */
	    DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    MV_DDR_TEMP_LOW} },		/* temperature */
	BUS_MASK_32BIT,			/* subphys mask */
	MV_DDR_CFG_DEFAULT,		/* ddr configuration data source */
	{ {0} },			/* raw spd data */
	{0}				/* timing parameters */
};

struct mv_ddr_topology_map *mv_ddr_topology_map_get(void)
{
	/* Return the board topology as defined in the board code */
	return &ts7800_noecc_topology_map;
}

int board_early_init_f(void)
{
   /**
   0x11111111
      MPP[0],  UART0_RXD, 1
      MPP[1],  UART0_TXD, 1
      MPP[2],  I2C_0_CLK, 1
      MPP[3],  I2C_0_DAT, 1
      MPP[4],  MPP4_GE_MDC, 1
      MPP[5],  MPP5_GE_MDIO, 1
      MPP[6],  MPP6_GE_TXCLK, 1
      MPP[7],  MPP7_GE_TXD0, 1 
   0x11111111
      MPP[8],  MPP8_GE_TXD1, 1
      MPP[9],  MPP9_GE_TXD2, 1
      MPP[10], MPP10_GE_TXD3, 1
      MPP[11], MPP11_GE_TXCTL, 1
      MPP[12], MPP12_GE_RXD0, 1
      MPP[13], MPP13_GE_RXD1, 1
      MPP[14], MPP14_GE_RXD2, 1
      MPP[15], MPP15_GE_RXD3, 1
   0x11000011
      MPP[16], MPP16_GE_RXCTL, 1
      MPP[17], MPP17_GE_RXCLK, 1
      MPP[18], WIFI_IRQ#, 0
      MPP[19], CPU_IRQ, 0
      MPP[20], NC
      MPP[21], NC
      MPP[22], SPI_0_MOSI, 1
      MPP[23], SPI_0_CLK, 1 
   0x00001011
      MPP[24], SPI_0_MISO, 1
      MPP[25], SPI_0_BOOT_CS0#, 1
      MPP[26], NC
      MPP[27], SPI_0_WIFI_CS2#, 1
      MPP[28], SPI_0_CS3#, 0 (SPI CS not an available function on this pin, so use GPIO) 
      MPP[29], GE_PHY_INT#, 0
      MPP[30], CPU_SPEED_1, 0
      MPP[31], CPU_SPEED_2, 0
   0x00000000
      MPP[32], NC
      MPP[33], CPU_SPEED_0, 0
      MPP[34], CPU_SPEED_3, 0
      MPP[35], CPU_SPEED_4, 0
      MPP[36], CPU_TYPE_0, 0
      MPP[37], NC
      MPP[38], NC
      MPP[39], NC
   0x00000000
      MPP[40], NC
      MPP[41], NC
      MPP[42], EN_MMC_PWR, 0
      MPP[43], EN_FAN, 0
      MPP[44], CPU_TYPE_1, 0
      MPP[45], EN_USB_HOST_5V, 0
      MPP[46], FPGA_FLASH_SELECT, 0
      MPP[47], NC
   0x55010500
      MPP[48], NC
      MPP[49], ACCEL_2_INT, 0
      MPP[50], EMMC_CMD, 5
      MPP[51], SPREAD_SPECTRUM#, 0
      MPP[52], DETECT_MSATA, 1
      MPP[53], DETECT_9478, 0
      MPP[54], EMMC_D3, 5
      MPP[55], EMMC_D0, 5
   0x00005550
      MPP[56], NC
      MPP[57], EMMC_CLK, 5
      MPP[58], EMMC_D1, 5
      MPP[59], EMMC_D2, 5         
   
   */
   /* Configure MPP */
   writel(0x11111111, MVEBU_MPP_BASE + 0x00); /* 7:0 */
	writel(0x11111111, MVEBU_MPP_BASE + 0x04); /* 15:8 */
	writel(0x11000011, MVEBU_MPP_BASE + 0x08); /* 23:16 */
	writel(0x00001011, MVEBU_MPP_BASE + 0x0c); /* 31:24 */
	writel(0x00000000, MVEBU_MPP_BASE + 0x10); /* 39:32 */
	writel(0x00000000, MVEBU_MPP_BASE + 0x14); /* 47:40 */
	writel(0x55010500, MVEBU_MPP_BASE + 0x18); /* 55:48 */
	writel(0x00005550, MVEBU_MPP_BASE + 0x1c); /* 63:56 */
   
		/* Set GPP Out value */
	writel(BOARD_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(BOARD_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(BOARD_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(BOARD_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(BOARD_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(BOARD_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
	
#if (0)
	writel(0x11111111, MVEBU_MPP_BASE + 0x00); /* 7:0 */
	writel(0x11111111, MVEBU_MPP_BASE + 0x04); /* 15:8 */
	writel(0x11120001, MVEBU_MPP_BASE + 0x08); /* 23:16 */
	writel(0x22222211, MVEBU_MPP_BASE + 0x0c); /* 31:24 */
	writel(0x22220000, MVEBU_MPP_BASE + 0x10); /* 39:32 */
	writel(0x00000004, MVEBU_MPP_BASE + 0x14); /* 47:40 */
	writel(0x55000500, MVEBU_MPP_BASE + 0x18); /* 55:48 */
	writel(0x00005550, MVEBU_MPP_BASE + 0x1c); /* 63:56 */
	
	
	/* Set GPP Out value */
	writel(BOARD_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(BOARD_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(BOARD_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(BOARD_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(BOARD_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(BOARD_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
#endif

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int board_late_init(void)
{
	char tmp_buf[10];
	unsigned int syscon_reg;

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_revision", "P1");
	env_set("board_name", "TS-7800-V2");
	env_set("board_model", "7800-V2");
#endif

	writel(0, SOC_REGS_PHY_BASE + 0xa381c);   /* Test Configuration Reg in RTC */

	syscon_reg = fpga_peek32(0);
	
	printf("fpga_rev=0x%02X\n"
	       "board_id=0x%04X\n", syscon_reg & 0xFF, (syscon_reg >> 8) & 0xFFFF);
	
	snprintf(tmp_buf, sizeof(tmp_buf), 
	   "0x%02X", syscon_reg & 0xFF);       
	env_set("fpga_rev", tmp_buf);       
	snprintf(tmp_buf, sizeof(tmp_buf), 
	   "0x%04X", (syscon_reg >> 8) & 0xFFFF);       
	env_set("board_id", tmp_buf);
	
	syscon_reg = fpga_peek32(4);

	strcpy(tmp_buf, (syscon_reg & (1 << 30))?"on":"off");
	env_set("jp_sdboot", tmp_buf);
	
	strcpy(tmp_buf, (syscon_reg & (1 << 31))?"on":"off");
	env_set("jp_uboot", tmp_buf);

	hw_watchdog_init();

	return 0;
}

uint32_t board_rng_seed(void)
{
	fpga_peek32(0x44);
}

int checkboard(void)
{
	return 0;
}

int board_eth_init(bd_t *bis)
{
	return cpu_eth_init(bis);
}