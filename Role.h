#ifndef SGC_ROLE_H__
#define SGC_ROLE_H__

// There are three "modes" for executing a circuit.
// + In "Garbler" mode, we generate a garbling of the circuit.
// + In "Evaluator" mode, we evaluate a circuit garbling.
// + In "Cleartext" mode, we simulate everything in cleartext.
// + In "Speculative" mode, we perform a dry run of the circuit to determine
//   how large the circuit is such that we can appropriately allocate resources.
enum class Role {
    // who garbling the circuits
    Garbler,
    // who evaluating the circuits
    Evaluator,
    // who simulating everything in cleartext (i.e., not 2PC)
    Cleartext,
    // dry run
    Speculate
};

#endif