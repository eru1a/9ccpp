#!/bin/bash

try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  g++ -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 'return 0;'
try 42 'return 42;'
try 21 'return 5+20-4;'
try 41 'return  12 + 34 - 5 ;'
try 47 'return 5+6*7;'
try 15 'return 5*(9-6);'
try 4 'return (3+5)/2;'
try 10 'return -10+20;'
try 10 'return - -10;'
try 10 'return - - +10;'

try 0 'return 0==1;'
try 1 'return 42==42;'
try 1 'return 0!=1;'
try 0 'return 42!=42;'

try 1 'return 0<1;'
try 0 'return 1<1;'
try 0 'return 2<1;'
try 1 'return 0<=1;'
try 1 'return 1<=1;'
try 0 'return 2<=1;'

try 1 'return 1>0;'
try 0 'return 1>1;'
try 0 'return 1>2;'
try 1 'return 1>=0;'
try 1 'return 1>=1;'
try 0 'return 1>=2;'

try 3 'return foo = 3;'
try 14 'foo123 = 3; bar = 5 * 6 - 8; return foo123 + bar / 2;'

try 1 'return 1; 2; 3;'
try 2 '1; return 2; 3;'
try 3 '1; 2; return 3;'
try 15 'returnx = 3; return1 = 5; return returnx * return1;'

try 3 'if (0) return 2; return 3;'
try 3 'if (0) return 2; else return 3;'
try 3 'if (1-1) return 2; else return 3;'
try 2 'if (1) return 2; return 3;'
try 2 'if (1) return 2; else return 3;'
try 2 'if (2-1) return 2; else return 3;'

echo OK
