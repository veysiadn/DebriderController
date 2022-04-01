# Debrider Controller Device Control Software
Debrider Controller Software with Qt
### Considering that you already installed Raspian Buster to your Raspberry Pi.
### Note : It's important that you install Buster version of Raspian not the Bullseye version, because wiringpi, and several other libraries are not available for Bullseye version.
## Before building project ; Make sure that you have an internet connection
### STEP 1 ;
##### Do update & upgrade, than install Qt5 for installing building environment.

```sh
sudo apt-get update
sudo apt-get upgrade 
sudo apt-get install build-essential qtcreator qt5-default qt5-doc qt5-doc-html qtbase5-doc-html qtbase5-examples libqt5serialport5-dev -y
sudo apt-get install wiringpi git clang
sudo ldconfig
sudo /sbin/ldconfig -v

```

### STEP 2 ;
#####  Download and install EPOS4 command library 
```sh

git clone https://github.com/veysiadn/epos-linux-library
cd epos-linux-library/EPOS-Linux-Library-En
sudo chmod a+x install.sh
sudo ./install.sh

```

### Step 3 ;
#### Start QT Creator and open pro file this repository, configure it based on your system and now you can build it and get the executable.

Contact : veysi.adin@kist.re.kr
