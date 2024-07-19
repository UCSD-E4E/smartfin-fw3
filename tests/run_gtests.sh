#!/usr/bin/env bash 

cmake  .. -Bbuild/
make -Cbuild/
mkdir -p outputs
./build/googletests 



#create graphs
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt 1> /dev/null
python3 scheduler_proccessor.py

