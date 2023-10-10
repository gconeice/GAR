FROM gcc:11

RUN set -ex;                  \
    apt-get update;           \
    apt-get install -y cmake; \
    mkdir /epigram

ADD /test /epigram/test
ADD CMakeLists.txt /epigram/CMakeLists.txt
ADD /src /epigram/src

RUN set -ex;     \
    cd /epigram; \
    cmake .;     \
    make;        \
    ./tests
