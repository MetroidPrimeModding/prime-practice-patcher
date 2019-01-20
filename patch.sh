#!/usr/bin/env bash
platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
   chmod +x prime_practice_patcher_macos
   ./prime_practice_patcher_macos patch prime.iso
else
   chmod +x prime_practice_patcher_linux
   ./prime_practice_patcher_linux patch prime.iso
fi
