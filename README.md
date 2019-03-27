# Linknx [![Build Status](https://travis-ci.org/linknx/linknx.svg?branch=master)](https://travis-ci.org/linknx/linknx)
This repository is a migration of the legacy one hosted on Sourceforge
(:pserver:anonymous@linknx.cvs.sourceforge.net:/cvsroot/linknx)

The migration is aimed at simplifying the collaboration and maintenance of
Linknx's source code in the future.

## Documentation
Read the [wiki pages](https://sourceforge.net/p/linknx/wiki/Main_Page/) on sourceforge before the full wiki is migrated here. Thanks for your patience!

## How to build linknx
The best option is to download a tarball corresponding to a specific version (see links below). Then, run
```
./configure
make install
```
If the build finishes with errors, review them carefully. The most common cause is a dependency that is not installed on your system.

If you need to build the latest revision from the *master* branch (which is neither required nor recommended for standard usage), do not forget to update the GNU build system first.
```
autoreconf --install
```
should suffice. Then, run configure and make install as for other versions.

## Downloads
### Latest stable release (0.0.1.37)
This is the latest version as of today. Unless you know what you are doing, you should use this version. Download a zip
of it [here](https://github.com/linknx/linknx/archive/0.0.1.37.zip).
Implements a redesigned computation of next occurrences for periodic tasks. This redesign was initiated by a problem reported and worked around in issue #37. The rework itself was implemented with issue #39. 

### Old stable release (0.0.1.36)
This is the previous version known as being stable for production. Download a zip
of it [here](https://github.com/linknx/linknx/archive/0.0.1.36.zip).
Fixes issue 33.

For a list of all releases, visit [this page](https://github.com/linknx/linknx/releases).
