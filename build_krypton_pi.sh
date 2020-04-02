#!/bin/bash

WORKDIR=$(pwd)

./build_rpi_debian_packages.sh

cd build

sudo make install DESTDIR=$WORKDIR/kodi17root
sudo make -j4 -C $WORKDIR/tools/depends/target/binary-addons PREFIX=$WORKDIR/kodi17root/opt/krypton ADDONS="pvr.hts"

sudo cp "$WORKDIR/tools/EventClients/Clients/Kodi Send/kodi-send.py" $WORKDIR/kodi17root/opt/krypton/bin/kodi-send

sudo chown -R root:root $WORKDIR/kodi17root

tar --directory=$WORKDIR/kodi17root -cf $WORKDIR/../krypton-pi.tar .

cd $WORKDIR
sudo git clean -xffd
