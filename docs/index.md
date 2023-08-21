## Getting Started


### Introduction

Smartfin FW-3 is the third inderation of the smartfin project, with the major break switching to a new Tracker System On a Chip (T-SOM) based board as opposed to the previous E-Series board.

As part of this shift, much of the code was re-writen, but the core logic largely stayed the same. The GPS, IMU, and file system firmware were re-written for the T-SOM, while most of the other sensors, such as the Water, Temperature, and base system logic remained consistant with Smartfin FW-2. 

### Preparing to contribute

First, read through our archeturual overview (see [architechture.md](architecture.md) and [arch.puml](arch.puml)). 

Then, have a look at the overall program flow: [fw_flow.puml](fw_flow.puml)
