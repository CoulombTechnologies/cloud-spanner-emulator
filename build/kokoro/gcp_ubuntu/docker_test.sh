#
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#!/bin/bash
base_SHA=3e80349f74f93ea2891424b1463a58ab4dabcc1d4d157b5a9e7eb0143e1a4e87
cpp_SHA=2636fd77defb341b04d82a951f0eea0d7e30e944dcaa485b8ec134866ab4ecb3
csharp_SHA=3f9108c182b5d67b9ed0f40ff69a367ad0cbacf1b66aad47914206574fb2ef1f
go_SHA=e4b906ae3bfd82279002b036ab76dd95d18d17d5b1ba88e953a1d32837640a2d
java_SHA=b6f8c4fc6abd4a292fee896ad3853e213c524ae94230f1fd49c9d74bdc315373
nodejs_SHA=f949f2ef74acb6b7a01718b6bf7ee2161d00ff087d2c8a1bde5cff32d24545a5
php_SHA=18f14e10474f6642faf427708821ddf089b73b7b58c84ce67ac339986e493260
py_SHA=f8f1c7b79133f7b50eecae5d2efa7534cd542616e24640eea58afaf44aba5e9c
ruby_SHA=4f1a19b115f71f7945bb8814ba5b92059cdca5a1859b3ed68784e65290d51c22

if [[ "$#" -eq 0 ]]; then
  echo "Running client library integration tests."
  DOCKER_ARGS="";
  DOCKER_CMD_INTEGRATION_TESTS="build/kokoro/gcp_ubuntu/integration_tests.sh"
  DOCKER_CMD_BUILD_AND_TEST="build/kokoro/gcp_ubuntu/build_and_test.sh"
elif [[ "$#" -eq 1 ]] && [[ "$1" = "--debug" ]]; then
  echo "Launching in integration test debug environment."
  # If '--debug' is specified, the script launches an interactive shell in the
  # integration test environment. This is useful for debugging integration test
  # failures. Tests can be run by setting the CLIENT_INTEGRATION_TESTS
  # environment variable and running the build.sh
  DOCKER_ARGS="-it"
  DOCKER_CMD_INTEGRATION_TESTS=""
  DOCKER_CMD_BUILD_AND_TEST=""
else
  echo "Invalid arguments to script: $@"
  echo "Usage: docker_test.sh [--debug]"
fi

# Fail on any error.
set -e

# Start echoing all the commands so they show in the log.
set -x

declare OUTPUT_DIR=""
declare LOG_DIR=""
declare SRC_DIR=""
# Account for in-Kokoro or not.
if [[ -n "${KOKORO_ARTIFACTS_DIR}" ]]; then
  # Switch to source root.
  SRC_DIR="${KOKORO_ARTIFACTS_DIR}/git/cloud-spanner-emulator"
  cd $SCR_DIR
  # On kokoro, script runs inside a Docker container as root, so $HOME
  # should point to the correct config.
  GCLOUD_CONFIG_DIR=${HOME}/.config/gcloud
  OUTPUT_DIR="${KOKORO_ARTIFACTS_DIR}"
  # Create test log directory.
  LOG_DIR="${KOKORO_ARTIFACTS_DIR}/logs"
  mkdir -p "${LOG_DIR}"
else
  # If running locally, this should be executed from the cloud-spanner-emulator
  # directory.
  SRC_DIR=$PWD
  # When running locally, you may have to use Docker with sudo, in which case,
  # the non-sudo user's gcloud config should be used, instead of the root
  # users's gcloud config.
  GCLOUD_CONFIG_DIR=$(eval echo ~$(logname))/.config/gcloud
  OUTPUT_DIR=$(mktemp -d)
  # Create test log directory.
  LOG_DIR=$(mktemp -d)
fi
echo "Placing results in: ${OUTPUT_DIR}"
echo "Placing logs in: ${LOG_DIR}"
echo "Using src directory: ${SRC_DIR}"

readonly BASE_DOCKER_IMAGE=gcr.io/cloud-spanner-emulator-builder/build-base@sha256:${base_SHA}
readonly CACHE_BUCKET="cloud-spanner-emulator-builder-bazel-cache"
echo "Client integration tests to run: ${CLIENT_INTEGRATION_TESTS}"

# Activate our GCloud credentials with docker to allow GCR access.
yes | gcloud auth configure-docker

echo "--------------------------------------------------------------------"
if [[ "$#" -eq 0 ]]; then
REMOTE_CACHE="https://storage.googleapis.com/${CACHE_BUCKET}/${BASE_DOCKER_IMAGE}"
container_id=$(docker create $DOCKER_ARGS \
    --env CC="/usr/bin/gcc" \
    --env CXX="/usr/bin/g++" \
    --env GCLOUD_DIR="/usr/bin" \
    --env CLIENT_LIB_DIR="/root/clients" \
    --env COPY_LOGS_TO="/logs" \
    --env REMOTE_CACHE="${REMOTE_CACHE}" \
    --env KOKORO_ARTIFACTS_DIR="${KOKORO_ARTIFACTS_DIR}" \
    --env EMULATOR_SRC_DIR="/src" \
    --volume "${GCLOUD_CONFIG_DIR}:/root/.config/gcloud" \
    --volume "${SRC_DIR}:/src" \
    --volume "${LOG_DIR}:/logs" \
    --workdir "/src" \
    --entrypoint="/bin/bash" \
    ${BASE_DOCKER_IMAGE} ${DOCKER_CMD_BUILD_AND_TEST})
docker start -a "$container_id"
docker wait "$container_id"
docker cp "$container_id":/src/bazel-bin/binaries/gateway_main_/gateway_main ${SRC_DIR}
docker cp "$container_id":/src/bazel-bin/binaries/emulator_main ${SRC_DIR}
docker rm "$container_id"
fi

IFS=','
for client in $CLIENT_INTEGRATION_TESTS
 do
    if [[ $client == "go" ]]; then
      SHA=$go_SHA
    elif [[ $client == "java" ]]; then
      SHA=$java_SHA
    elif [[ $client == "cpp" ]]; then
      SHA=$cpp_SHA
    elif [[ $client == "php" ]]; then
      SHA=$php_SHA
    elif [[ $client == "csharp" ]]; then
      SHA=$csharp_SHA
    elif [[ $client == "ruby" ]]; then
      SHA=$ruby_SHA
    elif [[ $client == "nodejs" ]]; then
      SHA=$nodejs_SHA
    elif [[ $client == "py" ]]; then
      SHA=$py_SHA
    else
    echo "Unrecognized client: \"${client}\"."
    fi
    DOCKER_IMAGE=gcr.io/cloud-spanner-emulator-builder/build-integration-${client}@sha256:${SHA}
    docker run $DOCKER_ARGS \
      --env CC="/usr/bin/gcc" \
      --env CXX="/usr/bin/g++" \
      --env GCLOUD_DIR="/usr/bin" \
      --env CLIENT_LIB_DIR="/root/clients" \
      --env COPY_LOGS_TO="/logs" \
      --env KOKORO_ARTIFACTS_DIR="${KOKORO_ARTIFACTS_DIR}" \
      --env CLIENT_INTEGRATION_TESTS="${client}" \
      --env EMULATOR_SRC_DIR="/src" \
      --volume "${GCLOUD_CONFIG_DIR}:/root/.config/gcloud" \
      --volume "${SRC_DIR}:/src" \
      --volume "${LOG_DIR}:/logs" \
      --workdir "/src" \
      --entrypoint="/bin/bash" \
      ${DOCKER_IMAGE} ${DOCKER_CMD_INTEGRATION_TESTS}
 done
