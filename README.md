# Shame
Towards low latency communication between processes and machines.

## About Name
* shame is mainly based on **Sha**red **Me**mory technology
* too quick, so I feel a little bit of shame

## Features
* communication between **processes** and **machines**
* both **UDPM** and **Shared Memory** supported
* **Protobuf** message supported (Any other protocol tools could be)
* Runs on **any platform** that Boost is available

## Dependencies
* Boost
* Protobuf (optional)

## Communication Test
### Conditions
* Intel i7-7700, 3.6GHz, 8 cores
* stress --cpu 4
* message frequency: 10Hz
* message frames: 1000

### Results
Communication Delay on 90th Percentile (Unit: Milliseconds)

| Message Size | 1MB | 10MB |
|---|---|---|
| [LCM](https://lcm-proj.github.io/) | 0.187 | 1.436 |
| Shame/UDPM | 0.180 | 1.378 |
| Shame/SHM | 0.119 | 0.824 |

## Quick Start
### Install dependencies
```bash
sudo apt install libboost-all-dev
sudo apt install libprotobuf-dev
sudo apt install protobuf-compiler
```

### Build
```bash
git clone https://github.com/hongxinliu/shame.git
mkdir -p shame/build
cd shame/build
cmake .. -DCMAKE_INSTALL_PREFIX=.
make -j8 install
```

### Examples
#### Terminal 1
Run a server who holds shared memory object:
```bash
./bin/shame_server Shame 104857600
```
which means construct a (100 MB) segment of shared memory with name "Shame".

#### Terminal 2
Run listener who receives messages:
```bash
./bin/listener_raw
```
or
```bash
./bin/listener_proto
```

#### Terminal 3
Run talker who publishes messages:
```bash
./bin/talker_raw
```
or
```bash
./bin/talker_proto
```

## TODO
* logging & playback tools
* support macOS and Windows
* support more languages
