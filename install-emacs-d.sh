cp -r ~/.emacs.d ~/.emacs.d.bk
cp ~/.emacs ~/.emacs.bk 
rm -rf ~/.emacs.d
rm -f ~/.emacs

git clone https://github.com/becxer/.emacs.d.git ~/.emacs.d

