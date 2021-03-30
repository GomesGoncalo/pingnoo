# Setting up Arch for building pingnoo.

## Uncomment mirrors
```
vi /etc/pacman.d/mirrorlist
```

## Install Prerequisites
```
pacman-key --init &&
pacman-key --populate archlinux

pacman --noconfirm -Syu

pacman --noconfirm -S wget &&
pacman --noconfirm -S git &&
pacman --noconfirm -S base-devel &&
pacman --noconfirm -S qt5-base &&
pacman --noconfirm -S qt5-tools &&
pacman --noconfirm -S qt5-quickcontrols &&
pacman --noconfirm -S dbus &&
pacman --noconfirm -S python3
```

### Download and install CMake
```
cd /tmp &&
wget https://github.com/Kitware/CMake/releases/download/v3.20.0-rc2/cmake-3.20.0-rc2-linux-x86_64.sh &&
chmod +x cmake-3.20.0-rc2-linux-x86_64.sh &&
./cmake-3.20.0-rc2-linux-x86_64.sh --skip-license --prefix=/usr/local
```