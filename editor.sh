#! /bin/bash
pushd code > /dev/null
emacs -q -l ../emacs/init.el & > /dev/null
popd > /dev/null
