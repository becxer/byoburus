#install git required to git prompt
sudo apt-get update
sudo apt-get install git

#replace bashrc & profile
cp ~/.bashrc ~/.bashrc.bk
cp ./data/bashrc ~/.bashrc
cp ~/.profile ~/.profile.bk
cp ./data/profile ~/.profile

#add git prompt & git completion
cp ./data/git-prompt.sh ~/.git-prompt.sh
cp ./data/git-completion.bash ~/.git-completion.bash

#add git config & message
cp ./data/gitconfig ~/.gitconfig
cp ./data/gitmessage.txt ~/.gitmessage.txt

#add bavatar
cp ./data/bavatar.txt ~/.bavatar.txt

source ~/.profile

echo "..."
echo "=================================================="
echo "Install Complete"
