#!/usr/bin/env bash

source_file='syscalls64.json'
if [[ $# == 1 ]]; then
    source_file=$1
fi

echo 'syscall_t syscall_table[] = {'
for row in $(jq -c '.syscalls[] | {number: .number, name: .name, argc: (.signature | length)}' $source_file); do
    index=$(echo $row | jq '.number')
    name=$(echo $row | jq '.name')
    argc=$(echo $row | jq '.argc')
    echo "    [$index] = {$name, $argc},"
done
echo '};'

#echo $json
