fakeuser
========

LD_PRELOAD module to create fake users (use with fakeroot)
Licence: GPL
(C) 2013 ProgAndy



    This file is part of fakeuser.

    fakeuser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    fakeuser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fakeuser.  If not, see <http://www.gnu.org/licenses/>.



`example-makepkg` contains an example for makepkg with a `package`-function 
running in a `fakeroot` with `libfakeuser.so` as a `LD_PRELOAD` module.
