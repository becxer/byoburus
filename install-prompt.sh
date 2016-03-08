#install git required to git prompt
sudo apt-get update
sudo apt-get install git
sudo apt-get install toilet
sudo apt-get install jp2a

#install zsh
sudo apt-get install zsh
sh -c "$(curl -fsSL https://raw.github.com/robbyrussell/oh-my-zsh/master/tools/install.sh)"

#replace bashrc & profile
cp ~/.bashrc ~/.bashrc.bk
cp ./data/bashrc ~/.bashrc
cp ~/.profile ~/.profile.bk
cp ./data/profile ~/.profile

#replace zshrc & plugin & theme
cp ~/.zshrc ~/.zshrc.bk
cp ./data/zshrc ~/.zshrc
cp ./data/byoburus.zsh-theme ~/.oh-my-zsh/themes/byoburus.zsh-theme
cp -r ./data/byoburus ~/.oh-my-zsh/plugins/byoburus
git clone https://github.com/djui/alias-tips.git ~/.oh-my-zsh/plugins/alias-tips

#add git prompt & git completion
cp ./data/git-prompt.sh ~/.git-prompt.sh
cp ./data/git-completion.bash ~/.git-completion.bash

#add git config & message
cp ./data/gitconfig ~/.gitconfig
cp ./data/gitmessage.txt ~/.gitmessage.txt

#add bavatar
cp ./data/bavatar.jpg ~/.bavatar.jpg

source ~/.profile

echo "..."
echo "=================================================="
echo "Install Complete"
