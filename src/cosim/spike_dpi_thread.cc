#include <string>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <iostream>
#include <exception>

/* ============== Spike C++ Class Definition =============== */
#include "sim.h"
#include "cfg.h"
#include "processor.h"
#include "memif.h"
#include "elfloader.h"
#include "elf.h"
#include "htif.h"
#include "mmu.h"
#include "decode.h"
#include "device.h"
/* ========================================================= */


#ifdef VPI_WRAPPER
    #include "vpi_user.h"
#elif DPI_WRAPPER
    #include "svdpi.h"
#endif

// Spike Instance Pointer
static sim_t* spike_sim_instance = nullptr;
static processor_t* spike_cpu_instance = nullptr;
static cfg_t* spike_cfg_instance = nullptr;

static std::thread spike_thread;
static std::thread spike_thread1;
static std::thread spike_thread2;
static std::mutex spike_mutex;
static std::atomic<bool> spike_exit(false);
static std::atomic<bool> spike_thread_running(false);
static std::atomic<bool> spike_thread_should_stop(false);


static void spike_step_thread() {
	while (!spike_exit.load()) {
		std::lock_guard<std::mutex> g(spike_mutex);
		spike_cpu_instance->step(1);

	    uint32_t target_pc = (uint32_t)spike_cpu_instance->get_state()->pc;
		    uint32_t encoded_instruction = 0;

		try {
			//std::lock_guard<std::mutex> g(spike_mutex);
	        mmu_t* mmu = spike_cpu_instance->get_mmu();
			
	        if (mmu) {
			  	insn_fetch_t fetched_insn_obj = mmu -> load_insn(target_pc);
	            encoded_instruction = fetched_insn_obj.insn.bits();
	            vpi_printf("[VPI_INFO] 			PC = 0x%08x -> Fetched instruction: 0x%08x\n", target_pc, encoded_instruction);
	        } else {
	            vpi_printf("[VPI_ERROR] $spike_get_instr: Failed to get MMU from Spike CPU instance.\n");
	            //vpi_free_object(arg_itr_h);
	            //return 1;
	        }
			
	    } catch (trap_t& t) {
		        vpi_printf("[VPI_ERROR] $spike_get_instr: Trap occurred during instruction fetch at PC 0x%08x (cause: %llu).\n", target_pc, (long long)t.cause());
		        encoded_instruction = 0xFFFFFFFF;
	    }
	}
}


static void spike_run_thread() {
    try {
     //   spike_thread_running.store(true);
        // sim->run() is blocking until program exit or trap; it's the main event loop (fesvr+devices+cores).
		vpi_printf("[VPI_INFO] Starting spike_sim_instance->run()...\n");
        spike_sim_instance->run();
		vpi_printf("[VPI_INFO] spike_sim_instance->run() returned. \n");
    } catch (const std::exception &e) {
        std::cerr << "[Spike VPI thread] exception in run(): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[Spike VPI thread] unknown exception in run()" << std::endl;
    }
    //spike_thread_running.store(false);
}


extern "C" {

#ifdef VPI_WRAPPER
	// ---------- VPI: $spike_init(elf_path_str) ----------
  	// Task: Initialize simulator and Load ELF file.
    PLI_INT32 spike_init_vpi_calltf(PLI_BYTE8 *user_data) {
        vpiHandle systf_handle, arg_iterator, arg_handle;
        s_vpi_value arg_value;

	        systf_handle = vpi_handle(vpiSysTfCall, NULL);

	        arg_iterator = vpi_iterate(vpiArgument, systf_handle);

	        if (!arg_iterator) {
	            vpi_printf("[VPI ERROR] $spike_init requires an ELF path argument.\n");
	            return 1;
	        }

	        arg_handle = vpi_scan(arg_iterator);

	        arg_value.format = vpiStringVal;
	        vpi_get_value(arg_handle, &arg_value);

	          std::string elf_path = arg_value.value.str;
	          //const char *elf_path = arg_value.value.str;

	        vpi_free_object(arg_iterator);

			std::lock_guard<std::mutex> g(spike_mutex);

	        if (spike_sim_instance) {
	            vpi_printf("[VPI WARN] Spike simulator is already initialized; resetting the existing instance.\n");
	            delete spike_sim_instance;
            spike_sim_instance = nullptr;
            spike_cpu_instance = nullptr;
            if (spike_cfg_instance) {
                delete spike_cfg_instance;
                spike_cfg_instance = nullptr;
            }
        }


        try {
            spike_cfg_instance = new cfg_t();
			spike_cfg_instance->isa = "RV32IMC";
            spike_cfg_instance->hartids.push_back(0);
            spike_cfg_instance->bootargs = nullptr;
			spike_cfg_instance->priv = DEFAULT_PRIV;
			spike_cfg_instance->real_time_clint = false;
			spike_cfg_instance->endianness = endianness_little;
       
    		std::vector<size_t> default_hartids;
    		default_hartids.reserve(spike_cfg_instance->nprocs());
    		for (size_t i = 0; i < spike_cfg_instance->nprocs(); ++i) {
      			default_hartids.push_back(i);
    		}
    		spike_cfg_instance->hartids = default_hartids;


			// Memory Layout
			std::vector<std::pair<reg_t, abstract_mem_t*>> mems_instance;// = make_mems(spike_cfg_instance->mem_layout);
			mems_instance.push_back({0x80000000, new mem_t(128 * 1024 * 1024)}); // {base address, memory size}


	            std::vector<device_factory_sargs_t> plugin_device_factories_dummy;
	            debug_module_config_t dm_config;
	            const char *log_path_dummy = nullptr;
	            bool dtb_enabled_dummy = true;
	            const char *dtb_file_dummy = nullptr;
	            bool socket_enabled_dummy = false;
            FILE *cmd_file_dummy = nullptr;
            std::optional<unsigned long long> instruction_limit_dummy;

            std::vector<std::string> args_vec;
            //args_vec.push_back("spike");
			//args_vec.push_back("--isa=rv32i");
            args_vec.push_back("/opt/riscv/riscv32-unknown-elf/bin/pk");
            args_vec.push_back(elf_path);
            //args_vec.push_back("--verbose");
           
			std::string spike_log_output_file = "spike_trace_vpi.log";

	            spike_sim_instance = new sim_t(
	                spike_cfg_instance,         // const cfg_t *cfg
	                false,                      // bool halted
	                mems_instance,                 		// std::vector<std::pair<reg_t, abstract_mem_t*>> mems
	                plugin_device_factories_dummy, // const std::vector<device_factory_sargs_t>& plugin_device_factories
	                args_vec,                   // const std::vector<std::string>& args
	                dm_config,                  // const debug_module_config_t &dm_config
 				  spike_log_output_file.c_str(),
                  //log_path_dummy,             // const char *log_path
                dtb_enabled_dummy,          // bool dtb_enabled
                dtb_file_dummy,             // const char *dtb_file
                socket_enabled_dummy,       // bool socket_enabled
                cmd_file_dummy,             // FILE *cmd_file
				//std::nullopt
                instruction_limit_dummy     // std::optional<unsigned long long> instruction_limit
            );


				spike_sim_instance->set_debug(false); // spike debug mode
            spike_cpu_instance = spike_sim_instance->get_core(0);
			spike_sim_instance->get_core(0)->reset();
            
				if (!spike_cpu_instance) {
	                vpi_printf("[VPI ERROR] $spike_init: failed to get CPU instance.\n");
	                if (spike_sim_instance) { delete spike_sim_instance; spike_sim_instance = nullptr; }
	                if (spike_cfg_instance) { delete spike_cfg_instance; spike_cfg_instance = nullptr; }
	                return 1;
            }

			spike_cpu_instance->enable_log_commits();

			//reg_t entry = 0;
			//memif_t memif(spike_sim_instance);
				//load_elf(elf_path, &memif, &entry, 0, 32);
			//spike_cpu_instance->get_state()->pc = entry;

			// Check elf_path
			std::ifstream elf_file_chk(elf_path, std::ios::ate | std::ios::binary);
			if (!elf_file_chk.is_open()) {
				std::cerr << "[VPI ERROR] $spike_init - ELF can't not be opend: " << elf_path << std::endl;
				return 1;
			}
			else {
            	std::cout << "[VPI INFO] $spike_init - ELF load success(" << elf_path << ")" << std::endl;
            	//std::cout << "[VPI INFO] $spike_init - Entry point = 0x" << std::hex << entry << std::endl; // 0x%lx
				//return elf_file_chk.tellg();
			}
	        } catch (const std::exception& e) {
	            vpi_printf("[VPI ERROR] $spike_init: exception while creating Spike sim_t: %s\n", e.what());
	            if (spike_sim_instance) { delete spike_sim_instance; spike_sim_instance = nullptr; }
	            if (spike_cfg_instance) { delete spike_cfg_instance; spike_cfg_instance = nullptr; }
	            return 1;
	        } /*catch (...) {
	            vpi_printf("[VPI ERROR] $spike_init: unknown exception while creating Spike sim_t.\n");
	            if (spike_sim_instance) { delete spike_sim_instance; spike_sim_instance = nullptr; }
	            if (spike_cfg_instance) { delete spike_cfg_instance; spike_cfg_instance = nullptr; }
            return 1;
        }*/

		//spike_sim_instance->run();
		//spike_sim_instance->start();

    
		// ----------- Launch background thread running sim->run() -----------
		
	    try {
			spike_thread_should_stop.store(false);
    	    spike_thread = std::thread(spike_run_thread);
			vpi_printf("[VPI_INFO] $spike_start: start run thread.\n");
	    } catch (const std::exception &e) {
		    vpi_printf("[VPI ERROR] $spike_init: fail to spawn thread: %s\n", e.what());
		    return 1;
		}
		

		//spike_thread1 = std::thread(spike_run_thread);

	        return 0;
    }

	PLI_INT32 spike_start_vpi_calltf(PLI_BYTE8 *user_data) {
	  /*
	    try {
			spike_thread_should_stop.store(false);
    	    spike_thread = std::thread(spike_run_thread);
			vpi_printf("[VPI_INFO] $spike_start: start run thread.\n");
	    } catch (const std::exception &e) {
		    vpi_printf("[VPI ERROR] $spike_start: fail to spawn thread: %s\n", e.what());
		    return 1;
		}
*/
		//spike_thread2 = std::thread(spike_step_thread);

     	return 0;
    }


	// ---------- VPI task: $spike_stop() ----------
	PLI_INT32 spike_stop_vpi_calltf(PLI_BYTE8 *user_data) {
	    std::lock_guard<std::mutex> g(spike_mutex);
	    if (!spike_sim_instance) {
	        vpi_printf("[VPI] $spike_stop: not initialized\n");
	        return 0;
	    }
	
	    // Try to request stop if sim_t exposes such an API (many versions don't)
	    try {
	        // --- ADAPT HERE: if your sim_t supports stop(), call it. Otherwise rely on thread join after program exit.
	        // spike_sim->stop(); // uncomment if available

	    } catch(...) {}
	
	    // If thread is joinable, join (may block until run() returns)
	    if (spike_thread.joinable()) {
	        vpi_printf("[VPI] $spike_stop: joining spike thread (may block)\n");
	        spike_thread.join();
	    }
	
	    // cleanup
	    delete spike_sim_instance;
	    spike_sim_instance = nullptr;
	    spike_cpu_instance = nullptr;
	    if (spike_cfg_instance) { delete spike_cfg_instance; spike_cfg_instance = nullptr; }
	
	    vpi_printf("[VPI] $spike_stop: cleaned up\n");
	    return 0;
	}


	/*
	// VPI: set PC trigger
	PLI_INT32 vpi_spike_set_pc_trigger_calltf(char*user) {
		s_vpi_value val;
		val.format = vpiIntVal;
		vpi_handle arg_iter = vpi_iterate(vpiArgument,NULL);
		vpi_handle arg = vpi_scan(arg_iter);
		vpi_get_value(arg,&val);
	
		pc_trigger = (uint64_t)val.value.integer;
		pc_trigger_valid = true;
		pc_trigger_reached = false;
	
		return 0;
	}
	*/



	// ---------- VPI: $spike_run_steps(N) ----------
	// Task: run N steps (instructions)
	PLI_INT32 spike_run_steps_vpi_calltf(PLI_BYTE8* user_data) {
		vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
		if (!systf_handle) {
			vpi_printf("[VPI ERROR] $spike_run_steps: invalid handle.\n");
			return 1;
		}

		// extract integer argument
		vpiHandle args_iter = vpi_iterate(vpiArgument, systf_handle);
		if (!args_iter) {
			vpi_printf("[VPI ERROR] $spike_run_steps: requires integer step count\n");
			return 1;
		}
		vpiHandle arg = vpi_scan(args_iter);
		s_vpi_value arg_val;
		arg_val.format = vpiIntVal;
		vpi_get_value(arg, &arg_val);
		int steps = arg_val.value.integer;
		vpi_free_object(args_iter);

		if (!spike_sim_instance || !spike_cpu_instance) {
			vpi_printf("[VPI ERROR] $spike_run_steps: spike not initialized\n");
			return 1;
		}

		if (steps <= 0) {
			vpi_printf("[VPI WARN] $spike_run_steps: non-positive steps (%d) ignored\n", steps);
			return 1;
		}

		// Run steps: call cpu->step repeatedly (many Spike versions accept step(1))
		//for (int i = 0; i < steps; ++i) {
		std::lock_guard<std::mutex> g(spike_mutex);
			spike_cpu_instance->step(steps);
		//}

		//vpi_printf("[VPI_INFO] $spike_run_steps: ran %d step(s)\n", steps);

		return 0;
	}



	// ---------- VPI: $spike_get_pc ----------
	// Task: Print PC value
	/*
	PLI_INT32 spike_get_pc_vpi_calltf(PLI_BYTE8* user_data) {
		if (!spike_sim_instance || !spike_cpu_instance) {
			vpi_printf("[VPI_ERROR] $spike_get_pc: Spike not initialized\n");
			return 1;
		}

		//reg_t pc = spike_cpu_instance->get_state()->pc;
        uint32_t pc = (uint32_t)spike_cpu_instance->get_state()->pc;
		vpi_printf("[VPI_INFO] PC = 0x%08x\n", pc);

		return 0;
	}
	*/

	PLI_INT32 spike_get_pc_vpi_calltf(PLI_BYTE8* user_data) {
	    vpiHandle tf_call_h;
	    vpiHandle arg_itr_h;
	    vpiHandle arg_h;
	    s_vpi_value arg_value;
	
		    tf_call_h = vpi_handle(vpiSysTfCall, NULL);
	    if (!tf_call_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_pc: Failed to get task call handle.\n");
	        return 1;
	    }
	
		    arg_itr_h = vpi_iterate(vpiArgument, tf_call_h);
	    if (!arg_itr_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_pc: No arguments provided to $spike_get_pc. Expected one argument.\n");
	        return 1;
	    }
	
		    arg_h = vpi_scan(arg_itr_h);
	    if (!arg_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_pc: Failed to get the first argument handle.\n");
		        vpi_free_object(arg_itr_h);
	        return 1;
	    }
	
		    if (!spike_sim_instance || !spike_cpu_instance) {
	        vpi_printf("[VPI_ERROR] $spike_get_pc: Spike not initialized.\n");
	        vpi_free_object(arg_itr_h);
	        return 1;
	    }
	
	    uint32_t pc = (uint32_t)spike_cpu_instance->get_state()->pc;
	    //vpi_printf("[VPI_INFO] PC = 0x%08x\n", pc);

		    arg_value.format = vpiIntVal;
		    arg_value.value.integer = (PLI_INT32)pc;

		    vpi_put_value(arg_h, &arg_value, NULL, vpiNoDelay);

		    vpi_free_object(arg_itr_h);

	    return 0;
	}
	
	
		// VPI: $spike_get_instr
	PLI_INT32 spike_get_instr_vpi_calltf(PLI_BYTE8* user_data) {
	    vpiHandle tf_call_h;
	    vpiHandle arg_itr_h;
		    vpiHandle pc_arg_h;
		    vpiHandle insn_arg_h;
	    s_vpi_value arg_value;
	    //reg_t target_pc = 0; 
	    uint32_t target_pc = 0; 
		    //insn_t encoded_instruction = 0;
		    uint32_t encoded_instruction = 0;

		    tf_call_h = vpi_handle(vpiSysTfCall, NULL);
	    if (!tf_call_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_instr: Failed to get task call handle.\n");
	        return 1;
	    }
	
		    arg_itr_h = vpi_iterate(vpiArgument, tf_call_h);
	    if (!arg_itr_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_instr: No arguments provided. Expected two arguments (PC, instruction_output).\n");
	        return 1;
	    }
	
		    pc_arg_h = vpi_scan(arg_itr_h);
	    if (!pc_arg_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_instr: Failed to get the first argument (PC address).\n");
	        vpi_free_object(arg_itr_h);
	        return 1;
	    }
	
		    insn_arg_h = vpi_scan(arg_itr_h);
	    if (!insn_arg_h) {
	        vpi_printf("[VPI_ERROR] $spike_get_instr: Failed to get the second argument (encoded_instruction output).\n");
	        vpi_free_object(arg_itr_h);
	        return 1;
	    }
	
		    if (!spike_cpu_instance) {
	        vpi_printf("[VPI_ERROR] $spike_get_instr: Spike CPU instance not initialized.\n");
	        vpi_free_object(arg_itr_h);
	        return 1;
	    }
	
		    arg_value.format = vpiIntVal;
		    vpi_get_value(pc_arg_h, &arg_value);
		    target_pc = (reg_t)arg_value.value.integer;
	
	    //vpi_printf("[VPI_INFO] Requesting instruction at PC = 0x%08x\n", target_pc);
	
		    try {

			std::lock_guard<std::mutex> g(spike_mutex);

		        mmu_t* mmu = spike_cpu_instance->get_mmu();
			
	        if (mmu) {
			  	insn_fetch_t fetched_insn_obj = mmu -> load_insn(target_pc);
	            encoded_instruction = fetched_insn_obj.insn.bits();
	            vpi_printf("[VPI_INFO] 			PC = 0x%08x -> Fetched instruction: 0x%08x\n", target_pc, encoded_instruction);
	        } else {
	            vpi_printf("[VPI_ERROR] $spike_get_instr: Failed to get MMU from Spike CPU instance.\n");
	            vpi_free_object(arg_itr_h);
	            return 1;
	        }
			
		    } catch (trap_t& t) {
			        vpi_printf("[VPI_ERROR] $spike_get_instr: Trap occurred during instruction fetch at PC 0x%08x (cause: %llu).\n", target_pc, (long long)t.cause());
			        encoded_instruction = 0xFFFFFFFF;
			    }

			    arg_value.format = vpiIntVal;
			    arg_value.value.integer = (PLI_INT32)encoded_instruction;

			    vpi_put_value(insn_arg_h, &arg_value, NULL, vpiNoDelay);

			    vpi_free_object(arg_itr_h);

			    return 0;
	}







	// ---------- VPI: $spike_get_reg(idx) ----------
	// Task: Print reigster idx(x0~x31) values
	PLI_INT32 spike_get_reg_vpi_calltf(PLI_BYTE8* user_data) {
		if (!spike_sim_instance || !spike_cpu_instance) {
			vpi_printf("[VPI ERROR] $spike_get_reg: spike not initialized\n");
			return 1;
		}

		vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
		if (!systf_handle) {
			vpi_printf("[VPI ERROR] $spike_get_reg: invalid handle\n");
			return 1;
		}

		vpiHandle args_iter = vpi_iterate(vpiArgument, systf_handle);
		if (!args_iter) {
			vpi_printf("[VPI ERROR] $spike_get_reg: missing reg index argument\n");
			return 1;
		}

		vpiHandle arg = vpi_scan(args_iter);
		s_vpi_value val;
		val.format = vpiIntVal;
		vpi_get_value(arg, &val);
		int idx = val.value.integer;
		vpi_free_object(args_iter);

		if (idx < 0 || idx >= 32) {
			vpi_printf("[VPI ERROR] $spike_get_reg: invalid reg index %d\n", idx);
			return 1;
		}

		//reg_t regval = spike_cpu_instance->get_state()->XPR[idx];
        uint32_t reg_val = (uint32_t)spike_cpu_instance->get_state()->XPR[idx];
		vpi_printf("[VPI_INFO] reg idx(x%d) = 0x%08x\n", idx, reg_val);

		return 0;
	}


	    PLI_INT32 cleanup_spike_vpi(p_cb_data cb_data_p) {
        if (spike_sim_instance) {
            delete spike_sim_instance;
            spike_sim_instance = nullptr;
            spike_cpu_instance = nullptr;
            std::cout << "[VPI_INFO] Spike simulator object cleaned up." << std::endl;
        }
        return 0;
    }

	    void register_spike_vpi_tasks() {
        // Task 1: $spike_init
        s_vpi_systf_data tf_data_init;
        tf_data_init.type = vpiSysTask;
        tf_data_init.sysfunctype = 0;
        tf_data_init.calltf = spike_init_vpi_calltf;
        tf_data_init.compiletf = NULL;
        tf_data_init.sizetf = NULL;
        tf_data_init.user_data = NULL;
        tf_data_init.tfname = "$spike_init";
        vpi_register_systf(&tf_data_init);

		// Task 2-1: $spike_start
        s_vpi_systf_data tf_data_start;
        tf_data_start.type = vpiSysTask;
        tf_data_start.sysfunctype = 0;
        tf_data_start.calltf = spike_start_vpi_calltf;
        tf_data_start.compiletf = NULL;
        tf_data_start.sizetf = NULL;
        tf_data_start.user_data = NULL;
        tf_data_start.tfname = "$spike_start";
        vpi_register_systf(&tf_data_start);

		 // Task 2-2: $spike_stop
        s_vpi_systf_data tf_data_stop;
        tf_data_stop.type = vpiSysTask;
        tf_data_stop.sysfunctype = 0;
        tf_data_stop.calltf = spike_stop_vpi_calltf;
        tf_data_stop.compiletf = NULL;
        tf_data_stop.sizetf = NULL;
        tf_data_stop.user_data = NULL;
        tf_data_stop.tfname = "$spike_stop";
        vpi_register_systf(&tf_data_stop);


        // Task 2: $spike_run_steps
        s_vpi_systf_data tf_data_run;
        tf_data_run.type = vpiSysTask;
        tf_data_run.sysfunctype = 0;
        tf_data_run.calltf = spike_run_steps_vpi_calltf;
        tf_data_run.compiletf = NULL;
        tf_data_run.sizetf = NULL;
        tf_data_run.user_data = NULL;
        tf_data_run.tfname = "$spike_run_steps";
        vpi_register_systf(&tf_data_run);

		// Task 3: $spike_get_pc
        s_vpi_systf_data tf_data_get_pc;
        tf_data_get_pc.type = vpiSysTask;
        tf_data_get_pc.sysfunctype = 0;
        tf_data_get_pc.calltf = spike_get_pc_vpi_calltf;
        tf_data_get_pc.compiletf = NULL;
		tf_data_get_pc.sizetf = NULL;
		tf_data_get_pc.user_data = NULL;
        tf_data_get_pc.tfname = "$spike_get_pc";
        vpi_register_systf(&tf_data_get_pc);

		// Task 4: $spike_get_instr
        s_vpi_systf_data tf_data_get_instr;
        tf_data_get_instr.type = vpiSysTask;
        tf_data_get_instr.sysfunctype = 0;
        tf_data_get_instr.calltf = spike_get_instr_vpi_calltf;
        tf_data_get_instr.compiletf = NULL;
		tf_data_get_instr.sizetf = NULL;
		tf_data_get_instr.user_data = NULL;
        tf_data_get_instr.tfname = "$spike_get_instr";
        vpi_register_systf(&tf_data_get_instr);

		// Task 4: $spike_get_reg
        s_vpi_systf_data tf_data_get_reg;
        tf_data_get_reg.type = vpiSysTask;
        tf_data_get_reg.sysfunctype = 0;
        tf_data_get_reg.calltf = spike_get_reg_vpi_calltf;
        tf_data_get_reg.compiletf = NULL;
		tf_data_get_reg.sizetf = NULL;
		tf_data_get_reg.user_data = NULL;
        tf_data_get_reg.tfname = "$spike_get_reg";
        vpi_register_systf(&tf_data_get_reg);


		// register callback in the end of simulation
        s_cb_data cb_data_end;
        cb_data_end.reason = cbEndOfSimulation;
        cb_data_end.cb_rtn = cleanup_spike_vpi;
        cb_data_end.obj = NULL;
        cb_data_end.time = NULL;
        cb_data_end.value = NULL;
        cb_data_end.user_data = NULL;
        vpi_register_cb(&cb_data_end);
    }


	    void (*vlog_startup_routines[])() = {
        register_spike_vpi_tasks,
        0
    };



#elif DPI_WRAPPER
 
	    int spike_init(const char* elf_path_c) {

	        std::string elf_path = elf_path_c;

	        if (spike_sim_instance) {
	            std::cout << "[DPI WARN] Spike simulator is already initialized; resetting it." << std::endl;
            delete spike_sim_instance;
            spike_sim_instance = nullptr;
            spike_cpu_instance = nullptr;
            if (spike_cfg_instance) {
                delete spike_cfg_instance;
                spike_cfg_instance = nullptr;
            }
        }

	        spike_cfg_instance = new cfg_t();
	        spike_cfg_instance->isa = "RV32IMC";
	        spike_cfg_instance->hartids.push_back(0);
	        spike_cfg_instance->bootargs = nullptr;

	        std::cout << "[DPI] cfg_t instance configured. ISA: "
	                  << (spike_cfg_instance->isa ? spike_cfg_instance->isa : "N/A")
	                  << ", Harts: " << spike_cfg_instance->nprocs() << std::endl;


//        std::vector<std::pair<reg_t, abstract_mem_t*>> mems;

	        std::vector<device_factory_sargs_t> plugin_device_factories;
	        debug_module_config_t dm_config;
        const char *log_path = nullptr;
        bool dtb_enabled = true;
        const char *dtb_file = nullptr;
        bool socket_enabled = true;
        FILE *cmd_file = nullptr;
        std::optional<unsigned long long> instruction_limit;

        std::vector<std::string> args_vec;
	        args_vec.push_back("spike");
	        args_vec.push_back(elf_path);

	        try {
	            spike_sim_instance = new sim_t(
	                spike_cfg_instance,         // const cfg_t *cfg
	                false,                      // bool halted
                mems,                       // std::vector<std::pair<reg_t, abstract_mem_t*>> mems
                plugin_device_factories,    // const std::vector<device_factory_sargs_t>& plugin_device_factories
                args_vec,                   // const std::vector<std::string>& args
                dm_config,                  // const debug_module_config_t &dm_config
                log_path,                   // const char *log_path
                dtb_enabled,                // bool dtb_enabled
                dtb_file,                   // const char *dtb_file
                socket_enabled,             // bool socket_enabled
                cmd_file,                   // FILE *cmd_file
                instruction_limit           // std::optional<unsigned long long> instruction_limit
            );
	        } catch (const std::exception& e) {
	            std::cerr << "[DPI ERROR] exception while creating Spike sim_t: " << e.what() << std::endl;
	            if (spike_cfg_instance) { delete spike_cfg_instance; spike_cfg_instance = nullptr; }
	            return 1;
	        }


	        spike_cpu_instance = spike_sim_instance->get_core(0);
	        if (!spike_cpu_instance) {
	            std::cerr << "[DPI ERROR] failed to get Spike processor_t instance." << std::endl;
	            delete spike_sim_instance; spike_sim_instance = nullptr;
	            delete spike_cfg_instance; spike_cfg_instance = nullptr;
	            return 1;
	        }

	        std::cout << "[DPI] Spike simulator initialized and ELF loaded: " << elf_path << std::endl;

	        return 0;
	    }

/*
	    void spike_step() {
     	if (!cpu) {
	   std::cerr << "[Spike] ERROR: CPU not initialized\n";
	   return;
	}
    	cpu->step(1);
    }

	    uint32_t spike_get_pc() {
    	return (uint32_t)cpu->get_state()->pc;
    }

	    uint32_t spike_get_reg(int idx) {
    	return (uint32_t)cpu->get_state()->XPR[idx];
    }


	    /*
    uint32_t spike_read_mem(uint32_t addr) {
    	uint32_t val;

	sim->debug_mmu->load_mem(addr, sizeof(val), (uint8_t*)&val);

	return val;
    }
    */

//#else
//    std::cerr << "[Spike] ERROR: DPI/VPI interface is not defined. Add the option under compiling with g++\n";
#endif

}
