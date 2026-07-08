#ifndef STATE_SET_OEM_NVIDIA_H
#define STATE_SET_OEM_NVIDIA_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief NVIDIA OEM State Set IDs */
enum nvidia_oem_pldm_state_set_ids {
	PLDM_NVIDIA_OEM_STATE_SET_NVLINK = 0x8000,
	PLDM_NVIDIA_OEM_STATE_SET_DEBUG_STATE = 0x8001,
	PLDM_NVIDIA_OEM_STATE_SET_CLINK = 0x8003
};

/** @brief PLDM state set ID 0x8000 NVLINK values (as emitted by device).
 *
 *  Value 1 reports an inactive link; value 17 reports a fully trained
 *  link; values 2..16 report distinct training / PLL / BIST /
 *  calibration failure modes. Consumers that only need a coarse
 *  link-state may collapse 2..16 to a single "error" state.
 */
enum nvidia_oem_pldm_state_set_nvlink_values {
	PLDM_STATE_SET_NVLINK_INACTIVE = 1,
	PLDM_STATE_SET_NVLINK_INVALID_SPEEDO_CODE = 2,
	PLDM_STATE_SET_NVLINK_INVALID_FREQ = 3,
	PLDM_STATE_SET_NVLINK_INVALID_LINK = 4,
	PLDM_STATE_SET_NVLINK_C2C0_TR_FAIL = 5,
	PLDM_STATE_SET_NVLINK_C2C1_TR_FAIL = 6,
	PLDM_STATE_SET_NVLINK_1D_PR_FAIL = 7,
	PLDM_STATE_SET_NVLINK_2D_VOS_FAIL = 8,
	PLDM_STATE_SET_NVLINK_PR_REMOTE_FAIL = 9,
	PLDM_STATE_SET_NVLINK_IOBIST_FAIL = 10,
	PLDM_STATE_SET_NVLINK_C2C0_REFPLL_FAIL = 11,
	PLDM_STATE_SET_NVLINK_C2C1_REFPLL_FAIL = 12,
	PLDM_STATE_SET_NVLINK_C2C0_PLLCAL_FAIL = 13,
	PLDM_STATE_SET_NVLINK_C2C1_PLLCAL_FAIL = 14,
	PLDM_STATE_SET_NVLINK_C2C0_CLKDET_FAIL = 15,
	PLDM_STATE_SET_NVLINK_C2C1_CLKDET_FAIL = 16,
	PLDM_STATE_SET_NVLINK_ACTIVE = 17
};

/** @brief PLDM state set ID 0x8003 CLINK values (as emitted by SatMC). */
enum nvidia_oem_pldm_state_set_clink_values {
	PLDM_STATE_SET_CLINK_INACTIVE = 1,
	PLDM_STATE_SET_CLINK_FAIL_INTR = 2,
	PLDM_STATE_SET_CLINK_FAIL_EXCEPTION = 3,
	PLDM_STATE_SET_CLINK_ACTIVE = 4
};

/** @brief PLDM state set ID 0x8001 debug inteface values  */
enum pldm_state_set_debug_interface_values {
	/* always enabled or toggled on */
	PLDM_STATE_SET_DEBUG_STATE_ENABLED = 1,

	/* disabled but toggleable */
	PLDM_STATE_SET_DEBUG_STATE_DISABLED = 2,

	/* offline, cannot be enabled */
	PLDM_STATE_SET_DEBUG_STATE_OFFLINE = 3
};

#ifdef __cplusplus
}
#endif

#endif /* STATE_SET_OEM_NVIDIA_H */
