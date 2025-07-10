#!/bin/bash

export DFTRACER_ENABLE=1
cd $CUSTOM_CI_BUILDS_DIR

# Clone if it doesn't exist
if [ ! -d "$CUSTOM_CI_BUILDS_DIR/dftracer" ]; then
    git clone $DFTRACER_REPO $CUSTOM_CI_BUILDS_DIR/dftracer
fi

cd dftracer

git checkout $CI_COMMIT_REF_NAME

export QUEUE=pdebug
export WALLTIME=1h

flux submit -N1 --ntasks-per-node=1 -p $QUEUE -t $WALLTIME --exclusive python3 tests/py/hip_test.py
