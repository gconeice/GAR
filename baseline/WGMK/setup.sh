sudo apt-get install -y build-essential openjdk-8-jdk git gcc-mipsel-linux-gnu g++-mips-linux-gnu llvm &&
sudo mkdir -p /opt/mipsel/usr/bin/ &&
sudo ln -s /usr/bin/mipsel-linux-gnu-gcc /opt/mipsel/usr/bin/mipsel-linux-gnu-gcc &&
sudo ln -s /usr/bin/llvm-objdump /opt/mipsel/usr/bin/mipsel-linux-gnu-llvm-objdump
