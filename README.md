# tnsdiff (TNS Difference and Merging Utility)
Command line utility for showing and merging differences in Oracle TNS entries

## Notes
This utility is not as complete as using Linix diff(1), diff3(1) or WinDiff, nor
is it intended to be. It loads multiple TNS entries, parsing them as it goes to
ensure they are compltant. It will display the differences or merge one into
another, upon request. it was thrown togther to meet a special need I had but
has become useful in other situations and is therefore been released for others
to use.

## Usage
```
tnsdiff -d|-m <file1>|<ldap1> <file2>|<ldap2>
```
The command couldn't be simpler, use ```-d``` to show differences and ```-m```
to show merged. The "left" file or ldap is always merged with the "right" file
or ldap. If "left" or "right" is missing then the process stops.

### Show difference
```
$ tnsdiff -d tnsname1.ora tnsnames2.ora
$ tnsdiff -d tnsname1.ora ldap://myhost:389
```

### Show merged
```
$ tnsdiff -m tnsname1.ora tnsnames2.ora
$ tnsdiff -m ldap://myhost:389 tnsnames2.ora
```

### Create merged
```
$ tnsdiff -m tnsname1.ora tnsnames2.ora > tnsnames3.ora
$ tnsdiff -m ldap://myhost:389 ldap://newhost:389 > tnsnames3.ora
```
When merging ldap as "left", "right" or "both" into a new file you can
use tns2ldap to load back into an ldap directory (see tnsutils).
