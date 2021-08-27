#!/bin/bash

pingNUM=10
target=$1
childs=()

fork_ping() {
        ping -q $target > /dev/null
}

for i in $(seq 1 ${pingNUM})
do
        ping -q $target > /dev/null &
        childs+=($!)
done

kill_cmd="kill"
for i in ${childs[@]}
do
        kill_cmd="${kill_cmd} $i"
done
echo $kill_cmd
