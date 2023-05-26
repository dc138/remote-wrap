#!/bin/bash

cd $(dirname -- "$0")/..
trap 'jobs -p | xargs kill' EXIT

echo Running http server
cd data/
python -m http.server 8080 &
cd ..

sleep 0.2

echo Running client
make RUNFLAGS="http://0.0.0.0:8080/sample.enc" run
