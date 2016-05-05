#apt-get setting 
sudo apt-add-repository ppa:webupd8team/java
sudo add-apt-repository ppa:git-core/ppa
sudo apt-get update

#linux tools
sudo apt-get install ssh
sudo apt-get install htop
sudo apt-get install git
sudo apt-get install iftop
sudo apt-get install curl
sudo apt-get install most

#install byobu
sudo apt-get install libncursesw5-dev
sudo apt-get install byobu
sudo pip install pydemon

#add git config & message
cp ./data/gitconfig ~/.gitconfig
cp ./data/gitmessage.txt ~/.gitmessage.txt
