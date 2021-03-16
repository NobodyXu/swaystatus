#!/bin/bash

exit_code=0

for each in $@; do
    echo -e '\nRunning' $each ...'\n'

    cd $(dirname $each)
    ./$(basename $each)

    [ $? -ne 0 ] && exit_code=1

    cd ..

    echo
done

exit $exit_code
