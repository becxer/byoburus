cp -r ~/.emacs.d ~/.emacs.d.bk
cp ~/.emacs ~/.emacs.bk 
rm -rf ~/.emacs.d
rm -f ~/.emacs

git clone https://github.com/becxer/.emacs.d.git ~/.emacs.d
sudo apt-get install ispell
sudo ln -s /usr/lib/ispell/american.hash /usr/lib/ispell/english.hash
