#! /bin/bash

echo "`./test "s.*" "asdf"` is equal 'sdf'"
echo "`./test "s.+" "assf"` is equal 'ssf'"
echo "`./test "s.*s" "assf"` is equal 'ss'"
echo "`./test "a.*s" "assf"` is equal 'as'"
echo "`./test "a.+s" "assf"` is equal 'ass'"
echo "`./test "^a.*s" "assf"` is equal 'as'"
echo "`./test "^a.s" "assf"` is equal 'ass'"
echo "`./test "asd?" "asd"` is equal 'asd'"
echo "`./test "asd?" "asf"` is equal 'as'"
