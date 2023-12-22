# bcx
A bc extension based on GNU bc 1.07.1 supports parallel 3-way Toom-Cook algorithm, improved division and sqrt.

Typical configure
```
CFLAGS=-O3 LDFLAGS=-lpthread ./configure --with-readline --with-paralmult
```

build
```
make
```

Run using all CPUs
```
$ bc
```

Run using given number of CPUs
```
$ BC_PARAL_CNT=5 bc
```

Enjoy.
