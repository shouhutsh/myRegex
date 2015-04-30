#! /bin/bash
./test "s.*" "asdf"
./test "s.+" "assf"
./test "s.*s" "assf"
./test "a.*s" "assf"
./test "a.+s" "assf"
./test "^a.*s" "assf"
./test "^a.s" "assf"
