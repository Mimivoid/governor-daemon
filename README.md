### About

- Adjusts the governor of each cpu between frequencies to either powersave or performance dynamically
- Adjusts cstates of processor given a certain frequency

#### Todo

- Use a json file for config of daemon
- Create systemd service file

#### Requirements
- cpupower
- hwloc
#### Compiling
``g++ main.cxx -Ofast -lhwloc -lcpupower``
#### Running
``sudo ./a.out``
