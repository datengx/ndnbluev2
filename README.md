# ndnbluev2
NDN over bluetooth version 2 (based on current ndn-cxx and NFD repos)

This project is modified based on the [NFD](http://named-data.net/doc/NFD/current/INSTALL.html) and [ndn-cxx](https://named-data.net/doc/ndn-cxx/current/INSTALL.html) project for the purpose of experimenting the RFCOMM Bluetooth extension. To see the most updated version of these two projects, please go to the original github repos.

# Build
To build, following the same procedures as the described in [here](https://named-data.net/doc/ndn-cxx/current/INSTALL.html) and [here](http://named-data.net/doc/NFD/current/INSTALL.html) for installing ndn-cxx and NFD, respectively. Pay attention to the dependencies.

You also need to install BlueZ on your system. For Ubuntu, do
```
sudo apt-get install bluez
```

The build that have been tested is using (gcc 6.3.0, Boost 1.6.2) and (gcc 5.4.0, Boost 1.5.8).

# How to create Bluetooth Face
To create Bluetooth RFCOMM faces, do
```
nfdc face create bluetooth://[<MAC address of the targeting BT device>]<channel # in the range [1-30]>.
```

For example:
```
nfdc face create bluetooth://[FF:FF:FF:FF:FF:FF]1.
```
