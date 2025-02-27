#!/usr/bin/env bash
(source .env && cmake --preset=default && cmake --build build && build/chatcurse "$@")
