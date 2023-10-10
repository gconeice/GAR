wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py

python3 install.py --install --tool --ot

sudo apt remove -y --purge --auto-remove cmake

sudo apt update && \
sudo apt install -y software-properties-common lsb-release && \
sudo apt clean all

wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"

sudo apt update
sudo apt install -y iftop emacs net-tools cmake clang-12 g++-10 libc++-dev

CC=clang-12 CXX=clang++-12 cmake ../

make -j
