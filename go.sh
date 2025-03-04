#!/usr/bin/env bash
if [[ "$1" == "build" ]]; then
  shift 1
  (source .env && cmake --preset=default && cmake --build build && build/chatcurse "$@")
else 
  (source .env && build/chatcurse "$@")
fi

