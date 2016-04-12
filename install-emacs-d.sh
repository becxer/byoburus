#install git required to git prompt
sudo apt-get update
sudo apt-get install git

cp -r ~/.emacs.d ~/.emacs.d.bk
cp ~/.emacs ~/.emacs.bk 
rm -rf ~/.emacs.d
rm -f ~/.emacs

git clone https://github.com/becxer/.emacs.d.git ~/.emacs.d

