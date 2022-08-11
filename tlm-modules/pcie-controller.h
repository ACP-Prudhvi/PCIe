/*
 * TLM-2.0 PCIe controller model.
 *
 * Copyright (c) 2022 Xilinx Inc.
 * Written by Francisco Iglesias.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef SC_PCIE_CONTROLLER_H__
#define SC_PCIE_CONTROLLER_H__

#include <map>
#include <list>
#include <pthread.h>
#include "systemc.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "libpcie-callbacks.h"
#include "pf-config.h"
#include "pcie/tlp_header_defs.h"

class PCIeController : public sc_core::sc_module
{
public:
	enum {
		//
		// TLP Header size in bytes (1 DW == 4 bytes)
		//
		TLPHdr_3DW_Sz = 12,
		TLPHdr_4DW_Sz = 16,
	};

	//
	// TLP interface
	//
	tlm_utils::simple_initiator_socket<PCIeController> init_socket;
	tlm_utils::simple_target_socket<PCIeController> tgt_socket;

	//
	// User logic interface
	//
	tlm_utils::simple_initiator_socket<PCIeController> bar0_init_socket;
	tlm_utils::simple_initiator_socket<PCIeController> bar1_init_socket;
	tlm_utils::simple_initiator_socket<PCIeController> bar2_init_socket;
	tlm_utils::simple_initiator_socket<PCIeController> bar3_init_socket;
	tlm_utils::simple_initiator_socket<PCIeController> bar4_init_socket;
	tlm_utils::simple_initiator_socket<PCIeController> bar5_init_socket;

	//
	// DMA from the user application to the PCIe interface
	//
	tlm_utils::simple_target_socket<PCIeController> dma_tgt_socket;

	SC_HAS_PROCESS(PCIeController);

	PCIeController(sc_core::sc_module_name name, PhysFuncConfig cfg);

	void init();

	void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
	void b_transport_dma(tlm::tlm_generic_payload& trans, sc_time& delay);

	void prod_pcie_thread();

	void tx_tlp(uint8_t *data, unsigned int len);

	void handle_MemRd(unsigned int bar_num, uint64_t addr,
			uint8_t *data, unsigned int len);

	void handle_MemWr(unsigned int bar_num, uint64_t addr,
			uint8_t *data, unsigned int len);

	pcie_core_settings_t *GetPCIeCoreSettings();

	pcie_state_t *GetPCIeState();

	pci_bar_type_t GetBarType(int fn, pci_bar_num_t bar_num);

	inline PhysFuncConfig &GetPFConfig() { return m_cfg; };

	void PCIeHostReadDone();

private:
	unsigned int GetHdrLen(uint8_t data);
	void ByteSwap(uint8_t *data, unsigned int len);
	uint8_t GetFmt(uint8_t data);
	uint8_t GetType(uint8_t data);
	bool IsTLPMemWr(tlm::tlm_generic_payload *gp);

	void PCIeHostWrite(tlm::tlm_generic_payload& trans, sc_time& delay);
	void PCIeHostRead(tlm::tlm_generic_payload& trans, sc_time& delay);
	void TLP_tx_thread();

	std::list<tlm::tlm_generic_payload*> m_txList;
	sc_event m_tx_event;
	sc_event m_rx_event;

	tlm::tlm_generic_payload *m_dma_ongoing;
	sc_event m_dma_done_event;

	pcie_state_t *m_pcie_state;
	pcie_settings_t m_pcie_settings;

	pcie_core_settings_t m_core_settings;
	pcie_core_callbacks_t m_core_cbs;
	pcie_cfgspc_callbacks_t m_cfg_cbs;
	PhysFuncConfig m_cfg;
};

#endif