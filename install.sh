sudo apt-get install ssh
sudo apt-get install vim
sudo apt-get install htop

cp ~/.profile ~/.profile.bk
cp ./data/profile ~/.profile
echo "source .profile" >> ~/.bashrc 
source ~/.profile

cp ~/.vimrc ~/.vimrc.bk
cp ./data/vimrc ~/.vimrc

sudo apt-get install libncursesw5-dev
sudo apt-get install byobu

tar -jxvf ./data/vifm-0.7.7.tar.bz2
cd vifm-0.7.7
sudo ./configure
sudo make
sudo make install
echo " "
echo "=========="
echo "Install Complete"
