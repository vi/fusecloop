#!/usr/bin/perl -w
# Coded by _Vi. 31.8.2007. LGPL.
open(FF,"<VERSION");
$_ = <FF>;
m/(\d+\.\d+)\.(\d+)/;
close(FF);
open(FF,">VERSION");
print FF $1, '.', $2 + 1;
close(FF);
