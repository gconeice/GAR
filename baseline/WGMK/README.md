# Acknowledgements

This is the baseline [WGMK16] repo: https://github.com/wangxiao1254/Secure-Computation-of-MIPS-Machine-Code.

Eprint link: https://eprint.iacr.org/2015/547

We made some small modifications for testing and fixing bugs.

# Set Up Environment

**Update Debian apt source:**

	 echo "deb http://archive.debian.org/debian/ stretch main contrib non-free" > sources.list &&

	 echo "deb http://archive.debian.org/debian/ stretch-proposed-updates main contrib non-free" >> sources.list &&

	 echo "deb http://archive.debian.org/debian-security stretch/updates main contrib non-free" >> sources.list &&

	 sudo cp sources.list /etc/apt/sources.list &&

	 sudo apt-get update

**Build the compiling toolchain:**

	sudo bash setup.sh

We test above methods already on a vanilla Debian 9 x86_64 machine.

# How to Execute Benchmarks

**Note:** In each testinginput/benchmark/settings, we include a log tested by us.

## PSI

cp testinginput/psi/64/* ./    **(resp. 256, 1024)**

**Update the Server.address to the ip of Alice in Bob's file emulator.properties**

bash comp.sh set_intersection.c

Alice: ./run.sh a.out gen

Bob : ./run.sh a.out eva

## Dijkstra

cp testinginput/dij/40/* ./    **(resp. 60, 80, 100)**

**Update the Server.address to the ip of Alice in Bob's file emulator.properties**

bash comp.sh sparse_dij.c

Alice: ./run.sh a.out gen

Bob : ./run.sh a.out eva

# How to Read the Output

**Only focus on Alice side, it outputs:**

Run time: Time in the paper (see Figure 3)

MEM ORAM READ+WRITE: # RAM Accesses in the paper (see Figure 3)

PROG ORAM READ: # Instruction Fetch in the paper (see Figure 5)

**How to test the Comm. in the paper (see Figure 3)?**

We tested it using linux command `iftop`, e.g.

iftop -i ens5 -f 'port 54321'

# How to Simulate Network Latency

**We tested it only on LAN, namely, one machine executing both Alice and Bob**

**We tested it using benchmark PSI256.**

Re Figure 8 in the paper, we use commands:

sudo tc qdisc del dev lo root

sudo tc qdisc add dev lo root handle 1: tbf rate 2Gbit burst 100000 limit 10000

sudo tc qdisc add dev lo parent 1:1 handle 10: netem delay 25msec (resp. 50msec)
