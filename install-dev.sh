#apt-get setting 
sudo apt-add-repository ppa:webupd8team/java
sudo apt-get update

#linux tools
sudo apt-get install ssh
sudo apt-get install htop
sudo apt-get install git
sudo apt-get install iftop
sudo apt-get install curl

#install language for default
sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose
sudo apt-get install build-essential python-dev python-setuptools libatlas-dev libatlas3gf-base
sudo apt-get install python-pip

#install byobu
sudo apt-get install libncursesw5-dev
sudo apt-get install byobu
sudo pip install pydemon

#install for vifm (newer version & past version is not working..)
tar -jxvf ./data/vifm-0.7.7.tar.bz2
cd vifm-0.7.7
sudo ./configure
sudo make
sudo make install

