#! /bin/bash

if ! [[ -x hfreverse ]]; then
    echo "hfreverse executable does not exist"
    exit 1
fi

../tester/run-tests.sh $*


