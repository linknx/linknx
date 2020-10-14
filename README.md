# ![linknx](https://github.com/linknx/linknx/wiki/images/linknx-wide-dark-800x275.png)
![C/C++ CI](https://github.com/linknx/linknx/workflows/C/C++%20CI/badge.svg?branch=master)

This repository is a migration of the legacy one hosted on [Sourceforge](https://sourceforge.net/projects/linknx/) which is now deprecated.

## Documentation
Read the [wiki pages](https://github.com/linknx/linknx/wiki) for detailed information about how to build, install, configure and run linknx.

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
### Latest stable release ([0.0.1.38](https://github.com/linknx/linknx/releases/tag/0.0.1.38))
This is the latest version as of today. Unless you know what you are doing, you should use this version. Download a zip
of it [here](https://github.com/linknx/linknx/archive/0.0.1.38.zip).
Fixes issue #47.

### Old stable release ([0.0.1.37](https://github.com/linknx/linknx/releases/tag/0.0.1.37))
This is the previous version known as being stable for production. Download a zip
of it [here](https://github.com/linknx/linknx/archive/0.0.1.37.zip).
Implements a redesigned computation of next occurrences for periodic tasks. This redesign was initiated by a problem reported and worked around in Pull Request 37. The rework itself was implemented with Pull Request 39. 

For a list of all releases, visit [this page](https://github.com/linknx/linknx/releases).
