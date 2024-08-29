#!/usr/bin/env bash 

cmake  .. -Bbuild/
make -Cbuild/
mkdir -p outputs
mkdir -p inputs
mkdir -p no_check_outputs
mkdir -p no_check_inputs

./build/googletests 
./build/examine_behaivor


#create graphs
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt 1> /dev/null
python3 scheduler_proccessor.py

