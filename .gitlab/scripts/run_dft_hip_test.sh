

export DFTRACER_ENABLE=1
cd $CUSTOM_CI_BUILDS_DIR

# Clone if it doesn't exist
if [ ! -d "$CUSTOM_CI_BUILDS_DIR/dftracer" ]; then
    git clone $DFTRACER_REPO $CUSTOM_CI_BUILDS_DIR/dftracer
fi

cd dftracer

git checkout $CI_COMMIT_REF_NAME

SCHEDULER_CMD 1 1 python3 tests/py/hip_test.py
