# How to build
```
  make
```

# How to test on Firefox
```
  cp mozconfig-firefox <firefox>/.mozconfig
  cd <firefox>
  ./mach -v build -v &> make.coop.log
```

# How to intepret the output
The VTable and virtual call related information are dumped to files: /tmp/coop.out... 
```
  ./coop-parser.py -vf /tmp/coop.out.vf --layout /tmp/coop.out.layout --callsite /tmp/coop.out.callsite
```
