#!/bin/bash

echo 'syscall syscall_table[] = {'
for row in $(jq -c '.syscalls[] | {number: .number, name: .name, argc: (.signature | length)}' syscalls64.json); do
    index=$(echo $row | jq '.number')
    name=$(echo $row | jq '.name')
    argc=$(echo $row | jq '.argc')
    echo "    [$index] = {$name, $argc},"
done
echo '};'

#echo $json
