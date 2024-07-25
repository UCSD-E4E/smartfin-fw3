# Overview
The folder `tests` contains tests monatomic tests for the scheduler in the
 `src` directory. There is a bash script to manage the tests at 
 `tests/run_gtests`.

# Running tests
Use the following code from the `tests` directory:

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
