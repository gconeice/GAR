#include "Const.h"
#include <resource.h>

namespace SGC {

    std::size_t and_gate_without_sgc = 0;
    std::size_t and_gate_with_sgc = 0;

    std::size_t mem_n = 1024;
    std::size_t mem_w = 32;

    uint32_t reg_cnt = 13;
    uint32_t var_cnt = 1000;
  
    std::bitset<128> delta;
    PRF prf;
    SGC::PRG prg;

    std::bitset<128> backup_delta;
    PRF backup_prf;
    SGC::PRG backup_prg;

    // Initialized by both garbler and evaluator

    CircuitDesc circuit_desc = {
      .nAnd = 0,
      .nInp = 0,
      .nOut = 0
    };

    std::vector<std::bitset<128>> garbler_inputs; // just the right inputs
    std::vector<std::bitset<128>> material;
    std::vector<KappaBitString> material_buffer;
    
    std::vector<std::bitset<128>> output_decoding_table;
    size_t output_wire_idx = 0;

    // Only initialized by garbler
    std::vector<std::bitset<128>> evaluator_inputs_true;
    std::vector<std::bitset<128>> evaluator_inputs_false;

    // Only initialized by evaluator
    std::vector<std::bitset<128>> evaluator_inputs; // just the right inputs
    std::vector<bool> result_bits;
    size_t garbler_input_idx = 0;
    size_t evaluator_input_idx = 0;

    // TODO: change to PRF
    uint64_t garbler_seed;
    //uint32_t and_gate_id = 0;
    uint32_t material_id = 0;
    uint32_t buffer_idx = 0;
    uint32_t temp_buffer_idx = 0;

    uint32_t backup_and_gate_id = 0;

    void Backup() {
        // maybe redundant
        // backup_and_gate_id = and_gate_id;
        backup_delta = delta;
        backup_prf = prf;
        //backup_prg = prg;
    }

    void RestoreBackup() {
        // and_gate_id = backup_and_gate_id;
        delta = backup_delta;
        prf = backup_prf;
        //prg = backup_prg;
    }

    void AddToBuffer(KappaBitString &x) {
        SGC::material_buffer.push_back(x);
        SGC::material_id++;
    }

    /*
    void move_buffer_to_material() {
      KappaBitString cnt = material_buffer[buffer_idx];
      material.clear();
      uint32_t len = cnt.to_ulong();
      material.reserve(len);
      for (int i = 0; i < len; i++) material[i] = material_buffer[buffer_idx + i + 1];
      buffer_idx += len + 1;
    }
    */

    /*
    void move_material_to_buffer() {
      KappaBitString len = material.size();
      material_buffer.push_back(len);
      for (int i = 0; i < material.size(); i++)
        material_buffer.push_back(material[i]);
      material.clear();
    }
    */

    KappaBitString get_next_buffer() {
      //return material_buffer[buffer_id++];
      //SGC::material_id++;
      //return material_buffer[SGC::buffer_idx++];
      return material_buffer[SGC::material_id++];
    }

    /*
    void retrieve_material(std::vector<KappaBitString> &material, uint32_t &length) {
      int idx = 0;
      std::cerr << "material buffer size in ret mat " << SGC::material_buffer.size() << std::endl;
      while (SGC::material_buffer[SGC::buffer_idx + idx] != KappaBitString(0)) {
        material.push_back(SGC::material_buffer[SGC::buffer_idx + idx]);
        idx++;
      }
      std::cerr << "mat size " << material.size() << std::endl;
      length = material.size();
      SGC::material_id += length; // 1 not needed as it is not real material
      SGC::buffer_idx += (length + 1); // 1 for the all zero kappabitstring
    }
    */

    void setup_garbler(emp::NetIO *io) {
      // Test the connection
      {
        std::cerr << "Print Y if successfully connected: " << std::endl;
        // alice is SENDER
        io->send_data("Y", 1);
        // bob is RECEIVER
        char message;
        io->recv_data(&message, 1);
        std::cerr << message << std::endl;
        // flush the stream
        io->flush();
      }

      // change to delta in EpiGRAM
      /*
      SGC::delta = SGC::prg();
      SGC::delta[0] = 1;
      */

      SGC::delta = epdelta();

     //Label xxx = delta();
      //SGC::delta = static_cast<std::bitset<128>>(delta());
      //SGC::and_gate_id = 0;
      SGC::material_id = 0;
      SGC::buffer_idx = 0;

      std::cout << "Sending PRF key to evaluator... " << std::endl;
      io->send_data(prf.get_key(), sizeof(std::bitset<128>));
      io->flush();

    }

    void initiate_2pc_evaluation(emp::NetIO *io, unsigned int initial_port,
        size_t adjusted_bandwidth_delay_product) {
      std::string role = "sender";

      size_t acc_in_byte = 0;
      size_t acc_in_byte_gram = 0;
      // TODO package up properly
      // Garbler sends
      // 1. material
      // 2. his inputs
      // 3. evaluator's inputs via OT
      // 4. output decoding table

      // Evaluator does not know the sizes of the material a priori
      // Sending sizes of number of inputs is just for client convenience and is small enough
      uint64_t material_size = SGC::material_buffer.size();
      uint64_t garbler_num_inputs = SGC::garbler_inputs.size();
      // should be same size as false as labels are pairs
      uint64_t evaluator_num_inputs = SGC::evaluator_inputs_true.size();
      uint64_t output_decoding_table_size = SGC::output_decoding_table.size();
      uint64_t gram_mat_size = scost.rows;
      uint64_t gram_wmat_size = scost.wrows;
      uint64_t gram_bits_size = scost.bits;
      uint64_t gram_bytes_size = scost.bytes;
      
      /*
      std::cout << "DM " << gram_mat_size << ' '
                         << gram_wmat_size << ' ' 
                         << gram_bits_size << ' ' 
                         << gram_bytes_size << std::endl;
                         */
      
      std::vector<uint64_t> helper_variables= {material_size,
                                               garbler_num_inputs,
                                               evaluator_num_inputs,
                                               output_decoding_table_size,
                                               gram_mat_size,
                                               gram_wmat_size,
                                               gram_bits_size,
                                               gram_bytes_size};
      io->send_data(helper_variables.data(), helper_variables.size() * sizeof(uint64_t));
      acc_in_byte += helper_variables.size() * sizeof(uint64_t);
      io->flush();
      
      /*
      for (uint64_t i = 0; i < gram_mat_size/1000000; i++) {
        uint64_t cnt_this_time = (i+1 == gram_mat_size/1000000) ? gram_mat_size - i*1000000 :  1000000;
        io->send_data(gmat.data() + i*1000000, cnt_this_time * sizeof(Label));
        std::cout << i << std::endl;
        io->flush();
      }
      */
      transferLargeVector<Label>(gmat, gram_mat_size, role, adjusted_bandwidth_delay_product, "", initial_port);
      //io->send_data(gmat.data(), gram_mat_size * sizeof(Label));
      acc_in_byte += gram_mat_size * sizeof(Label);
      acc_in_byte_gram += gram_mat_size * sizeof(Label);
      transferLargeVector<WLabel>(gwmat, gram_wmat_size, role, adjusted_bandwidth_delay_product, "", initial_port);
      //io->send_data(gwmat.data(), gram_wmat_size * sizeof(WLabel));
      acc_in_byte += gram_wmat_size * sizeof(WLabel);
      acc_in_byte_gram += gram_wmat_size * sizeof(WLabel);
      bool * b_tmp_vec = new bool [gram_bits_size];
      for (int i = 0; i < gram_bits_size; i++)
      {
        b_tmp_vec[i] = gbits[i];
        //std::cout << gbits[i];
      }
      //std::cout << std::endl;
      io->send_data(b_tmp_vec, gram_bits_size * sizeof(bool)); io->flush();
      acc_in_byte += gram_bits_size * sizeof(bool);
      acc_in_byte_gram += gram_bits_size * sizeof(bool);
      delete[] b_tmp_vec;
      transferLargeVector<std::byte>(gbytes, gram_bytes_size, role, adjusted_bandwidth_delay_product, "", initial_port);
      //io->send_data(gbytes.data(), gram_bytes_size * sizeof(std::byte));
      acc_in_byte += gram_bytes_size * sizeof(std::byte);
      acc_in_byte_gram += gram_bytes_size * sizeof(std::byte);
      //io->flush();

      transferLargeVector<KappaBitString>(SGC::material_buffer, material_size, role, adjusted_bandwidth_delay_product, "", initial_port);
      //io->send_data(SGC::material_buffer.data(), material_size * sizeof(KappaBitString));
      acc_in_byte += material_size * sizeof(KappaBitString);
      io->send_data(SGC::garbler_inputs.data(), garbler_num_inputs * sizeof(KappaBitString));
      acc_in_byte += garbler_num_inputs * sizeof(KappaBitString);

      emp::IKNP<emp::NetIO> np(io);
        np.send(toBlock(SGC::evaluator_inputs_false[0]),
                toBlock(SGC::evaluator_inputs_true[0]),
                evaluator_num_inputs);

      //io->flush();

      io->send_data(SGC::output_decoding_table.data(),
        output_decoding_table_size * sizeof(std::bitset<128>));
      //io->flush();

      std::cout << "total gram materials size: " << acc_in_byte_gram << "bytes\n";
      std::cout << "total materials size: " << acc_in_byte << "bytes\n";
    }

    void setup_evaluator(bool *evaluator_inputs, emp::NetIO *io, const char *address, unsigned int initial_port,
                         size_t adjusted_bandwidth_delay_product) {
      std::string role = "receiver";
      // Test the connection
      {
        std::cerr << "Print Y if successfully connected: " << std::endl;
        // evaluator is RECEIVER
        char message;
        io->recv_data(&message, 1);
        std::cerr << message << std::endl;
        // evaluator is SENDER
        io->send_data("Y", 1);
        // flush the stream
        io->flush();
      }
      //SGC::and_gate_id = 0;
      SGC::material_id = 0;
      SGC::buffer_idx = 0;

      std::cout << "Receiving PRF key... " << std::endl;
      KappaBitString prf_key;
      io->recv_data(&prf_key, sizeof(KappaBitString));
      //io->flush();
      SGC::prf = PRF(prf_key);

      // Garbler sends
      // 1. material
      // 2. his inputs
      // 3. evaluator's inputs via OT
      // 4. output decoding table

      // helper_variables: material_size, garbler_num_inputs, evaluator_num_inputs, decoding table size
      std::vector<uint64_t> helper_variables = {0, 0, 0, 0, 0, 0, 0, 0};
      io->recv_data(helper_variables.data(), helper_variables.size() * sizeof(uint64_t));
      // io->flush();
      uint64_t material_size = helper_variables[0];
      uint64_t garbler_num_inputs = helper_variables[1];
      uint64_t evaluator_num_inputs = helper_variables[2];
      uint64_t output_decoding_table_size = helper_variables[3];
      uint64_t gram_mat_size = helper_variables[4];
      uint64_t gram_wmat_size = helper_variables[5];
      uint64_t gram_bits_size = helper_variables[6];
      uint64_t gram_bytes_size = helper_variables[7];
      // std::cout << "EDEBUG " << gram_mat_size << ' ' << gram_wmat_size << ' ' << gram_bits_size << ' ' << gram_bytes_size << std::endl;
      SGC::material_buffer.resize(material_size);
      SGC::garbler_inputs.resize(garbler_num_inputs);
      SGC::evaluator_inputs.resize(evaluator_num_inputs);
      SGC::output_decoding_table.resize(output_decoding_table_size);
      gmat.resize(gram_mat_size);
      gwmat.resize(gram_wmat_size);
      gbits.resize(gram_bits_size);
      gbytes.resize(gram_bytes_size);
      // Also reserve space for outputs

      /*      
      for (uint64_t i = 0; i < gram_mat_size/1000000; i++) {
        uint64_t cnt_this_time = (i+1 == gram_mat_size/1000000) ? gram_mat_size - i*1000000 :  1000000;
        io->recv_data(gmat.data() + i*1000000, cnt_this_time * sizeof(Label));
        std::cout << i << std::endl;
        io->flush();
      } 
      */

      transferLargeVector<Label>(gmat, gram_mat_size, role, adjusted_bandwidth_delay_product, address, initial_port);
      //io->recv_data(gmat.data(), gram_mat_size * sizeof(Label));
      transferLargeVector<WLabel>(gwmat, gram_wmat_size, role, adjusted_bandwidth_delay_product, address, initial_port);
      //io->recv_data(gwmat.data(), gram_wmat_size * sizeof(WLabel));
      bool * b_tmp_vec = new bool [gram_bits_size];
      io->recv_data(b_tmp_vec, gram_bits_size * sizeof(bool));
      for (int i = 0; i < gram_bits_size; i++)
      {
        gbits[i] = b_tmp_vec[i];
        //std::cout << gbits[i];
      }
      //std::cout << std::endl;
      delete[] b_tmp_vec;
      transferLargeVector<std::byte>(gbytes, gram_bytes_size, role, adjusted_bandwidth_delay_product, address, initial_port);
      //io->recv_data(gbytes.data(), gram_bytes_size * sizeof(std::byte));

      transferLargeVector<KappaBitString>(SGC::material_buffer, material_size, role, adjusted_bandwidth_delay_product, address, initial_port);
      //io->recv_data(SGC::material_buffer.data(), material_size * sizeof(KappaBitString));
      io->recv_data(SGC::garbler_inputs.data(), garbler_num_inputs * sizeof(KappaBitString));

      std::cerr << "sgc mat buf size in setup " << SGC::material_buffer.size() << std::endl;

      emp::IKNP<emp::NetIO> np(io);
      np.recv(toBlock(SGC::evaluator_inputs[0]),
              evaluator_inputs,
              evaluator_num_inputs);
      // io->flush();

      io->recv_data(SGC::output_decoding_table.data(), output_decoding_table_size * sizeof(KappaBitString));
      // io->flush();
    }

    std::bitset<128> HashHalfGates(const std::bitset<128> &w, const std::bitset<128> &id) {
      // This is the hash recommended in the half-gates paper p.11
      // It is a modified approach of bellare hoang rogaway
      std::bitset<128> aes_inp = w << 1;
      aes_inp ^= id;
      return prf(aes_inp) ^ aes_inp;
    }

    //TODO: check this hash
    KappaBitString HashSGC(const KappaBitString &w) {
        return prf(w) ^ w;
    }

    std::bitset<128> HashStandard(const std::bitset<128> &w1, const std::bitset<128> &w2, uint32_t &id) {
        // TODO
        std::bitset<128> aes_key = w1 << 2;
        aes_key ^= w2 << 1;
        aes_key ^= id;
        PRG aes(aes_key);
        return aes() ^ aes_key;
    }

    std::bitset<128> Find_Wire(const std::bitset<128> &wire, const bool &choose) {
        return wire ^ (choose ? delta : 0);
    }




    size_t computeNumberOfThreads(size_t material_size, size_t bandwidth_delay_product) {
      return ceil(static_cast<double>(material_size) / bandwidth_delay_product);
    }




};
