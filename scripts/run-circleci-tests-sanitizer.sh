#!/bin/bash
tests=(
    "/root/project/build/bin/fmi-tests --gtest_filter=-*ci_skip*"
    "/root/project/build/bin/helics_fmi-tests --gtest_filter=-*ci_skip*"
    "/root/project/build/bin/helics_fmi_executable-tests --gtest_filter=-*ci_skip*"
)

SUMRESULT=0
for test in "${tests[@]}"; do
    echo "${test}"
    eval "${test}"
    RESULT=$?
    echo "***Latest test result: "${RESULT}
    SUMRESULT=$((SUMRESULT + RESULT))
done
# Return 0 or a positive integer for failure
exit ${SUMRESULT}
