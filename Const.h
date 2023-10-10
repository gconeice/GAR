#ifndef SGC_CONST_H__
#define SGC_CONST_H__

// Project Headers
#include "Arithmetic.h"
#include "cryptographic_primitives/Prg.h"
#include "cryptographic_primitives/Prf.h"

// Standard Headers
#include <cstdint>
#include <bitset>
#include <iostream>
#include <thread>

// Submodule Headers
#include<emp-ot/emp-ot.h>
#include <emp-tool/emp-tool.h> // For NetIO

namespace SGC {

    extern std::size_t and_gate_without_sgc;
    extern std::size_t and_gate_with_sgc;

    extern std::size_t mem_n;
    extern std::size_t mem_w;


    extern uint32_t reg_cnt;
    extern uint32_t var_cnt;

    using KappaBitString = std::bitset<128>;

    enum BitOwner {GARBLER, EVALUATOR, INTERMEDIATE};

    // A "netlist description" specifies how large a circuit is.
    // "Speculative mode"'s primary job is to construct a netlist description.
    struct CircuitDesc {
      std::size_t nAnd;
      std::size_t nInp;
      std::size_t nOut;
    };

    struct CircuitPara {
      PRF prf;
      
    };

    extern CircuitDesc circuit_desc;

    extern KappaBitString delta;
    extern PRF prf;
    extern SGC::PRG prg;

    extern KappaBitString backup_delta;
    extern PRF backup_prf;
    extern SGC::PRG backup_prg;

    void Backup();
    void RestoreBackup();    
    void AddToBuffer(KappaBitString &x);

    // Initialized by both garbler and evaluator
    extern std::vector<KappaBitString> garbler_inputs; // just the right inputs
    extern std::vector<KappaBitString> material;    
    extern std::vector<KappaBitString> material_buffer; // sigma of all materials (with different languages) in some order

    void move_buffer_to_material();

    // TODO: implement this
    void move_material_to_buffer();
    void append_material_buffer(std::vector<KappaBitString>);
    KappaBitString get_next_buffer();
    void retrieve_material(std::vector<KappaBitString> &material, uint32_t &length);

    extern std::vector<KappaBitString> output_decoding_table;
    extern size_t output_wire_idx;

    // Only initialized by garbler
    extern std::vector<KappaBitString> evaluator_inputs_true;
    extern std::vector<KappaBitString> evaluator_inputs_false;

    // Only initialized by evaluator
    extern std::vector<KappaBitString> evaluator_inputs; // just the right inputs
    extern std::vector<bool> result_bits;
    extern size_t garbler_input_idx;
    extern size_t evaluator_input_idx;

    // Can be ahead of buffer_idx (buffer id follows the longest execution path)
    // material_id is increased topologically across branches
    extern uint32_t material_id;
    // Cannot be larger than material buffer
    // on the generator it follows the size of the buffer (NOT on the evaluator)
    extern uint32_t buffer_idx;
    extern uint32_t temp_buffer_idx; // this one is used in gengc material when separate buffer is used

    extern uint32_t backup_and_gate_id;

    // TODO: make it better for passing as results

    void setup_garbler(emp::NetIO *io);

    void initiate_2pc_evaluation(emp::NetIO *io, unsigned int initial_port,
                                 size_t adjusted_bandwidth_delay_product);

    void setup_evaluator(bool *evaluator_inputs, emp::NetIO *io, const char *address, unsigned int initial_port,
                         size_t adjusted_bandwidth_delay_product);

    KappaBitString HashHalfGates(const KappaBitString &w, const KappaBitString &id);

    KappaBitString HashStandard(const KappaBitString &w1, const KappaBitString &w2, uint32_t &id);

    KappaBitString HashSGC(const KappaBitString &w);

    KappaBitString Find_Wire(const KappaBitString &wire, const bool &choose);

    // Gen: Fragment(consisting of insts), seed -> vector<bool>
    //void GenSGCMaterial(std::vector<std::string> &fragment, KappaBitString &seed, std::vector<KappaBitString> &output);

    // Due to the presence of templates, implementing the following functions in the header file

    // Ensure arr is already initialized to the correct size when invoking as a receiver
    // address is empty when received (can be equivalent for convenience but its value is not used)
    template <typename T>
    //void connectionThread(std::vector<T> &arr, size_t start_idx, size_t num_elements,
    void connectionThread(T *arr, size_t start_idx, size_t num_elements,
                          const std::string& role,
                          const char *address, unsigned int port) {
      if (role == "receiver") {
        emp::NetIO *io = new emp::NetIO(address, port);
        //io->recv_data(arr.data() + start_idx, num_elements * sizeof(T));
        io->recv_data(arr + start_idx, num_elements * sizeof(T));
        io->flush();
        delete io;
      }

      if (role == "sender") {
        emp::NetIO *io = new emp::NetIO(nullptr, port);
        //io->send_data(arr.data() + start_idx, num_elements * sizeof(T));
        io->send_data(arr + start_idx, num_elements * sizeof(T));
        io->flush();
        delete io;
      }
    }

    size_t computeNumberOfThreads(size_t material_size, size_t bandwidth_delay_product); /* {
      return ceil(static_cast<double>(material_size) / bandwidth_delay_product);
    }*/

    template <typename T>
    void transferLargeVector(std::vector<T> &arr,
                             const size_t arrsize,
                             const std::string& role,
                             size_t bandwidth_delay_product,
                             const char *address, unsigned int initial_port) {
      size_t material_size = arrsize * sizeof(T);
      // Now we will form as many connections to maximize the throughput
      // ceiling of (material_size / bandwidth_delay_product)
      size_t num_connections = 32; //computeNumberOfThreads(material_size, bandwidth_delay_product);
      std::cout << "Number of Connections: " << num_connections << std::endl;

      // TODO In emp's net_io_channel (or system wide), ensure both the sender and receiver buffers are 2 * bandwidth_delay_product (as linux assumes half the buffers is used for internal kernel structures)

      std::vector<std::thread> threads;
      size_t start_idx = 0;
      size_t offset = arrsize / num_connections; // num_connections == 1 ? arrsize : bandwidth_delay_product / sizeof(T);
      if (arrsize % num_connections) offset++;

      for (size_t idx = 0; idx < num_connections; idx++) {
        unsigned int curr_port = initial_port + idx + 1;
        if (role == "sender") {
          //threads.push_back(std::thread(connectionThread<int>, std::ref(arr), start_idx, offset, role, address, curr_port));
          threads.push_back(std::thread(connectionThread<T>, arr.data(), start_idx, offset, role, "", curr_port));
        }
        if (role == "receiver") {
          //threads.push_back(std::thread(connectionThread<int>, std::ref(rcv_arr), start_idx, offset, role, "", curr_port));
          threads.push_back(std::thread(connectionThread<T>, arr.data(), start_idx, offset, role, address, curr_port));
        }
        start_idx += offset;
        if ((start_idx + offset) > arrsize) {
          offset = arrsize - start_idx;
          assert(idx == (num_connections - 2));
        }
      }

      for_each(threads.begin(), threads.end(), mem_fn(&std::thread::join));
    }

};

#endif