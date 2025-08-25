// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2025, Alexandr Zubtsov <internal.hellhound@mainlining.org>
 */

#include <linux/device.h>
#include <linux/interconnect.h>
#include <linux/interconnect-provider.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <dt-bindings/interconnect/qcom,palawan-rpmh.h>

#include "bcm-voter.h"
#include "icc-common.h"
#include "icc-rpmh.h"
#include "palawan.h"

static const struct regmap_config icc_regmap_config = {
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32,
	.fast_io = true,
};

static struct qcom_icc_qosbox qhm_qspi_qos = {
	.num_ports = 1,
	.port_offsets = { 0xc000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qhm_qspi = {
	.name = "qhm_qspi",
	.id = PALAWAN_MASTER_QSPI_0,
	.channels = 1,
	.buswidth = 4,
	.qosbox = &qhm_qspi_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A1NOC_SNOC },
};

static struct qcom_icc_qosbox qhm_qup0_qos = {
	.num_ports = 1,
	.port_offsets = { 0xd000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qhm_qup0 = {
	.name = "qhm_qup0",
	.id = PALAWAN_MASTER_QUP_0,
	.channels = 1,
	.buswidth = 4,
	.qosbox = &qhm_qup0_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A1NOC_SNOC },
};

static struct qcom_icc_qosbox xm_sdc2_qos = {
	.num_ports = 1,
	.port_offsets = { 0x19000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node xm_sdc2 = {
	.name = "xm_sdc2",
	.id = PALAWAN_MASTER_SDCC_2,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &xm_sdc2_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A1NOC_SNOC },
};

static struct qcom_icc_node xm_ufs_mem = {
	.name = "xm_ufs_mem",
	.id = PALAWAN_MASTER_UFS_MEM,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A1NOC_SNOC },
};

static struct qcom_icc_qosbox xm_usb3_0_qos = {
	.num_ports = 1,
	.port_offsets = { 0x10000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node xm_usb3_0 = {
	.name = "xm_usb3_0",
	.id = PALAWAN_MASTER_USB3_0,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &xm_usb3_0_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A1NOC_SNOC },
};

static struct qcom_icc_qosbox qhm_qdss_bam_qos = {
	.num_ports = 1,
	.port_offsets = { 0x12000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qhm_qdss_bam = {
	.name = "qhm_qdss_bam",
	.id = PALAWAN_MASTER_QDSS_BAM,
	.channels = 1,
	.buswidth = 4,
	.qosbox = &qhm_qdss_bam_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A2NOC_SNOC },
};

static struct qcom_icc_qosbox qhm_qup1_qos = {
	.num_ports = 1,
	.port_offsets = { 0x13000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qhm_qup1 = {
	.name = "qhm_qup1",
	.id = PALAWAN_MASTER_QUP_1,
	.channels = 1,
	.buswidth = 4,
	.qosbox = &qhm_qup1_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A2NOC_SNOC },
};

static struct qcom_icc_qosbox qxm_crypto_qos = {
	.num_ports = 1,
	.port_offsets = { 0x15000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qxm_crypto = {
	.name = "qxm_crypto",
	.id = PALAWAN_MASTER_CRYPTO,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &qxm_crypto_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A2NOC_SNOC },
};

static struct qcom_icc_qosbox qxm_ipa_qos = {
	.num_ports = 1,
	.port_offsets = { 0x16000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qxm_ipa = {
	.name = "qxm_ipa",
	.id = PALAWAN_MASTER_IPA,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &qxm_ipa_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A2NOC_SNOC },
};

static struct qcom_icc_qosbox xm_qdss_etr_0_qos = {
	.num_ports = 1,
	.port_offsets = { 0x17000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node xm_qdss_etr_0 = {
	.name = "xm_qdss_etr_0",
	.id = PALAWAN_MASTER_QDSS_ETR,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &xm_qdss_etr_0_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A2NOC_SNOC },
};

static struct qcom_icc_qosbox xm_qdss_etr_1_qos = {
	.num_ports = 1,
	.port_offsets = { 0x18000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node xm_qdss_etr_1 = {
	.name = "xm_qdss_etr_1",
	.id = PALAWAN_MASTER_QDSS_ETR_1,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &xm_qdss_etr_1_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_A2NOC_SNOC },
};

static struct qcom_icc_node qup0_core_master = {
	.name = "qup0_core_master",
	.id = PALAWAN_MASTER_QUP_CORE_0,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_QUP_CORE_0 },
};

static struct qcom_icc_node qup1_core_master = {
	.name = "qup1_core_master",
	.id = PALAWAN_MASTER_QUP_CORE_1,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_QUP_CORE_1 },
};

static struct qcom_icc_node qsm_cfg = {
	.name = "qsm_cfg",
	.id = PALAWAN_MASTER_CNOC_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 39,
	.links = { PALAWAN_SLAVE_AHB2PHY_SOUTH, PALAWAN_SLAVE_AHB2PHY_NORTH,
		   PALAWAN_SLAVE_CAMERA_CFG, PALAWAN_SLAVE_CLK_CTL,
		   PALAWAN_SLAVE_RBCPR_CX_CFG, PALAWAN_SLAVE_RBCPR_MMCX_CFG,
		   PALAWAN_SLAVE_RBCPR_MXC_CFG, PALAWAN_SLAVE_CPR_NSPCX,
		   PALAWAN_SLAVE_CRYPTO_0_CFG, PALAWAN_SLAVE_CX_RDPM,
		   PALAWAN_SLAVE_GFX3D_CFG, PALAWAN_SLAVE_I3C_IBI0_CFG,
		   PALAWAN_SLAVE_I3C_IBI1_CFG, PALAWAN_SLAVE_IMEM_CFG,
		   PALAWAN_SLAVE_CNOC_MSS, PALAWAN_SLAVE_MX_2_RDPM,
		   PALAWAN_SLAVE_MX_RDPM, PALAWAN_SLAVE_PCIE_RSCC,
		   PALAWAN_SLAVE_PDM, PALAWAN_SLAVE_QDSS_CFG,
		   PALAWAN_SLAVE_QSPI_0, PALAWAN_SLAVE_QUP_1,
		   PALAWAN_SLAVE_QUP_0, PALAWAN_SLAVE_SDCC_2,
		   PALAWAN_SLAVE_TCSR, PALAWAN_SLAVE_TLMM,
		   PALAWAN_SLAVE_UFS_MEM_CFG, PALAWAN_SLAVE_USB3_0,
		   PALAWAN_SLAVE_VENUS_CFG, PALAWAN_SLAVE_VSENSE_CTRL_CFG,
		   PALAWAN_SLAVE_WLAN, PALAWAN_SLAVE_CNOC_MNOC_HF_CFG,
		   PALAWAN_SLAVE_CNOC_MNOC_SF_CFG, PALAWAN_SLAVE_NSP_QTB_CFG,
		   PALAWAN_SLAVE_PCIE_ANOC_CFG, PALAWAN_SLAVE_WLAN_Q6_THROTTLE_CFG,
		   PALAWAN_SLAVE_SERVICE_CNOC_CFG, PALAWAN_SLAVE_QDSS_STM,
		   PALAWAN_SLAVE_TCU },
};

static struct qcom_icc_node qnm_gemnoc_cnoc = {
	.name = "qnm_gemnoc_cnoc",
	.id = PALAWAN_MASTER_GEM_NOC_CNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 12,
	.links = { PALAWAN_SLAVE_AOSS, PALAWAN_SLAVE_DISPLAY_CFG,
		   PALAWAN_SLAVE_IPA_CFG, PALAWAN_SLAVE_IPC_ROUTER_CFG,
		   PALAWAN_SLAVE_PCIE_0_CFG, PALAWAN_SLAVE_PRNG,
		   PALAWAN_SLAVE_TME_CFG, PALAWAN_SLAVE_APPSS,
		   PALAWAN_SLAVE_CNOC_CFG, PALAWAN_SLAVE_DDRSS_CFG,
		   PALAWAN_SLAVE_IMEM, PALAWAN_SLAVE_SERVICE_CNOC },
};

static struct qcom_icc_node qnm_gemnoc_pcie = {
	.name = "qnm_gemnoc_pcie",
	.id = PALAWAN_MASTER_GEM_NOC_PCIE_SNOC,
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_PCIE_0 },
};

static struct qcom_icc_qosbox alm_gpu_tcu_qos = {
	.num_ports = 1,
	.port_offsets = { 0xbf000 },
	.prio = 1,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node alm_gpu_tcu = {
	.name = "alm_gpu_tcu",
	.id = PALAWAN_MASTER_GPU_TCU,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &alm_gpu_tcu_qos,
	.num_links = 2,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_qosbox alm_sys_tcu_qos = {
	.num_ports = 1,
	.port_offsets = { 0xc1000 },
	.prio = 6,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node alm_sys_tcu = {
	.name = "alm_sys_tcu",
	.id = PALAWAN_MASTER_SYS_TCU,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &alm_sys_tcu_qos,
	.num_links = 2,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_node chm_apps = {
	.name = "chm_apps",
	.id = PALAWAN_MASTER_APPSS_PROC,
	.channels = 3,
	.buswidth = 32,
	.num_links = 3,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC,
		   PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_qosbox qnm_gpu_qos = {
	.num_ports = 2,
	.port_offsets = { 0x31000, 0x71000 },
	.prio = 0,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_gpu = {
	.name = "qnm_gpu",
	.id = PALAWAN_MASTER_GFX3D,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_gpu_qos,
	.num_links = 2,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_qosbox qnm_lpass_gemnoc_qos = {
	.num_ports = 1,
	.port_offsets = { 0xb5000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_lpass_gemnoc = {
	.name = "qnm_lpass_gemnoc",
	.id = PALAWAN_MASTER_LPASS_GEM_NOC,
	.channels = 1,
	.buswidth = 16,
	.qosbox = &qnm_lpass_gemnoc_qos,
	.num_links = 3,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC,
		   PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_node qnm_mdsp = {
	.name = "qnm_mdsp",
	.id = PALAWAN_MASTER_MSS_PROC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 3,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC,
		   PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_qosbox qnm_mnoc_hf_qos = {
	.num_ports = 2,
	.port_offsets = { 0x33000, 0x73000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_mnoc_hf = {
	.name = "qnm_mnoc_hf",
	.id = PALAWAN_MASTER_MNOC_HF_MEM_NOC,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_mnoc_hf_qos,
	.num_links = 2,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_qosbox qnm_mnoc_sf_qos = {
	.num_ports = 2,
	.port_offsets = { 0x35000, 0x75000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_mnoc_sf = {
	.name = "qnm_mnoc_sf",
	.id = PALAWAN_MASTER_MNOC_SF_MEM_NOC,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_mnoc_sf_qos,
	.num_links = 2,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_qosbox qnm_nsp_gemnoc_qos = {
	.num_ports = 2,
	.port_offsets = { 0x37000, 0x77000 },
	.prio = 0,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_nsp_gemnoc = {
	.name = "qnm_nsp_gemnoc",
	.id = PALAWAN_MASTER_COMPUTE_NOC,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_nsp_gemnoc_qos,
	.num_links = 3,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC,
		   PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_qosbox qnm_pcie_qos = {
	.num_ports = 1,
	.port_offsets = { 0xb7000 },
	.prio = 2,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_pcie = {
	.name = "qnm_pcie",
	.id = PALAWAN_MASTER_ANOC_PCIE_GEM_NOC,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &qnm_pcie_qos,
	.num_links = 2,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_qosbox qnm_snoc_sf_qos = {
	.num_ports = 1,
	.port_offsets = { 0xbb000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_snoc_sf = {
	.name = "qnm_snoc_sf",
	.id = PALAWAN_MASTER_SNOC_SF_MEM_NOC,
	.channels = 1,
	.buswidth = 16,
	.qosbox = &qnm_snoc_sf_qos,
	.num_links = 3,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC,
		   PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_node qxm_wlan_q6 = {
	.name = "qxm_wlan_q6",
	.id = PALAWAN_MASTER_WLAN_Q6,
	.channels = 1,
	.buswidth = 8,
	.num_links = 3,
	.links = { PALAWAN_SLAVE_GEM_NOC_CNOC, PALAWAN_SLAVE_LLCC,
		   PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_qosbox xm_gic_qos = {
	.num_ports = 1,
	.port_offsets = { 0xb9000 },
	.prio = 4,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node xm_gic = {
	.name = "xm_gic",
	.id = PALAWAN_MASTER_GIC,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &xm_gic_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_LLCC },
};

static struct qcom_icc_node qnm_lpiaon_noc = {
	.name = "qnm_lpiaon_noc",
	.id = PALAWAN_MASTER_LPIAON_NOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_LPASS_GEM_NOC },
};

static struct qcom_icc_node qnm_lpass_lpinoc = {
	.name = "qnm_lpass_lpinoc",
	.id = PALAWAN_MASTER_LPASS_LPINOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_LPIAON_NOC_LPASS_AG_NOC },
};

static struct qcom_icc_node qxm_lpinoc_dsp_axim = {
	.name = "qxm_lpinoc_dsp_axim",
	.id = PALAWAN_MASTER_LPASS_PROC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_LPICX_NOC_LPIAON_NOC },
};

static struct qcom_icc_node llcc_mc = {
	.name = "llcc_mc",
	.id = PALAWAN_MASTER_LLCC,
	.channels = 4,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_EBI1 },
};

static struct qcom_icc_qosbox qnm_camnoc_hf_qos = {
	.num_ports = 2,
	.port_offsets = { 0xa8000, 0xa9000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_camnoc_hf = {
	.name = "qnm_camnoc_hf",
	.id = PALAWAN_MASTER_CAMNOC_HF,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_camnoc_hf_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_HF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_camnoc_icp_qos = {
	.num_ports = 1,
	.port_offsets = { 0x2a000 },
	.prio = 4,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_camnoc_icp = {
	.name = "qnm_camnoc_icp",
	.id = PALAWAN_MASTER_CAMNOC_ICP,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &qnm_camnoc_icp_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_camnoc_sf_qos = {
	.num_ports = 2,
	.port_offsets = { 0x2b000, 0x2c000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_camnoc_sf = {
	.name = "qnm_camnoc_sf",
	.id = PALAWAN_MASTER_CAMNOC_SF,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_camnoc_sf_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_mdp_qos = {
	.num_ports = 2,
	.port_offsets = { 0xad000, 0xae000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_mdp = {
	.name = "qnm_mdp",
	.id = PALAWAN_MASTER_MDP,
	.channels = 2,
	.buswidth = 32,
	.qosbox = &qnm_mdp_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_HF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_video_qos = {
	.num_ports = 1,
	.port_offsets = { 0x30000 },
	.prio = 0,
	.urg_fwd = 1,
	.prio_fwd_disable = 0,
};

static struct qcom_icc_node qnm_video = {
	.name = "qnm_video",
	.id = PALAWAN_MASTER_VIDEO,
	.channels = 1,
	.buswidth = 32,
	.qosbox = &qnm_video_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_video_cv_cpu_qos = {
	.num_ports = 1,
	.port_offsets = { 0x32000 },
	.prio = 4,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_video_cv_cpu = {
	.name = "qnm_video_cv_cpu",
	.id = PALAWAN_MASTER_VIDEO_CV_PROC,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &qnm_video_cv_cpu_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_video_cvp_qos = {
	.num_ports = 1,
	.port_offsets = { 0x33000 },
	.prio = 3,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_video_cvp = {
	.name = "qnm_video_cvp",
	.id = PALAWAN_MASTER_VIDEO_PROC,
	.channels = 1,
	.buswidth = 32,
	.qosbox = &qnm_video_cvp_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_qosbox qnm_video_v_cpu_qos = {
	.num_ports = 1,
	.port_offsets = { 0x35000 },
	.prio = 4,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_video_v_cpu = {
	.name = "qnm_video_v_cpu",
	.id = PALAWAN_MASTER_VIDEO_V_PROC,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &qnm_video_v_cpu_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_node qsm_hf_mnoc_cfg = {
	.name = "qsm_hf_mnoc_cfg",
	.id = PALAWAN_MASTER_CNOC_MNOC_HF_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_SERVICE_MNOC_HF },
};

static struct qcom_icc_node qsm_sf_mnoc_cfg = {
	.name = "qsm_sf_mnoc_cfg",
	.id = PALAWAN_MASTER_CNOC_MNOC_SF_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_SERVICE_MNOC_SF },
};

static struct qcom_icc_node qxm_nsp = {
	.name = "qxm_nsp",
	.id = PALAWAN_MASTER_CDSP_PROC,
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_CDSP_MEM_NOC },
};

static struct qcom_icc_node qsm_pcie_anoc_cfg = {
	.name = "qsm_pcie_anoc_cfg",
	.id = PALAWAN_MASTER_PCIE_ANOC_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_SERVICE_PCIE_ANOC },
};

static struct qcom_icc_qosbox xm_pcie3_0_qos = {
	.num_ports = 1,
	.port_offsets = { 0xb000 },
	.prio = 3,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node xm_pcie3_0 = {
	.name = "xm_pcie3_0",
	.id = PALAWAN_MASTER_PCIE_0,
	.channels = 1,
	.buswidth = 8,
	.qosbox = &xm_pcie3_0_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_ANOC_PCIE_GEM_NOC },
};

static struct qcom_icc_node qnm_aggre1_noc = {
	.name = "qnm_aggre1_noc",
	.id = PALAWAN_MASTER_A1NOC_SNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_SNOC_GEM_NOC_SF },
};

static struct qcom_icc_node qnm_aggre2_noc = {
	.name = "qnm_aggre2_noc",
	.id = PALAWAN_MASTER_A2NOC_SNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_SNOC_GEM_NOC_SF },
};

static struct qcom_icc_qosbox qnm_apss_noc_qos = {
	.num_ports = 1,
	.port_offsets = { 0x1c000 },
	.prio = 2,
	.urg_fwd = 0,
	.prio_fwd_disable = 1,
};

static struct qcom_icc_node qnm_apss_noc = {
	.name = "qnm_apss_noc",
	.id = PALAWAN_MASTER_APSS_NOC,
	.channels = 1,
	.buswidth = 4,
	.qosbox = &qnm_apss_noc_qos,
	.num_links = 1,
	.links = { PALAWAN_SLAVE_SNOC_GEM_NOC_SF },
};

static struct qcom_icc_node qns_a1noc_snoc = {
	.name = "qns_a1noc_snoc",
	.id = PALAWAN_SLAVE_A1NOC_SNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_A1NOC_SNOC },
};

static struct qcom_icc_node qns_a2noc_snoc = {
	.name = "qns_a2noc_snoc",
	.id = PALAWAN_SLAVE_A2NOC_SNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_A2NOC_SNOC },
};

static struct qcom_icc_node qup0_core_slave = {
	.name = "qup0_core_slave",
	.id = PALAWAN_SLAVE_QUP_CORE_0,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qup1_core_slave = {
	.name = "qup1_core_slave",
	.id = PALAWAN_SLAVE_QUP_CORE_1,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_ahb2phy0 = {
	.name = "qhs_ahb2phy0",
	.id = PALAWAN_SLAVE_AHB2PHY_SOUTH,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_ahb2phy1 = {
	.name = "qhs_ahb2phy1",
	.id = PALAWAN_SLAVE_AHB2PHY_NORTH,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_camera_cfg = {
	.name = "qhs_camera_cfg",
	.id = PALAWAN_SLAVE_CAMERA_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_clk_ctl = {
	.name = "qhs_clk_ctl",
	.id = PALAWAN_SLAVE_CLK_CTL,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_cpr_cx = {
	.name = "qhs_cpr_cx",
	.id = PALAWAN_SLAVE_RBCPR_CX_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_cpr_mmcx = {
	.name = "qhs_cpr_mmcx",
	.id = PALAWAN_SLAVE_RBCPR_MMCX_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_cpr_mxc = {
	.name = "qhs_cpr_mxc",
	.id = PALAWAN_SLAVE_RBCPR_MXC_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_cpr_nspcx = {
	.name = "qhs_cpr_nspcx",
	.id = PALAWAN_SLAVE_CPR_NSPCX,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_crypto0_cfg = {
	.name = "qhs_crypto0_cfg",
	.id = PALAWAN_SLAVE_CRYPTO_0_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_cx_rdpm = {
	.name = "qhs_cx_rdpm",
	.id = PALAWAN_SLAVE_CX_RDPM,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_gpuss_cfg = {
	.name = "qhs_gpuss_cfg",
	.id = PALAWAN_SLAVE_GFX3D_CFG,
	.channels = 1,
	.buswidth = 8,
	.num_links = 0,
};

static struct qcom_icc_node qhs_i3c_ibi0_cfg = {
	.name = "qhs_i3c_ibi0_cfg",
	.id = PALAWAN_SLAVE_I3C_IBI0_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_i3c_ibi1_cfg = {
	.name = "qhs_i3c_ibi1_cfg",
	.id = PALAWAN_SLAVE_I3C_IBI1_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_imem_cfg = {
	.name = "qhs_imem_cfg",
	.id = PALAWAN_SLAVE_IMEM_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_mss_cfg = {
	.name = "qhs_mss_cfg",
	.id = PALAWAN_SLAVE_CNOC_MSS,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_mx_2_rdpm = {
	.name = "qhs_mx_2_rdpm",
	.id = PALAWAN_SLAVE_MX_2_RDPM,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_mx_rdpm = {
	.name = "qhs_mx_rdpm",
	.id = PALAWAN_SLAVE_MX_RDPM,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_pcie_rscc = {
	.name = "qhs_pcie_rscc",
	.id = PALAWAN_SLAVE_PCIE_RSCC,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_pdm = {
	.name = "qhs_pdm",
	.id = PALAWAN_SLAVE_PDM,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_qdss_cfg = {
	.name = "qhs_qdss_cfg",
	.id = PALAWAN_SLAVE_QDSS_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_qspi = {
	.name = "qhs_qspi",
	.id = PALAWAN_SLAVE_QSPI_0,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_qup0 = {
	.name = "qhs_qup0",
	.id = PALAWAN_SLAVE_QUP_0,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_qup1 = {
	.name = "qhs_qup1",
	.id = PALAWAN_SLAVE_QUP_1,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_sdc2 = {
	.name = "qhs_sdc2",
	.id = PALAWAN_SLAVE_SDCC_2,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_tcsr = {
	.name = "qhs_tcsr",
	.id = PALAWAN_SLAVE_TCSR,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_tlmm = {
	.name = "qhs_tlmm",
	.id = PALAWAN_SLAVE_TLMM,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_ufs_mem_cfg = {
	.name = "qhs_ufs_mem_cfg",
	.id = PALAWAN_SLAVE_UFS_MEM_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_usb3_0 = {
	.name = "qhs_usb3_0",
	.id = PALAWAN_SLAVE_USB3_0,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_venus_cfg = {
	.name = "qhs_venus_cfg",
	.id = PALAWAN_SLAVE_VENUS_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_vsense_ctrl_cfg = {
	.name = "qhs_vsense_ctrl_cfg",
	.id = PALAWAN_SLAVE_VSENSE_CTRL_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_wlan_q6 = {
	.name = "qhs_wlan_q6",
	.id = PALAWAN_SLAVE_WLAN,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qss_mnoc_hf_cfg = {
	.name = "qss_mnoc_hf_cfg",
	.id = PALAWAN_SLAVE_CNOC_MNOC_HF_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_MASTER_CNOC_MNOC_HF_CFG },
};

static struct qcom_icc_node qss_mnoc_sf_cfg = {
	.name = "qss_mnoc_sf_cfg",
	.id = PALAWAN_SLAVE_CNOC_MNOC_SF_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_MASTER_CNOC_MNOC_SF_CFG },
};

static struct qcom_icc_node qss_nsp_qtb_cfg = {
	.name = "qss_nsp_qtb_cfg",
	.id = PALAWAN_SLAVE_NSP_QTB_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qss_pcie_anoc_cfg = {
	.name = "qss_pcie_anoc_cfg",
	.id = PALAWAN_SLAVE_PCIE_ANOC_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_MASTER_PCIE_ANOC_CFG },
};

static struct qcom_icc_node qss_wlan_q6_throttle_cfg = {
	.name = "qss_wlan_q6_throttle_cfg",
	.id = PALAWAN_SLAVE_WLAN_Q6_THROTTLE_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node srvc_cnoc_cfg = {
	.name = "srvc_cnoc_cfg",
	.id = PALAWAN_SLAVE_SERVICE_CNOC_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node xs_qdss_stm = {
	.name = "xs_qdss_stm",
	.id = PALAWAN_SLAVE_QDSS_STM,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node xs_sys_tcu_cfg = {
	.name = "xs_sys_tcu_cfg",
	.id = PALAWAN_SLAVE_TCU,
	.channels = 1,
	.buswidth = 8,
	.num_links = 0,
};

static struct qcom_icc_node qhs_aoss = {
	.name = "qhs_aoss",
	.id = PALAWAN_SLAVE_AOSS,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_display_cfg = {
	.name = "qhs_display_cfg",
	.id = PALAWAN_SLAVE_DISPLAY_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_ipa = {
	.name = "qhs_ipa",
	.id = PALAWAN_SLAVE_IPA_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_ipc_router = {
	.name = "qhs_ipc_router",
	.id = PALAWAN_SLAVE_IPC_ROUTER_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_pcie0_cfg = {
	.name = "qhs_pcie0_cfg",
	.id = PALAWAN_SLAVE_PCIE_0_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_prng = {
	.name = "qhs_prng",
	.id = PALAWAN_SLAVE_PRNG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qhs_tme_cfg = {
	.name = "qhs_tme_cfg",
	.id = PALAWAN_SLAVE_TME_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qss_apss = {
	.name = "qss_apss",
	.id = PALAWAN_SLAVE_APPSS,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qss_cfg = {
	.name = "qss_cfg",
	.id = PALAWAN_SLAVE_CNOC_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 1,
	.links = { PALAWAN_MASTER_CNOC_CFG },
};

static struct qcom_icc_node qss_ddrss_cfg = {
	.name = "qss_ddrss_cfg",
	.id = PALAWAN_SLAVE_DDRSS_CFG,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qxs_imem = {
	.name = "qxs_imem",
	.id = PALAWAN_SLAVE_IMEM,
	.channels = 1,
	.buswidth = 8,
	.num_links = 0,
};

static struct qcom_icc_node srvc_cnoc_main = {
	.name = "srvc_cnoc_main",
	.id = PALAWAN_SLAVE_SERVICE_CNOC,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node xs_pcie_0 = {
	.name = "xs_pcie_0",
	.id = PALAWAN_SLAVE_PCIE_0,
	.channels = 1,
	.buswidth = 8,
	.num_links = 0,
};

static struct qcom_icc_node qns_gem_noc_cnoc = {
	.name = "qns_gem_noc_cnoc",
	.id = PALAWAN_SLAVE_GEM_NOC_CNOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_GEM_NOC_CNOC },
};

static struct qcom_icc_node qns_llcc = {
	.name = "qns_llcc",
	.id = PALAWAN_SLAVE_LLCC,
	.channels = 4,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_LLCC },
};

static struct qcom_icc_node qns_pcie = {
	.name = "qns_pcie",
	.id = PALAWAN_SLAVE_MEM_NOC_PCIE_SNOC,
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.links = { PALAWAN_MASTER_GEM_NOC_PCIE_SNOC },
};

static struct qcom_icc_node qns_lpass_ag_noc_gemnoc = {
	.name = "qns_lpass_ag_noc_gemnoc",
	.id = PALAWAN_SLAVE_LPASS_GEM_NOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_LPASS_GEM_NOC },
};

static struct qcom_icc_node qns_lpass_aggnoc = {
	.name = "qns_lpass_aggnoc",
	.id = PALAWAN_SLAVE_LPIAON_NOC_LPASS_AG_NOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_LPIAON_NOC },
};

static struct qcom_icc_node qns_lpi_aon_noc = {
	.name = "qns_lpi_aon_noc",
	.id = PALAWAN_SLAVE_LPICX_NOC_LPIAON_NOC,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_LPASS_LPINOC },
};

static struct qcom_icc_node ebi = {
	.name = "ebi",
	.id = PALAWAN_SLAVE_EBI1,
	.channels = 4,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qns_mem_noc_hf = {
	.name = "qns_mem_noc_hf",
	.id = PALAWAN_SLAVE_MNOC_HF_MEM_NOC,
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.links = { PALAWAN_MASTER_MNOC_HF_MEM_NOC },
};

static struct qcom_icc_node qns_mem_noc_sf = {
	.name = "qns_mem_noc_sf",
	.id = PALAWAN_SLAVE_MNOC_SF_MEM_NOC,
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.links = { PALAWAN_MASTER_MNOC_SF_MEM_NOC },
};

static struct qcom_icc_node srvc_mnoc_hf = {
	.name = "srvc_mnoc_hf",
	.id = PALAWAN_SLAVE_SERVICE_MNOC_HF,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node srvc_mnoc_sf = {
	.name = "srvc_mnoc_sf",
	.id = PALAWAN_SLAVE_SERVICE_MNOC_SF,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qns_nsp_gemnoc = {
	.name = "qns_nsp_gemnoc",
	.id = PALAWAN_SLAVE_CDSP_MEM_NOC,
	.channels = 2,
	.buswidth = 32,
	.num_links = 1,
	.links = { PALAWAN_MASTER_COMPUTE_NOC },
};

static struct qcom_icc_node qns_pcie_mem_noc = {
	.name = "qns_pcie_mem_noc",
	.id = PALAWAN_SLAVE_ANOC_PCIE_GEM_NOC,
	.channels = 1,
	.buswidth = 8,
	.num_links = 1,
	.links = { PALAWAN_MASTER_ANOC_PCIE_GEM_NOC },
};

static struct qcom_icc_node srvc_pcie_aggre_noc = {
	.name = "srvc_pcie_aggre_noc",
	.id = PALAWAN_SLAVE_SERVICE_PCIE_ANOC,
	.channels = 1,
	.buswidth = 4,
	.num_links = 0,
};

static struct qcom_icc_node qns_gemnoc_sf = {
	.name = "qns_gemnoc_sf",
	.id = PALAWAN_SLAVE_SNOC_GEM_NOC_SF,
	.channels = 1,
	.buswidth = 16,
	.num_links = 1,
	.links = { PALAWAN_MASTER_SNOC_SF_MEM_NOC },
};

static struct qcom_icc_bcm bcm_acv = {
	.name = "ACV",
	.enable_mask = BIT(0),
	.num_nodes = 1,
	.nodes = { &ebi },
};

static struct qcom_icc_bcm bcm_ce0 = {
	.name = "CE0",
	.num_nodes = 1,
	.nodes = { &qxm_crypto },
};

static struct qcom_icc_bcm bcm_cn0 = {
	.name = "CN0",
	.enable_mask = BIT(0),
	.keepalive = true,
	.num_nodes = 52,
	.nodes = { &qsm_cfg, &qhs_ahb2phy0,
		   &qhs_ahb2phy1, &qhs_camera_cfg,
		   &qhs_clk_ctl, &qhs_cpr_cx,
		   &qhs_cpr_mmcx, &qhs_cpr_mxc,
		   &qhs_cpr_nspcx, &qhs_crypto0_cfg,
		   &qhs_cx_rdpm, &qhs_gpuss_cfg,
		   &qhs_i3c_ibi0_cfg, &qhs_i3c_ibi1_cfg,
		   &qhs_imem_cfg, &qhs_mss_cfg,
		   &qhs_mx_2_rdpm, &qhs_mx_rdpm,
		   &qhs_pcie_rscc, &qhs_pdm,
		   &qhs_qdss_cfg, &qhs_qspi,
		   &qhs_sdc2, &qhs_tcsr,
		   &qhs_tlmm, &qhs_ufs_mem_cfg,
		   &qhs_usb3_0, &qhs_venus_cfg,
		   &qhs_vsense_ctrl_cfg, &qhs_wlan_q6,
		   &qss_mnoc_hf_cfg, &qss_mnoc_sf_cfg,
		   &qss_nsp_qtb_cfg, &qss_pcie_anoc_cfg,
		   &qss_wlan_q6_throttle_cfg, &srvc_cnoc_cfg,
		   &xs_qdss_stm, &xs_sys_tcu_cfg,
		   &qnm_gemnoc_cnoc, &qnm_gemnoc_pcie,
		   &qhs_aoss, &qhs_ipa,
		   &qhs_ipc_router, &qhs_pcie0_cfg,
		   &qhs_prng, &qhs_tme_cfg,
		   &qss_apss, &qss_cfg,
		   &qss_ddrss_cfg, &qxs_imem,
		   &srvc_cnoc_main, &xs_pcie_0 },
};

static struct qcom_icc_bcm bcm_co0 = {
	.name = "CO0",
	.enable_mask = BIT(0),
	.num_nodes = 2,
	.nodes = { &qxm_nsp, &qns_nsp_gemnoc },
};

static struct qcom_icc_bcm bcm_lp0 = {
	.name = "LP0",
	.num_nodes = 2,
	.nodes = { &qnm_lpass_lpinoc, &qns_lpass_aggnoc },
};

static struct qcom_icc_bcm bcm_mc0 = {
	.name = "MC0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &ebi },
};

static struct qcom_icc_bcm bcm_mm0 = {
	.name = "MM0",
	.num_nodes = 1,
	.nodes = { &qns_mem_noc_hf },
};

static struct qcom_icc_bcm bcm_mm1 = {
	.name = "MM1",
	.enable_mask = BIT(0),
	.num_nodes = 7,
	.nodes = { &qnm_camnoc_hf, &qnm_camnoc_icp,
		   &qnm_camnoc_sf, &qnm_video_cv_cpu,
		   &qnm_video_cvp, &qnm_video_v_cpu,
		   &qns_mem_noc_sf },
};

static struct qcom_icc_bcm bcm_qup0 = {
	.name = "QUP0",
	.keepalive = true,
	.vote_scale = 1,
	.num_nodes = 1,
	.nodes = { &qup0_core_slave },
};

static struct qcom_icc_bcm bcm_qup1 = {
	.name = "QUP1",
	.keepalive = true,
	.vote_scale = 1,
	.num_nodes = 1,
	.nodes = { &qup1_core_slave },
};

static struct qcom_icc_bcm bcm_sh0 = {
	.name = "SH0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_llcc },
};

static struct qcom_icc_bcm bcm_sh1 = {
	.name = "SH1",
	.enable_mask = BIT(0),
	.num_nodes = 14,
	.nodes = { &alm_gpu_tcu, &alm_sys_tcu,
		   &chm_apps, &qnm_gpu,
		   &qnm_mdsp, &qnm_mnoc_hf,
		   &qnm_mnoc_sf, &qnm_nsp_gemnoc,
		   &qnm_pcie, &qnm_snoc_sf,
		   &qxm_wlan_q6, &xm_gic,
		   &qns_gem_noc_cnoc, &qns_pcie },
};

static struct qcom_icc_bcm bcm_sn0 = {
	.name = "SN0",
	.keepalive = true,
	.num_nodes = 1,
	.nodes = { &qns_gemnoc_sf },
};

static struct qcom_icc_bcm bcm_sn2 = {
	.name = "SN2",
	.num_nodes = 1,
	.nodes = { &qnm_aggre1_noc },
};

static struct qcom_icc_bcm bcm_sn3 = {
	.name = "SN3",
	.num_nodes = 1,
	.nodes = { &qnm_aggre2_noc },
};

static struct qcom_icc_bcm bcm_sn4 = {
	.name = "SN4",
	.num_nodes = 1,
	.nodes = { &qns_pcie_mem_noc },
};

static struct qcom_icc_node * const aggre1_noc_nodes[] = {
	[MASTER_QSPI_0] = &qhm_qspi,
	[MASTER_QUP_0] = &qhm_qup0,
	[MASTER_SDCC_2] = &xm_sdc2,
	[MASTER_UFS_MEM] = &xm_ufs_mem,
	[MASTER_USB3_0] = &xm_usb3_0,
	[SLAVE_A1NOC_SNOC] = &qns_a1noc_snoc,
};

static const struct qcom_icc_desc palawan_aggre1_noc = {
	.config = &icc_regmap_config,
	.nodes = aggre1_noc_nodes,
	.num_nodes = ARRAY_SIZE(aggre1_noc_nodes),
};

static struct qcom_icc_bcm * const aggre2_noc_bcms[] = {
	&bcm_ce0,
};

static struct qcom_icc_node * const aggre2_noc_nodes[] = {
	[MASTER_QDSS_BAM] = &qhm_qdss_bam,
	[MASTER_QUP_1] = &qhm_qup1,
	[MASTER_CRYPTO] = &qxm_crypto,
	[MASTER_IPA] = &qxm_ipa,
	[MASTER_QDSS_ETR] = &xm_qdss_etr_0,
	[MASTER_QDSS_ETR_1] = &xm_qdss_etr_1,
	[SLAVE_A2NOC_SNOC] = &qns_a2noc_snoc,
};

static const struct qcom_icc_desc palawan_aggre2_noc = {
	.config = &icc_regmap_config,
	.nodes = aggre2_noc_nodes,
	.num_nodes = ARRAY_SIZE(aggre2_noc_nodes),
	.bcms = aggre2_noc_bcms,
	.num_bcms = ARRAY_SIZE(aggre2_noc_bcms),
};

static struct qcom_icc_bcm * const clk_virt_bcms[] = {
	&bcm_qup0,
	&bcm_qup1,
};

static struct qcom_icc_node * const clk_virt_nodes[] = {
	[MASTER_QUP_CORE_0] = &qup0_core_master,
	[MASTER_QUP_CORE_1] = &qup1_core_master,
	[SLAVE_QUP_CORE_0] = &qup0_core_slave,
	[SLAVE_QUP_CORE_1] = &qup1_core_slave,
};

static const struct qcom_icc_desc palawan_clk_virt = {
	.nodes = clk_virt_nodes,
	.num_nodes = ARRAY_SIZE(clk_virt_nodes),
	.bcms = clk_virt_bcms,
	.num_bcms = ARRAY_SIZE(clk_virt_bcms),
};

static struct qcom_icc_bcm * const config_noc_bcms[] = {
	&bcm_cn0,
};

static struct qcom_icc_node * const config_noc_nodes[] = {
	[MASTER_CNOC_CFG] = &qsm_cfg,
	[SLAVE_AHB2PHY_SOUTH] = &qhs_ahb2phy0,
	[SLAVE_AHB2PHY_NORTH] = &qhs_ahb2phy1,
	[SLAVE_CAMERA_CFG] = &qhs_camera_cfg,
	[SLAVE_CLK_CTL] = &qhs_clk_ctl,
	[SLAVE_RBCPR_CX_CFG] = &qhs_cpr_cx,
	[SLAVE_RBCPR_MMCX_CFG] = &qhs_cpr_mmcx,
	[SLAVE_RBCPR_MXC_CFG] = &qhs_cpr_mxc,
	[SLAVE_CPR_NSPCX] = &qhs_cpr_nspcx,
	[SLAVE_CRYPTO_0_CFG] = &qhs_crypto0_cfg,
	[SLAVE_CX_RDPM] = &qhs_cx_rdpm,
	[SLAVE_GFX3D_CFG] = &qhs_gpuss_cfg,
	[SLAVE_I3C_IBI0_CFG] = &qhs_i3c_ibi0_cfg,
	[SLAVE_I3C_IBI1_CFG] = &qhs_i3c_ibi1_cfg,
	[SLAVE_IMEM_CFG] = &qhs_imem_cfg,
	[SLAVE_CNOC_MSS] = &qhs_mss_cfg,
	[SLAVE_MX_2_RDPM] = &qhs_mx_2_rdpm,
	[SLAVE_MX_RDPM] = &qhs_mx_rdpm,
	[SLAVE_PCIE_RSCC] = &qhs_pcie_rscc,
	[SLAVE_PDM] = &qhs_pdm,
	[SLAVE_QDSS_CFG] = &qhs_qdss_cfg,
	[SLAVE_QSPI_0] = &qhs_qspi,
	[SLAVE_QUP_0] = &qhs_qup0,
	[SLAVE_QUP_1] = &qhs_qup1,
	[SLAVE_SDCC_2] = &qhs_sdc2,
	[SLAVE_TCSR] = &qhs_tcsr,
	[SLAVE_TLMM] = &qhs_tlmm,
	[SLAVE_UFS_MEM_CFG] = &qhs_ufs_mem_cfg,
	[SLAVE_USB3_0] = &qhs_usb3_0,
	[SLAVE_VENUS_CFG] = &qhs_venus_cfg,
	[SLAVE_VSENSE_CTRL_CFG] = &qhs_vsense_ctrl_cfg,
	[SLAVE_WLAN] = &qhs_wlan_q6,
	[SLAVE_CNOC_MNOC_HF_CFG] = &qss_mnoc_hf_cfg,
	[SLAVE_CNOC_MNOC_SF_CFG] = &qss_mnoc_sf_cfg,
	[SLAVE_NSP_QTB_CFG] = &qss_nsp_qtb_cfg,
	[SLAVE_PCIE_ANOC_CFG] = &qss_pcie_anoc_cfg,
	[SLAVE_WLAN_Q6_THROTTLE_CFG] = &qss_wlan_q6_throttle_cfg,
	[SLAVE_SERVICE_CNOC_CFG] = &srvc_cnoc_cfg,
	[SLAVE_QDSS_STM] = &xs_qdss_stm,
	[SLAVE_TCU] = &xs_sys_tcu_cfg,
};

static const struct qcom_icc_desc palawan_config_noc = {
	.config = &icc_regmap_config,
	.nodes = config_noc_nodes,
	.num_nodes = ARRAY_SIZE(config_noc_nodes),
	.bcms = config_noc_bcms,
	.num_bcms = ARRAY_SIZE(config_noc_bcms),
};

static struct qcom_icc_bcm * const cnoc_main_bcms[] = {
	&bcm_cn0,
};

static struct qcom_icc_node * const cnoc_main_nodes[] = {
	[MASTER_GEM_NOC_CNOC] = &qnm_gemnoc_cnoc,
	[MASTER_GEM_NOC_PCIE_SNOC] = &qnm_gemnoc_pcie,
	[SLAVE_AOSS] = &qhs_aoss,
	[SLAVE_DISPLAY_CFG] = &qhs_display_cfg,
	[SLAVE_IPA_CFG] = &qhs_ipa,
	[SLAVE_IPC_ROUTER_CFG] = &qhs_ipc_router,
	[SLAVE_PCIE_0_CFG] = &qhs_pcie0_cfg,
	[SLAVE_PRNG] = &qhs_prng,
	[SLAVE_TME_CFG] = &qhs_tme_cfg,
	[SLAVE_APPSS] = &qss_apss,
	[SLAVE_CNOC_CFG] = &qss_cfg,
	[SLAVE_DDRSS_CFG] = &qss_ddrss_cfg,
	[SLAVE_IMEM] = &qxs_imem,
	[SLAVE_SERVICE_CNOC] = &srvc_cnoc_main,
	[SLAVE_PCIE_0] = &xs_pcie_0,
};

static const struct qcom_icc_desc palawan_cnoc_main = {
	.config = &icc_regmap_config,
	.nodes = cnoc_main_nodes,
	.num_nodes = ARRAY_SIZE(cnoc_main_nodes),
	.bcms = cnoc_main_bcms,
	.num_bcms = ARRAY_SIZE(cnoc_main_bcms),
};

static struct qcom_icc_bcm * const gem_noc_bcms[] = {
	&bcm_sh0,
	&bcm_sh1,
};

static struct qcom_icc_node * const gem_noc_nodes[] = {
	[MASTER_GPU_TCU] = &alm_gpu_tcu,
	[MASTER_SYS_TCU] = &alm_sys_tcu,
	[MASTER_APPSS_PROC] = &chm_apps,
	[MASTER_GFX3D] = &qnm_gpu,
	[MASTER_LPASS_GEM_NOC] = &qnm_lpass_gemnoc,
	[MASTER_MSS_PROC] = &qnm_mdsp,
	[MASTER_MNOC_HF_MEM_NOC] = &qnm_mnoc_hf,
	[MASTER_MNOC_SF_MEM_NOC] = &qnm_mnoc_sf,
	[MASTER_COMPUTE_NOC] = &qnm_nsp_gemnoc,
	[MASTER_ANOC_PCIE_GEM_NOC] = &qnm_pcie,
	[MASTER_SNOC_SF_MEM_NOC] = &qnm_snoc_sf,
	[MASTER_WLAN_Q6] = &qxm_wlan_q6,
	[MASTER_GIC] = &xm_gic,
	[SLAVE_GEM_NOC_CNOC] = &qns_gem_noc_cnoc,
	[SLAVE_LLCC] = &qns_llcc,
	[SLAVE_MEM_NOC_PCIE_SNOC] = &qns_pcie,
};

static const struct qcom_icc_desc palawan_gem_noc = {
	.config = &icc_regmap_config,
	.nodes = gem_noc_nodes,
	.num_nodes = ARRAY_SIZE(gem_noc_nodes),
	.bcms = gem_noc_bcms,
	.num_bcms = ARRAY_SIZE(gem_noc_bcms),
};

static struct qcom_icc_node * const lpass_ag_noc_nodes[] = {
	[MASTER_LPIAON_NOC] = &qnm_lpiaon_noc,
	[SLAVE_LPASS_GEM_NOC] = &qns_lpass_ag_noc_gemnoc,
};

static const struct qcom_icc_desc palawan_lpass_ag_noc = {
	.config = &icc_regmap_config,
	.nodes = lpass_ag_noc_nodes,
	.num_nodes = ARRAY_SIZE(lpass_ag_noc_nodes),
};

static struct qcom_icc_bcm * const lpass_lpiaon_noc_bcms[] = {
	&bcm_lp0,
};

static struct qcom_icc_node * const lpass_lpiaon_noc_nodes[] = {
	[MASTER_LPASS_LPINOC] = &qnm_lpass_lpinoc,
	[SLAVE_LPIAON_NOC_LPASS_AG_NOC] = &qns_lpass_aggnoc,
};

static const struct qcom_icc_desc palawan_lpass_lpiaon_noc = {
	.config = &icc_regmap_config,
	.nodes = lpass_lpiaon_noc_nodes,
	.num_nodes = ARRAY_SIZE(lpass_lpiaon_noc_nodes),
	.bcms = lpass_lpiaon_noc_bcms,
	.num_bcms = ARRAY_SIZE(lpass_lpiaon_noc_bcms),
};

static struct qcom_icc_node * const lpass_lpicx_noc_nodes[] = {
	[MASTER_LPASS_PROC] = &qxm_lpinoc_dsp_axim,
	[SLAVE_LPICX_NOC_LPIAON_NOC] = &qns_lpi_aon_noc,
};

static const struct qcom_icc_desc palawan_lpass_lpicx_noc = {
	.config = &icc_regmap_config,
	.nodes = lpass_lpicx_noc_nodes,
	.num_nodes = ARRAY_SIZE(lpass_lpicx_noc_nodes),
};

static struct qcom_icc_bcm * const mc_virt_bcms[] = {
	&bcm_acv,
	&bcm_mc0,
};

static struct qcom_icc_node * const mc_virt_nodes[] = {
	[MASTER_LLCC] = &llcc_mc,
	[SLAVE_EBI1] = &ebi,
};

static const struct qcom_icc_desc palawan_mc_virt = {
	.nodes = mc_virt_nodes,
	.num_nodes = ARRAY_SIZE(mc_virt_nodes),
	.bcms = mc_virt_bcms,
	.num_bcms = ARRAY_SIZE(mc_virt_bcms),
};

static struct qcom_icc_bcm * const mmss_noc_bcms[] = {
	&bcm_mm0,
	&bcm_mm1,
};

static struct qcom_icc_node * const mmss_noc_nodes[] = {
	[MASTER_CAMNOC_HF] = &qnm_camnoc_hf,
	[MASTER_CAMNOC_ICP] = &qnm_camnoc_icp,
	[MASTER_CAMNOC_SF] = &qnm_camnoc_sf,
	[MASTER_MDP] = &qnm_mdp,
	[MASTER_VIDEO] = &qnm_video,
	[MASTER_VIDEO_CV_PROC] = &qnm_video_cv_cpu,
	[MASTER_VIDEO_PROC] = &qnm_video_cvp,
	[MASTER_VIDEO_V_PROC] = &qnm_video_v_cpu,
	[MASTER_CNOC_MNOC_HF_CFG] = &qsm_hf_mnoc_cfg,
	[MASTER_CNOC_MNOC_SF_CFG] = &qsm_sf_mnoc_cfg,
	[SLAVE_MNOC_HF_MEM_NOC] = &qns_mem_noc_hf,
	[SLAVE_MNOC_SF_MEM_NOC] = &qns_mem_noc_sf,
	[SLAVE_SERVICE_MNOC_HF] = &srvc_mnoc_hf,
	[SLAVE_SERVICE_MNOC_SF] = &srvc_mnoc_sf,
};

static const struct qcom_icc_desc palawan_mmss_noc = {
	.config = &icc_regmap_config,
	.nodes = mmss_noc_nodes,
	.num_nodes = ARRAY_SIZE(mmss_noc_nodes),
	.bcms = mmss_noc_bcms,
	.num_bcms = ARRAY_SIZE(mmss_noc_bcms),
};

static struct qcom_icc_bcm * const nsp_noc_bcms[] = {
	&bcm_co0,
};

static struct qcom_icc_node * const nsp_noc_nodes[] = {
	[MASTER_CDSP_PROC] = &qxm_nsp,
	[SLAVE_CDSP_MEM_NOC] = &qns_nsp_gemnoc,
};

static const struct qcom_icc_desc palawan_nsp_noc = {
	.config = &icc_regmap_config,
	.nodes = nsp_noc_nodes,
	.num_nodes = ARRAY_SIZE(nsp_noc_nodes),
	.bcms = nsp_noc_bcms,
	.num_bcms = ARRAY_SIZE(nsp_noc_bcms),
};

static struct qcom_icc_bcm * const pcie_anoc_bcms[] = {
	&bcm_sn4,
};

static struct qcom_icc_node * const pcie_anoc_nodes[] = {
	[MASTER_PCIE_ANOC_CFG] = &qsm_pcie_anoc_cfg,
	[MASTER_PCIE_0] = &xm_pcie3_0,
	[SLAVE_ANOC_PCIE_GEM_NOC] = &qns_pcie_mem_noc,
	[SLAVE_SERVICE_PCIE_ANOC] = &srvc_pcie_aggre_noc,
};

static const struct qcom_icc_desc palawan_pcie_anoc = {
	.config = &icc_regmap_config,
	.nodes = pcie_anoc_nodes,
	.num_nodes = ARRAY_SIZE(pcie_anoc_nodes),
	.bcms = pcie_anoc_bcms,
	.num_bcms = ARRAY_SIZE(pcie_anoc_bcms),
};

static struct qcom_icc_bcm * const system_noc_bcms[] = {
	&bcm_sn0,
	&bcm_sn2,
	&bcm_sn3,
};

static struct qcom_icc_node * const system_noc_nodes[] = {
	[MASTER_A1NOC_SNOC] = &qnm_aggre1_noc,
	[MASTER_A2NOC_SNOC] = &qnm_aggre2_noc,
	[MASTER_APSS_NOC] = &qnm_apss_noc,
	[SLAVE_SNOC_GEM_NOC_SF] = &qns_gemnoc_sf,
};

static const struct qcom_icc_desc palawan_system_noc = {
	.config = &icc_regmap_config,
	.nodes = system_noc_nodes,
	.num_nodes = ARRAY_SIZE(system_noc_nodes),
	.bcms = system_noc_bcms,
	.num_bcms = ARRAY_SIZE(system_noc_bcms),
};

static const struct of_device_id qnoc_of_match[] = {
	{ .compatible = "qcom,palawan-aggre1-noc", .data = &palawan_aggre1_noc },
	{ .compatible = "qcom,palawan-aggre2-noc", .data = &palawan_aggre2_noc },
	{ .compatible = "qcom,palawan-clk-virt", .data = &palawan_clk_virt },
	{ .compatible = "qcom,palawan-config-noc", .data = &palawan_config_noc },
	{ .compatible = "qcom,palawan-cnoc-main", .data = &palawan_cnoc_main },
	{ .compatible = "qcom,palawan-gem-noc", .data = &palawan_gem_noc },
	{ .compatible = "qcom,palawan-lpass-ag-noc", .data = &palawan_lpass_ag_noc },
	{ .compatible = "qcom,palawan-lpass-lpiaon-noc", .data = &palawan_lpass_lpiaon_noc },
	{ .compatible = "qcom,palawan-lpass-lpicx-noc", .data = &palawan_lpass_lpicx_noc },
	{ .compatible = "qcom,palawan-mc-virt", .data = &palawan_mc_virt },
	{ .compatible = "qcom,palawan-mmss-noc", .data = &palawan_mmss_noc },
	{ .compatible = "qcom,palawan-nsp-noc", .data = &palawan_nsp_noc },
	{ .compatible = "qcom,palawan-pcie-anoc", .data = &palawan_pcie_anoc },
	{ .compatible = "qcom,palawan-system-noc", .data = &palawan_system_noc },
	{ }
};
MODULE_DEVICE_TABLE(of, qnoc_of_match);

static struct platform_driver qnoc_driver = {
	.probe = qcom_icc_rpmh_probe,
	.remove = qcom_icc_rpmh_remove,
	.driver = {
		.name = "qnoc-palawan",
		.of_match_table = qnoc_of_match,
	},
};

static int __init qnoc_driver_init(void)
{
	return platform_driver_register(&qnoc_driver);
}
core_initcall(qnoc_driver_init);

static void __exit qnoc_driver_exit(void)
{
	platform_driver_unregister(&qnoc_driver);
}
module_exit(qnoc_driver_exit);

MODULE_DESCRIPTION("Palawan NoC driver");
MODULE_LICENSE("GPL");
