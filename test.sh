#! /bin/bash
./regex "s.*" "asdf"
./regex "s.+" "assf"
./regex "s.*s" "assf"
./regex "a.*s" "assf"
./regex "a.+s" "assf"
./regex "^a.*s" "assf"
./regex "^a.s" "assf"
