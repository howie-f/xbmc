#!/bin/bash

sudo git clean -xffd
WORKDIR=$(pwd)

mkdir $WORKDIR/kodi-build17
cd $WORKDIR/kodi-build17

cmake ../project/cmake \
        -DCMAKE_INSTALL_PREFIX=/opt/krypton \
        -DENABLE_BLURAY=NO \
        -DENABLE_CEC=NO \
        -DENABLE_NFS=NO \
        -DENABLE_PULSEAUDIO=NO \
        -DENABLE_SMBCLIENT=NO \
        -DENABLE_BLUETOOTH=NO

cmake --build . -- -j$(getconf _NPROCESSORS_ONLN)

sudo make install DESTDIR=$WORKDIR/kodi17root

sudo make -j4 -C $WORKDIR/tools/depends/target/binary-addons \
     PREFIX=$WORKDIR/kodi17root/opt/krypton \
     ADDONS="pvr.hts visualization.goom"

sudo tar --directory=$WORKDIR/kodi17root -xf $WORKDIR/visualization.projectm/pm.tar
sudo cp $WORKDIR/tools/EventClients/Clients/Kodi\ Send/kodi-send.py $WORKDIR/kodi17root/opt/krypton/bin/kodi-send
sudo chown -R root:root $WORKDIR/kodi17root
tar --numeric-owner --directory=$WORKDIR/kodi17root -czpf - . | ssh htpc-wz "cat > krypton.tar.gz && sudo ~/scripts/liveupdate.sh"

cd $WORKDIR
sudo git clean -xffd
