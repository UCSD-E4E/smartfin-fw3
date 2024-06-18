DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
OUTPUTS_DIR="$DIR/outputs"
mkdir -p "$OUTPUTS_DIR"
TITLE="out" 
NUM_RUNS=15
declare -a NUM_LIST=()
hasargs=0
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--title)
            hasargs=1
            TITLE="$2"
            shift 
            shift 
            ;;
        -n|--num-runs) 
            hasargs=1
            NUM_RUNS="$2"
            shift 
            shift 
            ;;
        *)
            NUM_LIST+=("$1")
            shift
            ;;
    esac
done





cd "$DIR/build"
cmake .. 
make
cd ..
if [ $hasargs -eq 0 ]; then
    
    $DIR/build/test 15 > "$OUTPUTS_DIR/log.txt"
    python3 "$DIR/scheduler_proccessor.py" "intended"
    $DIR/build/test 17 3 200 8 800 13 800 > "$OUTPUTS_DIR/log.txt"
    python3 "$DIR/scheduler_proccessor.py" "delayed"
else
    $DIR/build/test $NUM_RUNS "${NUM_LIST[@]}" > "$OUTPUTS_DIR/log.txt"
    python3 "$DIR/scheduler_proccessor.py"  "$TITLE"
fi
