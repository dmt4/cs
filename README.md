Cluster Semaphore
=================

First steps.

# Compilation
## Prerequisites
* systemd using kdbus
Install kdbus for the appropriate kernel version.

    git clone -b v4.3 https://github.com/systemd/kdbus.git
    pushd kdbus && make && sudo make install

Add `kdbus=1` to the the boot flags and reboot.

* systemd headers
* modern c++ compiler

This shall do:

    make && make jadd


# Running 
 
    ./jadd.sh name request
    ./jlst.sh
    ./jadd.sh id1 id2 ...

    for i in `seq 100`; do ./jadd n$i r$i; done

    sudo -u otheruser ./jadd name request; done
    
    ./jdel.sh `seq 110`

`jadd` is a compiled version of `jadd.sh`. They can be used interchangeably.

