#install git required to git prompt
sudo add-apt-repository ppa:git-core/ppa
sudo apt-get update
sudo apt-get install git

git clone https://github.com/becxer/vimrc.git
cd vimrc
./install-vim.sh
