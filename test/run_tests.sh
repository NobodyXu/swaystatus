#!/bin/bash

echo Env variable PYTHONPATH = ${PYTHONPATH}

exit_code=0

for each in $1; do
    echo -e '\nRunning' $each ...
    ./$each

    [ $? -ne 0 ] && exit_code=1

    echo
done

exit $exit_code
