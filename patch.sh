#!/usr/bin/env bash
platform='unknown'
unamestr=`uname`
if [[ "$unamestr" == 'Darwin' ]]; then
   ./prime-practice-mod-patcher-macos patch prime.iso
else
   ./prime-practice-mod-patcher-linux patch prime.iso
fi
