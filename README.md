Cluster Semaphore
=================

First steps. Very far from usable.


### Prerequisites
* systemd headers
* modern c++ compiler
* ninja build system
* kdbus

If not available from repos, install kdbus for the appropriate kernel version with:

    git clone -b v4.3 https://github.com/systemd/kdbus.git
    pushd kdbus && make && sudo make install

Add `kdbus=1` to the the boot flags and reboot.

## Compilation

`ninja` or `ninja all` shall compile, activate the service and print its status.

`ninja clean` deletes all generated files and deactivates the service.


## Running 

Look up the `test` target in `build.ninja`. The commands are quite crude so far but useful for debugging

`jadd` is a compiled version of `jadd.sh`. They are interchangeable.
