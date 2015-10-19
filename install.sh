#linux tools
sudo apt-get install ssh
sudo apt-get install vim
sudo apt-get install htop

#install language for default
sudo apt-add-repository ppa:webupd8team/java
sudo apt-get update
sudo apt-get install oracle-java8-installer
sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose

#replace vimrc
cp ~/.vimrc ~/.vimrc.bk
cp ./data/vimrc ~/.vimrc

#install byobu
sudo apt-get install libncursesw5-dev
sudo apt-get install byobu

#install for vifm (newer version & past version is not working..)
tar -jxvf ./data/vifm-0.7.7.tar.bz2
cd vifm-0.7.7
sudo ./configure
sudo make
sudo make install

echo "..."
echo "=================================================="
echo "Install Complete"
