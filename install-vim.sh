#apt-get setting 
sudo apt-get update

#linux tools
sudo apt-get install vim
sudo apt-get install exuberant-ctags

#install library for default
sudo apt-get install python-numpy python-scipy python-matplotlib ipython ipython-notebook python-pandas python-sympy python-nose
sudo apt-get install build-essential python-dev python-setuptools libatlas-dev libatlas3gf-base
sudo apt-get install python-pip

#install through pip
sudo pip install jedi

#replace vimrc
cp ~/.vimrc ~/.vimrc.bk
cp ./data/vimrc ~/.vimrc

#replace ctags
cp ~/.ctags ~/.ctags.bk
cp ./data/ctags ~/.ctags

#install for vifm (newer version & past version is not working..)
tar -jxvf ./data/vifm-0.7.7.tar.bz2
cd vifm-0.7.7
sudo ./configure
sudo make
sudo make install

#install vim plugins
mkdir -p ~/.vim/autoload ~/.vim/bundle;
curl -LSso ~/.vim/autoload/pathogen.vim https://tpo.pe/pathogen.vim
cd ~/.vim/bundle
git clone https://github.com/scrooloose/nerdtree.git
git clone https://github.com/majutsushi/tagbar.git
git clone https://github.com/aperezdc/vim-template.git
git clone --recursive https://github.com/davidhalter/jedi-vim.git

echo "..."
echo "=================================================="
echo "Install Complete"
