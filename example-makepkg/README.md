fakeuser makepkg example
========================

`./fakepkg` loads `libfakeuser.so` from parent directory and adds `fakeadd` to `PATH`, 
then executes `makepkg`.

NOTE: Works only if `fakeroot` is enabled in `makepkg.conf` (This is the default)

The package will contain 4 files in `/var/fakeuser-demo` and create a user and a group.   

* group: `agroup`
* user: `testuser`
* files:
    * `normal` -> no modifications
    * `grp` -> change group to agroup
    * `usr` -> change user to testuser
    * `all` -> user to testuser and 

Everything will be removed when package is uninstalled.
