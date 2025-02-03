FROM ubuntu:24.04

RUN apt-get update && apt-get install -y cmake build-essential gdb libncurses-dev

RUN mkdir -p /code/build

WORKDIR /code/build

COPY CMakeLists.txt /code
COPY external /code/external
COPY lib /code/lib
COPY pc_hal /code/pc_hal
COPY tests /code/tests
COPY src /code/src

RUN cmake ..
RUN make -j

CMD ["./pc_hal/smartfin_pc"]