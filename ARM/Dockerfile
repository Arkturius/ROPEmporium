FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update and install essential packages
RUN apt update && apt install -y		\
    build-essential						\
	libc6-dev-armel-cross				\
    libc6-armel-cross					\
    python3								\
    python3-pip							\
    python3-dev							\
    qemu-user							\
    binutils							\
    binutils-arm-linux-gnueabi			\
    && rm -rf /var/lib/apt/lists/*

RUN dpkg --add-architecture armel &&	\
    apt-get update &&					\
    rm -rf /var/lib/apt/lists/*

# Install useful Python packages for CTF
RUN pip3 install \
    ropper

# Enable ARM emulation
RUN echo ':arm:M::\x7fELF\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x28\x00:\xff\xff\xff\xff\xff\xff\xff\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfe\xff\xff\xff:/usr/bin/qemu-arm-static:' > /proc/sys/fs/binfmt_misc/register || true

# Create a working directory
WORKDIR /ctf

# Set up environment
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

ENV QEMU_LD_PREFIX=/usr/arm-linux-gnueabi
RUN ln -sf /usr/arm-linux-gnueabi/lib/ld-linux.so.3 /lib/ld-linux.so.3 || true

# Default command
CMD ["/bin/bash"]
