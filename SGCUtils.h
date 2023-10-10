#ifndef SGCUTILS_H__
#define SGCUTILS_H__

#include "Const.h"
#include "Bit32.h"
#include "BasicType.h"

// EpiGRAM

#include <lazypermutation.h>
#include <permute.h>
#include <util.h>
#include <epigram.h>
#include <arithmetic.h>
#include <cpu.h>


namespace SGC {



    /*
    Our Instruction Set Assembly (ISA):
    0 ADD, DST := SRC1 + SRC2
    1 MUL, DST := SRC1 * SRC2
    2 ACC, DST := !SRC1 (for now, change to GRAM in the future)
    */

    void single_inst(Inst &inst, std::vector<SGC::Bit32<Role::Evaluator>> &reg, std::vector<KappaBitString> &evmaterials);

    struct EVSession {
        Fragment prog;
        int pc;
        int nonce_id;
        int mat_id;
        std::vector<SGC::Bit32<Role::Evaluator>> reg;
        std::vector<KappaBitString> evmaterials;
        SGC::Bit32<Role::Evaluator> idx_reg;
        SGC::Bit32<Role::Evaluator> val_reg;
        EVSession() {
            pc = nonce_id = mat_id = 0;
            reg.clear();
            evmaterials.clear();
        }

        // flahs access has returning result in this session language
        void Continue(std::vector<KappaBitString> &res) {
            if (pc < prog.size()) {
                if (prog[pc].op == OPCODE::LOAD)
                    for (int i = 0; i < BIT32::SIZE; i++) reg[prog[pc].dst].bits[i].wire = res[i];
                pc++;
            }
        }

        void Skip() { 
            if (pc < prog.size()) pc++; 
        }

        bool Step() {
            // default is finished
            bool res = true;

            // set environment
            auto backup_id = SGC::material_id;
            SGC::temp_buffer_idx = mat_id;
            SGC::material_id = nonce_id;

            // execute until ending or ACC
            for (; pc < prog.size() && prog[pc].op != OPCODE::LOAD && prog[pc].op != OPCODE::STORE; pc++) SGC::single_inst(prog[pc], reg, evmaterials);

            // somehow hit ACC, either LOAD(2) or STORE(3)
            if (pc < prog.size()) {
                res = false; // not finished yet
                if (prog[pc].op == OPCODE::LOAD) { // LOAD                
                    idx_reg = reg[prog[pc].src1];
                    for (int i = 0; i < BIT32::SIZE; i++) val_reg.bits[i].wire = 0;
                }
                if (prog[pc].op == OPCODE::STORE) { // STORE
                    idx_reg = reg[prog[pc].dst];
                    val_reg = reg[prog[pc].src1];
                }
            } else {
                for (int i = 0; i < BIT32::SIZE; i++) idx_reg.bits[i].wire = 0;
                for (int i = 0; i < BIT32::SIZE; i++) val_reg.bits[i].wire = 0;
            }

            // pause environment            
            nonce_id = SGC::material_id;
            mat_id = SGC::temp_buffer_idx;
            SGC::material_id = backup_id;
            return res;
        }        
        /*
        bool Step(SGC::Bit32<Role::Evaluator> &src) {
            // default is finished
            bool res = true;

            // set environment
            auto backup_id = SGC::material_id;
            SGC::temp_buffer_idx = mat_id;
            SGC::material_id = nonce_id;

            // execute until ending or ACC
            for (; pc < prog.size() && prog[pc].op != 2; pc++) SGC::single_inst(prog[pc], reg, evmaterials);

            // somehow hit ACC
            if (pc < prog.size()) {
                res = false; // not finished yet
                src = reg[prog[pc].src1];
            } else {
                for (int i = 0; i < BIT32::SIZE; i++) src.bits[i].wire = 0;
            }

            // pause environment            
            nonce_id = SGC::material_id;
            mat_id = SGC::temp_buffer_idx;
            SGC::material_id = backup_id;
            return res;
        }
        */
    };

    struct SplitReply {
        std::vector<KappaBitString> delta;
        std::vector<KappaBitString> b;
        std::vector<SGC::PRG> prg;
    };

    struct GCMaterial {
        std::vector<KappaBitString> gc_material;
        std::vector<KappaBitString> input;
        std::vector<KappaBitString> output;        
        KappaBitString gc_delta;
        KappaBitString hash_key;
    };    

    // Gen: Fragment(consisting of insts), seed -> vector<bool>
    void GenGCMaterial(Fragment &fragment, KappaBitString &seed, uint32_t &nop, GCMaterial &res, uint32_t &SGT_length, size_t branch_idx);
    void GenGCMaterial(Fragment &fragment, KappaBitString &seed, uint32_t &nop, std::vector<KappaBitString> &gc_material, uint32_t &SGT_length, size_t branch_idx);

    // GenTable: true seed (expanding into a tree), Tree(fake seeds), vector<Fragments> -> vector<true material>, data structure(maybe tree?) to store junks
    /*
    void GetGenTable(std::vector<KappaBitString> &true_seed, 
                     std::vector<KappaBitString> &fake_seed, 
                     std::vector<Fragment> &fragments,
                     std::vector<std::vector<KappaBitString>> &true_materials,
                     std::vector<std::vector<std::vector<KappaBitString>>> &fake_materials);
    */

    // EvalGC: GableTable (not SG), Fragment, garble inputs -> garble labels of the "output" regs
    void EvalGC(std::vector<KappaBitString> &material_, Fragment &fragment, std::vector<KappaBitString> &init_wire, std::vector<KappaBitString> &output_wire, KappaBitString &hash_key,
                int material_id_before_gen, size_t SGT_length, int bid);

    // demux:
    // For G: backup original language -> generate true_seed_tree and fake_seed_tree -> materials of Gen(F_i, true_seed_i) -> append XOR materials
    //        -> restore original language -> do the logic (figure out) for demux
    // For G, the logic(vector<Bit32<Role::Garbler>> reg): a. choose branch according to reg[0]; b. translate reg[0]->reg[3] to the reg[0] branch language
    //                                                     c. send tree of seed according to reg[0]

    /*
    void Demux_G(std::vector<Fragment> &fragments, std::vector<Bit32<Role::Garbler>> &reg);

    void Demux_E(std::vector<Fragment> &fragments, std::vector<Bit32<Role::Evaluator>> &reg);

    void GOT(SGC::Bit<Role::Garbler> &b, KappaBitString &K0, KappaBitString &K1);
    KappaBitString GOT(SGC::Bit<Role::Evaluator> &b);
    */

    std::pair<KappaBitString, KappaBitString> GbCondAdd(KappaBitString &x, KappaBitString &b, KappaBitString &r);
    std::pair<KappaBitString, KappaBitString> EvCondAdd(KappaBitString &x, KappaBitString &b);    
    std::pair<KappaBitString, KappaBitString> EvCondAdd(KappaBitString &x, KappaBitString &b, KappaBitString &r);

    // researchs on kappa*logn CondAdd, which is interesting but useless now
    // think about a use case
    // k depth, i.e., 2^k branches
    void EvBranchingTree(std::vector<KappaBitString> &b, std::vector<std::vector<KappaBitString>> &res);
    void EvBitCondAdd(std::vector<std::vector<KappaBitString>> &passing_tree, KappaBitString &x, std::vector<KappaBitString> &res);
    void GbBranchingTree(std::vector<KappaBitString> &b, std::vector<std::vector<KappaBitString>> &res);
    void GbBitCondAdd(std::vector<std::vector<KappaBitString>> &passing_tree, KappaBitString &x, std::vector<KappaBitString> &res);


    std::pair<KappaBitString, KappaBitString> GbCondAdd(KappaBitString &x, KappaBitString &b, KappaBitString &offset, KappaBitString &mat);
    // stacking split mux
    void GbSplitMux(KappaBitString left_seed, KappaBitString right_seed, KappaBitString current_delta, std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials);
    void GbSplitMuxWithJunkProcess(int node_id, std::vector<std::vector<KappaBitString>> &junk_mat, KappaBitString left_seed, KappaBitString right_seed, KappaBitString current_delta, 
                                   std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials, 
                                   std::vector<std::vector<std::vector<KappaBitString>>> &junkres, 
                                   std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree,
                                   std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta, std::vector<SGC::PRG> &prg_tree);
    void EvSplitMux(int node_id, std::vector<KappaBitString> &seed_tree, std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &evmaterials);
    void EvSplitMux(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree, std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, std::vector<KappaBitString> &evmaterials, std::vector<std::vector<KappaBitString>> &evres, std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta); 
    // For G, what it is?    

    // SplitMatGenerate
    // For E is play G in-his-head
    // For G is play E in-her-head (expand junk seed)
    void GenerateSplitMatTree(std::vector<KappaBitString> &seed_tree, int depth, int x_cnt, std::vector<std::vector<KappaBitString>> &mat_tree);

    // JoinGadget
    // join x array into one x with global language
    KappaBitString GbJoinMux(int node_id, std::vector<KappaBitString> &x_array, 
                             std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree,
                             std::vector<std::vector<KappaBitString>> &junk_b,
                             std::vector<std::vector<KappaBitString>> &junk_delta,
                             std::vector<std::vector<KappaBitString>> &junk_input,
                             std::vector<KappaBitString> &gbmaterials);
    KappaBitString EvJoinMux(std::vector<KappaBitString> &mat_tree, std::vector<KappaBitString> &b_tree, std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &x);

    

    // demux+mux:
    // For E: evaluate demux logic (will give E seed tree and labels to evaluate) -> backup original language (delta + prf) -> Generate GenTable
    //        -> read XOR materials -> for every possible branch, call EvalGC -> decode mux (TODO: what is this?)


    // Sharing Seed Trees
    void GbSeedTree(std::vector<Bit<Role::Garbler>> &point_func, std::vector<KappaBitString> &bit_encode, std::vector<KappaBitString> &good_seed, std::vector<KappaBitString> &bad_seed);
    void EvSeedTree(std::vector<Bit<Role::Evaluator>> &point_func, std::vector<KappaBitString> &bit_encode, std::vector<KappaBitString> &seed_tree);


    // ISA


    // Handle the fragments
    void GbFragment(SGC::PRG &current_prg, std::vector<SGC::Bit32<Role::Garbler>> &current_reg, const SGC::Fragment &fragment, std::vector<KappaBitString> &gbmaterials, std::vector<SGC::Bit32<Role::Garbler>> &idx_array, std::vector<SGC::Bit32<Role::Garbler>> &val_array);
    void EvFragment(std::vector<SGC::Bit32<Role::Evaluator>> &current_reg, const SGC::Fragment &fragment, std::vector<KappaBitString> &evmaterials);


    // Handle Vec of Frags
    void GbStackFragments(const std::vector<Fragment> &fragments, std::vector<KappaBitString> &good_seed, 
                          std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg, std::vector<KappaBitString> &gbmaterials);

    // stackfragments with seeds
    void GbStackFragments(int node_id, KappaBitString current_seed,
                          const std::vector<Fragment> &fragments, const int &leave_bound, const int &max_length,
                          std::vector<KappaBitString> &gbmaterials);
    void GenerateCondMatTree(std::vector<KappaBitString> &seed_tree, const std::vector<Fragment> &fragments, const int &max_length,
                             std::vector<std::vector<KappaBitString>> &mat_tree);

    void EvStackFragments(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree, 
                          const std::vector<Fragment> &Fragments, const int &leave_bound,
                          std::vector<KappaBitString> &evmaterials,
                          std::vector<std::vector<KappaBitString>> &ev_input,
                          std::vector<std::vector<SGC::Bit32<Role::Evaluator>>> &ev_output);

    void GbProcessJunk(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree,
                       const std::vector<Fragment> &fragments, const int &leave_bound,
                       int lidx, int ridx,
                       std::vector<std::vector<std::vector<KappaBitString>>> &junk_in,
                       std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> &junk_output,
                       std::vector<KappaBitString> &gbmaterials);       

    void GbStackFragmentsWithJunkProcess(int node_id, KappaBitString current_seed,
                                         const std::vector<Fragment> &fragments, const int &leave_bound, const int &max_length,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg,
                                         std::vector<KappaBitString> &gbmaterials,
                                         std::vector<std::vector<KappaBitString>> &mat_tree,
                                         int lidx, int ridx,
                                         std::vector<std::vector<std::vector<KappaBitString>>> &junk_in,
                                         std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> &junk_output);

    void GbJoinRegistry(std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg, // for "active" wires
                        std::vector<std::vector<std::vector<SGC::Bit32<Role::Evaluator>>>> &junk_output, // for determistic "inactive" junk
                        std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree, 
                        std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,                        
                        std::vector<SGC::Bit32<Role::Garbler>> &join_reg);      

    void GbJoinRegistry(std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg, // for "active" wires
                        std::vector<std::vector<SGC::EVSession>> &junk_session, // for determistic "inactive" junk
                        std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree, 
                        std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,                        
                        std::vector<SGC::Bit32<Role::Garbler>> &join_reg);

    void EvJoinRegistry(std::vector<std::vector<SGC::Bit32<Role::Evaluator>>> &final_reg,
                        std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta,
                        std::vector<SGC::Bit32<Role::Evaluator>> &join_reg);

    void EvJoinRegistry(std::vector<SGC::EVSession> &ev_session,
                        std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta,
                        std::vector<SGC::Bit32<Role::Evaluator>> &join_reg);

    void EvGenerateSession(int node_id, std::vector<std::vector<KappaBitString>> &mat_tree, 
                           const std::vector<Fragment> &fragments, const int &leave_bound,
                           std::vector<KappaBitString> &evmaterials,
                           std::vector<std::vector<KappaBitString>> &ev_input,
                           std::vector<SGC::EVSession> &ev_session);

    void GbStackFragmentsWithJunkProcess(int node_id, KappaBitString current_seed,
                                         const std::vector<Fragment> &fragments, const int &leave_bound, const int &max_length,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &final_reg,
                                         std::vector<KappaBitString> &gbmaterials,
                                         std::vector<std::vector<KappaBitString>> &mat_tree,
                                         int lidx, int ridx,
                                         std::vector<std::vector<std::vector<KappaBitString>>> &junk_in,
                                         std::vector<std::vector<SGC::EVSession>> &junk_session,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &idx_array,
                                         std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &val_array);     

    void GbStepProcessJunk(std::vector<std::vector<SGC::EVSession>> &junk_session);      
     
    // Flash Join

    // Gb's Flash Join
    void GbFlashJoin(int acc_id, std::vector<std::vector<SGC::EVSession>> &junk_session, 
                     std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &idx_array,
                     std::vector<std::vector<SGC::Bit32<Role::Garbler>>> &val_array,
                     std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree, 
                     std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,                        
                     std::vector<KappaBitString> &flashmaterials,
                     SGC::Bit32<Role::Garbler> &flash_idx, 
                     SGC::Bit32<Role::Garbler> &flash_val);

    // Ev's Flash Join
    void EvFlashJoin(int acc_id, std::vector<SGC::EVSession> &ev_session, 
                     std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta,
                     SGC::Bit32<Role::Evaluator> &flash_idx,
                     SGC::Bit32<Role::Evaluator> &flash_val);

    // Flash ACC
    void GbACC(SGC::Bit32<Role::Garbler> &flash_src, SGC::Bit32<Role::Garbler> &flash_dst,
               std::vector<KappaBitString> &flashmaterials);
    void EvACC(SGC::Bit32<Role::Evaluator> &flash_src, SGC::Bit32<Role::Evaluator> &flash_dst);

    // Flash Split

    void GbSplitMux(int node_id, KappaBitString left_seed, KappaBitString right_seed, KappaBitString current_delta, 
                    std::vector<KappaBitString> &b, std::vector<KappaBitString> &x, 
                    std::vector<KappaBitString> &gbmaterials,
                    SplitReply &record);
    void GenerateSplitMatTree(std::vector<KappaBitString> &seed_tree, int depth, int x_cnt, 
                              std::vector<std::vector<KappaBitString>> &mat_tree, std::vector<SplitReply> &split_record);
         
    void GbFlashSplit(int node_id, 
                      std::vector<KappaBitString> &x, 
                      std::vector<KappaBitString> &gbmaterials,
                      SplitReply &record);
    void GenerateFlashSplitMatTree(int depth, int x_cnt, 
                                   std::vector<std::vector<KappaBitString>> &mat_tree, std::vector<SplitReply> &split_record);
    void EvFlashSplit(int node_id, int mat_node, std::vector<std::vector<KappaBitString>> &mat_tree, 
                      std::vector<KappaBitString> &x, std::vector<KappaBitString> &evmaterials, 
                      std::vector<std::vector<KappaBitString>> &evres,
                      std::vector<KappaBitString> &evb, std::vector<KappaBitString> &evdelta);   
    void GbFlashSplitMuxWithJunkProcess(int node_id, std::vector<std::vector<KappaBitString>> &junk_mat, 
                                        std::vector<KappaBitString> &x, std::vector<KappaBitString> &gbmaterials,
                                        std::vector<std::vector<std::vector<KappaBitString>>> &junkres,
                                        std::vector<KappaBitString> &delta_tree, std::vector<KappaBitString> &cond_tree,
                                        std::vector<std::vector<KappaBitString>> &junk_b, std::vector<std::vector<KappaBitString>> &junk_delta,
                                        std::vector<SGC::PRG> &prg_tree);

    // Extract Information From Fragments
    void ExtractAccessPattern(const std::vector<Fragment> &fragments, std::vector<std::vector<bool>> &access_pattern);

    template <Role R>
    inline void GetRW(const int &accid, std::vector<std::vector<bool>> &accesss_pattern,
                      std::vector<SGC::Bit<R>> &point_func, SGC::Bit<R> &rw_bit) {
        for (int i = 0; i < accesss_pattern.size(); i++) {
            if (accid < accesss_pattern[i].size() && accesss_pattern[i][accid]) 
                rw_bit = rw_bit ^ point_func[i];
        }       
    }

    // GRAM Functions
    inline void PrepareAccess(EpiGRAM<Mode::S> &sram, const int &w, const int &n, const int cnt) {
        std::vector<Garbled::Bit<Mode::S>> acc_idx(log2(n));
        std::vector<Garbled::Bit<Mode::S>> acc_val(w);
        Garbled::Bit<Mode::S> rw;
        for (int i = 0; i < cnt; i++) {
            sram.access([&](auto buff) {
                // store only on STR instruction
                buff ^= acc_val;
                buff &= ~rw;
                buff ^= acc_val;
            }, acc_idx);
        }
    }

    inline SGC::Bit32<Role::Garbler> GbACC(EpiGRAM<Mode::G> &gram, const int &w, const int &n,
                                           SGC::Bit32<Role::Garbler> &idx, SGC::Bit32<Role::Garbler> &val, SGC::Bit<Role::Garbler> &rw) {
        SGC::Bit32<Role::Garbler> res;
        int idx_width = log2(n);
        std::vector<Garbled::Bit<Mode::G>> gram_idx(idx_width);
        for (int i = 0; i < idx_width; i++) gram_idx[i] = Garbled::Bit<Mode::G>(Label(idx.bits[i].wire));
        std::vector<Garbled::Bit<Mode::G>> gram_val(w);
        for (int i = 0; i < w; i++) gram_val[i] = Garbled::Bit<Mode::G>(Label(val.bits[i].wire));
        Garbled::Bit<Mode::G> gram_rw(Label(rw.wire));
        auto acc_result = 
            gram.access([&](auto buff) {
                // store only on STR instruction
                buff ^= gram_val;
                buff &= ~gram_rw;
                buff ^= gram_val;
            }, gram_idx);     
        for (int i = 0; i < BIT32::SIZE; i++) res.bits[i].wire = acc_result[i].G();
        return res;
    }

    inline SGC::Bit32<Role::Evaluator> EvACC(EpiGRAM<Mode::E> &gram, const int &w, const int &n,
                                             SGC::Bit32<Role::Evaluator> &idx, SGC::Bit32<Role::Evaluator> &val, SGC::Bit<Role::Evaluator> &rw) {
        SGC::Bit32<Role::Evaluator> res;
        int idx_width = log2(n);
        std::vector<Garbled::Bit<Mode::E>> gram_idx(idx_width);
        for (int i = 0; i < idx_width; i++) gram_idx[i] = Garbled::Bit<Mode::E>(Label(idx.bits[i].wire));
        std::vector<Garbled::Bit<Mode::E>> gram_val(w);
        for (int i = 0; i < w; i++) gram_val[i] = Garbled::Bit<Mode::E>(Label(val.bits[i].wire));
        Garbled::Bit<Mode::E> gram_rw(Label(rw.wire));
        auto acc_result = 
            gram.access([&](auto buff) {
                // store only on STR instruction
                buff ^= gram_val;
                buff &= ~gram_rw;
                buff ^= gram_val;
            }, gram_idx);     
        for (int i = 0; i < BIT32::SIZE; i++) res.bits[i].wire = acc_result[i].E();
        return res;
    }

    // Execution of Segments
    void GbExecuteOneSeg(std::vector<SGC::Bit32<Role::Garbler>> &reg, const std::vector<Fragment> &fragments, EpiGRAM<Mode::G> &mem);
    void EvExecuteOneSeg(std::vector<SGC::Bit32<Role::Evaluator>> &reg, const std::vector<Fragment> &fragments, EpiGRAM<Mode::E> &mem);

    // Execution of single BB
    void GbExecuteSingleBB(std::vector<BasicBlock> &CFG, std::vector<uint32_t> &active_bb, 
                           std::vector<SGC::Bit32<Role::Garbler>> &reg, EpiGRAM<Mode::G> &mem);
    void EvExecuteSingleBB(std::vector<BasicBlock> &CFG, std::vector<uint32_t> &active_bb, 
                           std::vector<SGC::Bit32<Role::Evaluator>> &reg, EpiGRAM<Mode::E> &mem);    
    void GbExecuteSingleBBWithoutStacking(std::vector<SGC::Bit32<Role::Garbler>> &reg, EpiGRAM<Mode::G> &mem, const SGC::Fragment &fragment);
    void EvExecuteSingleBBWithoutStacking(std::vector<SGC::Bit32<Role::Evaluator>> &reg, EpiGRAM<Mode::E> &mem, const SGC::Fragment &fragment);

    // Some Utils For E's Calculation on Mat Fetching Size
    inline int GetSplitMuxSize(const int bsize, const int xsize) {
        return 2 * bsize + bsize * (bsize - 1) / 2 * 3 + 3 * bsize * xsize;
    }

    inline int GetMaxLength(const std::vector<Fragment> &fragments) {
        int res = 0;
        for (int fid = 0; fid < fragments.size(); fid++) {
            int tmp_res = 0;
            for (auto &inst : fragments[fid]) {
                switch (inst.op) {
                    case OPCODE::ADD:
                        tmp_res += 64;
                        break;
                    case OPCODE::ADDI:
                        tmp_res += 64;
                        break;
                    case OPCODE::SUB:
                        tmp_res += 64;
                        break;                        
                    case OPCODE::SUBI:
                        tmp_res += 64;
                        break;                        
                    case OPCODE::MUL:
                        tmp_res += 2048;
                        break;
                    case OPCODE::CMP:
                        tmp_res += 68;
                        break;
                    case OPCODE::EQ:
                        tmp_res += 62;
                        break;
                    case OPCODE::EQI:
                        tmp_res += 62;
                        break;
                    case OPCODE::AND0:
                        tmp_res += 64;
                        break;
                    case OPCODE::AND1:
                        tmp_res += 64;
                        break;
                    case OPCODE::ANDN0:
                        tmp_res += 64;
                        break;
                    case OPCODE::ANDN1:
                        tmp_res += 64;
                        break;
                    case OPCODE::CCOPY:
                        tmp_res += 128;
                        break;
                    default:      
                        break;              
                }
            }
            and_gate_without_sgc += tmp_res;
            res = std::max(res, tmp_res);
        }
        and_gate_with_sgc += res;
        return res;
    }

};

#endif